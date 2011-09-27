#include "libsnes.hpp"
#include "../burner.h"

#include <stdio.h>

static snes_video_refresh_t video_cb = NULL;
static snes_audio_sample_t audio_cb = NULL;
static snes_input_poll_t poll_cb = NULL;
static snes_input_state_t input_cb = NULL;

//global (static) variables for FBA
static int nAudSampleRate = 48000;
static int nAudSegCount = 6;
static int nAudSegLen = 0;
int16_t * pAudNextsound = NULL;

int bDrvOkay = 0;
static bool bInputOkay = false;
static bool bAudOkay = false;
static bool bVidOkay = false;
//bAltPause
static int bRunPause = 0;
static int nAppVirtualFps = 6000;
//nBurnFPS
//nBurnSoundRate;					
//pBurnSoundOut;
//nBurnSoundLen;
static int nVidImageWidth = 640;
static int nVidImageHeight = 480;
static int nVidImageDepth = 0; // todo
static int nVidImageBPP = 0; // todo
static unsigned char * pVidTransImage = NULL;
static unsigned int * pVidTransPalette = NULL;
extern unsigned int nCurrentFrame;


unsigned snes_library_revision_major(void)
{
   return 1;
}

unsigned snes_library_revision_minor(void)
{
   return 0;
}

const char *snes_library_id(void)
{
   return "FBANext";
}

void snes_set_video_refresh(snes_video_refresh_t cb)
{
   video_cb = cb;
}

void snes_set_audio_sample(snes_audio_sample_t cb)
{
   audio_cb = cb;
}

void snes_set_input_poll(snes_input_poll_t cb)
{
   poll_cb = cb;
}

void snes_set_input_state(snes_input_state_t cb)
{
   input_cb = cb;
}

void snes_set_controller_port_device(bool, unsigned)
{}

void snes_set_cartridge_basename(const char*)
{}

static uint8_t *state_buf = NULL;

void snes_init(void)
{
   state_buf = new uint8_t[2000000];
}

static unsigned serialize_size = 0;

static void configAppLoadXml()
{}

static int AudWriteSilence(int)
{
	if(pAudNextSound)
		memset(pAudNextSound, 0, nAudAllocSegLen);
	return 0;
}

static int audioInit()
{
	bAudOkay = true;
	return 0;
}

static bool lock(unsigned int *&data, unsigned &pitch)
{
	pitch = iwidth * sizeof(unsigned int);
	return data = buffer;
}

#define VidSCopyImage32(dst_ori) \
   register uint16_t lineSize = nVidImageWidth << 2; \
   uint16_t height = nVidImageHeight; \
   uint8_t * dst = (uint8_t *)dst_ori; \
   do{ \
      height--; \
      memcpy(dst, ps, lineSize); \
      ps += s; \
      dst += pitch; \
   }while(height);

static void VidCopyFrame(void)
{
	unsigned int* pd;
	unsigned int pitch;
	lock(pd, pitch);
	uint8_t * ps = pVidImage + (nVidImageLeft << 2);
	int s = nVidImageWidth << 2;
	VidSCopyImage32(pd);
	unsigned int inwidth = nGameImageWidth;
	unsigned int inheight = nGameImageHeight;

}

static void VidFrame(void)
{
	if (pVidTransImage)
	{
		uint16_t * pSrc = (uint16_t *)pVidTransImage;
		uint8_t * pDest = pVidImage;
		if (bVidRecalcPalette)
		{
			uint64_t r = 0;
			do{
				uint64_t g = 0;
				do{
					uint64_t b = 0;
					do{
						uint64_t r_ = r | (r >> 5);
						uint64_t g_ = g | (g >> 5);
						uint64_t b_ = b | (b >> 5);
						pVidTransPalette[(r << 7) | (g << 2) | (b >> 3)] = ARGB(r_,g_,b_);
						b += 8;
					}while(b < 256);
					g += 8;
				}while(g < 256);
				r += 8;
			}while(r < 256);
			bVidRecalcPalette = false;
		}
		pBurnDraw = pVidTransImage;
		nBurnPitch = nVidImageWidth << 1;
		BurnDrvFrame();
		//_psglRender();
		/* set avi buffer, modified by regret */
		pBurnDraw = NULL;
		nBurnPitch = 0;
		int y = 0;
		do{
			int x = 0;
			do{
				((uint32_t *)pDest)[x] = pVidTransPalette[pSrc[x]];
				x++;
			}while(x < nVidImageWidth);
			y++;
			pSrc += nVidImageWidth;
			pDest += nVidImagePitch;
		}while(y < nVidImageHeight);
	}
	else
	{
		pBurnDraw = pVidImage;
		nBurnPitch = nVidImagePitch;
		BurnDrvFrame();
		//_psglRender();
		/* set avi buffer, modified by regret */
		pBurnDraw = NULL;
		nBurnPitch = 0;
	}
}

