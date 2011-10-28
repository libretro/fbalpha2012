#include "libsnes.hpp"
#include "../burner.h"
#include "../gameinp.h"
#include "../../interface/inp_keys.h"
#include "../../burn/state.h"
#include "archive.h" // FEX wrapper.
#include <string.h>
#include <stdio.h>

#include <vector>
#include <string>
#include <ctype.h>

// FBA cruft.

unsigned ArcadeJoystick;

struct ROMFIND
{
	unsigned int nState;
	int nArchive;
	int nPos;
};

#define STAT_NOFIND	0
#define STAT_OK		1
#define STAT_CRC	   2
#define STAT_SMALL	3
#define STAT_LARGE	4

static std::vector<std::string> g_find_list_path;
static ROMFIND g_find_list[1024];
static unsigned g_rom_count;

#define AUDIO_SEGMENT_LENGTH 534
#define AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS (534 * 2)

static uint16_t g_fba_frame[1024 * 1024];
static uint16_t g_fba_frame_conv[1024 * 1024];
static int16_t g_audio_buf[AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS];
/////

// libsnes globals

static snes_video_refresh_t video_cb;
static snes_audio_sample_t audio_cb;
static snes_input_poll_t poll_cb;
static snes_input_state_t input_cb;
void snes_set_video_refresh(snes_video_refresh_t cb) { video_cb = cb; }
void snes_set_audio_sample(snes_audio_sample_t cb) { audio_cb = cb; }
void snes_set_input_poll(snes_input_poll_t cb) { poll_cb = cb; }
void snes_set_input_state(snes_input_state_t cb) { input_cb = cb; }

// SSNES extension.
static snes_environment_t environ_cb;
void snes_set_environment(snes_environment_t cb) { environ_cb = cb; }

static char g_rom_name[1024];
static char g_rom_dir[1024];
static char g_basename[1024];

/////
static void poll_input();
static bool init_input();

void snes_init()
{
   BurnLibInit();
}

void snes_term()
{
   BurnDrvExit();
   BurnLibExit();
}

static bool g_reset;
static uint8_t *g_reset_ptr;
static uint8_t *g_service_ptr;
void snes_power() { g_reset = true; }
void snes_reset() { g_reset = true; }


// Copy stuff :o
static inline void blit_regular(unsigned width, unsigned height, unsigned pitch)
{
   for (unsigned y = 0; y < height; y++)
      memcpy(g_fba_frame_conv + y * 1024, g_fba_frame + y * (pitch >> 1), width * sizeof(uint16_t));

   video_cb(g_fba_frame_conv, width, height);
}

static inline void blit_flipped(unsigned width, unsigned height, unsigned pitch)
{
   for (unsigned y = 0; y < height; y++)
      memcpy(g_fba_frame_conv + (height - 1 - y) * 1024, g_fba_frame + y * (pitch >> 1), width * sizeof(uint16_t));

   video_cb(g_fba_frame_conv, width, height);
}

static inline void blit_vertical(unsigned width, unsigned height, unsigned pitch)
{
   unsigned in_width = height;
   unsigned in_height = width;
   pitch >>= 1;

   // Flip y and x coords pretty much ...
   for (unsigned y = 0; y < in_height; y++)
      for (unsigned x = 0; x < in_width; x++)
         g_fba_frame_conv[(height - x - 1) * 1024 + y] = g_fba_frame[y * pitch + x];

   video_cb(g_fba_frame_conv, width, height);
}

static inline void blit_vertical_flipped(unsigned width, unsigned height, unsigned pitch)
{
   unsigned in_width = height;
   unsigned in_height = width;
   pitch >>= 1;

   // Flip y and x coords pretty much ...
   for (unsigned y = 0; y < in_height; y++)
      for (unsigned x = 0; x < in_width; x++)
         g_fba_frame_conv[x * 1024 + (width - 1 - y)] = g_fba_frame[y * pitch + x];

   video_cb(g_fba_frame_conv, width, height);
}


