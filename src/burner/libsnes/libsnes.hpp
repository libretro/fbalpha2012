#ifndef LIBSNES_HPP
#define LIBSNES_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SNES_PORT_1  0
#define SNES_PORT_2  1

#define SNES_DEVICE_NONE         0
#define SNES_DEVICE_JOYPAD       1
#define SNES_DEVICE_MULTITAP     2
#define SNES_DEVICE_MOUSE        3
#define SNES_DEVICE_SUPER_SCOPE  4
#define SNES_DEVICE_JUSTIFIER    5
#define SNES_DEVICE_JUSTIFIERS   6

#define SNES_DEVICE_ID_JOYPAD_B        0
#define SNES_DEVICE_ID_JOYPAD_Y        1
#define SNES_DEVICE_ID_JOYPAD_SELECT   2
#define SNES_DEVICE_ID_JOYPAD_START    3
#define SNES_DEVICE_ID_JOYPAD_UP       4
#define SNES_DEVICE_ID_JOYPAD_DOWN     5
#define SNES_DEVICE_ID_JOYPAD_LEFT     6
#define SNES_DEVICE_ID_JOYPAD_RIGHT    7
#define SNES_DEVICE_ID_JOYPAD_A        8
#define SNES_DEVICE_ID_JOYPAD_X        9
#define SNES_DEVICE_ID_JOYPAD_L       10
#define SNES_DEVICE_ID_JOYPAD_R       11

#define SNES_DEVICE_ID_MOUSE_X      0
#define SNES_DEVICE_ID_MOUSE_Y      1
#define SNES_DEVICE_ID_MOUSE_LEFT   2
#define SNES_DEVICE_ID_MOUSE_RIGHT  3

#define SNES_DEVICE_ID_SUPER_SCOPE_X        0
#define SNES_DEVICE_ID_SUPER_SCOPE_Y        1
#define SNES_DEVICE_ID_SUPER_SCOPE_TRIGGER  2
#define SNES_DEVICE_ID_SUPER_SCOPE_CURSOR   3
#define SNES_DEVICE_ID_SUPER_SCOPE_TURBO    4
#define SNES_DEVICE_ID_SUPER_SCOPE_PAUSE    5

#define SNES_DEVICE_ID_JUSTIFIER_X        0
#define SNES_DEVICE_ID_JUSTIFIER_Y        1
#define SNES_DEVICE_ID_JUSTIFIER_TRIGGER  2
#define SNES_DEVICE_ID_JUSTIFIER_START    3

#define SNES_REGION_NTSC  0
#define SNES_REGION_PAL   1

#define SNES_MEMORY_CARTRIDGE_RAM       0
#define SNES_MEMORY_CARTRIDGE_RTC       1
#define SNES_MEMORY_BSX_RAM             2
#define SNES_MEMORY_BSX_PRAM            3
#define SNES_MEMORY_SUFAMI_TURBO_A_RAM  4
#define SNES_MEMORY_SUFAMI_TURBO_B_RAM  5
#define SNES_MEMORY_GAME_BOY_RAM        6
#define SNES_MEMORY_GAME_BOY_RTC        7

#define SNES_MEMORY_WRAM    100
#define SNES_MEMORY_APURAM  101
#define SNES_MEMORY_VRAM    102
#define SNES_MEMORY_OAM     103
#define SNES_MEMORY_CGRAM   104

//FBA
#define P1_COIN	0x06
#define P1_START 0x02
#define P1_LEFT 0xCB
#define P1_RIGHT 0xCD
#define P1_UP 0xC8
#define P1_DOWN 0xD0
#define P1_FIRE1 0x2C
#define P1_FIRE2 0x2D
#define P1_FIRE3 0x2E
#define P1_FIRE4 0x2F
#define P1_FIRE5 0x1F
#define P1_FIRE6 0x20
#define P1_SERVICE 0x3C

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


typedef void (*snes_video_refresh_t)(const uint16_t *data, unsigned width, unsigned height);
typedef void (*snes_audio_sample_t)(uint16_t left, uint16_t right);
typedef void (*snes_input_poll_t)(void);
typedef int16_t (*snes_input_state_t)(bool port, unsigned device, unsigned index, unsigned id);

unsigned snes_library_revision_major(void);
unsigned snes_library_revision_minor(void);
const char *snes_library_id(void);

void snes_set_video_refresh(snes_video_refresh_t);
void snes_set_audio_sample(snes_audio_sample_t);
void snes_set_input_poll(snes_input_poll_t);
void snes_set_input_state(snes_input_state_t);

void snes_set_controller_port_device(bool port, unsigned device);
void snes_set_cartridge_basename(const char *basename);

void snes_init(void);
void snes_term(void);
void snes_power(void);
void snes_reset(void);
void snes_run(void);

unsigned snes_serialize_size(void);
bool snes_serialize(uint8_t *data, unsigned size);
bool snes_unserialize(const uint8_t *data, unsigned size);

void snes_cheat_reset(void);
void snes_cheat_set(unsigned index, bool enabled, const char *code);

bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
);

bool snes_load_cartridge_bsx_slotted(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
);

bool snes_load_cartridge_bsx(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
);

bool snes_load_cartridge_sufami_turbo(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *sta_xml, const uint8_t *sta_data, unsigned sta_size,
  const char *stb_xml, const uint8_t *stb_data, unsigned stb_size
);

bool snes_load_cartridge_super_game_boy(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *dmg_xml, const uint8_t *dmg_data, unsigned dmg_size
);

void snes_unload_cartridge(void);

bool snes_get_region(void);
uint8_t* snes_get_memory_data(unsigned id);
unsigned snes_get_memory_size(unsigned id);

#ifdef __cplusplus
}
#endif

#endif
