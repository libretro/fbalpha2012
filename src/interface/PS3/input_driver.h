#ifndef INPUT_DRIVER_H
#define INPUT_DRIVER_H

#include "inp_keys.h"		// Key codes

#define LIST_DEVICES
#define MAX_KEYBOARD	(0)
#define MAX_JOYSTICK	(4)
#define MAX_JOYAXIS	(4)
#define MAX_MOUSE	(0)
#define MAX_MOUSEAXIS	(0)

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

extern int ArcadeJoystick;
extern bool DoReset;
extern int GameStatus;

#endif