void snes_run()
{
   int width, height;
   BurnDrvGetVisibleSize(&width, &height);
   pBurnDraw = (uint8_t*)g_fba_frame;

   unsigned drv_flags = BurnDrvGetFlags();
   if (drv_flags & BDF_ORIENTATION_VERTICAL)
      nBurnPitch = height * sizeof(uint16_t);
   else
      nBurnPitch = width * sizeof(uint16_t);

   nBurnLayer = 0xff;
   pBurnSoundOut = g_audio_buf;
   nBurnSoundRate = 32000;
   nBurnSoundLen = AUDIO_SEGMENT_LENGTH;
   nCurrentFrame++;

   poll_input();

   BurnDrvFrame();

   if ((drv_flags & (BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED)) == (BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED))
      blit_vertical_flipped(width, height, nBurnPitch);
   else if (drv_flags & BDF_ORIENTATION_VERTICAL)
      blit_vertical(width, height, nBurnPitch);
   else if (drv_flags & BDF_ORIENTATION_FLIPPED)
      blit_flipped(width, height, nBurnPitch);
   else
      blit_regular(width, height, nBurnPitch);

   for (unsigned i = 0; i < AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS; i += 2)
      audio_cb(g_audio_buf[i + 0], g_audio_buf[i + 1]);
}

static uint8_t *write_state_ptr;
static const uint8_t *read_state_ptr;
static unsigned state_size;

static int burn_write_state_cb(BurnArea *pba)
{
   memcpy(write_state_ptr, pba->Data, pba->nLen);
   write_state_ptr += pba->nLen;
   return 0;
}

static int burn_read_state_cb(BurnArea *pba)
{
   memcpy(pba->Data, read_state_ptr, pba->nLen);
   read_state_ptr += pba->nLen;
   return 0;
}

static int burn_dummy_state_cb(BurnArea *pba)
{
   state_size += pba->nLen;
}

unsigned snes_serialize_size()
{
   if (state_size)
      return state_size;

   BurnAcb = burn_dummy_state_cb;
   state_size = 0;
   BurnAreaScan(ACB_VOLATILE | ACB_WRITE, 0);
   return state_size;
}

bool snes_serialize(uint8_t *data, unsigned size)
{
   if (size != state_size)
      return false;

   BurnAcb = burn_write_state_cb;
   write_state_ptr = data;
   BurnAreaScan(ACB_VOLATILE | ACB_WRITE, 0);

   return true;
}

bool snes_unserialize(const uint8_t *data, unsigned size)
{
   if (size != state_size)
      return false;
   BurnAcb = burn_read_state_cb;
   read_state_ptr = data;
   BurnAreaScan(ACB_VOLATILE | ACB_READ, 0);

   return true;
}

void snes_cheat_reset() {}
void snes_cheat_set(unsigned, bool, const char*) {}

static int find_rom_by_crc(unsigned crc, const ArcEntry *list, unsigned elems)
{
   for (unsigned i = 0; i < elems; i++)
   {
      if (list[i].nCrc == crc)
         return i;
   }

   return -1;
}

static void free_archive_list(ArcEntry *list, unsigned count)
{
   if (list)
   {
      for (unsigned i = 0; i < count; i++)
         free(list[i].szName);
      free(list);
   }
}

static int archive_load_rom(uint8_t *dest, int *wrote, int i)
{
   if (i < 0 || i >= g_rom_count)
      return 1;

   int archive = g_find_list[i].nArchive;

   if (archiveOpen(g_find_list_path[archive].c_str()))
      return 1;

   BurnRomInfo ri = {0};
   BurnDrvGetRomInfo(&ri, i);
  
   if (archiveLoadFile(dest, ri.nLen, g_find_list[i].nPos, wrote))
   {
      archiveClose();
      return 1;
   }

   archiveClose();
   return 0;
}

