#include "libsnes.hpp"
#include "../burner.h"
#include "../gameinp.h"
#include "../../burn/state.h"
#include <string.h>
#include <stdio.h>

//#include "../archive.h" // FEX wrapper.

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
static ROMFIND g_find_list[32];
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

static char g_rom_name[1024];
static char g_rom_dir[1024];
static char g_basename[1024];

/////

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

#define MAX_BINDS 64
struct fba_bind
{
   uint8_t *ptr;
   unsigned snes;
   unsigned player;
};

struct bind_conv
{
   const char *fba;
   unsigned snes;
};

#define DECL_MAP(fba, snes) { fba, SNES_DEVICE_ID_JOYPAD_##snes }
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static fba_bind bind_map[MAX_BINDS];
static unsigned bind_map_count;

static const bind_conv neogeo_map[] = {
   DECL_MAP("coin", SELECT),
   DECL_MAP("start", START),
   DECL_MAP("up", UP),
   DECL_MAP("down", DOWN),
   DECL_MAP("left", LEFT),
   DECL_MAP("right", RIGHT),
   DECL_MAP("fire 1", B),
   DECL_MAP("fire 2", A),
   DECL_MAP("fire 3", X),
   DECL_MAP("fire 4", Y),
};

static void check_generic_input(const char *name, uint8_t *ptr)
{
   if (strcmp(name, "reset") == 0)
      g_reset_ptr = ptr;
   else if (strcmp(name, "service") == 0)
      g_service_ptr = ptr;
   else if (strcmp(name, "dip") == 0)
      fprintf(stderr, "DIP set to 0x%x\n", (unsigned)*ptr);
}

static void init_neogeo_binds()
{
   for (unsigned i = 0; i < bind_map_count; i++)
   {
      BurnInputInfo bii;
      BurnDrvGetInputInfo(&bii, i);
      if (!bii.szInfo)
         bii.szInfo = "";

      bind_map[i].ptr = bii.pVal;

      int player = bii.szName[1] - '1';
      bind_map[i].player = player;

      std::string name_(bii.szInfo + 3);
      for (unsigned j = 0; j < name_.size(); j++)
         name_[j] = tolower(name_[j]);

      const char *name = name_.c_str();
      unsigned snes = ~0;

      check_generic_input(bii.szInfo, bii.pVal);

      for (unsigned j = 0; j < ARRAY_SIZE(neogeo_map); j++)
      {
         if (strstr(name, neogeo_map[j].fba))
         {
            snes = neogeo_map[j].snes;
            break;
         }
      }

      bind_map[i].snes = snes;
   }
}

static const bind_conv cps_map[] = {
   DECL_MAP("coin", SELECT),
   DECL_MAP("start", START),
   DECL_MAP("up", UP),
   DECL_MAP("down", DOWN),
   DECL_MAP("left", LEFT),
   DECL_MAP("right", RIGHT),
   DECL_MAP("weak punch", Y),
   DECL_MAP("medium punch", X),
   DECL_MAP("strong punch", L),
   DECL_MAP("weak kick", B),
   DECL_MAP("medium kick", A),
   DECL_MAP("strong kick", R),
   DECL_MAP("button 4", X),

   // for Super Puzzle Fighter II Turbo
   DECL_MAP("rotate left", Y),
   DECL_MAP("rotate right", B),

   // for Mega man 2
   DECL_MAP("jump", X),

   // for Eco figher
   DECL_MAP("turn 1", X),
   DECL_MAP("attack", Y),
   DECL_MAP("turn 2", B),

   // for Dungeons & Dragons
   DECL_MAP("use", B),
   DECL_MAP("select", A),

   // for Pang
   DECL_MAP("shot1", Y),
   DECL_MAP("shot2", B),

   // for Pro Gear
   DECL_MAP("shot", Y),
   DECL_MAP("bomb", X),
   DECL_MAP("auto", B),

   // for Super Gem Fighter / Pocket Fighter (only has Punch/kick)
   DECL_MAP("special", A),
   DECL_MAP("punch", X),
   DECL_MAP("kick", B),
};

static void init_cps_binds()
{
	for (unsigned i = 0; i < bind_map_count; i++)
	{
		BurnInputInfo bii;
		BurnDrvGetInputInfo(&bii, i);
		if (!bii.szName)
			bii.szName = "";

		bind_map[i].ptr = bii.pVal;

		int player = bii.szName[1] - '1';
		bind_map[i].player = player;

		std::string name_(bii.szName + 3);
		for (unsigned j = 0; j < name_.size(); j++)
			name_[j] = tolower(name_[j]);

		const char *name = name_.c_str();
		unsigned snes = ~0;

      check_generic_input(bii.szInfo, bii.pVal);

      for (unsigned j = 0; j < ARRAY_SIZE(cps_map); j++)
      {
         if (strstr(name, cps_map[j].fba))
         {
            snes = cps_map[j].snes;
            break;
         }
      }

		bind_map[i].snes = snes;
	}
}

static const bind_conv dummy_map[] = {
   DECL_MAP("coin", SELECT),
   DECL_MAP("start", START),
   DECL_MAP("up", UP),
   DECL_MAP("down", DOWN),
   DECL_MAP("left", LEFT),
   DECL_MAP("right", RIGHT),
   DECL_MAP("button 1", Y),
   DECL_MAP("button 2", X),
   DECL_MAP("button 3", B),
};