static int VidExit()
{
	if (!bVidOkay)
		return 1;

	//int nRet = VidDriver(nVidActive)->Exit();
	bAudOkay = false;
	bAudPlaying = false;

	bVidOkay = false;

	nVidImageWidth = DEFAULT_IMAGE_WIDTH;
	nVidImageHeight = DEFAULT_IMAGE_HEIGHT;

	nVidImageBPP = nVidImageDepth = 0;
	nBurnPitch = nBurnBpp = 0;

	free(pVidTransPalette);
	pVidTransPalette = NULL;
	free(pVidTransImage);
	pVidTransImage = NULL;

	return nRet;
}

static int VidInit()
{
	VidExit(); 

	int nRet = 1;

	if (bDrvOkay)
	{
		nVidActive = nVidSelect;						 
		if ((nRet = VidDriver(nVidActive)->Init()) == 0)
		{
			nBurnBpp = nVidImageBPP; // Set Burn library Bytes per pixel
			bVidOkay = true;

			if (bDrvOkay && (BurnDrvGetFlags() & BDF_16BIT_ONLY) && nVidImageBPP > 2)
			{
				nBurnBpp = 2;

				pVidTransPalette = (unsigned int*)malloc(transPaletteSize * sizeof(int));
				pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * (nVidImageBPP >> 1) * sizeof(short));

				BurnHighCol = HighCol15;

				if (pVidTransPalette == NULL || pVidTransImage == NULL)
				{
					VidExit();
					nRet = 1;
				}
			}
		}
	}

	return nRet;
}

static int InputInit(void)
{
	bInputOkay = true;
	return bInputOkay;
}

static int mediaInit()
{
	if (!bInputOkay)
		InputInit(); // Init Input

	nBaseFps = nBurnFPS;
	nAppVirtualFps = nBurnFPS;

	if (!bAudOkay)
	{
		audioInit();					// Init Sound (not critical if it fails)

		if (!bAudOkay)
		{
			// Make sure the error will be visible
		}
	}

	// Assume no sound
	nBurnSoundRate = 0;					
	pBurnSoundOut = NULL;

	if (bAudOkay)
	{
		nBurnSoundRate = nAudSampleRate;
		nBurnSoundLen = nAudSegLen;
	}

	if (!bVidOkay)
	{
		VidInit(); // Reinit the video plugin

		if (bVidOkay && ((bRunPause && bAltPause) || !bDrvOkay))
			VidRedraw();
	}

	return 0;
}

int directLoadGame(const char * name)
{
	int RomOK = 1;

	if (!name)
		return 1;

	nVidFullscreen = 1;

	if (_tcsicmp(&name[_tcslen(name) - 3], _T(".fs")) == 0)
	{
		if (BurnStateLoad(name, 1, &DrvInitCallback))
			return 1;
	}
	else
	{
		char * p = getBaseName(name);			// get game name
		unsigned int i = BurnDrvGetIndexByNameA(p);	// load game

		if (i < nBurnDrvCount)
		{
			RomOK = BurnerDrvInit(i, true);
			bAltPause = 0;
		}
	}

	return RomOK;
}

static void fba_init(const char *tmppath)
{
	configAppLoadXml(); // no, this isn't actually XML at all
	BurnLibInit();
	getAllRomsetInfo();
	nVidFullscreen = 1;
	BurnExtLoadOneRom = archiveLoadoneFIle;
	InitRomList();
	InitInputList();
	InitDipList();

	BuildRomList();

	directLoadGame(strdup(tmppath));
	mediaInit();
	bVidOkay = bAudOkay = true;
	//serialize_size = CPUWriteState_libgba(state_buf, 2000000);
}

void snes_term(void)
{
   delete[] state_buf;
}

void snes_power(void)
{}

void snes_reset(void)
{}