// This code is very confusing. The original code is even more confusing :(
static bool open_archive()
{
   // FBA wants some roms ... Figure out how many.
   g_rom_count = 0;
   while (!BurnDrvGetRomInfo(0, g_rom_count))
      g_rom_count++;

   g_find_list_path.clear();

   // Check if we have said archives.
   // Check if archives are found. These are relative to g_rom_dir.
   char *rom_name;
   for (unsigned index = 0; index < 32; index++)
   {
      if (BurnDrvGetArchiveName(&rom_name, index, false))
         continue;

      char path[1024];
      snprintf(path, sizeof(path), "%s/%s", g_rom_dir, rom_name);

      int ret = archiveCheck(path, 0);
      if (ret == ARC_NONE)
         continue;

      g_find_list_path.push_back(path);
   }

   memset(g_find_list, 0, sizeof(g_find_list));

   for (unsigned z = 0; z < g_find_list_path.size(); z++)
   {
      if (archiveOpen(g_find_list_path[z].c_str()))
         continue;

      ArcEntry *list;
      int count;
      archiveGetList(&list, &count);

      // Try to map the ROMs FBA wants to ROMs we find inside our pretty archives ...
      for (unsigned i = 0; i < g_rom_count; i++)
      {
         if (g_find_list[i].nState == STAT_OK)
            continue;

         BurnRomInfo ri = {0};
         BurnDrvGetRomInfo(&ri, i);

         int index = find_rom_by_crc(ri.nCrc, list, count);
         if (index < 0)
            continue;

         // Yay, we found it!
         g_find_list[i].nArchive = z;
         g_find_list[i].nPos = index;
         g_find_list[i].nState = STAT_OK;

         // Sanity checking ...
         //if (!(ri.nType & BRF_OPT) && ri.nCrc)
         //   nTotalSize += ri.nLen;

         if (list[index].nLen == ri.nLen)
         {
            if (ri.nCrc && list[index].nCrc != ri.nCrc)
               g_find_list[i].nState = STAT_CRC;
         }
         else if (list[index].nLen < ri.nLen)
            g_find_list[i].nState = STAT_SMALL;
         else if (list[index].nLen > ri.nLen)
            g_find_list[i].nState = STAT_LARGE;
      }

      free_archive_list(list, count);

      archiveClose();
   }

   BurnExtLoadRom = archive_load_rom;
   return true;
}

static bool fba_init(unsigned driver)
{
   nBurnDrvSelect = driver;

   if (!open_archive())
      return false;

   BurnDrvInit();

   if (environ_cb)
   {
      int width, height;
      BurnDrvGetVisibleSize(&width, &height);
      snes_geometry geom = { width, height, width, height };
      environ_cb(SNES_ENVIRONMENT_SET_GEOMETRY, &geom);
   }

   return true;
}

static unsigned int HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t = 0;
	t |= (r << 7) & 0x7c00;
	t |= (g << 2) & 0x03e0;
	t |= (b >> 3) & 0x001f;
	return t;
}

static void init_video()
{
   nBurnBpp = 2;
   BurnHighCol = HighCol15;
}

static void init_audio()
{
   pBurnSoundOut = g_audio_buf;
   nBurnSoundRate = 32000;
   nBurnSoundLen = AUDIO_SEGMENT_LENGTH;
}

// Infer paths from basename.
bool snes_load_cartridge_normal(const char*, const uint8_t *, unsigned)
{
   unsigned i = BurnDrvGetIndexByName(g_basename);
   if (i < nBurnDrvCount)
   {
      init_video();
      init_audio();

      if (!fba_init(i))
         return false;

      init_input();

      return true;
   }
   else
      return false;
}

void snes_set_cartridge_basename(const char *basename)
{
   snprintf(g_rom_name, sizeof(g_rom_name), "%s.zip", basename);
   strcpy(g_rom_dir, g_rom_name);

   char *split = strrchr(g_rom_dir, '/');
   if (!split)
      split = strrchr(g_rom_dir, '\\');
   if (split)
      *split = '\0';

   if (split)
   {
      strcpy(g_basename, split + 1);
      split = strrchr(g_basename, '.');
      if (split)
         *split = '\0';
   }

   fprintf(stderr, "PATH:     %s\n", g_rom_name);
   fprintf(stderr, "DIR:      %s\n", g_rom_dir);
   fprintf(stderr, "BASENAME: %s\n", g_basename);
}

