#include "libsnes.hpp"
#include "../burner.h"
#include "../gameinp.h"
#include <string.h>
#include <stdio.h>

#include <vector>
#include <string>

// FBA cruft.

unsigned JukeboxSoundCommand;
unsigned JukeboxSoundLatch;
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

#define AUDIO_SEGMENT_LENGTH 801
#define AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS 1602

static uint16_t g_fba_frame[1024 * 1024];
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

void snes_power() {}
void snes_reset() {}

void snes_run()
{
   pBurnDraw = (uint8_t*)g_fba_frame;
   nBurnPitch = 2048;
   pBurnSoundOut = g_audio_buf;
   nBurnSoundRate = 32000;
   nBurnSoundLen = AUDIO_SEGMENT_LENGTH;

   BurnDrvFrame();

   video_cb(g_fba_frame, 360, 240);

   for (unsigned i = 0; i < AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS; i += 2)
      audio_cb(g_audio_buf[i + 0], g_audio_buf[i + 1]);
}

unsigned snes_serialize_size() { return 0; }
bool snes_serialize(uint8_t*, unsigned) { return false; }
bool snes_unserialize(const uint8_t*, unsigned) { return false; }

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

   return true;
}




static unsigned int HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t  = (r << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (b >> 3) & 0x001F;
	return t;
}

static void init_video()
{
   nBurnBpp = 2;
   BurnReinitScrn = init_video;
   BurnHighCol = HighCol15;
}

static void init_audio()
{
}

// Infer paths from basename.
bool snes_load_cartridge_normal(const char*, const uint8_t *, unsigned)
{
   unsigned i = BurnDrvGetIndexByNameA(g_basename);
   if (i < nBurnDrvCount)
   {
      if (!fba_init(i))
         return false;

      init_video();
      init_audio();
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