// This will process all PC-side inputs and optionally update the emulated game side.
static void InputMake(void)
{
	struct GameInp* pgi;
	unsigned int i;

	// Do one frames worth of keyboard input sliders
	// Begin of InputTick()

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		int nAdd = 0;
		if ((pgi->nInput &  GIT_GROUP_SLIDER) == 0) // not a slider
			continue;

		if (pgi->nInput == GIT_KEYSLIDER)
		{
			// Get states of the two keys
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[0]))
				nAdd -= 0x100;
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[1]))
				nAdd += 0x100;
		}

		// nAdd is now -0x100 to +0x100

		// Change to slider speed
		nAdd *= pgi->Input.Slider.nSliderSpeed;
		nAdd /= 0x100;

		if (pgi->Input.Slider.nSliderCenter)
		{ // Attact to center
			int v = pgi->Input.Slider.nSliderValue - 0x8000;
			v *= (pgi->Input.Slider.nSliderCenter - 1);
			v /= pgi->Input.Slider.nSliderCenter;
			v += 0x8000;
			pgi->Input.Slider.nSliderValue = v;
		}

		pgi->Input.Slider.nSliderValue += nAdd;
		// Limit slider
		if (pgi->Input.Slider.nSliderValue < 0x0100)
			pgi->Input.Slider.nSliderValue = 0x0100;
		if (pgi->Input.Slider.nSliderValue > 0xFF00)
			pgi->Input.Slider.nSliderValue = 0xFF00;
	}
	// End of InputTick()

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		if (pgi->Input.pVal == NULL)
			continue;

		switch (pgi->nInput)
		{
			case 0: // Undefined
				pgi->Input.nVal = 0;
				break;
			case GIT_CONSTANT: // Constant value
				pgi->Input.nVal = pgi->Input.Constant.nConst;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH:
				{ // Digital input
					int s = CinpState(pgi->Input.Switch.nCode);

					if (pgi->nType & BIT_GROUP_ANALOG)
					{
						// Set analog controls to full
						if (s)
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
						if (s)
							pgi->Input.nVal = 1;
						else
							pgi->Input.nVal = 0;
						*(pgi->Input.pVal) = pgi->Input.nVal;
					}

					break;
				}
			case GIT_KEYSLIDER:						// Keyboard slider
				{
					int nSlider = pgi->Input.Slider.nSliderValue;
					if (pgi->nType == BIT_ANALOG_REL) {
						nSlider -= 0x8000;
						nSlider >>= 4;
					}

					pgi->Input.nVal = (unsigned short)nSlider;
#ifdef LSB_FIRST
					*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
					*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
					break;
				}
		}
	}
}


void snes_run(void)
{
	fba_audio(pAudNextSound);
	nCurrentFrame++;
	VidFrame();
	InputMake();
}


unsigned snes_serialize_size(void)
{
   return serialize_size;
}

bool snes_serialize(uint8_t *data, unsigned size)
{
   //return CPUWriteState_libgba(data, size);
}

bool snes_unserialize(const uint8_t *data, unsigned size)
{
   //return CPUReadState_libgba(data, size);
}

void snes_cheat_reset(void)
{}

void snes_cheat_set(unsigned, bool, const char*)
{}

bool snes_load_cartridge_normal(const char*, const uint8_t *rom_data, unsigned rom_size)
{
	const char *tmppath = tmpnam(NULL);
	if (!tmppath)
		return false;

	FILE *file = fopen(tmppath, "wb");
	if (!file)
		return false;

	fwrite(rom_data, 1, rom_size, file);
	fclose(file);
	unsigned ret = fba_init(tmppath);
	unlink(tmppath);

	//TODO: POSIX or 1 for success?
	return 1;
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

void snes_unload_cartridge(void)
{}

bool snes_get_region(void)
{
   return SNES_REGION_NTSC;
}

uint8_t *snes_get_memory_data(unsigned id)
{
   if (id != SNES_MEMORY_CARTRIDGE_RAM)
      return 0;
   return flashSaveMemory;
}

unsigned snes_get_memory_size(unsigned id)
{
   if (id != SNES_MEMORY_CARTRIDGE_RAM)
      return 0;

   return 0x10000;
}

void fba_audio(int16_t * audio_buf, int length)
{
	pBurnSoundOut = audio_buf;

	int16_t * currentSound = audio_buf;
	for (int i = 0; i < length; i += 2)
		audio_cb(currentSound[i + 0], currentSound[i + 1]);
}
#endif

static uint16_t pix_buf[160 * 1024];

// Stubs

#if 0
u32 systemReadJoypad(int)
{
   poll_cb();

   u32 J = 0;

   static const unsigned binds[] = {
      SNES_DEVICE_ID_JOYPAD_A,
      SNES_DEVICE_ID_JOYPAD_B,
      SNES_DEVICE_ID_JOYPAD_SELECT,
      SNES_DEVICE_ID_JOYPAD_START,
      SNES_DEVICE_ID_JOYPAD_RIGHT,
      SNES_DEVICE_ID_JOYPAD_LEFT,
      SNES_DEVICE_ID_JOYPAD_UP,
      SNES_DEVICE_ID_JOYPAD_DOWN,
      SNES_DEVICE_ID_JOYPAD_R,
      SNES_DEVICE_ID_JOYPAD_L
   };

   for (unsigned i = 0; i < 10; i++)
      J |= input_cb(SNES_PORT_1, SNES_DEVICE_JOYPAD, 0, binds[i]) << i;

   return J;
}
#endif