bool snes_load_cartridge_bsx_slotted(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

bool snes_load_cartridge_bsx(
  const char*, const uint8_t *, unsigned,
  const char*, const uint8_t *, unsigned
)
{ return false; }

bool snes_load_cartridge_sufami_turbo(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

bool snes_load_cartridge_super_game_boy(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

void snes_unload_cartridge(void) {}

bool snes_get_region() { return SNES_REGION_NTSC; }

uint8_t *snes_get_memory_data(unsigned) { return 0; }
unsigned snes_get_memory_size(unsigned) { return 0; }

unsigned snes_library_revision_major() { return 1; }
unsigned snes_library_revision_minor() { return 3; }

const char *snes_library_id() { return "FBANext/libsnes"; }
void snes_set_controller_port_device(bool, unsigned) {}

// Stub
void Reinitialise(void)
{}

// Input stuff.

// Ref GamcPlayer() in ../gamc.cpp
#define P1_COIN	FBK_5
#define P1_START  FBK_1
#define P1_LEFT   FBK_LEFTARROW
#define P1_RIGHT  FBK_RIGHTARROW
#define P1_UP     FBK_UPARROW
#define P1_DOWN   FBK_DOWNARROW
#define P1_FIRE1  FBK_A
#define P1_FIRE2  FBK_S
#define P1_FIRE3  FBK_D
#define P1_FIRE4  FBK_Z
#define P1_FIRE5  FBK_X
#define P1_FIRE6  FBK_C
#define P1_SERVICE FBK_F2

#define P2_COIN 0x07
#define P2_START 0x03
#define P2_LEFT 0x4000
#define P2_RIGHT 0x4001
#define P2_UP 0x4002
#define P2_DOWN 0x4003
#define P2_FIRE1 0x4080
#define P2_FIRE2 0x4081
#define P2_FIRE3 0x4082
#define P2_FIRE4 0x4083
#define P2_FIRE5 0x4084
#define P2_FIRE6 0x4085

#define P3_COIN 0x08
#define P3_START 0x04
#define P3_LEFT 0x4100
#define P3_RIGHT 0x4101
#define P3_UP 0x4102
#define P3_DOWN 0x4103
#define P3_FIRE1 0x4180
#define P3_FIRE2 0x4181
#define P3_FIRE3 0x4182
#define P3_FIRE4 0x4183
#define P3_FIRE5 0x4184
#define P3_FIRE6 0x4185

#define P4_COIN 0x09
#define P4_START 0x05
#define P4_LEFT 0x4200
#define P4_RIGHT 0x4201
#define P4_UP 0x4202
#define P4_DOWN 0x4203
#define P4_FIRE1 0x4280
#define P4_FIRE2 0x4281
#define P4_FIRE3 0x4282
#define P4_FIRE4 0x4283
#define P4_FIRE5 0x4284
#define P4_FIRE6 0x4285

static unsigned char keybinds[0x5000][2] = {0}; 
#define _B(x) SNES_DEVICE_ID_JOYPAD_##x
static bool init_input()
{
   GameInpInit();
   GameInpDefault();

   bool has_analog = false;
   struct GameInp* pgi = GameInp;
   for (unsigned i = 0; i < nGameInpCount; i++, pgi++)
   {
      if (pgi->nType == BIT_ANALOG_REL)
      {
         has_analog = true;
         break;
      }
   }

   // Bind to nothing.
   for (unsigned i = 0; i < 0x5000; i++)
      keybinds[i][0] = 0xff;

   // Reset
   //keybinds[FBK_F3		][0] = -1u;
   //keybinds[FBK_F3		][1] = 0;
   ///

   keybinds[P1_COIN	][0] = _B(SELECT);
   keybinds[P1_COIN	][1] = 0;
   keybinds[P1_START	][0] = _B(START);
   keybinds[P1_START	][1] = 0;
   keybinds[P1_UP		][0] = _B(UP);
   keybinds[P1_UP		][1] = 0;
   keybinds[P1_DOWN	][0] = _B(DOWN);
   keybinds[P1_DOWN	][1] = 0;
   keybinds[P1_LEFT	][0] = _B(LEFT);
   keybinds[P1_LEFT	][1] = 0;
   keybinds[P1_RIGHT	][0] = _B(RIGHT);
   keybinds[P1_RIGHT	][1] = 0;
   keybinds[P1_FIRE1	][0] = _B(Y);
   keybinds[P1_FIRE1	][1] = 0;
   keybinds[P1_FIRE2	][0] = _B(X);
   keybinds[P1_FIRE2	][1] = 0;
   keybinds[P1_FIRE3	][0] = _B(L);
   keybinds[P1_FIRE3	][1] = 0;
   keybinds[P1_FIRE4	][0] = _B(B);
   keybinds[P1_FIRE4	][1] = 0;
   keybinds[P1_FIRE5	][0] = _B(A);
   keybinds[P1_FIRE5	][1] = 0;
   keybinds[P1_FIRE6	][0] = _B(R);
   keybinds[P1_FIRE6	][1] = 0;
#if 0
   keybinds[0x88		][0] = L2;
   keybinds[0x88		][1] = 0;
   keybinds[0x8A		][0] = R2;
   keybinds[0x8A		][1] = 0;
   keybinds[0x3b		][0] = L3;
   keybinds[0x3b		][1] = 0;
   keybinds[P1_SERVICE	][0] = R3;
   keybinds[P1_SERVICE	][1] = 0;
   keybinds[0x21		][0] = R2;
   keybinds[0x21		][1] = 0;
#endif

   keybinds[P2_COIN	][0] = _B(SELECT);
   keybinds[P2_COIN	][1] = 1;
   keybinds[P2_START	][0] = _B(START);
   keybinds[P2_START	][1] = 1;
   keybinds[P2_UP		][0] = _B(UP);
   keybinds[P2_UP		][1] = 1;
   keybinds[P2_DOWN	][0] = _B(DOWN);
   keybinds[P2_DOWN	][1] = 1;
   keybinds[P2_LEFT	][0] = _B(LEFT);
   keybinds[P2_LEFT	][1] = 1;
   keybinds[P2_RIGHT	][0] = _B(RIGHT);
   keybinds[P2_RIGHT	][1] = 1;
   keybinds[P2_FIRE1	][0] = _B(Y);
   keybinds[P2_FIRE1	][1] = 1;
   keybinds[P2_FIRE2	][0] = _B(X);
   keybinds[P2_FIRE2	][1] = 1;
   keybinds[P2_FIRE3	][0] = _B(L);
   keybinds[P2_FIRE3	][1] = 1;
   keybinds[P2_FIRE4	][0] = _B(B);
   keybinds[P2_FIRE4	][1] = 1;
   keybinds[P2_FIRE5	][0] = _B(A);
   keybinds[P2_FIRE5	][1] = 1;
   keybinds[P2_FIRE6	][0] = _B(R);
   keybinds[P2_FIRE6	][1] = 1;

#if 0
   keybinds[0x4088		][0] = L2;
   keybinds[0x4088		][1] = 1;
   keybinds[0x408A		][0] = R2;
   keybinds[0x408A		][1] = 1;
   keybinds[0x408b		][0] = L3;
   keybinds[0x408b		][1] = 1;
   keybinds[0x408c		][0] = R3;
   keybinds[0x408c		][1] = 1;
#endif

#if 0
   keybinds[P3_COIN	][0] = SELECT;
   keybinds[P3_COIN	][1] = 2;
   keybinds[P3_START	][0] = START;
   keybinds[P3_START	][1] = 2;
   keybinds[P3_UP		][0] = UP;
   keybinds[P3_UP		][1] = 2;
   keybinds[P3_DOWN	][0] = DOWN;
   keybinds[P3_DOWN	][1] = 2;
   keybinds[P3_LEFT	][0] = LEFT;
   keybinds[P3_LEFT	][1] = 2;
   keybinds[P3_RIGHT	][0] = RIGHT;
   keybinds[P3_RIGHT	][1] = 2;
   keybinds[P3_FIRE1	][0] = CROSS;
   keybinds[P3_FIRE1	][1] = 2;
   keybinds[P3_FIRE2	][0] = CIRCLE;
   keybinds[P3_FIRE2	][1] = 2;
   keybinds[P3_FIRE3	][0] = SQUARE;
   keybinds[P3_FIRE3	][1] = 2;
   keybinds[P3_FIRE4	][0] = TRIANGLE;
   keybinds[P3_FIRE4	][1] = 2;
   keybinds[P3_FIRE5	][0] = L1;
   keybinds[P3_FIRE5	][1] = 2;
   keybinds[P3_FIRE6	][0] = R1;
   keybinds[P3_FIRE6	][1] = 2;
   keybinds[0x4188		][0] = L2;
   keybinds[0x4188		][1] = 2;
   keybinds[0x418A		][0] = R2;
   keybinds[0x418A		][1] = 2;
   keybinds[0x418b		][0] = L3;
   keybinds[0x418b		][1] = 2;
   keybinds[0x418c		][0] = R3;
   keybinds[0x418c		][1] = 2;

   keybinds[P4_COIN	][0] = SELECT;
   keybinds[P4_COIN	][1] = 3;
   keybinds[P4_START	][0] = START;
   keybinds[P4_START	][1] = 3;
   keybinds[P4_UP		][0] = UP;
   keybinds[P4_UP		][1] = 3;
   keybinds[P4_DOWN	][0] = DOWN;
   keybinds[P4_DOWN	][1] = 3;
   keybinds[P4_LEFT	][0] = LEFT;
   keybinds[P4_LEFT	][1] = 3;
   keybinds[P4_RIGHT	][0] = RIGHT;
   keybinds[P4_RIGHT	][1] = 3;
   keybinds[P4_FIRE1	][0] = CROSS;
   keybinds[P4_FIRE1	][1] = 3;
   keybinds[P4_FIRE2	][0] = CIRCLE;
   keybinds[P4_FIRE2	][1] = 3;
   keybinds[P4_FIRE3	][0] = SQUARE;
   keybinds[P4_FIRE3	][1] = 3;
   keybinds[P4_FIRE4	][0] = TRIANGLE;
   keybinds[P4_FIRE4	][1] = 3;
   keybinds[P4_FIRE5	][0] = L1;
   keybinds[P4_FIRE5	][1] = 3;
   keybinds[P4_FIRE6	][0] = R1;
   keybinds[P4_FIRE6	][1] = 3;
   keybinds[0x4288		][0] = L2;
   keybinds[0x4288		][1] = 3;
   keybinds[0x428A		][0] = R2;
   keybinds[0x428A		][1] = 3;
   keybinds[0x428b		][0] = L3;
   keybinds[0x428b		][1] = 3;
   keybinds[0x428c		][0] = R3;
   keybinds[0x428c		][1] = 3;
#endif

   return has_analog;
}
#undef _B

static void poll_input()
{
   poll_cb();

   struct GameInp* pgi = GameInp;
   unsigned controller_binds_count = nGameInpCount;

   for (unsigned i = 0; i < controller_binds_count; i++, pgi++)
   {
      switch (pgi->nInput)
      {
         case GIT_CONSTANT: // Constant value
            pgi->Input.nVal = pgi->Input.Constant.nConst;
            *(pgi->Input.pVal) = pgi->Input.nVal;
            break;
         case GIT_SWITCH:
            // Digital input
            //uint64_t reset = DoReset;
            unsigned id = keybinds[pgi->Input.Switch.nCode][0];
            unsigned port = keybinds[pgi->Input.Switch.nCode][1];
            bool state = input_cb(port, SNES_DEVICE_JOYPAD, 0, id);

            if (pgi->nType & BIT_GROUP_ANALOG)
            {
               // Set analog controls to full
               if (state)
                  pgi->Input.nVal = 0xFFFF;
               else
                  pgi->Input.nVal = 0x0001;
#ifdef LSB_FIRST
               *(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
               *((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
            }
            else
            {
               // Binary controls
               if (state)
                  pgi->Input.nVal = 1;
               else
                  pgi->Input.nVal = 0;
               *(pgi->Input.pVal) = pgi->Input.nVal;
            }

            break;
      }
   }
}

// Stubs
int QuoteRead(char **, char **, char*) { return 1; }
char *LabelCheck(char *, char *) { return 0; }
const int nConfigMinVersion = 0x020921;