static void init_dummy_binds()
{
   for (unsigned i = 0; i < bind_map_count; i++)
   {
	   BurnInputInfo bii;
	   BurnDrvGetInputInfo(&bii, i);
	   bind_map[i].snes = i;
	   bind_map[i].ptr = bii.pVal;
	   bind_map[i].player = i & 1;

	   std::string name_(bii.szName + 3);
	   for (unsigned j = 0; j < name_.size(); j++)
		   name_[j] = tolower(name_[j]);

	   const char *name = name_.c_str();
	   unsigned snes = ~0;

      check_generic_input(bii.szInfo, bii.pVal);

      for (unsigned j = 0; j < ARRAY_SIZE(dummy_map); j++)
      {
         if (strstr(name, dummy_map[j].fba))
         {
            snes = dummy_map[j].snes;
            break;
         }
      }

	   bind_map[i].snes = snes;
   }
}

static void init_input()
{
   bind_map_count = 0;
   while (BurnDrvGetInputInfo(0, bind_map_count) == 0)
      bind_map_count++;

   switch (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK)
   {
      case HARDWARE_CAPCOM_CPS1:
      case HARDWARE_CAPCOM_CPS1_QSOUND:
      case HARDWARE_CAPCOM_CPS1_GENERIC:
      case HARDWARE_CAPCOM_CPSCHANGER:
      case HARDWARE_CAPCOM_CPS2:
      case HARDWARE_CAPCOM_CPS3:
         init_cps_binds();
         break;

      case HARDWARE_SNK_NEOGEO:
         init_neogeo_binds();
         break;

      default:
         init_dummy_binds();
         fprintf(stderr, "WARNING: No specific button config was found for this driver! Using default controls..\n");
         break;
   }
}

// Very incomplete ... Just do digital input :D
static void poll_input()
{
   for (unsigned i = 0; i < bind_map_count; i++)
   {
      *bind_map[i].ptr = input_cb(bind_map[i].player,
            SNES_DEVICE_JOYPAD, 0, bind_map[i].snes);
   }

   if (g_reset_ptr)
   {
      *g_reset_ptr = g_reset;
      g_reset = false;
   }

   if (g_service_ptr)
   {
      static bool old_service = false;
      bool new_service = 
         input_cb(0, SNES_DEVICE_JOYPAD, 0, SNES_DEVICE_ID_JOYPAD_L) &&
         input_cb(0, SNES_DEVICE_JOYPAD, 0, SNES_DEVICE_ID_JOYPAD_R) &&
         input_cb(0, SNES_DEVICE_JOYPAD, 0, SNES_DEVICE_ID_JOYPAD_START);
      *g_service_ptr = new_service && !old_service;

      old_service = new_service;
   }
}

static inline void blit_regular(unsigned width, unsigned height, unsigned pitch)
{
   for (unsigned y = 0; y < height; y++)
      memcpy(g_fba_frame_conv + y * 1024, g_fba_frame + y * (pitch >> 1), width * sizeof(uint16_t));

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

void snes_run()
{
   int width, height;
   BurnDrvGetVisibleSize(&width, &height);
   pBurnDraw = (uint8_t*)g_fba_frame;

   nBurnPitch = width * sizeof(uint16_t);

   nBurnLayer = 0xff;
   pBurnSoundOut = g_audio_buf;
   nBurnSoundRate = 32000;
   nBurnSoundLen = AUDIO_SEGMENT_LENGTH;
   nCurrentFrame++;

   poll_cb();
   poll_input();

   BurnDrvFrame();

   if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL)
      blit_vertical(width, height, height * 2);
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

//FIXME
#if 0
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
#endif

static bool fba_init(unsigned driver)
{
   nBurnDrvSelect = driver;

	//FIXME
	#if 0
   if (!open_archive())
      return false;
     #endif

   BurnDrvInit();
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

static void reset_dips()
{
   // Just checking DIP options ...
   BurnDIPInfo bdi;
   fprintf(stderr, "=== DIP enumeration ===\n");
   for (unsigned i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
   {
      fprintf(stderr, "DIP #%u:\n", i);
      fprintf(stderr, "   nInput:   %d\n", (int)bdi.nInput);
      fprintf(stderr, "   nFlags:   0x%x\n", (unsigned)bdi.nFlags);
      fprintf(stderr, "   nMask:    0x%x\n", (unsigned)bdi.nMask);
      fprintf(stderr, "   nSetting: %d\n", (int)bdi.nSetting);
      fprintf(stderr, "   szText:   %s\n", bdi.szText);
   }
   fprintf(stderr, "=== DIP END ===\n");
}

// Infer paths from basename.
bool snes_load_cartridge_normal(const char*, const uint8_t *, unsigned)
{
//FIXME
#if 0
   unsigned i = BurnDrvGetIndexByName(g_basename);
   if (i < nBurnDrvCount)
   {
      init_video();
      init_audio();

      if (!fba_init(i))
         return false;

      init_input();
      reset_dips();

      return true;
   }
   else
#endif
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

