// FB Alpha Neo Geo driver module

#include "neogeo.h"

#include "bitswap.h"

static struct BurnRomInfo emptyRomDesc[] = {
	{ "",                    0,          0, 0 },
};

// -----------------------------------------------------------------------------
// Common data

static struct BurnInputInfo neogeoInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	NeoButton2 + 0,	"p1 coin"},			//  0
//	{"P1 Coin 2",	BIT_DIGITAL,	NeoButton2 + 3,	"p1 coin 2"},
	{"P1 Start",	BIT_DIGITAL,	NeoButton1 + 0,	"p1 start"},		//	1
	{"P1 Select",	BIT_DIGITAL,	NeoButton1 + 1,	"p1 select"},		//  2

	{"P1 Up",		BIT_DIGITAL,	NeoJoy1 + 0,	"p1 up"},			//  3
	{"P1 Down",		BIT_DIGITAL,	NeoJoy1 + 1,	"p1 down"},			//  4
	{"P1 Left",		BIT_DIGITAL,	NeoJoy1 + 2,	"p1 left"},			//	5
	{"P1 Right",	BIT_DIGITAL,	NeoJoy1 + 3,	"p1 right"},		//	6
	{"P1 Button A",	BIT_DIGITAL,	NeoJoy1 + 4,	"p1 fire 1"},		//	7
	{"P1 Button B",	BIT_DIGITAL,	NeoJoy1 + 5,	"p1 fire 2"},		//	8
	{"P1 Button C",	BIT_DIGITAL,	NeoJoy1 + 6,	"p1 fire 3"},		//	9
	{"P1 Button D",	BIT_DIGITAL,	NeoJoy1 + 7,	"p1 fire 4"},		//	A

	{"P2 Coin",		BIT_DIGITAL,	NeoButton2 + 1,	"p2 coin"},			//	B
//	{"P2 Coin 2",	BIT_DIGITAL,	NeoButton2 + 4,	"p2 coin 2"},
	{"P2 Start",	BIT_DIGITAL,	NeoButton1 + 2,	"p2 start"},		//	C
	{"P2 Select",	BIT_DIGITAL,	NeoButton1 + 3,	"p2 select"},		//	D

	{"P2 Up",		BIT_DIGITAL,	NeoJoy2 + 0,	"p2 up"},			//	E
	{"P2 Down",		BIT_DIGITAL,	NeoJoy2 + 1,	"p2 down"},			//  F
	{"P2 Left",		BIT_DIGITAL,	NeoJoy2 + 2,	"p2 left"},			// 10
	{"P2 Right",	BIT_DIGITAL,	NeoJoy2 + 3,	"p2 right"},		// 11
	{"P2 Button A",	BIT_DIGITAL,	NeoJoy2 + 4,	"p2 fire 1"},		// 12
	{"P2 Button B",	BIT_DIGITAL,	NeoJoy2 + 5,	"p2 fire 2"},		// 13
	{"P2 Button C",	BIT_DIGITAL,	NeoJoy2 + 6,	"p2 fire 3"},		// 14
	{"P2 Button D",	BIT_DIGITAL,	NeoJoy2 + 7,	"p2 fire 4"},		// 15

	{"Reset",		BIT_DIGITAL,	&NeoReset,		"reset"},			// 16
	{"Test",		BIT_DIGITAL,	NeoDiag + 0,	"diag"},			// 17
	{"Service",		BIT_DIGITAL,	NeoButton2 + 2,	"service"},			// 18
	{"Dip 1",		BIT_DIPSWITCH,	NeoInput + 4,	"dip"},				// 19
	{"Dip 2",		BIT_DIPSWITCH,	NeoInput + 5,	"dip"},				// 1A

	{"System",		BIT_DIPSWITCH,	&NeoSystem,		"dip"},				// 1B
	{"Slots",		BIT_DIPSWITCH,	&nNeoNumSlots,	"dip"},				// 1C

	{"Debug Dip 1",	BIT_DIPSWITCH,	NeoDebugDip + 0,"dip"},				// 1D
	{"Debug Dip 2",	BIT_DIPSWITCH,	NeoDebugDip + 1,"dip"},				// 1E
};

STDINPUTINFO(neogeo)

#define A(a, b, c, d) { a, b, (unsigned char*)(c), d }

static struct BurnInputInfo neopaddleInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	NeoButton2 + 0,	"p1 coin"},			//  0
	{"P1 Start",	BIT_DIGITAL,	NeoButton1 + 0,	"p1 start"},		//	1
	{"P1 Select",	BIT_DIGITAL,	NeoButton1 + 1,	"p1 select"},		//  2

	{"P1 Up",		BIT_DIGITAL,	NeoJoy1 + 0,	"p1 up"},			//  3
	{"P1 Down",		BIT_DIGITAL,	NeoJoy1 + 1,	"p1 down"},			//  4
	{"P1 Left",		BIT_DIGITAL,	NeoJoy1 + 2,	"p1 left"},			//	5
	{"P1 Right",	BIT_DIGITAL,	NeoJoy1 + 3,	"p1 right"},		//	6
	{"P1 Button A",	BIT_DIGITAL,	NeoJoy1 + 4,	"p1 fire 1"},		//	7
	{"P1 Button B",	BIT_DIGITAL,	NeoJoy1 + 5,	"p1 fire 2"},		//	8
	{"P1 Button C",	BIT_DIGITAL,	NeoJoy1 + 6,	"p1 fire 3"},		//	9
	{"P1 Button D",	BIT_DIGITAL,	NeoJoy1 + 7,	"mouse button 1"},	//	A
   A("P1 Paddle",	BIT_ANALOG_REL, NeoAxis + 0,	"mouse x-axis"),	//	B

	{"P2 Coin",		BIT_DIGITAL,	NeoButton2 + 1,	"p2 coin"},			//	C
	{"P2 Start",	BIT_DIGITAL,	NeoButton1 + 2,	"p2 start"},		//	D
	{"P2 Select",	BIT_DIGITAL,	NeoButton1 + 3,	"p2 select"},		//	E

	{"P2 Up",		BIT_DIGITAL,	NeoJoy2 + 0,	"p2 up"},			//	F
	{"P2 Down",		BIT_DIGITAL,	NeoJoy2 + 1,	"p2 down"},			// 10
	{"P2 Left",		BIT_DIGITAL,	NeoJoy2 + 2,	"p2 left"},			// 11
	{"P2 Right",	BIT_DIGITAL,	NeoJoy2 + 3,	"p2 right"},		// 12
	{"P2 Button A",	BIT_DIGITAL,	NeoJoy2 + 4,	"p2 fire 1"},		// 13
	{"P2 Button B",	BIT_DIGITAL,	NeoJoy2 + 5,	"p2 fire 2"},		// 14
	{"P2 Button C",	BIT_DIGITAL,	NeoJoy2 + 6,	"p2 fire 3"},		// 15
	{"P2 Button D",	BIT_DIGITAL,	NeoJoy2 + 7,	"p2 fire 4"},		// 16
   A("P2 Paddle",	BIT_ANALOG_REL, NeoAxis + 1,	"p2 x-axis"),		// 17

	{"Reset",		BIT_DIGITAL,	&NeoReset,		"reset"},			// 18
	{"Test",		BIT_DIGITAL,	NeoDiag + 0,	"diag"},			// 19
	{"Service",		BIT_DIGITAL,	NeoButton2 + 2,	"service"},			// 1A
	{"Dip 1",		BIT_DIPSWITCH,	NeoInput + 4,	"dip"},				// 1B
	{"Dip 2",		BIT_DIPSWITCH,	NeoInput + 5,	"dip"},				// 1C

	{"System",		BIT_DIPSWITCH,	&NeoSystem,		"dip"},				// 1D
	{"Slots",		BIT_DIPSWITCH,	&nNeoNumSlots,	"dip"},				// 1E

	{"Debug Dip 1",	BIT_DIPSWITCH,	NeoDebugDip + 0,"dip"},				// 1F
	{"Debug Dip 2",	BIT_DIPSWITCH,	NeoDebugDip + 1,"dip"},				// 1G
};

STDINPUTINFO(neopaddle)

static struct BurnInputInfo neotrackballInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	NeoButton2 + 0,	"p1 coin"},			//  0
	{"P1 Start",	BIT_DIGITAL,	NeoButton1 + 0,	"p1 start"},		//	1

   A("P1 X Axis",	BIT_ANALOG_REL,	NeoAxis + 0,	"mouse x-axis"),	//	2
   A("P1 Y Axis",	BIT_ANALOG_REL,	NeoAxis + 1,	"mouse y-axis"),	//	3

	{"P1 Button A",	BIT_DIGITAL,	NeoJoy2 + 4,	"mouse button 1"},	//	4
	{"P1 Button B",	BIT_DIGITAL,	NeoJoy2 + 5,	"mouse button 2"},	//	5

	{"P2 Coin",		BIT_DIGITAL,	NeoButton2 + 1,	"p2 coin"},			//	6
	{"P2 Start",	BIT_DIGITAL,	NeoButton1 + 2,	"p2 start"},		//	7

	{"P2 Button A",	BIT_DIGITAL,	NeoJoy2 + 6,	"p2 fire 1"},		//  8
	{"P2 Button B",	BIT_DIGITAL,	NeoJoy2 + 7,	"p2 fire 2"},		//  9

	{"Reset",		BIT_DIGITAL,	&NeoReset,		"reset"},			//  A
	{"Test",		BIT_DIGITAL,	NeoDiag + 0,	"diag"},			//  B
	{"Service",		BIT_DIGITAL,	NeoButton2 + 2,	"service"},			//  C
	{"Dip 1",		BIT_DIPSWITCH,	NeoInput + 4,	"dip"},				//  D
	{"Dip 2",		BIT_DIPSWITCH,	NeoInput + 5,	"dip"},				//  E

	{"System",		BIT_DIPSWITCH,	&NeoSystem,		"dip"},				//  F
	{"Slots",		BIT_DIPSWITCH,	&nNeoNumSlots,	"dip"},				// 10

	{"Debug Dip 1",	BIT_DIPSWITCH,	NeoDebugDip + 0,"dip"},				// 11
	{"Debug Dip 2",	BIT_DIPSWITCH,	NeoDebugDip + 1,"dip"},				// 12
};

STDINPUTINFO(neotrackball)

#undef A

static struct BurnInputInfo neodualInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	NeoButton2 + 0,	"p1 coin"},			//  0
	{"P1 Start",	BIT_DIGITAL,	NeoButton1 + 0,	"p1 start"},		//	1
	{"P1 Select",	BIT_DIGITAL,	NeoButton1 + 1,	"p1 select"},		//  2

	{"P1 Up",		BIT_DIGITAL,	NeoJoy1 + 0,	"p1 up"},			//  3
	{"P1 Down",		BIT_DIGITAL,	NeoJoy1 + 1,	"p1 down"},			//  4
	{"P1 Left",		BIT_DIGITAL,	NeoJoy1 + 2,	"p1 left"},			//	5
	{"P1 Right",	BIT_DIGITAL,	NeoJoy1 + 3,	"p1 right"},		//	6
	{"P1 Button A",	BIT_DIGITAL,	NeoJoy1 + 4,	"p1 fire 1"},		//	7
	{"P1 Button B",	BIT_DIGITAL,	NeoJoy1 + 5,	"p1 fire 2"},		//	8
	{"P1 Button C",	BIT_DIGITAL,	NeoJoy1 + 6,	"p1 fire 3"},		//	9
	{"P1 Button D",	BIT_DIGITAL,	NeoJoy1 + 7,	"p1 fire 4"},		//	A

	{"P2 Coin",		BIT_DIGITAL,	NeoButton2 + 1,	"p2 coin"},			//	B
	{"P2 Start",	BIT_DIGITAL,	NeoButton1 + 2,	"p2 start"},		//	C
	{"P2 Select",	BIT_DIGITAL,	NeoButton1 + 3,	"p2 select"},		//	D

	{"P2 Up",		BIT_DIGITAL,	NeoJoy2 + 0,	"p2 up"},			//	E
	{"P2 Down",		BIT_DIGITAL,	NeoJoy2 + 1,	"p2 down"},			//  F
	{"P2 Left",		BIT_DIGITAL,	NeoJoy2 + 2,	"p2 left"},			// 10
	{"P2 Right",	BIT_DIGITAL,	NeoJoy2 + 3,	"p2 right"},		// 11
	{"P2 Button A",	BIT_DIGITAL,	NeoJoy2 + 4,	"p2 fire 1"},		// 12
	{"P2 Button B",	BIT_DIGITAL,	NeoJoy2 + 5,	"p2 fire 2"},		// 13
	{"P2 Button C",	BIT_DIGITAL,	NeoJoy2 + 6,	"p2 fire 3"},		// 14
	{"P2 Button D",	BIT_DIGITAL,	NeoJoy2 + 7,	"p2 fire 4"},		// 15

	{"P3 Coin",		BIT_DIGITAL,	NeoButton4 + 0,	"p3 coin"},			// 16
	{"P3 Start",	BIT_DIGITAL,	NeoButton3 + 0,	"p3 start"},		// 17
	{"P3 Select",	BIT_DIGITAL,	NeoButton3 + 1,	"p3 select"},		// 18

	{"P3 Up",		BIT_DIGITAL,	NeoJoy3 + 0,	"p3 up"},			// 19
	{"P3 Down",		BIT_DIGITAL,	NeoJoy3 + 1,	"p3 down"},			// 1A
	{"P3 Left",		BIT_DIGITAL,	NeoJoy3 + 2,	"p3 left"},			// 1B
	{"P3 Right",	BIT_DIGITAL,	NeoJoy3 + 3,	"p3 right"},		// 1C
	{"P3 Button A",	BIT_DIGITAL,	NeoJoy3 + 4,	"p3 fire 1"},		// 1D
	{"P3 Button B",	BIT_DIGITAL,	NeoJoy3 + 5,	"p3 fire 2"},		// 1E
	{"P3 Button C",	BIT_DIGITAL,	NeoJoy3 + 6,	"p3 fire 3"},		// 1F
	{"P3 Button D",	BIT_DIGITAL,	NeoJoy3 + 7,	"p3 fire 4"},		// 20

	{"P4 Coin",		BIT_DIGITAL,	NeoButton4 + 1,	"p4 coin"},			// 21
	{"P4 Start",	BIT_DIGITAL,	NeoButton3 + 2,	"p4 start"},		// 22
	{"P4 Select",	BIT_DIGITAL,	NeoButton3 + 3,	"p4 select"},		// 23

	{"P4 Up",		BIT_DIGITAL,	NeoJoy4 + 0,	"p4 up"},			// 24
	{"P4 Down",		BIT_DIGITAL,	NeoJoy4 + 1,	"p4 down"},			// 25
	{"P4 Left",		BIT_DIGITAL,	NeoJoy4 + 2,	"p4 left"},			// 26
	{"P4 Right",	BIT_DIGITAL,	NeoJoy4 + 3,	"p4 right"},		// 27
	{"P4 Button A",	BIT_DIGITAL,	NeoJoy4 + 4,	"p4 fire 1"},		// 28
	{"P4 Button B",	BIT_DIGITAL,	NeoJoy4 + 5,	"p4 fire 2"},		// 29
	{"P4 Button C",	BIT_DIGITAL,	NeoJoy4 + 6,	"p4 fire 3"},		// 2A
	{"P4 Button D",	BIT_DIGITAL,	NeoJoy4 + 7,	"p4 fire 4"},		// 2B

	{"Reset",		BIT_DIGITAL,	&NeoReset,		"reset"},			// 2C

	{"Test 1",		BIT_DIGITAL,	NeoDiag + 0,	"diag"},			// 2D
	{"Service 1",	BIT_DIGITAL,	NeoButton2 + 2,	"service"},			// 2E
	{"Test 2",		BIT_DIGITAL,	NeoDiag + 1,	""},				// 2F
	{"Service 2",	BIT_DIGITAL,	NeoButton4 + 2,	""},				// 30

	{"Dip 1",		BIT_DIPSWITCH,	NeoInput + 4,	"dip"},				// 31
	{"Dip 2",		BIT_DIPSWITCH,	NeoInput + 5,	"dip"},				// 32

	{"System",		BIT_DIPSWITCH,	&NeoSystem,		"dip"},				// 33
	{"Slots",		BIT_DIPSWITCH,	&nNeoNumSlots,	"dip"},				// 34

	{"Debug Dip 1",	BIT_DIPSWITCH,	NeoDebugDip + 0,"dip"},				// 35
	{"Debug Dip 2",	BIT_DIPSWITCH,	NeoDebugDip + 1,"dip"},				// 36
};

STDINPUTINFO(neodual)

static struct BurnInputInfo neomahjongInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	NeoButton2 + 0,	"p1 coin"},			//  0
	{"P1 Start",	BIT_DIGITAL,	NeoButton1 + 0,	"p1 start"},		//	1
	{"P1 Select",	BIT_DIGITAL,	NeoButton1 + 1,	"p1 select"},		//  2

	{"P1 Up",		BIT_DIGITAL,	NeoJoy1 + 0,	"p1 up"},			//  3
	{"P1 Down",		BIT_DIGITAL,	NeoJoy1 + 1,	"p1 down"},			//  4
	{"P1 Left",		BIT_DIGITAL,	NeoJoy1 + 2,	"p1 left"},			//	5
	{"P1 Right",	BIT_DIGITAL,	NeoJoy1 + 3,	"p1 right"},		//	6
	{"P1 Button A",	BIT_DIGITAL,	NeoJoy1 + 4,	"p1 fire 1"},		//	7
	{"P1 Button B",	BIT_DIGITAL,	NeoJoy1 + 5,	"p1 fire 2"},		//	8
	{"P1 Button C",	BIT_DIGITAL,	NeoJoy1 + 6,	"p1 fire 3"},		//	9
	{"P1 Button D",	BIT_DIGITAL,	NeoJoy1 + 7,	"p1 fire 4"},		//	A

	{"A",			BIT_DIGITAL,	NeoButton1 +  8,	"mah a"},		//  B
	{"B",			BIT_DIGITAL,	NeoButton1 +  9,	"mah b"},		//  C
	{"C",			BIT_DIGITAL,	NeoButton1 + 10,	"mah c"},		//  D
	{"D",			BIT_DIGITAL,	NeoButton1 + 11,	"mah d"},		//  E
	{"E",			BIT_DIGITAL,	NeoButton1 + 12,	"mah e"},		//  F
	{"F",			BIT_DIGITAL,	NeoButton1 + 13,	"mah f"},		// 10
	{"G",			BIT_DIGITAL,	NeoButton1 + 14,	"mah g"},		// 11
	{"H",			BIT_DIGITAL,	NeoButton1 + 16,	"mah h"},		// 12
	{"I",			BIT_DIGITAL,	NeoButton1 + 17,	"mah i"},		// 13
	{"J",			BIT_DIGITAL,	NeoButton1 + 18,	"mah j"},		// 14
	{"K",			BIT_DIGITAL,	NeoButton1 + 19,	"mah k"},		// 15
	{"L",			BIT_DIGITAL,	NeoButton1 + 20,	"mah l"},		// 16
	{"M",			BIT_DIGITAL,	NeoButton1 + 21,	"mah m"},		// 17
	{"N",			BIT_DIGITAL,	NeoButton1 + 22,	"mah n"},		// 18
	{"Pon",			BIT_DIGITAL,	NeoButton1 + 24,	"mah pon"},		// 19
	{"Chi",			BIT_DIGITAL,	NeoButton1 + 25,	"mah chi"},		// 1A
	{"Kan",			BIT_DIGITAL,	NeoButton1 + 26,	"mah kan"},		// 1B
	{"Ron",			BIT_DIGITAL,	NeoButton1 + 27,	"mah ron"},		// 1C
	{"Reach",		BIT_DIGITAL,	NeoButton1 + 28,	"mah reach"},		// 1D

	{"P2 Coin",		BIT_DIGITAL,	NeoButton2 + 1,	"p2 coin"},			// 1E
	{"P2 Start",	BIT_DIGITAL,	NeoButton1 + 2,	"p2 start"},		// 1F
	{"P2 Select",	BIT_DIGITAL,	NeoButton1 + 3,	"p2 select"},		// 20

	{"P2 Up",		BIT_DIGITAL,	NeoJoy2 + 0,	"p2 up"},			// 21
	{"P2 Down",		BIT_DIGITAL,	NeoJoy2 + 1,	"p2 down"},			// 22
	{"P2 Left",		BIT_DIGITAL,	NeoJoy2 + 2,	"p2 left"},			// 23
	{"P2 Right",	BIT_DIGITAL,	NeoJoy2 + 3,	"p2 right"},		// 24
	{"P2 Button A",	BIT_DIGITAL,	NeoJoy2 + 4,	"p2 fire 1"},		// 25
	{"P2 Button B",	BIT_DIGITAL,	NeoJoy2 + 5,	"p2 fire 2"},		// 26
	{"P2 Button C",	BIT_DIGITAL,	NeoJoy2 + 6,	"p2 fire 3"},		// 27
	{"P2 Button D",	BIT_DIGITAL,	NeoJoy2 + 7,	"p2 fire 4"},		// 28

	{"Reset",		BIT_DIGITAL,	&NeoReset,		"reset"},			// 29
	{"Test",		BIT_DIGITAL,	NeoDiag + 0,	"diag"},			// 2A
	{"Service",		BIT_DIGITAL,	NeoButton2 + 2,	"service"},			// 2B
	{"Dip 1",		BIT_DIPSWITCH,	NeoInput + 4,	"dip"},				// 2C
	{"Dip 2",		BIT_DIPSWITCH,	NeoInput + 5,	"dip"},				// 2D

	{"System",		BIT_DIPSWITCH,	&NeoSystem,		"dip"},				// 2E
	{"Slots",		BIT_DIPSWITCH,	&nNeoNumSlots,	"dip"},				// 2F

	{"Debug Dip 1",	BIT_DIPSWITCH,	NeoDebugDip + 0,"dip"},				// 30
	{"Debug Dip 2",	BIT_DIPSWITCH,	NeoDebugDip + 1,"dip"},				// 31
};

STDINPUTINFO(neomahjong)

static struct BurnInputInfo vlinerInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	NeoButton2 + 0,	"p1 coin"},
	{"P1 Payout",	BIT_DIGITAL,	NeoButton1 + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	NeoJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	NeoJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	NeoJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	NeoJoy1 + 3,	"p1 right"},
	{"P1 Big",	BIT_DIGITAL,	NeoJoy1 + 4,	"p1 fire 1"},
	{"P1 Small",	BIT_DIGITAL,	NeoJoy1 + 5,	"p1 fire 2"},
	{"P1 Double Up",	BIT_DIGITAL,	NeoJoy1 + 6,	"p1 fire 3"},
	{"P1 Start/Collect",	BIT_DIGITAL,	NeoJoy1 + 7,	"p1 fire 4"},

	{"P2 Coin",		BIT_DIGITAL,	NeoButton2 + 1,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	NeoButton1 + 2,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	NeoJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	NeoJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	NeoJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	NeoJoy2 + 3,	"p2 right"},
	{"P2 Button A",	BIT_DIGITAL,	NeoJoy2 + 4,	"p2 fire 1"},
	{"P2 Button B",	BIT_DIGITAL,	NeoJoy2 + 5,	"p2 fire 2"},
	{"P2 Button C",	BIT_DIGITAL,	NeoJoy2 + 6,	"p2 fire 3"},
	{"P2 Button D",	BIT_DIGITAL,	NeoJoy2 + 7,	"p2 fire 4"},

	{"Operator Menu",	BIT_DIGITAL,	NeoButton2 + 4,	"op menu"},
	{"Clear Credit",	BIT_DIGITAL,	NeoButton2 + 5,	"clear credit"},
	{"Hopper Out",		BIT_DIGITAL,	NeoButton2 + 7,	"hopper"},

	{"Reset",		BIT_DIGITAL,	&NeoReset,		"reset"},
	{"Test",		BIT_DIGITAL,	NeoDiag + 0,	"diag"},
	{"Dip 1",		BIT_DIPSWITCH,	NeoInput + 4,	"dip"},
	{"Dip 2",		BIT_DIPSWITCH,	NeoInput + 5,	"dip"},

	{"System",		BIT_DIPSWITCH,	&NeoSystem,		"dip"},
	{"Slots",		BIT_DIPSWITCH,	&nNeoNumSlots,	"dip"},

	{"Debug Dip 1",	BIT_DIPSWITCH,	NeoDebugDip + 0,"dip"},
	{"Debug Dip 2",	BIT_DIPSWITCH,	NeoDebugDip + 1,"dip"},
};

STDINPUTINFO(vliner)

static struct BurnDIPInfo neodefaultDIPList[] = {
	// Offset
	{0x19,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x80, NULL},
	{0x03,	0xFF, 0xFF,	0x02, NULL},

	{0,		0xFE, 0,	2,	  "Autofire"},
	{0x00,	0x01, 0x04,	0x00, "Off"},
	{0x00,	0x01, 0x04,	0x04, "On"},
};

static struct BurnDIPInfo neousadefaultDIPList[] = {
	// Offset
	{0x19,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x83, NULL},
	{0x03,	0xFF, 0xFF,	0x02, NULL},

	{0,		0xFE, 0,	2,	  "Autofire"},
	{0x00,	0x01, 0x04,	0x00, "Off"},
	{0x00,	0x01, 0x04,	0x04, "On"},
};

static struct BurnDIPInfo neopaddleDIPList[] = {
	// Offset
	{0x1B,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x86, NULL},
	{0x03,	0xFF, 0xFF,	0x02, NULL},
};

static struct BurnDIPInfo neodualDIPList[] = {
	// Offset
	{0x31,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x86, NULL},
	{0x03,	0xFF, 0xFF,	0x02, NULL},

	{0,		0xFE, 0,	2,	  "Autofire"},
	{0x00,	0x01, 0x04,	0x00, "Off"},
	{0x00,	0x01, 0x04,	0x04, "On"},
};

static struct BurnDIPInfo neomahjongDIPList[] = {
	// Offset
	{0x2C,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x84, NULL},
	{0x03,	0xFF, 0xFF,	0x02, NULL},

	{0,		0xFE, 0,	2,	  "Mahjong control panel"},
	{0x00,	0x01, 0x04,	0x00, "Off"},
	{0x00,	0x01, 0x04,	0x04, "On"},
};

static struct BurnDIPInfo neogeoDIPList[] = {
	// DIP 1
	{0,	0xFE, 0,	2,    "Setting mode"                     },
	{0x00,	0x01, 0x01,	0x00, "Off"                              },
	{0x00,	0x01, 0x01,	0x01, "On"                               },

	// 1 or 2 coin slots
	{0,	0xFE, 0,	2,    "Coin chutes"                      },
	{0x00,	0x02, 0x02,	0x00, "1"                                },
	{0x02,	0x00, 0x3F,	0x06, NULL                               },
	{0x00,	0x02, 0x02,	0x02, "2"                                },
	{0x02,	0x00, 0x3F,	0x06, NULL                               },
	// 2 or 4 coin slots
	{0,	0xFE, 0,	2,    "Coin chutes"                      },
	{0x00,	0x82, 0x02,	0x00, "2"                                },
	{0x02,	0x00, 0x3F,	0x06, NULL                               },
	{0x00,	0x82, 0x02,	0x02, "4"                                },
	{0x02,	0x00, 0x3F,	0x06, NULL                               },

	{0,	0xFE, 0,	5,    "Commmunicaton"                    },
	{0x00,	0x01, 0x38,	0x00, "Disabled"                         },
	{0x00,	0x01, 0x38,	0x08, "Setting 1"                        },
	{0x00,	0x01, 0x38,	0x10, "Setting 2"                        },
	{0x00,	0x01, 0x38,	0x18, "Setting 3"                        },
	{0x00,	0x01, 0x38,	0x20, "Setting 4"                        },
	{0x00,	0x01, 0x38,	0x28, "Setting 5"                        },
	{0x00,	0x01, 0x38,	0x30, "Setting 6"                        },
	{0x00,	0x01, 0x38,	0x38, "Setting 7"                        },
	
	{0,     0xFE, 0,	2,    "Free play"                        },
	{0x00,	0x01, 0x40,	0x00, "Off"                              },
	{0x00,	0x01, 0x40,	0x40, "On"                               },
	
	{0,	0xFE, 0,	2,    "Stop mode"                        },
	{0x00,	0x01, 0x80,	0x00, "Off"                              },
	{0x00,	0x01, 0x80,	0x80, "On"                               },

	// Fake DIPs
	// BIOS
	{0,	0xFD, 0,	23,   "BIOS"                             },
	{0x02,	0x01, 0x1f,	0x00, "MVS Asia/Europe ver. 6 (1 slot)"  },
	{0x02,	0x01, 0x1f,	0x01, "MVS Asia/Europe ver. 5 (1 slot)"  },
	{0x02,	0x01, 0x1f,	0x02, "MVS Asia/Europe ver. 3 (4 slot)"  },
	{0x02,	0x01, 0x1f,	0x03, "MVS USA ver. 5 (2 slot)"          },
	{0x02,	0x01, 0x1f,	0x04, "MVS USA ver. 5 (6 slot)"          },
	{0x02,	0x01, 0x1f,	0x05, "MVS Japan ver. 6 (? slot)"        },
	{0x02,	0x01, 0x1f,	0x06, "MVS Japan ver. 5 (? slot)"        },
	{0x02,	0x01, 0x1f,	0x07, "MVS Japan ver. 3 (4 slot)"        },
	{0x02,	0x01, 0x1f,	0x08, "NEO-MVH MV1C"                     },
	{0x02,	0x01, 0x1f,	0x09, "MVS Japan (J3)"                   },
	{0x02,	0x01, 0x1f,	0x0a, "AES Japan"                        },
	{0x02,	0x01, 0x1f,	0x0b, "AES Asia"                         },
	{0x02,	0x01, 0x1f,	0x0c, "Development Kit"                  },
	{0x02,	0x01, 0x1f,	0x0d, "Deck ver. 6 (Git Ver 1.3)"        },
	{0x02,	0x01, 0x1f,	0x0e, "Universe BIOS ver. 2.3"           },
	{0x02,	0x01, 0x1f,	0x0f, "Universe BIOS ver. 2.3 (alt)"     },
	{0x02,	0x01, 0x1f,	0x10, "Universe BIOS ver. 2.2"           },
	{0x02,	0x01, 0x1f,	0x11, "Universe BIOS ver. 2.1"           },
	{0x02,	0x01, 0x1f,	0x12, "Universe BIOS ver. 2.0"           },
	{0x02,	0x01, 0x1f,	0x13, "Universe BIOS ver. 1.3"           },
	{0x02,	0x01, 0x1f,	0x14, "Universe BIOS ver. 1.2"           },
	{0x02,	0x01, 0x1f,	0x15, "Universe BIOS ver. 1.2 (alt)"     },
	{0x02,	0x01, 0x1f,	0x16, "Universe BIOS ver. 1.1"           },
	{0x02,	0x01, 0x1f,	0x17, "Universe BIOS ver. 1.0"           },

	// Number of cartridge slots
	{0,	0xFD, 0,	3,    "Cartridge Slots"                  },
	{0x03,	0x01, 0xFF,	0x02, "1 / 2"                            },
	{0x03,	0x01, 0xFF,	0x04, "4"                                },
	{0x03,	0x01, 0xFF,	0x06, "6"                                },
	{0x03,	0x01, 0xFF,	0x01, "1 / 2"                            },

	// Memory card
	{0,	0xFD, 0,	2,    "Memory card"                      },
	{0x02,	0x01, 0x80,	0x80, "Writable"                         },
	{0x02,	0x01, 0x80,	0x00, "Write-protected"                  },
	
	{0,	0xFD, 0,	2,    "Default card type"                },
	{0x02,	0x01, 0x40,	0x40, "1 Megabit"                        },
	{0x02,	0x01, 0x40,	0x00, "Normal"                           },
};

STDDIPINFOEXT(neogeo, neodefault, neogeo)
STDDIPINFOEXT(neogeousa, neousadefault, neogeo)
STDDIPINFOEXT(neopaddle, neopaddle, neogeo)
STDDIPINFOEXT(neodual, neodual, neogeo)
STDDIPINFOEXT(neomahjong, neomahjong, neogeo)

static struct BurnDIPInfo neotrackballDIPList[] = {
	// Defaults
	{0x0D,	0xFF, 0xFF,	0x00, NULL},
	{0x0E,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x0F,	0xFF, 0xFF,	0x86, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Setting mode"},
	{0x0D,	0x01, 0x01,	0x00, "Off"},
	{0x0D,	0x01, 0x01,	0x01, "On"},
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x0D,	0x01, 0x02,	0x00, "1"},
	{0x0D,	0x01, 0x02,	0x02, "2"},
	{0,		0xFE, 0,	2,	  "Autofire"},
	{0x0D,	0x01, 0x04,	0x00, "Off"},
	{0x0D,	0x01, 0x04,	0x04, "On"},
	{0,		0xFE, 0,	5,	  "Commmunicaton"},
	{0x0D,	0x01, 0x38,	0x00, "Disabled"},
	{0x0D,	0x01, 0x38,	0x08, "Setting 1"},
	{0x0D,	0x01, 0x38,	0x10, "Setting 2"},
	{0x0D,	0x01, 0x38,	0x18, "Setting 3"},
	{0x0D,	0x01, 0x38,	0x20, "Setting 4"},
	{0x0D,	0x01, 0x38,	0x28, "Setting 5"},
	{0x0D,	0x01, 0x38,	0x30, "Setting 6"},
	{0x0D,	0x01, 0x38,	0x38, "Setting 7"},
	{0,		0xFE, 0,	2,	  "Free play"},
	{0x0D,	0x01, 0x40,	0x00, "Off"},
	{0x0D,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  "Stop mode"},
	{0x0D,	0x01, 0x80,	0x00, "Off"},
	{0x0D,	0x01, 0x80,	0x80, "On"},

	// MVS/AES system
	{0,		0xFD, 0,	2,	  NULL},
	{0x0F,	0x02, 0x04,	0x04, "MVS system"},
	{0x0F,	0x00, 0x30,	0x00, NULL},
	{0x0F,	0x02, 0x04,	0x00, "AES system"},
	{0x0F,	0x00, 0x30,	0x00, NULL},
	// Region
	{0,		0xFD, 0,	3,	  "Region"},
	{0x0F,	0x01, 0x03,	0x00, "Japan"},
	{0x0F,	0x01, 0x03,	0x01, "USA"},
	{0x0F,	0x01, 0x03,	0x02, "Asia / Europe"},
	{0x0F,	0x01, 0x03,	0x03, "Japan"},

	// Memory card
	{0,		0xFD, 0,	2,	  "Memory card"},
	{0x0F,	0x01, 0x80,	0x80, "Writable"},
	{0x0F,	0x01, 0x80,	0x00, "Write-protected"},
	{0,		0xFD, 0,	2,	  "Default card type"},
	{0x0F,	0x01, 0x40,	0x40, "1 Megabit"},
	{0x0F,	0x01, 0x40,	0x00, "Normal"},
};

static struct BurnDIPInfo ms5pcbDIPList[] = {
	// Offset
	{0x19,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x86, NULL},

	{0,		0xFE, 0,	2,	  "Autofire"},
	{0x0D,	0x01, 0x04,	0x00, "Off"},
	{0x0D,	0x01, 0x04,	0x04, "On"},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Setting mode"},
	{0x00,	0x01, 0x01,	0x00, "Off"},
	{0x00,	0x01, 0x01,	0x01, "On"},

	// 1 or 2 coin slots
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x00,	0x02, 0x02,	0x00, "1"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	{0x00,	0x02, 0x02,	0x02, "2"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	// 2 or 4 coin slots
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x00,	0x82, 0x02,	0x00, "2"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	{0x00,	0x82, 0x02,	0x02, "4"},
	{0x02,	0x00, 0x3F,	0x06, NULL},

	{0,		0xFE, 0,	5,	  "Commmunicaton"},
	{0x00,	0x01, 0x38,	0x00, "Disabled"},
	{0x00,	0x01, 0x38,	0x08, "Setting 1"},
	{0x00,	0x01, 0x38,	0x10, "Setting 2"},
	{0x00,	0x01, 0x38,	0x18, "Setting 3"},
	{0x00,	0x01, 0x38,	0x20, "Setting 4"},
	{0x00,	0x01, 0x38,	0x28, "Setting 5"},
	{0x00,	0x01, 0x38,	0x30, "Setting 6"},
	{0x00,	0x01, 0x38,	0x38, "Setting 7"},
	{0,		0xFE, 0,	2,	  "Free play"},
	{0x00,	0x01, 0x40,	0x00, "Off"},
	{0x00,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  "Stop mode"},
	{0x00,	0x01, 0x80,	0x00, "Off"},
	{0x00,	0x01, 0x80,	0x80, "On"},
	
	// Region
	{0,		0xFD, 0,	2,	  "Region"},
	{0x02,	0x02, 0x03,	0x02, "Asia / Europe"},
	{0x02,	0x00, 0x30,	0x00, NULL},
	{0x02,	0x02, 0x03,	0x03, "Japan"},
	{0x02,	0x00, 0x30,	0x00, NULL},
};

static struct BurnDIPInfo svcpcbDIPList[] = {
	// Offset
	{0x19,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x86, NULL},

	{0,		0xFE, 0,	2,	  "Autofire"},
	{0x0D,	0x01, 0x04,	0x00, "Off"},
	{0x0D,	0x01, 0x04,	0x04, "On"},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Setting mode"},
	{0x00,	0x01, 0x01,	0x00, "Off"},
	{0x00,	0x01, 0x01,	0x01, "On"},

	// 1 or 2 coin slots
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x00,	0x02, 0x02,	0x00, "1"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	{0x00,	0x02, 0x02,	0x02, "2"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	// 2 or 4 coin slots
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x00,	0x82, 0x02,	0x00, "2"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	{0x00,	0x82, 0x02,	0x02, "4"},
	{0x02,	0x00, 0x3F,	0x06, NULL},

	{0,		0xFE, 0,	5,	  "Commmunicaton"},
	{0x00,	0x01, 0x38,	0x00, "Disabled"},
	{0x00,	0x01, 0x38,	0x08, "Setting 1"},
	{0x00,	0x01, 0x38,	0x10, "Setting 2"},
	{0x00,	0x01, 0x38,	0x18, "Setting 3"},
	{0x00,	0x01, 0x38,	0x20, "Setting 4"},
	{0x00,	0x01, 0x38,	0x28, "Setting 5"},
	{0x00,	0x01, 0x38,	0x30, "Setting 6"},
	{0x00,	0x01, 0x38,	0x38, "Setting 7"},
	{0,		0xFE, 0,	2,	  "Free play"},
	{0x00,	0x01, 0x40,	0x00, "Off"},
	{0x00,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  "Stop mode"},
	{0x00,	0x01, 0x80,	0x00, "Off"},
	{0x00,	0x01, 0x80,	0x80, "On"},

	// Region
	{0,		0xFD, 0,	2,	  "Region"},
	{0x02,	0x02, 0x03,	0x02, "Asia / Europe"},
	{0x02,	0x00, 0x30,	0x00, NULL},
	{0x02,	0x02, 0x03,	0x03, "Japan"},
	{0x02,	0x00, 0x30,	0x00, NULL},
};

static struct BurnDIPInfo kf2k3pcbDIPList[] = {
	// Offset
	{0x19,	0xF0, 0x00,	0x00, NULL},

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0x7F,	0x00, NULL},
	// Fake DIPs
	{0x02,	0xFF, 0xFF,	0x86, NULL},

	{0,		0xFE, 0,	2,	  "Autofire"},
	{0x0D,	0x01, 0x04,	0x00, "Off"},
	{0x0D,	0x01, 0x04,	0x04, "On"},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Setting mode"},
	{0x00,	0x01, 0x01,	0x00, "Off"},
	{0x00,	0x01, 0x01,	0x01, "On"},

	// 1 or 2 coin slots
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x00,	0x02, 0x02,	0x00, "1"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	{0x00,	0x02, 0x02,	0x02, "2"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	// 2 or 4 coin slots
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x00,	0x82, 0x02,	0x00, "2"},
	{0x02,	0x00, 0x3F,	0x06, NULL},
	{0x00,	0x82, 0x02,	0x02, "4"},
	{0x02,	0x00, 0x3F,	0x06, NULL},

	{0,		0xFE, 0,	5,	  "Commmunicaton"},
	{0x00,	0x01, 0x38,	0x00, "Disabled"},
	{0x00,	0x01, 0x38,	0x08, "Setting 1"},
	{0x00,	0x01, 0x38,	0x10, "Setting 2"},
	{0x00,	0x01, 0x38,	0x18, "Setting 3"},
	{0x00,	0x01, 0x38,	0x20, "Setting 4"},
	{0x00,	0x01, 0x38,	0x28, "Setting 5"},
	{0x00,	0x01, 0x38,	0x30, "Setting 6"},
	{0x00,	0x01, 0x38,	0x38, "Setting 7"},
	{0,		0xFE, 0,	2,	  "Free play"},
	{0x00,	0x01, 0x40,	0x00, "Off"},
	{0x00,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  "Stop mode"},
	{0x00,	0x01, 0x80,	0x00, "Off"},
	{0x00,	0x01, 0x80,	0x80, "On"},
};

STDDIPINFO(neotrackball)
STDDIPINFO(ms5pcb)
STDDIPINFO(svcpcb)
STDDIPINFO(kf2k3pcb)

// Rom information
static struct BurnRomInfo neogeoRomDesc[] = {
	{ "asia-s3.rom",       0x20000, 0x91b64be3, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT           }, //  0 MVS Asia/Europe ver. 6 (1 slot)
	{ "sp-s2.sp1",         0x20000, 0x9036d879, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  1 MVS Asia/Europe ver. 5 (1 slot)
	{ "sp-s.sp1",          0x20000, 0xc7f2fa45, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  2 MVS Asia/Europe ver. 3 (4 slot)
	{ "usa_2slt.bin",      0x20000, 0xe72943de, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  3 MVS USA ver. 5 (2 slot)
	{ "sp-e.sp1",          0x20000, 0x2723a5b5, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  4 MVS USA ver. 5 (6 slot)
	{ "vs-bios.rom",       0x20000, 0xf0e8f27d, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  5 MVS Japan ver. 6 (? slot)
	{ "sp-j2.sp1",         0x20000, 0xacede59C, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  6 MVS Japan ver. 5 (? slot)
	{ "sp1.jipan.1024",    0x20000, 0x9fb0abe4, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  7 MVS Japan ver. 3 (4 slot)
	{ "sp-45.sp1",         0x80000, 0x03cc9f6a, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  8 NEO-MVH MV1C
	{ "japan-j3.bin",      0x20000, 0xdff6d41f, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, //  9 MVS Japan (J3)
#if !defined (ROM_VERIFY)
	{ "neo-po.bin",        0x20000, 0x16d0c132, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 10 AES Japan
	{ "neo-epo.bin",       0x20000, 0xd27a71f1, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 11 AES Asia
	{ "neodebug.bin",      0x20000, 0x698ebb7d, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 12 Development Kit
#else
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 10
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 11
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 12
#endif
	{ "sp-1v1_3db8c.bin",  0x20000, 0x162f0ebe, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 13 Deck ver. 6 (Git Ver 1.3)
	{ "uni-bios_2_3.rom",  0x20000, 0x27664eb5, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 14 Universe BIOS ver. 2.3
	{ "uni-bios_2_3o.rom", 0x20000, 0x601720ae, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 15 Universe BIOS ver. 2.3 (alt)
	{ "uni-bios_2_2.rom",  0x20000, 0x2d50996a, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 16 Universe BIOS ver. 2.2
	{ "uni-bios_2_1.rom",  0x20000, 0x8dabf76b, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 17 Universe BIOS ver. 2.1
	{ "uni-bios_2_0.rom",  0x20000, 0x0c12c2ad, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 18 Universe BIOS ver. 2.0
	{ "uni-bios_1_3.rom",  0x20000, 0xb24b44a0, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 19 Universe BIOS ver. 1.3
	{ "uni-bios_1_2.rom",  0x20000, 0x4fa698e9, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 20 Universe BIOS ver. 1.2
	{ "uni-bios_1_2o.rom", 0x20000, 0xe19d3ce9, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 21 Universe BIOS ver. 1.2 (alt)
	{ "uni-bios_1_1.rom",  0x20000, 0x5dda0d84, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 22 Universe BIOS ver. 1.1
	{ "uni-bios_1_0.rom",  0x20000, 0x0ce453a0, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT }, // 23 Universe BIOS ver. 1.0
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 24 Trackball BIOS loaded here
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 25 PCB BIOS loaded here

	{ "sm1.sm1",           0x20000, 0x94416d67, BRF_ESS | BRF_PRG | BRF_BIOS },                        // 26 Z80 BIOS
	{ "sfix.sfix",         0x20000, 0xc2ea0cfd, BRF_GRA | BRF_BIOS },                                  // 27 Text layer tiles
	{ "000-lo.lo",         0x20000, 0x5a86cff2, BRF_ESS | BRF_BIOS },                                  // 28 Zoom table
};

STD_ROM_PICK(neogeo)
STD_ROM_FN(neogeo)

static struct BurnRomInfo neotrackballRomDesc[] = {
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  0
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  1
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  2
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  3
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  4
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  5
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  6
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  7
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  8
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  9
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 10
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 11
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 12
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 13
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 14
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 15
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 16
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 17
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 18
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 19
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 20
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 21
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 22
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 23
	{ "236-bios.sp1",      0x20000, 0x853e6b96, BRF_ESS | BRF_PRG | BRF_BIOS                        }, // 24 Trackball BIOS
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 25

	{ "sm1.sm1",           0x20000, 0x94416d67, BRF_ESS | BRF_PRG | BRF_BIOS },                        // 26 Z80 BIOS
	{ "sfix.sfix",         0x20000, 0xc2ea0cfd, BRF_GRA | BRF_BIOS },                                  // 27 Text layer tiles
	{ "000-lo.lo",         0x20000, 0x5a86cff2, BRF_ESS | BRF_BIOS },                                  // 28 Zoom table
};


static struct BurnRomInfo ms5pcbBIOSRomDesc[] = {
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  0
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  1
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  2
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  3
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  4
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  5
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  6
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  7
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  8
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  9
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 10
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 11
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 12
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 13
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 14
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 15
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 16
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 17
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 18
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 19
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 20
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 21
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 22
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 23
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 24
	{ "sp-4x.sp1",         0x80000, 0xb4590283, BRF_ESS | BRF_PRG | BRF_BIOS                        }, // 25 PCB BIOS

	{ "",                  0x00000, 0x00000000, 0                                                   }, // 26 Z80 BIOS
	{ "sfix.sfix",         0x20000, 0xc2ea0cfd, BRF_GRA | BRF_BIOS },                                  // 27 Text layer tiles
	{ "000-lo.lo",         0x20000, 0x5a86cff2, BRF_ESS | BRF_BIOS },                                  // 28 Zoom table
};


static struct BurnRomInfo svcpcbBIOSRomDesc[] = {
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  0
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  1
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  2
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  3
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  4
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  5
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  6
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  7
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  8
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  9
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 10
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 11
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 12
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 13
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 14
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 15
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 16
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 17
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 18
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 19
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 20
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 21
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 22
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 23
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 24
	{ "sp-4x.sp1",         0x80000, 0xb4590283, BRF_ESS | BRF_PRG | BRF_BIOS                        }, // 25 PCB BIOS

	{ "",                  0x00000, 0x00000000, 0                                                   }, // 26 Z80 BIOS
	{ "sfix.sfix",         0x20000, 0xc2ea0cfd, BRF_GRA | BRF_BIOS },                                  // 27 Text layer tiles
	{ "000-lo.lo",         0x20000, 0x5a86cff2, BRF_ESS | BRF_BIOS },                                  // 28 Zoom table
};


static struct BurnRomInfo kf2k3pcbBIOSRomDesc[] = {
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  0
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  1
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  2
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  3
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  4
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  5
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  6
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  7
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  8
	{ "",                  0x00000, 0x00000000, 0                                                   }, //  9
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 10
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 11
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 12
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 13
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 14
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 15
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 16
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 17
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 18
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 19
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 20
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 21
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 22
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 23
	{ "",                  0x00000, 0x00000000, 0                                                   }, // 24
	{ "spj.sp1",           0x80000, 0x148dd727, BRF_ESS | BRF_PRG | BRF_BIOS                        }, // 25 PCB BIOS

	{ "",                  0x00000, 0x00000000, 0                                                   }, // 26 Z80 BIOS
	{ "sfix.sfix",         0x20000, 0xc2ea0cfd, BRF_GRA | BRF_BIOS },                                  // 27 Text layer tiles
	{ "000-lo.lo",         0x20000, 0x5a86cff2, BRF_ESS | BRF_BIOS },                                  // 28 Zoom table
};


struct BurnDriver BurnDrvNeoGeo = {
	"neogeo", NULL, NULL, NULL, "1990",
	"Neo Geo MVS System\0", "BIOS only", "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_BOARDROM, 0, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_BIOS, 0,
	NULL, neogeoRomInfo, neogeoRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	nNeoScreenWidth, 224, 4, 3
};

// ----------------------------------------------------------------------------
// Extra code for SMA protection

int nSMARandomNumber;

static unsigned int nNeoSMARNGAddress[2] = { 0, 0 };

// Bank in an area of memory
void NeoSMABankswitch()
{
	SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fe3ff, SM_ROM);
	if (nNeoSMARNGAddress[0] > 0 || nNeoSMARNGAddress[1] > 0) {
		SekMapMemory(Neo68KROM + nNeo68KROMBank + 0x0fe800, 0x2fe800, 0x2ffbff, SM_ROM);
	} else {
		SekMapMemory(Neo68KROM + nNeo68KROMBank + 0x0fe800, 0x2fe800, 0x2fffff, SM_ROM);
	}
}

// Extra protection
unsigned short __fastcall neogeoReadWordSMA9A37(unsigned int sekAddress)
{
	if (sekAddress == 0x2fe446) {
		return 0x9a37;
	}

	return *((unsigned short*)(Neo68KROM + nNeo68KROMBank + sekAddress - 0x200000));
}

unsigned char __fastcall neogeoReadByteSMA9A37(unsigned int sekAddress)
{
	if (sekAddress == 0x2fe446) {
		return 0x9a;
	}
	if (sekAddress == 0x2fe447) {
		return 0x37;
	}

	return Neo68KROM[(nNeo68KROMBank + sekAddress - 0x200000) ^ 1];
}

// Random number generator
unsigned short __fastcall neogeoReadWordSMARNG(unsigned int sekAddress)
{
	if (sekAddress == nNeoSMARNGAddress[0] || sekAddress == nNeoSMARNGAddress[1]) {
		int nRandomNubmer = nSMARandomNumber;

		int nNewbit = (
				(nSMARandomNumber >>  2) ^
				(nSMARandomNumber >>  3) ^
				(nSMARandomNumber >>  5) ^
				(nSMARandomNumber >>  6) ^
				(nSMARandomNumber >>  7) ^
				(nSMARandomNumber >> 11) ^
				(nSMARandomNumber >> 12) ^
				(nSMARandomNumber >> 15)) & 1;

		nSMARandomNumber = ((nSMARandomNumber << 1) | nNewbit) & 0x0fffff;

		return nRandomNubmer;
	}

	return *((unsigned short*)(Neo68KROM + nNeo68KROMBank + sekAddress - 0x200000));
}

unsigned char __fastcall neogeoReadByteSMARNG(unsigned int sekAddress)
{
	if ((sekAddress & ~1) == nNeoSMARNGAddress[0] || (sekAddress & ~1) == nNeoSMARNGAddress[1]) {
		int nRandomNubmer = nSMARandomNumber;

		int nNewbit = (
				(nSMARandomNumber >>  2) ^
				(nSMARandomNumber >>  3) ^
				(nSMARandomNumber >>  5) ^
				(nSMARandomNumber >>  6) ^
				(nSMARandomNumber >>  7) ^
				(nSMARandomNumber >> 11) ^
				(nSMARandomNumber >> 12) ^
				(nSMARandomNumber >> 15)) & 1;

		nSMARandomNumber = ((nSMARandomNumber << 1) | nNewbit) & 0x0fffff;

		if (sekAddress & 1) {
			return nRandomNubmer & 0xff;
		} else {
			return nRandomNubmer >> 8;
		}
	}

	return Neo68KROM[(nNeo68KROMBank + sekAddress - 0x200000) ^ 1];
}

int NeoSMAScan(int nAction, int*)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nSMARandomNumber);
	}

	return 0;
}

// Install handlers
static int NeoSMAInit(void (*pInitCallback)(), pSekWriteWordHandler pBankswitchHandler, unsigned int nRNGAddress0, unsigned int nRNGAddress1)
{
	int nRet;

	pNeoInitCallback = pInitCallback;
	pNeoBankswitchCallback = NeoSMABankswitch;
	pNeoScanCallback = NeoSMAScan;

	nRet = NeoInit();

	if (nRet == 0) {

		SekMapHandler(4,		0x2ffc00, 0x2fffff, SM_WRITE);	// Bankswitch

		SekSetWriteWordHandler(4, pBankswitchHandler);

		SekMapHandler(5,		0x2fe400, 0x2fe7ff, SM_ROM);	// Protection

		SekSetReadWordHandler(5, neogeoReadWordSMA9A37);
		SekSetReadByteHandler(5, neogeoReadByteSMA9A37);

		nNeoSMARNGAddress[0] = nRNGAddress0; nNeoSMARNGAddress[1] = nRNGAddress1;
		if (nNeoSMARNGAddress[0] > 0 || nNeoSMARNGAddress[1] > 0) {
			SekMapHandler(6,	0x2ffc00, 0x2fffff, SM_ROM);	// Randum number generator

			SekSetReadWordHandler(6, neogeoReadWordSMARNG);
			SekSetReadByteHandler(6, neogeoReadByteSMARNG);
		}
	}

	nSMARandomNumber = 0x2345;

	return nRet;
}

int NeoSMAExit()
{
	NeoExit();

	nNeoSMARNGAddress[0] = nNeoSMARNGAddress[1] = 0;

	return 0;
}

// ----------------------------------------------------------------------------

static unsigned char CartRAM[0x2000];

void NeoPVCPallette01() // unpack palette word to RGB dword
{
	unsigned char b1, b2;
	b1 = CartRAM[0x1fe1];
	b2 = CartRAM[0x1fe0];

	CartRAM[0x1fe2] = ((b2 & 0xf) << 1) | ((b1 >> 4) & 1);
	CartRAM[0x1fe3] = (((b2 >> 4) & 0xf) << 1) | ((b1 >> 5) & 1);
	CartRAM[0x1fe4] = ((b1 & 0xf) << 1) | ((b1 >> 6) & 1);
	CartRAM[0x1fe5] = b1 >> 7;
}

void NeoPVCPallette02() // pack RGB dword to palette word
{
	unsigned char b1, b2, b3, b4;
	b1 = CartRAM[0x1fe9];
	b2 = CartRAM[0x1fe8];
	b3 = CartRAM[0x1feb];
	b4 = CartRAM[0x1fea];

	CartRAM[0x1fec] = (b2 >> 1) | ((b1 >> 1) << 4);
	CartRAM[0x1fed] = (b4 >> 1) | ((b2 & 1) << 4) | ((b1 & 1) << 5) | ((b4 & 1) << 6) | ((b3 & 1) << 7);
}

void NeoPVCBankswitch()
{
	unsigned int nBank  = (CartRAM[0x1ff3] << 16) | (CartRAM[0x1ff2] << 8) | CartRAM[0x1ff1];
		     nBank += (Neo68KROM[0x108] & 0x10) << 16;	// for kof2003 (bank 0 is $100000)

	if (nNeo68KROMBank != nBank)
	{
		nNeo68KROMBank = nBank;
		SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fdfff, SM_ROM);
	}

	CartRAM[0x1ff0]  = 0xa0;
	CartRAM[0x1ff1] &= 0xfe;
	CartRAM[0x1ff3] &= 0x7f;
}

void __fastcall PVCWriteByteBankSwitch(unsigned int sekAddress, unsigned char byteValue)
{
	CartRAM[(sekAddress & 0x1fff) ^ 1] = byteValue;
	if (sekAddress >= 0x2fffe0 && sekAddress <= 0x2fffe1) NeoPVCPallette01();
	else if(sekAddress >= 0x2fffe8 && sekAddress <= 0x2fffeb) NeoPVCPallette02();
	else if  (sekAddress >= 0x2ffff0 && sekAddress <= 0x2ffff3) NeoPVCBankswitch();
}

void __fastcall PVCWriteWordBankSwitch(unsigned int sekAddress, unsigned short wordValue)
{
	*((unsigned short *)(CartRAM + (sekAddress & 0x1ffe))) = wordValue;
	if (sekAddress >= 0x2fffe0 && sekAddress <= 0x2fffe1) NeoPVCPallette01();
	else if (sekAddress >= 0x2fffe8 && sekAddress <= 0x2fffeb) NeoPVCPallette02();
	else if (sekAddress >= 0x2ffff0 && sekAddress <= 0x2ffff3) NeoPVCBankswitch();
}

void pvcMapBank()
{
	SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fdfff, SM_ROM);
}

static int NeoPVCScan(int nAction, int*)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= CartRAM;
		ba.nLen		= 0x00002000;
		ba.nAddress	= 0;
		ba.szName	= "Neo-PVC 68K RAM";
		BurnAcb(&ba);
	}

	return 0;
}

static int NeoPVCInit()
{
	int nRet;

	memset(CartRAM, 0, 0x2000);

	pNeoBankswitchCallback = pvcMapBank;
	pNeoScanCallback = NeoPVCScan;

	nRet = NeoInit();

	if (nRet == 0) {
		// Install cart PVC protections
		SekMapMemory(CartRAM, 0x2fe000,  0x2fffff,  SM_READ);

		SekMapHandler(5,      0x2fe000,  0x2fffff, SM_WRITE);
		SekSetWriteWordHandler(5,    PVCWriteWordBankSwitch);
		SekSetWriteByteHandler(5,    PVCWriteByteBankSwitch);
	}

	return nRet;
}

// ----------------------------------------------------------------------------
// Extra code for PCM2 protection

struct PCM2DecryptVInfo { int nAddressXor; int nAddressOffset; unsigned char nDataXor[8]; };

static void PCM2DecryptV(PCM2DecryptVInfo* pInfo)
{
	unsigned char* dst = (unsigned char*)malloc(0x01000000);

	if (dst) {
		memcpy (dst, YM2610ADPCMAROM, 0x01000000);

		for (int i = 0; i < 0x01000000; i++) {
			int nAddress = ((i & 0x00FEFFFE) | ((i >> 16) & 1)  | ((i & 1) << 16)) ^ pInfo->nAddressXor;

			YM2610ADPCMAROM[nAddress] = dst[(i + pInfo->nAddressOffset) & 0xffffff] ^ pInfo->nDataXor[nAddress & 0x07];
		}

		free (dst);
	}
}


// ----------------------------------------------------------------------------
// Normal Games


// NAM-1975

static struct BurnRomInfo nam1975RomDesc[] = {
	{ "001-p1.p1",    0x080000, 0xcc9fc951, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "001-s1.s1",    0x020000, 0x7988ba51, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "001-c1.c1",    0x080000, 0x32ea98e1, 3 | BRF_GRA },           //  2 Sprite data
	{ "001-c2.c2",    0x080000, 0xcbc4064c, 3 | BRF_GRA },           //  3 
	{ "001-c3.c3",    0x080000, 0x0151054c, 3 | BRF_GRA },           //  4 
	{ "001-c4.c4",    0x080000, 0x0a32570d, 3 | BRF_GRA },           //  5 
	{ "001-c5.c5",    0x080000, 0x90b74cc2, 3 | BRF_GRA },           //  6 
	{ "001-c6.c6",    0x080000, 0xe62bed58, 3 | BRF_GRA },           //  7 

	{ "001-m1.m1",    0x040000, 0xba874463, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "001-v11.v11",  0x080000, 0xa7c3d5e5, 5 | BRF_SND },           //  9 Sound data
	{ "001-v21.v21",  0x080000, 0x55e670b3, 6 | BRF_SND },           // 10 
	{ "001-v22.v22",  0x080000, 0xab0d8368, 6 | BRF_SND },           // 11 
	{ "001-v23.v23",  0x080000, 0xdf468e28, 6 | BRF_SND },           // 12 
};

STDROMPICKEXT(nam1975, nam1975, neogeo)
STD_ROM_FN(nam1975)

struct BurnDriver BurnDrvnam1975 = {
	"nam1975", NULL, "neogeo", NULL, "1990",
	"NAM-1975\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_MISC, 0,
	NULL, nam1975RomInfo, nam1975RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Baseball Stars Professional

static struct BurnRomInfo bstarsRomDesc[] = {
	{ "002-pg.p1",    0x080000, 0xc100b5f5, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "002-s1.s1",    0x020000, 0x1a7fd0c6, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "002-c1.c1",    0x080000, 0xaaff2a45, 3 | BRF_GRA },           //  2 Sprite data
	{ "002-c2.c2",    0x080000, 0x3ba0f7e4, 3 | BRF_GRA },           //  3 
	{ "002-c3.c3",    0x080000, 0x96f0fdfa, 3 | BRF_GRA },           //  4 
	{ "002-c4.c4",    0x080000, 0x5fd87f2f, 3 | BRF_GRA },           //  5 
	{ "002-c5.c5",    0x080000, 0x807ed83b, 3 | BRF_GRA },           //  6 
	{ "002-c6.c6",    0x080000, 0x5a3cad41, 3 | BRF_GRA },           //  7 

	{ "002-m1.m1",    0x040000, 0x4ecaa4ee, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "002-v11.v11",  0x080000, 0xb7b925bd, 5 | BRF_SND },           //  9 Sound data
	{ "002-v12.v12",  0x080000, 0x329f26fc, 5 | BRF_SND },           // 10 
	{ "002-v13.v13",  0x080000, 0x0c39f3c8, 5 | BRF_SND },           // 11 
	{ "002-v14.v14",  0x080000, 0xc7e11c38, 5 | BRF_SND },           // 12 
	{ "002-v21.v21",  0x080000, 0x04a733d1, 6 | BRF_SND },           // 13 
};

STDROMPICKEXT(bstars, bstars, neogeo)
STD_ROM_FN(bstars)

struct BurnDriver BurnDrvbstars = {
	"bstars", NULL, "neogeo", NULL, "1990",
	"Baseball Stars Professional (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, bstarsRomInfo, bstarsRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


static struct BurnRomInfo bstarshRomDesc[] = {
	{ "002-p1.p1",    0x080000, 0x3bc7790e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "002-s1.s1",    0x020000, 0x1a7fd0c6, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "002-c1.c1",    0x080000, 0xaaff2a45, 3 | BRF_GRA },           //  2 Sprite data
	{ "002-c2.c2",    0x080000, 0x3ba0f7e4, 3 | BRF_GRA },           //  3 
	{ "002-c3.c3",    0x080000, 0x96f0fdfa, 3 | BRF_GRA },           //  4 
	{ "002-c4.c4",    0x080000, 0x5fd87f2f, 3 | BRF_GRA },           //  5 
	{ "002-c5.c5",    0x080000, 0x807ed83b, 3 | BRF_GRA },           //  6 
	{ "002-c6.c6",    0x080000, 0x5a3cad41, 3 | BRF_GRA },           //  7 

	{ "002-m1.m1",    0x040000, 0x4ecaa4ee, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "002-v11.v11",  0x080000, 0xb7b925bd, 5 | BRF_SND },           //  9 Sound data
	{ "002-v12.v12",  0x080000, 0x329f26fc, 5 | BRF_SND },           // 10 
	{ "002-v13.v13",  0x080000, 0x0c39f3c8, 5 | BRF_SND },           // 11 
	{ "002-v14.v14",  0x080000, 0xc7e11c38, 5 | BRF_SND },           // 12 
	{ "002-v21.v21",  0x080000, 0x04a733d1, 6 | BRF_SND },           // 13 
};

STDROMPICKEXT(bstarsh, bstarsh, neogeo)
STD_ROM_FN(bstarsh)

struct BurnDriver BurnDrvbstarsh = {
	"bstarsh", "bstars", "neogeo", NULL, "1990",
	"Baseball Stars Professional (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, bstarshRomInfo, bstarshRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Top Player's Golf

static struct BurnRomInfo tpgolfRomDesc[] = {
	{ "003-p1.p1",    0x080000, 0xf75549ba, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "003-p2.p2",    0x080000, 0xb7809a8f, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "003-s1.s1",    0x020000, 0x7b3eb9b1, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "003-c1.c1",    0x080000, 0x0315fbaf, 3 | BRF_GRA },           //  3 Sprite data
	{ "003-c2.c2",    0x080000, 0xb4c15d59, 3 | BRF_GRA },           //  4 
	{ "003-c3.c3",    0x080000, 0x8ce3e8da, 3 | BRF_GRA },           //  5 
	{ "003-c4.c4",    0x080000, 0x29725969, 3 | BRF_GRA },           //  6 
	{ "003-c5.c5",    0x080000, 0x9a7146da, 3 | BRF_GRA },           //  7 
	{ "003-c6.c6",    0x080000, 0x1e63411a, 3 | BRF_GRA },           //  8 
	{ "003-c7.c7",    0x080000, 0x2886710c, 3 | BRF_GRA },           //  9 
	{ "003-c8.c8",    0x080000, 0x422af22d, 3 | BRF_GRA },           // 10 

	{ "003-m1.m1",    0x020000, 0x4cc545e6, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "003-v11.v11",  0x080000, 0xff97f1cb, 5 | BRF_SND },           // 12 Sound data
	{ "003-v21.v21",  0x080000, 0xd34960c6, 6 | BRF_SND },           // 13 
	{ "003-v22.v22",  0x080000, 0x9a5f58d4, 6 | BRF_SND },           // 14 
	{ "003-v23.v23",  0x080000, 0x30f53e54, 6 | BRF_SND },           // 15 
	{ "003-v24.v24",  0x080000, 0x5ba0f501, 6 | BRF_SND },           // 16 
};

STDROMPICKEXT(tpgolf, tpgolf, neogeo)
STD_ROM_FN(tpgolf)

struct BurnDriver BurnDrvtpgolf = {
	"tpgolf", NULL, "neogeo", NULL, "1990",
	"Top Player's Golf\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, tpgolfRomInfo, tpgolfRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Mahjong Kyoretsuden

static struct BurnRomInfo mahretsuRomDesc[] = {
	{ "004-p1.p1",    0x080000, 0xfc6f53db, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "004-s1.s1",    0x020000, 0x2bd05a06, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "004-c1.c1",    0x080000, 0xf1ae16bc, 3 | BRF_GRA },           //  2 Sprite data
	{ "004-c2.c2",    0x080000, 0xbdc13520, 3 | BRF_GRA },           //  3 
	{ "004-c3.c3",    0x080000, 0x9c571a37, 3 | BRF_GRA },           //  4 
	{ "004-c4.c4",    0x080000, 0x7e81cb29, 3 | BRF_GRA },           //  5 

	{ "004-m1.m1",    0x020000, 0xc71fbb3b, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "004-v11.v11",  0x080000, 0xb2fb2153, 5 | BRF_SND },           //  7 Sound data
	{ "004-v12.v12",  0x080000, 0x8503317b, 5 | BRF_SND },           //  8 
	{ "004-v21.v21",  0x080000, 0x4999fb27, 6 | BRF_SND },           //  9 
	{ "004-v22.v22",  0x080000, 0x776fa2a2, 6 | BRF_SND },           // 10 
	{ "004-v23.v23",  0x080000, 0xb3e7eeea, 6 | BRF_SND },           // 11 
};

STDROMPICKEXT(mahretsu, mahretsu, neogeo)
STD_ROM_FN(mahretsu)

struct BurnDriver BurnDrvmahretsu = {
	"mahretsu", NULL, "neogeo", NULL, "1990",
	"Mahjong Kyoretsuden\0", NULL, "SNK", "Neo Geo",
	L"\u9EBB\u96C0\u72C2\u5217\u4F1D - real mah-jong game\0Mahjong Kyoretsuden\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_MAHJONG, GBF_MAHJONG, 0,
	NULL, mahretsuRomInfo, mahretsuRomName, NULL, NULL, neomahjongInputInfo, neomahjongDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Magician Lord (set 1)

static struct BurnRomInfo maglordRomDesc[] = {
	{ "005-pg1.p1",   0x080000, 0xbd0a492d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "005-s1.s1",    0x020000, 0x1c5369a2, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "005-c1.c1",    0x080000, 0x806aee34, 3 | BRF_GRA },           //  2 Sprite data
	{ "005-c2.c2",    0x080000, 0x34aa9a86, 3 | BRF_GRA },           //  3 
	{ "005-c3.c3",    0x080000, 0xc4c2b926, 3 | BRF_GRA },           //  4 
	{ "005-c4.c4",    0x080000, 0x9c46dcf4, 3 | BRF_GRA },           //  5 
	{ "005-c5.c5",    0x080000, 0x69086dec, 3 | BRF_GRA },           //  6 
	{ "005-c6.c6",    0x080000, 0xab7ac142, 3 | BRF_GRA },           //  7 

	{ "005-m1.m1",    0x040000, 0x26259f0f, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "005-v11.v11",  0x080000, 0xcc0455fd, 5 | BRF_SND },           //  9 Sound data
	{ "005-v21.v21",  0x080000, 0xf94ab5b7, 6 | BRF_SND },           // 10 
	{ "005-v22.v22",  0x080000, 0x232cfd04, 6 | BRF_SND },           // 11 
};

STDROMPICKEXT(maglord, maglord, neogeo)
STD_ROM_FN(maglord)

struct BurnDriver BurnDrvmaglord = {
	"maglord", NULL, "neogeo", NULL, "1990",
	"Magician Lord (set 1)\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, maglordRomInfo, maglordRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Magician Lord (set 2)

static struct BurnRomInfo maglordhRomDesc[] = {
	{ "005-p1.p1",    0x080000, 0x599043c5, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "005-s1.s1",    0x020000, 0x1c5369a2, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "005-c1.c1",    0x080000, 0x806aee34, 3 | BRF_GRA },           //  2 Sprite data
	{ "005-c2.c2",    0x080000, 0x34aa9a86, 3 | BRF_GRA },           //  3 
	{ "005-c3.c3",    0x080000, 0xc4c2b926, 3 | BRF_GRA },           //  4 
	{ "005-c4.c4",    0x080000, 0x9c46dcf4, 3 | BRF_GRA },           //  5 
	{ "005-c5.c5",    0x080000, 0x69086dec, 3 | BRF_GRA },           //  6 
	{ "005-c6.c6",    0x080000, 0xab7ac142, 3 | BRF_GRA },           //  7 

	{ "005-m1.m1",    0x040000, 0x26259f0f, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "005-v11.v11",  0x080000, 0xcc0455fd, 5 | BRF_SND },           //  9 Sound data
	{ "005-v21.v21",  0x080000, 0xf94ab5b7, 6 | BRF_SND },           // 10 
	{ "005-v22.v22",  0x080000, 0x232cfd04, 6 | BRF_SND },           // 11 
};

STDROMPICKEXT(maglordh, maglordh, neogeo)
STD_ROM_FN(maglordh)

struct BurnDriver BurnDrvmaglordh = {
	"maglordh", "maglord", "neogeo", NULL, "1990",
	"Magician Lord (set 2)\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, maglordhRomInfo, maglordhRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Riding Hero (set 1)

static struct BurnRomInfo ridheroRomDesc[] = {
	{ "006-p1.p1",    0x080000, 0xd4aaf597, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "006-s1.s1",    0x020000, 0xeb5189f0, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "006-c1.c1",    0x080000, 0x4a5c7f78, 3 | BRF_GRA },           //  2 Sprite data
	{ "006-c2.c2",    0x080000, 0xe0b70ece, 3 | BRF_GRA },           //  3 
	{ "006-c3.c3",    0x080000, 0x8acff765, 3 | BRF_GRA },           //  4 
	{ "006-c4.c4",    0x080000, 0x205e3208, 3 | BRF_GRA },           //  5 

	{ "006-m1.m1",    0x040000, 0x92e7b4fe, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "006-v11.v11",  0x080000, 0xcdf74a42, 5 | BRF_SND },           //  7 Sound data
	{ "006-v12.v12",  0x080000, 0xe2fd2371, 5 | BRF_SND },           //  8 
	{ "006-v21.v21",  0x080000, 0x94092bce, 6 | BRF_SND },           //  9 
	{ "006-v22.v22",  0x080000, 0x4e2cd7c3, 6 | BRF_SND },           // 10 
	{ "006-v23.v23",  0x080000, 0x069c71ed, 6 | BRF_SND },           // 11 
	{ "006-v24.v24",  0x080000, 0x89fbb825, 6 | BRF_SND },           // 12 
	
	{ "hd6301v1p.com", 0x001000, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STDROMPICKEXT(ridhero, ridhero, neogeo)
STD_ROM_FN(ridhero)

struct BurnDriver BurnDrvridhero = {
	"ridhero", NULL, "neogeo", NULL, "1990",
	"Riding Hero (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_RACING, 0,
	NULL, ridheroRomInfo, ridheroRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Riding Hero (set 2)

static struct BurnRomInfo ridherohRomDesc[] = {
	{ "006-pg1.p1",   0x080000, 0x52445646, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "006-s1.s1",    0x020000, 0xeb5189f0, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "006-c1.c1",    0x080000, 0x4a5c7f78, 3 | BRF_GRA },           //  2 Sprite data
	{ "006-c2.c2",    0x080000, 0xe0b70ece, 3 | BRF_GRA },           //  3 
	{ "006-c3.c3",    0x080000, 0x8acff765, 3 | BRF_GRA },           //  4 
	{ "006-c4.c4",    0x080000, 0x205e3208, 3 | BRF_GRA },           //  5 

	{ "006-m1.m1",    0x040000, 0x92e7b4fe, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "006-v11.v11",  0x080000, 0xcdf74a42, 5 | BRF_SND },           //  7 Sound data
	{ "006-v12.v12",  0x080000, 0xe2fd2371, 5 | BRF_SND },           //  8 
	{ "006-v21.v21",  0x080000, 0x94092bce, 6 | BRF_SND },           //  9 
	{ "006-v22.v22",  0x080000, 0x4e2cd7c3, 6 | BRF_SND },           // 10 
	{ "006-v23.v23",  0x080000, 0x069c71ed, 6 | BRF_SND },           // 11 
	{ "006-v24.v24",  0x080000, 0x89fbb825, 6 | BRF_SND },           // 12 
	
	{ "hd6301v1p.com", 0x001000, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STDROMPICKEXT(ridheroh, ridheroh, neogeo)
STD_ROM_FN(ridheroh)

struct BurnDriver BurnDrvridheroh = {
	"ridheroh", "ridhero", "neogeo", NULL, "1990",
	"Riding Hero (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_RACING, 0,
	NULL, ridherohRomInfo, ridherohRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Alpha Mission II / ASO II - Last Guardian

static struct BurnRomInfo alpham2RomDesc[] = {
	{ "007-p1.p1",    0x080000, 0x5b266f47, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "007-p2.p2",    0x020000, 0xeb9c1044, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "007-s1.s1",    0x020000, 0x85ec9acf, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "007-c1.c1",    0x100000, 0x8fba8ff3, 3 | BRF_GRA },           //  3 Sprite data
	{ "007-c2.c2",    0x100000, 0x4dad2945, 3 | BRF_GRA },           //  4 
	{ "007-c3.c3",    0x080000, 0x68c2994e, 3 | BRF_GRA },           //  5 
	{ "007-c4.c4",    0x080000, 0x7d588349, 3 | BRF_GRA },           //  6 

	{ "007-m1.m1",    0x020000, 0x28dfe2cd, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "007-v1.v1",    0x100000, 0xcd5db931, 5 | BRF_SND },           //  8 Sound data
	{ "007-v2.v2",    0x100000, 0x63e9b574, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(alpham2, alpham2, neogeo)
STD_ROM_FN(alpham2)

struct BurnDriver BurnDrvalpham2 = {
	"alpham2", NULL, "neogeo", NULL, "1991",
	"Alpha Mission II / ASO II - Last Guardian\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, alpham2RomInfo, alpham2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ninja Combat (set 1)

static struct BurnRomInfo ncombatRomDesc[] = {
	{ "009-p1.p1",    0x080000, 0xb45fcfbf, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "009-s1.s1",    0x020000, 0xd49afee8, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "009-c1.c1",    0x080000, 0x33cc838e, 3 | BRF_GRA },           //  2 Sprite data
	{ "009-c2.c2",    0x080000, 0x26877feb, 3 | BRF_GRA },           //  3 
	{ "009-c3.c3",    0x080000, 0x3b60a05d, 3 | BRF_GRA },           //  4 
	{ "009-c4.c4",    0x080000, 0x39c2d039, 3 | BRF_GRA },           //  5 
	{ "009-c5.c5",    0x080000, 0x67a4344e, 3 | BRF_GRA },           //  6 
	{ "009-c6.c6",    0x080000, 0x2eca8b19, 3 | BRF_GRA },           //  7 

	{ "009-m1.m1",    0x020000, 0xb5819863, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "009-v11.v11",  0x080000, 0xcf32a59c, 5 | BRF_SND },           //  9 Sound data
	{ "009-v12.v12",  0x080000, 0x7b3588b7, 5 | BRF_SND },           // 10 
	{ "009-v13.v13",  0x080000, 0x505a01b5, 5 | BRF_SND },           // 11 
	{ "009-v21.v21",  0x080000, 0x365f9011, 6 | BRF_SND },           // 12 
};

STDROMPICKEXT(ncombat, ncombat, neogeo)
STD_ROM_FN(ncombat)

struct BurnDriver BurnDrvncombat = {
	"ncombat", NULL, "neogeo", NULL, "1990",
	"Ninja Combat (set 1)\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, ncombatRomInfo, ncombatRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ninja Combat (set 2)

static struct BurnRomInfo ncombathRomDesc[] = {
	{ "009-pg1.p1",   0x080000, 0x8e9f0add, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "009-s1.s1",    0x020000, 0xd49afee8, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "009-c1.c1",    0x080000, 0x33cc838e, 3 | BRF_GRA },           //  2 Sprite data
	{ "009-c2.c2",    0x080000, 0x26877feb, 3 | BRF_GRA },           //  3 
	{ "009-c3.c3",    0x080000, 0x3b60a05d, 3 | BRF_GRA },           //  4 
	{ "009-c4.c4",    0x080000, 0x39c2d039, 3 | BRF_GRA },           //  5 
	{ "009-c5.c5",    0x080000, 0x67a4344e, 3 | BRF_GRA },           //  6 
	{ "009-c6.c6",    0x080000, 0x2eca8b19, 3 | BRF_GRA },           //  7 

	{ "009-m1.m1",    0x020000, 0xb5819863, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "009-v11.v11",  0x080000, 0xcf32a59c, 5 | BRF_SND },           //  9 Sound data
	{ "009-v12.v12",  0x080000, 0x7b3588b7, 5 | BRF_SND },           // 10 
	{ "009-v13.v13",  0x080000, 0x505a01b5, 5 | BRF_SND },           // 11 
	{ "009-v21.v21",  0x080000, 0x365f9011, 6 | BRF_SND },           // 12 
};

STDROMPICKEXT(ncombath, ncombath, neogeo)
STD_ROM_FN(ncombath)

struct BurnDriver BurnDrvncombath = {
	"ncombath", "ncombat", "neogeo", NULL, "1990",
	"Ninja Combat (set 2)\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, ncombathRomInfo, ncombathRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Cyber-Lip

static struct BurnRomInfo cyberlipRomDesc[] = {
	{ "010-p1.p1",    0x080000, 0x69a6b42d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "010-s1.s1",    0x020000, 0x79a35264, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "010-c1.c1",    0x080000, 0x8bba5113, 3 | BRF_GRA },           //  2 Sprite data
	{ "010-c2.c2",    0x080000, 0xcbf66432, 3 | BRF_GRA },           //  3 
	{ "010-c3.c3",    0x080000, 0xe4f86efc, 3 | BRF_GRA },           //  4 
	{ "010-c4.c4",    0x080000, 0xf7be4674, 3 | BRF_GRA },           //  5 
	{ "010-c5.c5",    0x080000, 0xe8076da0, 3 | BRF_GRA },           //  6 
	{ "010-c6.c6",    0x080000, 0xc495c567, 3 | BRF_GRA },           //  7 

	{ "010-m1.m1",    0x020000, 0x8be3a078, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "010-v11.v11",  0x080000, 0x90224d22, 5 | BRF_SND },           //  9 Sound data
	{ "010-v12.v12",  0x080000, 0xa0cf1834, 5 | BRF_SND },           // 10 
	{ "010-v13.v13",  0x080000, 0xae38bc84, 5 | BRF_SND },           // 11 
	{ "010-v14.v14",  0x080000, 0x70899bd2, 5 | BRF_SND },           // 12 
	{ "010-v21.v21",  0x080000, 0x586f4cb2, 6 | BRF_SND },           // 13 
};

STDROMPICKEXT(cyberlip, cyberlip, neogeo)
STD_ROM_FN(cyberlip)

struct BurnDriver BurnDrvcyberlip = {
	"cyberlip", NULL, "neogeo", NULL, "1990",
	"Cyber-Lip\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, cyberlipRomInfo, cyberlipRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The Super Spy

static struct BurnRomInfo superspyRomDesc[] = {
	{ "011-p1.p1",    0x080000, 0xc7f944b5, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "sp2.p2",       0x020000, 0x811a4faf, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "011-s1.s1",    0x020000, 0xec5fdb96, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "011-c1.c1",    0x100000, 0xcae7be57, 3 | BRF_GRA },           //  3 Sprite data
	{ "011-c2.c2",    0x100000, 0x9e29d986, 3 | BRF_GRA },           //  4 
	{ "011-c3.c3",    0x100000, 0x14832ff2, 3 | BRF_GRA },           //  5 
	{ "011-c4.c4",    0x100000, 0xb7f63162, 3 | BRF_GRA },           //  6 

	{ "011-m1.m1",    0x040000, 0xca661f1b, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "011-v11.v11",  0x100000, 0x5c674d5c, 5 | BRF_SND },           //  8 Sound data
	{ "011-v12.v12",  0x080000, 0x9f513d5a, 5 | BRF_SND },           //  9 
	{ "011-v21.v21",  0x080000, 0x426cd040, 6 | BRF_SND },           // 10 
};

STDROMPICKEXT(superspy, superspy, neogeo)
STD_ROM_FN(superspy)

struct BurnDriver BurnDrvsuperspy = {
	"superspy", NULL, "neogeo", NULL, "1990",
	"The Super Spy\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_MISC, 0,
	NULL, superspyRomInfo, superspyRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Mutation Nation

static struct BurnRomInfo mutnatRomDesc[] = {
	{ "014-p1.p1",    0x080000, 0x6f1699c8, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "014-s1.s1",    0x020000, 0x99419733, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "014-c1.c1",    0x100000, 0x5e4381bf, 3 | BRF_GRA },           //  2 Sprite data
	{ "014-c2.c2",    0x100000, 0x69ba4e18, 3 | BRF_GRA },           //  3 
	{ "014-c3.c3",    0x100000, 0x890327d5, 3 | BRF_GRA },           //  4 
	{ "014-c4.c4",    0x100000, 0xe4002651, 3 | BRF_GRA },           //  5 

	{ "014-m1.m1",    0x020000, 0xb6683092, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "014-v1.v1",    0x100000, 0x25419296, 5 | BRF_SND },           //  7 Sound data
	{ "014-v2.v2",    0x100000, 0x0de53d5e, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(mutnat, mutnat, neogeo)
STD_ROM_FN(mutnat)

struct BurnDriver BurnDrvmutnat = {
	"mutnat", NULL, "neogeo", NULL, "1992",
	"Mutation Nation\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, mutnatRomInfo, mutnatRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// King of the Monsters (set 1)

static struct BurnRomInfo kotmRomDesc[] = {
	{ "016-p1.p1",    0x080000, 0x1b818731, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "016-p2.p2",    0x020000, 0x12afdc2b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "016-s1.s1",    0x020000, 0x1a2eeeb3, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "016-c1.c1",    0x100000, 0x71471c25, 3 | BRF_GRA },           //  3 Sprite data
	{ "016-c2.c2",    0x100000, 0x320db048, 3 | BRF_GRA },           //  4 
	{ "016-c3.c3",    0x100000, 0x98de7995, 3 | BRF_GRA },           //  5 
	{ "016-c4.c4",    0x100000, 0x070506e2, 3 | BRF_GRA },           //  6 

	{ "016-m1.m1",    0x020000, 0x9da9ca10, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "016-v1.v1",    0x100000, 0x86c0a502, 5 | BRF_SND },           //  8 Sound data
	{ "016-v2.v2",    0x100000, 0x5bc23ec5, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(kotm, kotm, neogeo)
STD_ROM_FN(kotm)

struct BurnDriver BurnDrvkotm = {
	"kotm", NULL, "neogeo", NULL, "1991",
	"King of the Monsters (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, kotmRomInfo, kotmRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// King of the Monsters (set 2)

static struct BurnRomInfo kotmhRomDesc[] = {
	{ "016-hp1.p1",   0x080000, 0xb774621e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "016-p2.p2",    0x020000, 0x12afdc2b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "016-s1.s1",    0x020000, 0x1a2eeeb3, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "016-c1.c1",    0x100000, 0x71471c25, 3 | BRF_GRA },           //  3 Sprite data
	{ "016-c2.c2",    0x100000, 0x320db048, 3 | BRF_GRA },           //  4 
	{ "016-c3.c3",    0x100000, 0x98de7995, 3 | BRF_GRA },           //  5 
	{ "016-c4.c4",    0x100000, 0x070506e2, 3 | BRF_GRA },           //  6 

	{ "016-m1.m1",    0x020000, 0x9da9ca10, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "016-v1.v1",    0x100000, 0x86c0a502, 5 | BRF_SND },           //  8 Sound data
	{ "016-v2.v2",    0x100000, 0x5bc23ec5, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(kotmh, kotmh, neogeo)
STD_ROM_FN(kotmh)

struct BurnDriver BurnDrvkotmh = {
	"kotmh", "kotm", "neogeo", NULL, "1991",
	"King of the Monsters (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, kotmhRomInfo, kotmhRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Sengoku / Sengoku Denshou (set 1)

static struct BurnRomInfo sengokuRomDesc[] = {
	{ "017-p1.p1",    0x080000, 0xf8a63983, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "017-p2.p2",    0x020000, 0x3024bbb3, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "017-s1.s1",    0x020000, 0xb246204d, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "017-c1.c1",    0x100000, 0xb4eb82a1, 3 | BRF_GRA },           //  3 Sprite data
	{ "017-c2.c2",    0x100000, 0xd55c550d, 3 | BRF_GRA },           //  4 
	{ "017-c3.c3",    0x100000, 0xed51ef65, 3 | BRF_GRA },           //  5 
	{ "017-c4.c4",    0x100000, 0xf4f3c9cb, 3 | BRF_GRA },           //  6 

	{ "017-m1.m1",    0x020000, 0x9b4f34c6, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "017-v1.v1",    0x100000, 0x23663295, 5 | BRF_SND },           //  8 Sound data
	{ "017-v2.v2",    0x100000, 0xf61e6765, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(sengoku, sengoku, neogeo)
STD_ROM_FN(sengoku)

struct BurnDriver BurnDrvsengoku = {
	"sengoku", NULL, "neogeo", NULL, "1991",
	"Sengoku / Sengoku Denshou (set 1)\0", NULL, "SNK", "Neo Geo",
	L"Sengoku (set 1)\0\u6226\u56FD\u4F1D\u627F (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, sengokuRomInfo, sengokuRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Sengoku / Sengoku Denshou (set 2)

static struct BurnRomInfo sengokuhRomDesc[] = {
	{ "017-hp1.p1",   0x080000, 0x33eccae0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "017-p2.p2",    0x020000, 0x3024bbb3, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "017-s1.s1",    0x020000, 0xb246204d, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "017-c1.c1",    0x100000, 0xb4eb82a1, 3 | BRF_GRA },           //  3 Sprite data
	{ "017-c2.c2",    0x100000, 0xd55c550d, 3 | BRF_GRA },           //  4 
	{ "017-c3.c3",    0x100000, 0xed51ef65, 3 | BRF_GRA },           //  5 
	{ "017-c4.c4",    0x100000, 0xf4f3c9cb, 3 | BRF_GRA },           //  6 

	{ "017-m1.m1",    0x020000, 0x9b4f34c6, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "017-v1.v1",    0x100000, 0x23663295, 5 | BRF_SND },           //  8 Sound data
	{ "017-v2.v2",    0x100000, 0xf61e6765, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(sengokuh, sengokuh, neogeo)
STD_ROM_FN(sengokuh)

struct BurnDriver BurnDrvsengokuh = {
	"sengokuh", "sengoku", "neogeo", NULL, "1991",
	"Sengoku / Sengoku Denshou (set 2)\0", NULL, "SNK", "Neo Geo",
	L"Sengoku (set 2)\0\u6226\u56FD\u4F1D\u627F (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, sengokuhRomInfo, sengokuhRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Burning Fight (set 1)

static struct BurnRomInfo burningfRomDesc[] = {
	{ "018-p1.p1",    0x080000, 0x4092c8db, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "018-s1.s1",    0x020000, 0x6799ea0d, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "018-c1.c1",    0x100000, 0x25a25e9b, 3 | BRF_GRA },           //  2 Sprite data
	{ "018-c2.c2",    0x100000, 0xd4378876, 3 | BRF_GRA },           //  3 
	{ "018-c3.c3",    0x100000, 0x862b60da, 3 | BRF_GRA },           //  4 
	{ "018-c4.c4",    0x100000, 0xe2e0aff7, 3 | BRF_GRA },           //  5 

	{ "018-m1.m1",    0x020000, 0x0c939ee2, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "018-v1.v1",    0x100000, 0x508c9ffc, 5 | BRF_SND },           //  7 Sound data
	{ "018-v2.v2",    0x100000, 0x854ef277, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(burningf, burningf, neogeo)
STD_ROM_FN(burningf)

struct BurnDriver BurnDrvburningf = {
	"burningf", NULL, "neogeo", NULL, "1991",
	"Burning Fight (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, burningfRomInfo, burningfRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Burning Fight (set 2)

static struct BurnRomInfo burningfhRomDesc[] = {
	{ "018-hp1.p1",   0x080000, 0xddffcbf4, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "018-s1.s1",    0x020000, 0x6799ea0d, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "018-c1.c1",    0x100000, 0x25a25e9b, 3 | BRF_GRA },           //  2 Sprite data
	{ "018-c2.c2",    0x100000, 0xd4378876, 3 | BRF_GRA },           //  3 
	{ "018-c3.c3",    0x100000, 0x862b60da, 3 | BRF_GRA },           //  4 
	{ "018-c4.c4",    0x100000, 0xe2e0aff7, 3 | BRF_GRA },           //  5 

	{ "018-m1.m1",    0x020000, 0x0c939ee2, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "018-v1.v1",    0x100000, 0x508c9ffc, 5 | BRF_SND },           //  7 Sound data
	{ "018-v2.v2",    0x100000, 0x854ef277, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(burningfh, burningfh, neogeo)
STD_ROM_FN(burningfh)

struct BurnDriver BurnDrvburningfh = {
	"burningfh", "burningf", "neogeo", NULL, "1991",
	"Burning Fight (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, burningfhRomInfo, burningfhRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// League Bowling

static struct BurnRomInfo lbowlingRomDesc[] = {
	{ "019-p1.p1",    0x080000, 0xa2de8445, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "019-s1.s1",    0x020000, 0x5fcdc0ed, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "019-c1.c1",    0x080000, 0x4ccdef18, 3 | BRF_GRA },           //  2 Sprite data
	{ "019-c2.c2",    0x080000, 0xd4dd0802, 3 | BRF_GRA },           //  3 

	{ "019-m1.m1",    0x020000, 0xd568c17d, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "019-v11.v11",  0x080000, 0x0fb74872, 5 | BRF_SND },           //  5 Sound data
	{ "019-v12.v12",  0x080000, 0x029faa57, 5 | BRF_SND },           //  6 
	{ "019-v21.v21",  0x080000, 0x2efd5ada, 6 | BRF_SND },           //  7 
	
	{ "hd6301v1p.com", 0x001000, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STDROMPICKEXT(lbowling, lbowling, neogeo)
STD_ROM_FN(lbowling)

struct BurnDriver BurnDrvlbowling = {
	"lbowling", NULL, "neogeo", NULL, "1990",
	"League Bowling\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, lbowlingRomInfo, lbowlingRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ghost Pilots (set 1)

static struct BurnRomInfo gpilotsRomDesc[] = {
	{ "020-p1.p1",    0x080000, 0xe6f2fe64, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "020-p2.p2",    0x020000, 0xedcb22ac, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "020-s1.s1",    0x020000, 0xa6d83d53, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "020-c1.c1",    0x100000, 0xbd6fe78e, 3 | BRF_GRA },           //  3 Sprite data
	{ "020-c2.c2",    0x100000, 0x5f4a925c, 3 | BRF_GRA },           //  4 
	{ "020-c3.c3",    0x100000, 0xd1e42fd0, 3 | BRF_GRA },           //  5 
	{ "020-c4.c4",    0x100000, 0xedde439b, 3 | BRF_GRA },           //  6 

	{ "020-m1.m1",    0x020000, 0x48409377, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "020-v11.v11",  0x100000, 0x1b526c8b, 5 | BRF_SND },           //  8 Sound data
	{ "020-v12.v12",  0x080000, 0x4a9e6f03, 5 | BRF_SND },           //  9 
	{ "020-v21.v21",  0x080000, 0x7abf113d, 6 | BRF_SND },           // 10 
};

STDROMPICKEXT(gpilots, gpilots, neogeo)
STD_ROM_FN(gpilots)

struct BurnDriver BurnDrvgpilots = {
	"gpilots", NULL, "neogeo", NULL, "1991",
	"Ghost Pilots (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, gpilotsRomInfo, gpilotsRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ghost Pilots (set 2)

static struct BurnRomInfo gpilotshRomDesc[] = {
	{ "020-hp1.p1",   0x080000, 0x7cdb01ce, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "020-p2.p2",    0x020000, 0xedcb22ac, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "020-s1.s1",    0x020000, 0xa6d83d53, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "020-c1.c1",    0x100000, 0xbd6fe78e, 3 | BRF_GRA },           //  3 Sprite data
	{ "020-c2.c2",    0x100000, 0x5f4a925c, 3 | BRF_GRA },           //  4 
	{ "020-c3.c3",    0x100000, 0xd1e42fd0, 3 | BRF_GRA },           //  5 
	{ "020-c4.c4",    0x100000, 0xedde439b, 3 | BRF_GRA },           //  6 

	{ "020-m1.m1",    0x020000, 0x48409377, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "020-v11.v11",  0x100000, 0x1b526c8b, 5 | BRF_SND },           //  8 Sound data
	{ "020-v12.v12",  0x080000, 0x4a9e6f03, 5 | BRF_SND },           //  9 
	{ "020-v21.v21",  0x080000, 0x7abf113d, 6 | BRF_SND },           // 10 
};

STDROMPICKEXT(gpilotsh, gpilotsh, neogeo)
STD_ROM_FN(gpilotsh)

struct BurnDriver BurnDrvgpilotsh = {
	"gpilotsh", "gpilots", "neogeo", NULL, "1991",
	"Ghost Pilots (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, gpilotshRomInfo, gpilotshRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Puzzled / Joy Joy Kid

static struct BurnRomInfo joyjoyRomDesc[] = {
	{ "021-p1.p1",    0x080000, 0x39c3478f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "021-s1.s1",    0x020000, 0x6956d778, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "021-c1.c1",    0x080000, 0x509250ec, 3 | BRF_GRA },           //  2 Sprite data
	{ "021-c2.c2",    0x080000, 0x09ed5258, 3 | BRF_GRA },           //  3 

	{ "021-m1.m1",    0x040000, 0x5a4be5e8, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "021-v11.v11",  0x080000, 0x66c1e5c4, 5 | BRF_SND },           //  5 Sound data
	{ "021-v21.v21",  0x080000, 0x8ed20a86, 6 | BRF_SND },           //  6 
};

STDROMPICKEXT(joyjoy, joyjoy, neogeo)
STD_ROM_FN(joyjoy)

struct BurnDriver BurnDrvjoyjoy = {
	"joyjoy", NULL, "neogeo", NULL, "1990",
	"Puzzled / Joy Joy Kid\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, joyjoyRomInfo, joyjoyRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Blue's Journey / Raguy

static struct BurnRomInfo bjourneyRomDesc[] = {
	{ "022-p1.p1",    0x100000, 0x6a2f6d4a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "022-s1.s1",    0x020000, 0x843c3624, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "022-c1.c1",    0x100000, 0x4d47a48c, 3 | BRF_GRA },           //  2 Sprite data
	{ "022-c2.c2",    0x100000, 0xe8c1491a, 3 | BRF_GRA },           //  3 
	{ "022-c3.c3",    0x080000, 0x66e69753, 3 | BRF_GRA },           //  4 
	{ "022-c4.c4",    0x080000, 0x71bfd48a, 3 | BRF_GRA },           //  5 

	{ "022-m1.m1",    0x020000, 0x8e1d4ab6, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "022-v11.v11",  0x100000, 0x2cb4ad91, 5 | BRF_SND },           //  7 Sound data
	{ "022-v22.v22",  0x100000, 0x65a54d13, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(bjourney, bjourney, neogeo)
STD_ROM_FN(bjourney)

struct BurnDriver BurnDrvbjourney = {
	"bjourney", NULL, "neogeo", NULL, "1990",
	"Blue's Journey / Raguy\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, bjourneyRomInfo, bjourneyRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Quiz Daisousa Sen - The Last Count Down

static struct BurnRomInfo quizdaisRomDesc[] = {
	{ "023-p1.p1",    0x100000, 0xc488fda3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "023-s1.s1",    0x020000, 0xac31818a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "023-c1.c1",    0x100000, 0x2999535a, 3 | BRF_GRA },           //  2 Sprite data
	{ "023-c2.c2",    0x100000, 0x876a99e6, 3 | BRF_GRA },           //  3 

	{ "023-m1.m1",    0x020000, 0x2a2105e0, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "023-v1.v1",    0x100000, 0xa53e5bd3, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(quizdais, quizdais, neogeo)
STD_ROM_FN(quizdais)

struct BurnDriver BurnDrvquizdais = {
	"quizdais", NULL, "neogeo", NULL, "1991",
	"Quiz Daisousa Sen - The Last Count Down\0", NULL, "SNK", "Neo Geo",
	L"\u30AF\u30A4\u30BA\u5927\u635C\u67FB\u7DDA - the last count down\0Quiz Daisousa Sen\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_QUIZ, 0,
	NULL, quizdaisRomInfo, quizdaisRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Quiz Daisousa Sen - The Last Count Down (Korea)

static struct BurnRomInfo quizdaiskRomDesc[] = {
	{ "123-p1.p1",    0x100000, 0xa6f35eae, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "123-s1.s1",    0x020000, 0x53de938a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "123-c1.c1",    0x100000, 0xe579a606, 3 | BRF_GRA },           //  2 Sprite data
	{ "123-c2.c2",    0x100000, 0xe182c837, 3 | BRF_GRA },           //  3 
	
	{ "123-m1.m1",    0x020000, 0xd67f53f9, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "123-v1.v1",    0x200000, 0x986f4af9, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(quizdaisk, quizdaisk, neogeo)
STD_ROM_FN(quizdaisk)

struct BurnDriver BurnDrvquizdaisk = {
	"quizdaisk", "quizdais", "neogeo", NULL, "1991",
	"Quiz Daisousa Sen - The Last Count Down (Korea)\0", NULL, "SNK", "Neo Geo",
	L"\u30AF\u30A4\u30BA\u5927\u635C\u67FB\u7DDA - the last count down (Korea)\0Quiz Daisousa Sen (Korea)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_QUIZ, 0,
	NULL, quizdaiskRomInfo, quizdaiskRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Last Resort

static struct BurnRomInfo lresortRomDesc[] = {
	{ "024-p1.p1",    0x080000, 0x89c4ab97, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "024-s1.s1",    0x020000, 0x5cef5cc6, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "024-c1.c1",    0x100000, 0x3617c2dc, 3 | BRF_GRA },           //  2 Sprite data
	{ "024-c2.c2",    0x100000, 0x3f0a7fd8, 3 | BRF_GRA },           //  3 
	{ "024-c3.c3",    0x080000, 0xe9f745f8, 3 | BRF_GRA },           //  4 
	{ "024-c4.c4",    0x080000, 0x7382fefb, 3 | BRF_GRA },           //  5 

	{ "024-m1.m1",    0x020000, 0xcec19742, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "024-v1.v1",    0x100000, 0xefdfa063, 5 | BRF_SND },           //  7 Sound data
	{ "024-v2.v2",    0x100000, 0x3c7997c0, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(lresort, lresort, neogeo)
STD_ROM_FN(lresort)

struct BurnDriver BurnDrvlresort = {
	"lresort", NULL, "neogeo", NULL, "1992",
	"Last Resort\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_HORSHOOT, 0,
	NULL, lresortRomInfo, lresortRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Eight Man

static struct BurnRomInfo eightmanRomDesc[] = {
	{ "025-p1.p1",    0x080000, 0x43344cb0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "025-s1.s1",    0x020000, 0xa402202b, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "025-c1.c1",    0x100000, 0x555e16a4, 3 | BRF_GRA },           //  2 Sprite data
	{ "025-c2.c2",    0x100000, 0xe1ee51c3, 3 | BRF_GRA },           //  3 
	{ "025-c3.c3",    0x080000, 0x0923d5b0, 3 | BRF_GRA },           //  4 
	{ "025-c4.c4",    0x080000, 0xe3eca67b, 3 | BRF_GRA },           //  5 

	{ "025-m1.m1",    0x020000, 0x9927034c, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "025-v1.v1",    0x100000, 0x4558558a, 5 | BRF_SND },           //  7 Sound data
	{ "025-v2.v2",    0x100000, 0xc5e052e9, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(eightman, eightman, neogeo)
STD_ROM_FN(eightman)

struct BurnDriver BurnDrveightman = {
	"eightman", NULL, "neogeo", NULL, "1991",
	"Eight Man\0", NULL, "SNK / Pallas", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, eightmanRomInfo, eightmanRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Minnasanno Okagesamadesu

static struct BurnRomInfo minasanRomDesc[] = {
	{ "027-p1.p1",    0x080000, 0xc8381327, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "027-s1.s1",    0x020000, 0xe5824baa, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "027-c1.c1",    0x100000, 0xd0086f94, 3 | BRF_GRA },           //  2 Sprite data
	{ "027-c2.c2",    0x100000, 0xda61f5a6, 3 | BRF_GRA },           //  3 
	{ "027-c3.c3",    0x100000, 0x08df1228, 3 | BRF_GRA },           //  4 
	{ "027-c4.c4",    0x100000, 0x54e87696, 3 | BRF_GRA },           //  5 

	{ "027-m1.m1",    0x020000, 0xadd5a226, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "027-v11.v11",  0x100000, 0x59ad4459, 5 | BRF_SND },           //  7 Sound data
	{ "027-v21.v21",  0x100000, 0xdf5b4eeb, 6 | BRF_SND },           //  8 
};

STDROMPICKEXT(minasan, minasan, neogeo)
STD_ROM_FN(minasan)

struct BurnDriver BurnDrvminasan = {
	"minasan", NULL, "neogeo", NULL, "1990",
	"Minnasanno Okagesamadesu\0", NULL, "Monolith Corp.", "Neo Geo",
	L"\u307F\u306A\u3055\u3093\u306E\u304A\u304B\u3052\u3055\u307E\u3067\u3059\uFF01\0Minnasanno Okagesamadesu\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_MAHJONG, GBF_MAHJONG, 0,
	NULL, minasanRomInfo, minasanRomName, NULL, NULL, neomahjongInputInfo, neomahjongDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Legend of Success Joe / Ashitano Joe Densetsu

static struct BurnRomInfo legendosRomDesc[] = {
	{ "029-p1.p1",    0x080000, 0x9d563f19, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "029-s1.s1",    0x020000, 0xbcd502f0, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "029-c1.c1",    0x100000, 0x2f5ab875, 3 | BRF_GRA },           //  2 Sprite data
	{ "029-c2.c2",    0x100000, 0x318b2711, 3 | BRF_GRA },           //  3 
	{ "029-c3.c3",    0x100000, 0x6bc52cb2, 3 | BRF_GRA },           //  4 
	{ "029-c4.c4",    0x100000, 0x37ef298c, 3 | BRF_GRA },           //  5 

	{ "029-m1.m1",    0x020000, 0x6f2843f0, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "029-v1.v1",    0x100000, 0x85065452, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(legendos, legendos, neogeo)
STD_ROM_FN(legendos)

struct BurnDriver BurnDrvlegendos = {
	"legendos", NULL, "neogeo", NULL, "1991",
	"Legend of Success Joe / Ashitano Joe Densetsu\0", NULL, "SNK", "Neo Geo",
	L"Legend of Success Joe\0\u3042\u3057\u305F\u306E\u30B8\u30E7\u30FC\u4F1D\u8AAC\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, legendosRomInfo, legendosRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// 2020 Super Baseball (set 1)

static struct BurnRomInfo bb2020RomDesc[] = {
	{ "030-p1.p1",    0x080000, 0xd396c9cb, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "030-s1.s1",    0x020000, 0x7015b8fc, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "030-c1.c1",    0x100000, 0x4f5e19bd, 3 | BRF_GRA },           //  2 Sprite data
	{ "030-c2.c2",    0x100000, 0xd6314bf0, 3 | BRF_GRA },           //  3 
	{ "030-c3.c3",    0x100000, 0x47fddfee, 3 | BRF_GRA },           //  4 
	{ "030-c4.c4",    0x100000, 0x780d1c4e, 3 | BRF_GRA },           //  5 

	{ "030-m1.m1",    0x020000, 0x4cf466ec, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "030-v1.v1",    0x100000, 0xd4ca364e, 5 | BRF_SND },           //  7 Sound data
	{ "030-v2.v2",    0x100000, 0x54994455, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(bb2020, bb2020, neogeo)
STD_ROM_FN(bb2020)

struct BurnDriver BurnDrvbb2020 = {
	"2020bb", NULL, "neogeo", NULL, "1991",
	"2020 Super Baseball (set 1)\0", NULL, "SNK / Pallas", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, bb2020RomInfo, bb2020RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// 2020 Super Baseball (set 2)

static struct BurnRomInfo bb2020aRomDesc[] = {
	{ "030-epr.p1",   0x080000, 0xc59be3dd, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "030-s1.s1",    0x020000, 0x7015b8fc, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "030-c1.c1",    0x100000, 0x4f5e19bd, 3 | BRF_GRA },           //  2 Sprite data
	{ "030-c2.c2",    0x100000, 0xd6314bf0, 3 | BRF_GRA },           //  3 
	{ "030-c3.c3",    0x100000, 0x47fddfee, 3 | BRF_GRA },           //  4 
	{ "030-c4.c4",    0x100000, 0x780d1c4e, 3 | BRF_GRA },           //  5 

	{ "030-m1.m1",    0x020000, 0x4cf466ec, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "030-v1.v1",    0x100000, 0xd4ca364e, 5 | BRF_SND },           //  7 Sound data
	{ "030-v2.v2",    0x100000, 0x54994455, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(bb2020a, bb2020a, neogeo)
STD_ROM_FN(bb2020a)

struct BurnDriver BurnDrvbb2020a = {
	"2020bba", "2020bb", "neogeo", NULL, "1991",
	"2020 Super Baseball (set 2)\0", NULL, "SNK / Pallas", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, bb2020aRomInfo, bb2020aRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// 2020 Super Baseball (set 3)

static struct BurnRomInfo bb2020hRomDesc[] = {
	{ "030-pg1.p1",   0x080000, 0x12d048d7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "030-s1.s1",    0x020000, 0x7015b8fc, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "030-c1.c1",    0x100000, 0x4f5e19bd, 3 | BRF_GRA },           //  2 Sprite data
	{ "030-c2.c2",    0x100000, 0xd6314bf0, 3 | BRF_GRA },           //  3 
	{ "030-c3.c3",    0x100000, 0x47fddfee, 3 | BRF_GRA },           //  4 
	{ "030-c4.c4",    0x100000, 0x780d1c4e, 3 | BRF_GRA },           //  5 

	{ "030-m1.m1",    0x020000, 0x4cf466ec, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "030-v1.v1",    0x100000, 0xd4ca364e, 5 | BRF_SND },           //  7 Sound data
	{ "030-v2.v2",    0x100000, 0x54994455, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(bb2020h, bb2020h, neogeo)
STD_ROM_FN(bb2020h)

struct BurnDriver BurnDrvbb2020h = {
	"2020bbh", "2020bb", "neogeo", NULL, "1991",
	"2020 Super Baseball (set 3)\0", NULL, "SNK / Pallas", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, bb2020hRomInfo, bb2020hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Soccer Brawl (set 2)

static struct BurnRomInfo socbrawlhRomDesc[] = {
	{ "031-p1.p1",    0x080000, 0xa2801c24, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "031-s1.s1",    0x020000, 0x4c117174, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "031-c1.c1",    0x100000, 0xbd0a4eb8, 3 | BRF_GRA },           //  2 Sprite data
	{ "031-c2.c2",    0x100000, 0xefde5382, 3 | BRF_GRA },           //  3 
	{ "031-c3.c3",    0x080000, 0x580f7f33, 3 | BRF_GRA },           //  4 
	{ "031-c4.c4",    0x080000, 0xed297de8, 3 | BRF_GRA },           //  5 

	{ "031-m1.m1",    0x020000, 0xcb37427c, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "031-v1.v1",    0x100000, 0xcc78497e, 5 | BRF_SND },           //  7 Sound data
	{ "031-v2.v2",    0x100000, 0xdda043c6, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(socbrawlh, socbrawlh, neogeo)
STD_ROM_FN(socbrawlh)

struct BurnDriver BurnDrvsocbrawlh = {
	"socbrawlh", "socbrawl", "neogeo", NULL, "1991",
	"Soccer Brawl (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSFOOTBALL, 0,
	NULL, socbrawlhRomInfo, socbrawlhRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Soccer Brawl (set 1)

static struct BurnRomInfo socbrawlRomDesc[] = {
	{ "031-pg1.p1",   0x080000, 0x17f034a7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "031-s1.s1",    0x020000, 0x4c117174, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "031-c1.c1",    0x100000, 0xbd0a4eb8, 3 | BRF_GRA },           //  2 Sprite data
	{ "031-c2.c2",    0x100000, 0xefde5382, 3 | BRF_GRA },           //  3 
	{ "031-c3.c3",    0x080000, 0x580f7f33, 3 | BRF_GRA },           //  4 
	{ "031-c4.c4",    0x080000, 0xed297de8, 3 | BRF_GRA },           //  5 

	{ "031-m1.m1",    0x020000, 0xcb37427c, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "031-v1.v1",    0x100000, 0xcc78497e, 5 | BRF_SND },           //  7 Sound data
	{ "031-v2.v2",    0x100000, 0xdda043c6, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(socbrawl, socbrawl, neogeo)
STD_ROM_FN(socbrawl)

struct BurnDriver BurnDrvsocbrawla = {
	"socbrawl", NULL, "neogeo", NULL, "1991",
	"Soccer Brawl (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSFOOTBALL, 0,
	NULL, socbrawlRomInfo, socbrawlRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Robo Army

static struct BurnRomInfo roboarmyRomDesc[] = {
	{ "032-p1.p1",    0x080000, 0xcd11cbd4, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "032-s1.s1",    0x020000, 0xac0daa1b, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "032-c1.c1",    0x100000, 0x97984c6c, 3 | BRF_GRA },           //  2 Sprite data
	{ "032-c2.c2",    0x100000, 0x65773122, 3 | BRF_GRA },           //  3 
	{ "032-c3.c3",    0x080000, 0x40adfccd, 3 | BRF_GRA },           //  4 
	{ "032-c4.c4",    0x080000, 0x462571de, 3 | BRF_GRA },           //  5 

	{ "032-m1.m1",    0x020000, 0x35ec952d, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "032-v1.v1",    0x100000, 0x63791533, 5 | BRF_SND },           //  7 Sound data
	{ "032-v2.v2",    0x100000, 0xeb95de70, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(roboarmy, roboarmy, neogeo)
STD_ROM_FN(roboarmy)

struct BurnDriver BurnDrvroboarmy = {
	"roboarmy", NULL, "neogeo", NULL, "1991",
	"Robo Army\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, roboarmyRomInfo, roboarmyRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Fatal Fury - King of Fighters / Garou Densetsu - shukumei no tatakai

static struct BurnRomInfo fatfury1RomDesc[] = {
	{ "033-p1.p1",    0x080000, 0x47ebdc2f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "033-p2.p2",    0x020000, 0xc473af1c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "033-s1.s1",    0x020000, 0x3c3bdf8c, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "033-c1.c1",    0x100000, 0x74317e54, 3 | BRF_GRA },           //  3 Sprite data
	{ "033-c2.c2",    0x100000, 0x5bb952f3, 3 | BRF_GRA },           //  4 
	{ "033-c3.c3",    0x100000, 0x9b714a7c, 3 | BRF_GRA },           //  5 
	{ "033-c4.c4",    0x100000, 0x9397476a, 3 | BRF_GRA },           //  6 

	{ "033-m1.m1",    0x020000, 0x5be10ffd, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "033-v1.v1",    0x100000, 0x212fd20d, 5 | BRF_SND },           //  8 Sound data
	{ "033-v2.v2",    0x100000, 0xfa2ae47f, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(fatfury1, fatfury1, neogeo)
STD_ROM_FN(fatfury1)

struct BurnDriver BurnDrvfatfury1 = {
	"fatfury1", NULL, "neogeo", NULL, "1991",
	"Fatal Fury - King of Fighters / Garou Densetsu - shukumei no tatakai\0", NULL, "SNK", "Neo Geo",
	L"Fatal Fury - king of fighters\0\u9913\u72FC\u4F1D\u8AAC - \u5BBF\u547D\u306E\u95D8\u3044\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, fatfury1RomInfo, fatfury1RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Football Frenzy

static struct BurnRomInfo fbfrenzyRomDesc[] = {
	{ "034-p1.p1",    0x080000, 0xcdef6b19, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "034-s1.s1",    0x020000, 0x8472ed44, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "034-c1.c1",    0x100000, 0x91c56e78, 3 | BRF_GRA },           //  2 Sprite data
	{ "034-c2.c2",    0x100000, 0x9743ea2f, 3 | BRF_GRA },           //  3 
	{ "034-c3.c3",    0x080000, 0xe5aa65f5, 3 | BRF_GRA },           //  4 
	{ "034-c4.c4",    0x080000, 0x0eb138cc, 3 | BRF_GRA },           //  5 

	{ "034-m1.m1",    0x020000, 0xf41b16b8, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "034-v1.v1",    0x100000, 0x50c9d0dd, 5 | BRF_SND },           //  7 Sound data
	{ "034-v2.v2",    0x100000, 0x5aa15686, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(fbfrenzy, fbfrenzy, neogeo)
STD_ROM_FN(fbfrenzy)

struct BurnDriver BurnDrvfbfrenzy = {
	"fbfrenzy", NULL, "neogeo", NULL, "1992",
	"Football Frenzy\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, fbfrenzyRomInfo, fbfrenzyRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Bakatonosama Mahjong Manyuki

static struct BurnRomInfo bakatonoRomDesc[] = {
	{ "036-p1.p1",    0x080000, 0x1c66b6fa, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "036-s1.s1",    0x020000, 0xf3ef4485, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "036-c1.c1",    0x100000, 0xfe7f1010, 3 | BRF_GRA },           //  2 Sprite data
	{ "036-c2.c2",    0x100000, 0xbbf003f5, 3 | BRF_GRA },           //  3 
	{ "036-c3.c3",    0x100000, 0x9ac0708e, 3 | BRF_GRA },           //  4 
	{ "036-c4.c4",    0x100000, 0xf2577d22, 3 | BRF_GRA },           //  5 

	{ "036-m1.m1",    0x020000, 0xf1385b96, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "036-v1.v1",    0x100000, 0x1c335dce, 5 | BRF_SND },           //  7 Sound data
	{ "036-v2.v2",    0x100000, 0xbbf79342, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(bakatono, bakatono, neogeo)
STD_ROM_FN(bakatono)

struct BurnDriver BurnDrvbakatono = {
	"bakatono", NULL, "neogeo", NULL, "1991",
	"Bakatonosama Mahjong Manyuki\0", NULL, "Monolith Corp.", "Neo Geo",
	L"\u30D0\u30AB\u6BBF\u69D8 \u9EBB\u96C0\u6F2B\u904A\u8A18\0Bakatonosama Mahjong Manyuki\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_MAHJONG, GBF_MAHJONG, 0,
	NULL, bakatonoRomInfo, bakatonoRomName, NULL, NULL, neomahjongInputInfo, neomahjongDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Crossed Swords

static struct BurnRomInfo crswordRomDesc[] = {
	{ "037-p1.p1",    0x080000, 0xe7f2553c, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "037-s1.s1",    0x020000, 0x74651f27, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "037-c1.c1",    0x100000, 0x09df6892, 3 | BRF_GRA },           //  2 Sprite data
	{ "037-c2.c2",    0x100000, 0xac122a78, 3 | BRF_GRA },           //  3 
	{ "037-c3.c3",    0x100000, 0x9d7ed1ca, 3 | BRF_GRA },           //  4 
	{ "037-c4.c4",    0x100000, 0x4a24395d, 3 | BRF_GRA },           //  5 

	{ "037-m1.m1",    0x020000, 0x9504b2c6, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "037-v1.v1",    0x100000, 0x61fedf65, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(crsword, crsword, neogeo)
STD_ROM_FN(crsword)

struct BurnDriver BurnDrvcrsword = {
	"crsword", NULL, "neogeo", NULL, "1991",
	"Crossed Swords\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_MISC, 0,
	NULL, crswordRomInfo, crswordRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Thrash Rally

static struct BurnRomInfo trallyRomDesc[] = {
	{ "038-p1.p1",    0x080000, 0x1e52a576, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "038-p2.p2",    0x080000, 0xa5193e2f, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "038-s1.s1",    0x020000, 0xfff62ae3, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "038-c1.c1",    0x100000, 0xc58323d4, 3 | BRF_GRA },           //  3 Sprite data
	{ "038-c2.c2",    0x100000, 0xbba9c29e, 3 | BRF_GRA },           //  4 
	{ "038-c3.c3",    0x080000, 0x3bb7b9d6, 3 | BRF_GRA },           //  5 
	{ "038-c4.c4",    0x080000, 0xa4513ecf, 3 | BRF_GRA },           //  6 

	{ "038-m1.m1",    0x020000, 0x0908707e, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "038-v1.v1",    0x100000, 0x5ccd9fd5, 5 | BRF_SND },           //  8 Sound data
	{ "038-v2.v2",    0x080000, 0xddd8d1e6, 5 | BRF_SND },           //  9 
	
	{ "hd6301v1p.hd6301v1", 0x001000, 0x00000000, BRF_OPT | BRF_NODUMP },
};

STDROMPICKEXT(trally, trally, neogeo)
STD_ROM_FN(trally)

struct BurnDriver BurnDrvtrally = {
	"trally", NULL, "neogeo", NULL, "1991",
	"Thrash Rally\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_RACING, 0,
	NULL, trallyRomInfo, trallyRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// King of the Monsters 2 - The Next Thing

static struct BurnRomInfo kotm2RomDesc[] = {
	{ "039-p1.p1",    0x080000, 0xb372d54c, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "039-p2.p2",    0x080000, 0x28661afe, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "039-s1.s1",    0x020000, 0x63ee053a, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "039-c1.c1",    0x200000, 0x6d1c4aa9, 3 | BRF_GRA },           //  3 Sprite data
	{ "039-c2.c2",    0x200000, 0xf7b75337, 3 | BRF_GRA },           //  4 
	{ "039-c3.c3",    0x080000, 0xbfc4f0b2, 3 | BRF_GRA },           //  5 
	{ "039-c4.c4",    0x080000, 0x81c9c250, 3 | BRF_GRA },           //  6 

	{ "039-m1.m1",    0x020000, 0x0c5b2ad5, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "039-v2.v2",    0x200000, 0x86d34b25, 5 | BRF_SND },           //  8 Sound data
	{ "039-v4.v4",    0x100000, 0x8fa62a0b, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(kotm2, kotm2, neogeo)
STD_ROM_FN(kotm2)

struct BurnDriver BurnDrvkotm2 = {
	"kotm2", NULL, "neogeo", NULL, "1992",
	"King of the Monsters 2 - The Next Thing\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_VSFIGHT, 0,
	NULL, kotm2RomInfo, kotm2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Sengoku 2 / Sengoku Denshou 2

static struct BurnRomInfo sengoku2RomDesc[] = {
	{ "040-p1.p1",    0x100000, 0x6dde02c2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "040-s1.s1",    0x020000, 0xcd9802a3, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "040-c1.c1",    0x200000, 0xfaa8ea99, 3 | BRF_GRA },           //  2 Sprite data
	{ "040-c2.c2",    0x200000, 0x87d0ec65, 3 | BRF_GRA },           //  3 
	{ "040-c3.c3",    0x080000, 0x24b5ba80, 3 | BRF_GRA },           //  4 
	{ "040-c4.c4",    0x080000, 0x1c9e9930, 3 | BRF_GRA },           //  5 

	{ "040-m1.m1",    0x020000, 0xd4de4bca, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "040-v1.v1",    0x200000, 0x71cb4b5d, 5 | BRF_SND },           //  7 Sound data
	{ "040-v2.v2",    0x100000, 0xc5cece01, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(sengoku2, sengoku2, neogeo)
STD_ROM_FN(sengoku2)

struct BurnDriver BurnDrvsengoku2 = {
	"sengoku2", NULL, "neogeo", NULL, "1993",
	"Sengoku 2 / Sengoku Denshou 2\0", NULL, "SNK", "Neo Geo",
	L"Sengoku 2\0\u6226\u56FD\u4F1D\u627F\uFF12\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_SCRFIGHT, 0,
	NULL, sengoku2RomInfo, sengoku2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Baseball Stars 2

static struct BurnRomInfo bstars2RomDesc[] = {
	{ "041-p1.p1",    0x080000, 0x523567fd, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "041-s1.s1",    0x020000, 0x015c5c94, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "041-c1.c1",    0x100000, 0xb39a12e1, 3 | BRF_GRA },           //  2 Sprite data
	{ "041-c2.c2",    0x100000, 0x766cfc2f, 3 | BRF_GRA },           //  3 
	{ "041-c3.c3",    0x100000, 0xfb31339d, 3 | BRF_GRA },           //  4 
	{ "041-c4.c4",    0x100000, 0x70457a0c, 3 | BRF_GRA },           //  5 

	{ "041-m1.m1",    0x020000, 0x15c177a6, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "041-v1.v1",    0x100000, 0xcb1da093, 5 | BRF_SND },           //  7 Sound data
	{ "041-v2.v2",    0x100000, 0x1c954a9d, 5 | BRF_SND },           //  8 
	{ "041-v3.v3",    0x080000, 0xafaa0180, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(bstars2, bstars2, neogeo)
STD_ROM_FN(bstars2)

struct BurnDriver BurnDrvbstars2 = {
	"bstars2", NULL, "neogeo", NULL, "1992",
	"Baseball Stars 2\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, bstars2RomInfo, bstars2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Quiz Meitantei Neo & Geo - Quiz Daisousa Sen part 2

static struct BurnRomInfo quizdai2RomDesc[] = {
	{ "042-p1.p1",    0x100000, 0xed719dcf, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "042-s1.s1",    0x020000, 0x164fd6e6, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "042-c1.c1",    0x100000, 0xcb5809a1, 3 | BRF_GRA },           //  2 Sprite data
	{ "042-c2.c2",    0x100000, 0x1436dfeb, 3 | BRF_GRA },           //  3 
	{ "042-c3.c3",    0x080000, 0xbcd4a518, 3 | BRF_GRA },           //  4 
	{ "042-c4.c4",    0x080000, 0xd602219b, 3 | BRF_GRA },           //  5 

	{ "042-m1.m1",    0x020000, 0xbb19995d, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "042-v1.v1",    0x100000, 0xaf7f8247, 5 | BRF_SND },           //  7 Sound data
	{ "042-v2.v2",    0x100000, 0xc6474b59, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(quizdai2, quizdai2, neogeo)
STD_ROM_FN(quizdai2)

struct BurnDriver BurnDrvquizdai2 = {
	"quizdai2", NULL, "neogeo", NULL, "1992",
	"Quiz Meitantei Neo & Geo - Quiz Daisousa Sen part 2\0", NULL, "SNK", "Neo Geo",
	L"\u30AF\u30A4\u30BA\u8FF7\u63A2\u5075\u30CD\u30AA\uFF06\u30B8\u30AA - quiz daisousa sen part 2\0Quiz Meintantei Neo Geo\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_QUIZ, 0,
	NULL, quizdai2RomInfo, quizdai2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// 3 Count Bout / Fire Suplex

static struct BurnRomInfo countb3RomDesc[] = {
	{ "043-p1.p1",    0x100000, 0xffbdd928, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "043-s1.s1",    0x020000, 0xc362d484, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "043-c1.c1",    0x200000, 0xbad2d67f, 3 | BRF_GRA },           //  2 Sprite data
	{ "043-c2.c2",    0x200000, 0xa7fbda95, 3 | BRF_GRA },           //  3 
	{ "043-c3.c3",    0x200000, 0xf00be011, 3 | BRF_GRA },           //  4 
	{ "043-c4.c4",    0x200000, 0x1887e5c0, 3 | BRF_GRA },           //  5 

	{ "043-m1.m1",    0x020000, 0x7eab59cb, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "043-v1.v1",    0x200000, 0x63688ce8, 5 | BRF_SND },           //  7 Sound data
	{ "043-v2.v2",    0x200000, 0xc69a827b, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(countb3, countb3, neogeo)
STD_ROM_FN(countb3)

struct BurnDriver BurnDrvcountb3 = {
	"3countb", NULL, "neogeo", NULL, "1993",
	"3 Count Bout / Fire Suplex\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_VSFIGHT, 0,
	NULL, countb3RomInfo, countb3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Art of Fighting / Ryuuko no Ken

static struct BurnRomInfo aofRomDesc[] = {
	{ "044-p1.p1",    0x080000, 0xca9f7a6d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "044-s1.s1",    0x020000, 0x89903f39, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "044-c1.c1",    0x200000, 0xddab98a7, 3 | BRF_GRA },           //  2 Sprite data
	{ "044-c2.c2",    0x200000, 0xd8ccd575, 3 | BRF_GRA },           //  3 
	{ "044-c3.c3",    0x200000, 0x403e898a, 3 | BRF_GRA },           //  4 
	{ "044-c4.c4",    0x200000, 0x6235fbaa, 3 | BRF_GRA },           //  5 

	{ "044-m1.m1",    0x020000, 0x0987e4bb, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "044-v2.v2",    0x200000, 0x3ec632ea, 5 | BRF_SND },           //  7 Sound data
	{ "044-v4.v4",    0x200000, 0x4b0f8e23, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(aof, aof, neogeo)
STD_ROM_FN(aof)

struct BurnDriver BurnDrvaof = {
	"aof", NULL, "neogeo", NULL, "1992",
	"Art of Fighting / Ryuuko no Ken\0", NULL, "SNK", "Neo Geo",
	L"Art of Fighting\0\u9F8D\u864E\u306E\u62F3\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_VSFIGHT, 0,
	NULL, aofRomInfo, aofRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown / Samurai Spirits (set 1)

static struct BurnRomInfo samshoRomDesc[] = {
	{ "045-p1.p1",    0x100000, 0xdfe51bf0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "045-pg2.sp2",  0x100000, 0x46745b94, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "045-s1.s1",    0x020000, 0x9142a4d3, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "045-c1.c1",    0x200000, 0x2e5873a4, 3 | BRF_GRA },           //  3 Sprite data
	{ "045-c2.c2",    0x200000, 0x04febb10, 3 | BRF_GRA },           //  4 
	{ "045-c3.c3",    0x200000, 0xf3dabd1e, 3 | BRF_GRA },           //  5 
	{ "045-c4.c4",    0x200000, 0x935c62f0, 3 | BRF_GRA },           //  6 
	{ "045-c51.c5",   0x100000, 0x81932894, 3 | BRF_GRA },           //  7 
	{ "045-c61.c6",   0x100000, 0xbe30612e, 3 | BRF_GRA },           //  8 

	{ "045-m1.m1",    0x020000, 0x95170640, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "045-v1.v1",    0x200000, 0x37f78a9b, 5 | BRF_SND },           // 10 Sound data
	{ "045-v2.v2",    0x200000, 0x568b20cf, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(samsho, samsho, neogeo)
STD_ROM_FN(samsho)

struct BurnDriver BurnDrvsamsho = {
	"samsho", NULL, "neogeo", NULL, "1993",
	"Samurai Shodown / Samurai Spirits (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samshoRomInfo, samshoRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Samurai Shodown / Samurai Spirits (set 2)

static struct BurnRomInfo samshohRomDesc[] = {
	{ "045-p1.p1",    0x100000, 0xdfe51bf0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "045-p2.sp2",   0x080000, 0x38ee9ba9, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "045-s1.s1",    0x020000, 0x9142a4d3, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "045-c1.c1",    0x200000, 0x2e5873a4, 3 | BRF_GRA },           //  3 Sprite data
	{ "045-c2.c2",    0x200000, 0x04febb10, 3 | BRF_GRA },           //  4 
	{ "045-c3.c3",    0x200000, 0xf3dabd1e, 3 | BRF_GRA },           //  5 
	{ "045-c4.c4",    0x200000, 0x935c62f0, 3 | BRF_GRA },           //  6 
	{ "045-c5.c5",    0x080000, 0xa2bb8284, 3 | BRF_GRA },           //  7 
	{ "045-c6.c6",    0x080000, 0x4fa71252, 3 | BRF_GRA },           //  8 

	{ "045-m1.m1",    0x020000, 0x95170640, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "045-v1.v1",    0x200000, 0x37f78a9b, 5 | BRF_SND },           // 10 Sound data
	{ "045-v2.v2",    0x200000, 0x568b20cf, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(samshoh, samshoh, neogeo)
STD_ROM_FN(samshoh)

struct BurnDriver BurnDrvsamshoh = {
	"samshoh", "samsho", "neogeo", NULL, "1993",
	"Samurai Shodown / Samurai Spirits (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samshohRomInfo, samshohRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Top Hunter - Roddy & Cathy (set 1)

static struct BurnRomInfo tophuntrRomDesc[] = {
	{ "046-p1.p1",    0x100000, 0x69fa9e29, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "046-p2.sp2",   0x100000, 0xf182cb3e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "046-s1.s1",    0x020000, 0x14b01d7b, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "046-c1.c1",    0x100000, 0xfa720a4a, 3 | BRF_GRA },           //  3 Sprite data
	{ "046-c2.c2",    0x100000, 0xc900c205, 3 | BRF_GRA },           //  4 
	{ "046-c3.c3",    0x100000, 0x880e3c25, 3 | BRF_GRA },           //  5 
	{ "046-c4.c4",    0x100000, 0x7a2248aa, 3 | BRF_GRA },           //  6 
	{ "046-c5.c5",    0x100000, 0x4b735e45, 3 | BRF_GRA },           //  7 
	{ "046-c6.c6",    0x100000, 0x273171df, 3 | BRF_GRA },           //  8 
	{ "046-c7.c7",    0x100000, 0x12829c4c, 3 | BRF_GRA },           //  9 
	{ "046-c8.c8",    0x100000, 0xc944e03d, 3 | BRF_GRA },           // 10 

	{ "046-m1.m1",    0x020000, 0x3f84bb9f, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "046-v1.v1",    0x100000, 0xc1f9c2db, 5 | BRF_SND },           // 12 Sound data
	{ "046-v2.v2",    0x100000, 0x56254a64, 5 | BRF_SND },           // 13 
	{ "046-v3.v3",    0x100000, 0x58113fb1, 5 | BRF_SND },           // 14 
	{ "046-v4.v4",    0x100000, 0x4f54c187, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(tophuntr, tophuntr, neogeo)
STD_ROM_FN(tophuntr)

struct BurnDriver BurnDrvtophuntr = {
	"tophuntr", NULL, "neogeo", NULL, "1994",
	"Top Hunter - Roddy & Cathy (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, tophuntrRomInfo, tophuntrRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Top Hunter - Roddy & Cathy (set 2)

static struct BurnRomInfo tophuntrhRomDesc[] = {
	{ "046-pg1.p1",   0x100000, 0x771e39bc, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "046-p2.sp2",   0x100000, 0xf182cb3e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "046-sg1.s1",   0x020000, 0x92e9ac8c, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "046-c1.c1",    0x100000, 0xfa720a4a, 3 | BRF_GRA },           //  3 Sprite data
	{ "046-c2.c2",    0x100000, 0xc900c205, 3 | BRF_GRA },           //  4 
	{ "046-c3.c3",    0x100000, 0x880e3c25, 3 | BRF_GRA },           //  5 
	{ "046-c4.c4",    0x100000, 0x7a2248aa, 3 | BRF_GRA },           //  6 
	{ "046-c5.c5",    0x100000, 0x4b735e45, 3 | BRF_GRA },           //  7 
	{ "046-c6.c6",    0x100000, 0x273171df, 3 | BRF_GRA },           //  8 
	{ "046-c7.c7",    0x100000, 0x12829c4c, 3 | BRF_GRA },           //  9 
	{ "046-c8.c8",    0x100000, 0xc944e03d, 3 | BRF_GRA },           // 10 

	{ "046-m1.m1",    0x020000, 0x3f84bb9f, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "046-v1.v1",    0x100000, 0xc1f9c2db, 5 | BRF_SND },           // 12 Sound data
	{ "046-v2.v2",    0x100000, 0x56254a64, 5 | BRF_SND },           // 13 
	{ "046-v3.v3",    0x100000, 0x58113fb1, 5 | BRF_SND },           // 14 
	{ "046-v4.v4",    0x100000, 0x4f54c187, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(tophuntrh, tophuntrh, neogeo)
STD_ROM_FN(tophuntrh)

struct BurnDriver BurnDrvtophuntrh = {
	"tophuntrh", "tophuntr", "neogeo", NULL, "1994",
	"Top Hunter - Roddy & Cathy (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, tophuntrhRomInfo, tophuntrhRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Fatal Fury 2 / Garou Densetsu 2 - arata-naru tatakai

static struct BurnRomInfo fatfury2RomDesc[] = {
	{ "047-p1.p1",    0x100000, 0xecfdbb69, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	
	{ "047-s1.s1",    0x020000, 0xd7dbbf39, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "047-c1.c1",    0x200000, 0xf72a939e, 3 | BRF_GRA },           //  3 Sprite data
	{ "047-c2.c2",    0x200000, 0x05119a0d, 3 | BRF_GRA },           //  4 
	{ "047-c3.c3",    0x200000, 0x01e00738, 3 | BRF_GRA },           //  5 
	{ "047-c4.c4",    0x200000, 0x9fe27432, 3 | BRF_GRA },           //  6 

	{ "047-m1.m1",    0x020000, 0x820b0ba7, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "047-v1.v1",    0x200000, 0xd9d00784, 5 | BRF_SND },           //  8 Sound data
	{ "047-v2.v2",    0x200000, 0x2c9a4b33, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(fatfury2, fatfury2, neogeo)
STD_ROM_FN(fatfury2)

static int prot_data;

unsigned char __fastcall fatfury2ReadByteProtection(unsigned int sekAddress)
{
	unsigned short res = (prot_data >> 24) & 0xff;

	switch (sekAddress) {
		case 0x255551:
		case 0x2ffff1:
		case 0x200001:
		case 0x2ff001:

		case 0x236001:
		case 0x236009:
			return res;

		case 0x236005:
		case 0x23600d:
			return ((res & 0xf0) >> 4) | ((res & 0x0f) << 4);
	}

	return 0;
}

void __fastcall fatfury2WriteByteProtection(unsigned int sekAddress, unsigned char)
{
	switch (sekAddress) {
		case 0x255551:
		case 0x2ffff1:
		case 0x2ff001:

		case 0x236001:
		case 0x236005:
		case 0x236009:
		case 0x23600d:
			prot_data <<= 8;
			break;
	}
}

void __fastcall fatfury2WriteWordProtection(unsigned int sekAddress, unsigned short)
{

	switch (sekAddress) {
		case 0x211112:				// data = 0x1111, expects 0xff000000 back
			prot_data = 0xff000000;

//			prot_data  = (wordValue & 0x1000) << 19;
//			prot_data |= (wordValue & 0x0100) << 22;
//			prot_data |= (wordValue & 0x0010) << 25;
//			prot_data |= (wordValue & 0x0001) << 28;
//			prot_data |= prot_data >> 4;

			break;

		case 0x233332:				// data = 0x3333, expects 0x0000ffff back
			prot_data = 0x0000ffff;
//			prot_data  = wordValue << 0;
//			prot_data |= wordValue << 2;

			break;

		case 0x244442:				// data = 0x4444, expects 0x00ff0000 back
			prot_data = 0x00ff0000;
//			prot_data  = (wordValue & 0x4000) <<  9;
//			prot_data |= (wordValue & 0x0400) << 12;
//			prot_data |= (wordValue & 0x0040) << 15;
//			prot_data |= (wordValue & 0x0004) << 18;
//			prot_data |= prot_data >> 4;

			break;

		case 0x255552:				// data == 0x5555; read back from 55550, ffff0, 00000, ff000
			prot_data = 0xff00ff00;
			break;

		case 0x256782:				// data == 0x1234; read back from 36000 *or* 36004
			prot_data = 0xf05a3601;
			break;

		case 0x242812:				// data == 0x1824; read back from 36008 *or* 3600c
			prot_data = 0x81422418;
			break;
	}
}

static int fatfury2Init()
{
	int nRet;

	nRet = NeoInit();

	if (nRet == 0) {
		// Install protection handler
		SekMapHandler(5,	0x200000, 0x2fffff, SM_WRITE);
		SekMapHandler(5,	0x200000, 0x2fffff, SM_READ);
		SekSetWriteWordHandler(5, fatfury2WriteWordProtection);
		SekSetWriteByteHandler(5, fatfury2WriteByteProtection);
		SekSetReadByteHandler(5, fatfury2ReadByteProtection);
	}

	return nRet;
}

struct BurnDriver BurnDrvfatfury2 = {
	"fatfury2", NULL, "neogeo", NULL, "1992",
	"Fatal Fury 2 / Garou Densetsu 2 - arata-naru tatakai\0", NULL, "SNK", "Neo Geo",
	L"Fatal Fury 2\0\u9913\u72FC\u4F1D\u8AAC\uFF12 - \u65B0\u305F\u306A\u308B\u95D8\u3044\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_VSFIGHT, FBF_FATFURY,
	NULL, fatfury2RomInfo, fatfury2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	fatfury2Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Jyanshin Densetsu - Quest of Jongmaster

static struct BurnRomInfo janshinRomDesc[] = {
	{ "048-p1.p1",    0x100000, 0xfa818cbb, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "048-s1.s1",    0x020000, 0x8285b25a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "048-c1.c1",    0x200000, 0x3fa890e9, 3 | BRF_GRA },           //  2 Sprite data
	{ "048-c2.c1",    0x200000, 0x59c48ad8, 3 | BRF_GRA },           //  3 

	{ "048-m1.m1",    0x020000, 0x310467c7, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "048-v1.v1",    0x200000, 0xf1947d2b, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(janshin, janshin, neogeo)
STD_ROM_FN(janshin)

struct BurnDriver BurnDrvjanshin = {
	"janshin", NULL, "neogeo", NULL, "1994",
	"Jyanshin Densetsu - Quest of Jongmaster\0", NULL, "Aicom", "Neo Geo",
	L"\u96C0\u795E\u4F1D\u8AAC - quest of jongmaster\0Jyanshin Densetsu\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_MAHJONG, GBF_MAHJONG, 0,
	NULL, janshinRomInfo, janshinRomName, NULL, NULL, neomahjongInputInfo, neomahjongDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Andro Dunos

static struct BurnRomInfo androdunRomDesc[] = {
	{ "049-p1.p1",    0x080000, 0x3b857da2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "049-p2.p2",    0x080000, 0x2f062209, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "049-s1.s1",    0x020000, 0x6349de5d, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "049-c1.c1",    0x100000, 0x7ace6db3, 3 | BRF_GRA },           //  3 Sprite data
	{ "049-c2.c2",    0x100000, 0xb17024f7, 3 | BRF_GRA },           //  4 

	{ "049-m1.m1",    0x020000, 0xedd2acf4, 4 | BRF_ESS | BRF_PRG }, //  5 Z80 code
	
	{ "049-v1.v1",    0x100000, 0xce43cb89, 5 | BRF_SND },           //  6 Sound data
};

STDROMPICKEXT(androdun, androdun, neogeo)
STD_ROM_FN(androdun)

struct BurnDriver BurnDrvandrodun = {
	"androdun", NULL, "neogeo", NULL, "1992",
	"Andro Dunos\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_HORSHOOT, 0,
	NULL, androdunRomInfo, androdunRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ninja Commando

static struct BurnRomInfo ncommandRomDesc[] = {
	{ "050-p1.p1",    0x100000, 0x4e097c40, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "050-s1.s1",    0x020000, 0xdb8f9c8e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "050-c1.c1",    0x100000, 0x87421a0a, 3 | BRF_GRA },           //  2 Sprite data
	{ "050-c2.c2",    0x100000, 0xc4cf5548, 3 | BRF_GRA },           //  3 
	{ "050-c3.c3",    0x100000, 0x03422c1e, 3 | BRF_GRA },           //  4 
	{ "050-c4.c4",    0x100000, 0x0845eadb, 3 | BRF_GRA },           //  5 

	{ "050-m1.m1",    0x020000, 0x6fcf07d3, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "050-v1.v1",    0x100000, 0x23c3ab42, 5 | BRF_SND },           //  7 Sound data
	{ "050-v2.v2",    0x080000, 0x80b8a984, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(ncommand, ncommand, neogeo)
STD_ROM_FN(ncommand)

struct BurnDriver BurnDrvncommand = {
	"ncommand", NULL, "neogeo", NULL, "1992",
	"Ninja Commando\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, ncommandRomInfo, ncommandRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Viewpoint

static struct BurnRomInfo viewpoinRomDesc[] = {
	{ "051-p1.p1",    0x100000, 0x17aa899d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "051-s1.s1",    0x020000, 0x9fea5758, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "051-c1.c1",    0x200000, 0xd624c132, 3 | BRF_GRA },           //  2 Sprite data
	{ "051-c2.c2",    0x200000, 0x40d69f1e, 3 | BRF_GRA },           //  3 

	{ "051-m1.m1",    0x020000, 0x8e69f29a, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "051-v2.v2",    0x200000, 0x019978b6, 5 | BRF_SND },           //  5 Sound data
	{ "051-v4.v4",    0x200000, 0x5758f38c, 5 | BRF_SND },           //  6 
};

STDROMPICKEXT(viewpoin, viewpoin, neogeo)
STD_ROM_FN(viewpoin)

struct BurnDriver BurnDrvviewpoin = {
	"viewpoin", NULL, "neogeo", NULL, "1992",
	"Viewpoint\0", NULL, "Sammy", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_SHOOT, 0,
	NULL, viewpoinRomInfo, viewpoinRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Super Sidekicks / Tokuten Ou

static struct BurnRomInfo ssidekiRomDesc[] = {
	{ "052-p1.p1",    0x080000, 0x9cd97256, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "052-s1.s1",    0x020000, 0x97689804, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "052-c1.c1",    0x200000, 0x53e1c002, 3 | BRF_GRA },           //  2 Sprite data
	{ "052-c2.c2",    0x200000, 0x776a2d1f, 3 | BRF_GRA },           //  3 

	{ "052-m1.m1",    0x020000, 0x49f17d2d, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "052-v1.v1",    0x200000, 0x22c097a5, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(ssideki, ssideki, neogeo)
STD_ROM_FN(ssideki)

struct BurnDriver BurnDrvssideki = {
	"ssideki", NULL, "neogeo", NULL, "1992",
	"Super Sidekicks / Tokuten Ou\0", NULL, "SNK", "Neo Geo",
	L"Super Sidekicks\0\u5F97\u70B9\u738B\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_SPORTSFOOTBALL, 0,
	NULL, ssidekiRomInfo, ssidekiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	fatfury2Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// World Heroes (set 1)

static struct BurnRomInfo wh1RomDesc[] = {
	{ "053-epr.p1",   0x080000, 0xd42e1e9a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "053-epr.p2",   0x080000, 0x0e33e8a3, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "053-s1.s1",    0x020000, 0x8c2c2d6b, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "053-c1.c1",    0x200000, 0x85eb5bce, 3 | BRF_GRA },           //  3 Sprite data
	{ "053-c2.c2",    0x200000, 0xec93b048, 3 | BRF_GRA },           //  4 
	{ "053-c3.c3",    0x100000, 0x0dd64965, 3 | BRF_GRA },           //  5 
	{ "053-c4.c4",    0x100000, 0x9270d954, 3 | BRF_GRA },           //  6 

	{ "053-m1.m1",    0x020000, 0x1bd9d04b, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "053-v2.v2",    0x200000, 0xa68df485, 5 | BRF_SND },           //  8 Sound data
	{ "053-v4.v4",    0x100000, 0x7bea8f66, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(wh1, wh1, neogeo)
STD_ROM_FN(wh1)

struct BurnDriver BurnDrvwh1 = {
	"wh1", NULL, "neogeo", NULL, "1992",
	"World Heroes (set 1)\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_VSFIGHT, 0,
	NULL, wh1RomInfo, wh1RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// World Heroes (set 2)

static struct BurnRomInfo wh1hRomDesc[] = {
	{ "053-p1.p1",    0x080000, 0x95b574cb, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "053-p2.p2",    0x080000, 0xf198ed45, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "053-s1.s1",    0x020000, 0x8c2c2d6b, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "053-c1.c1",    0x200000, 0x85eb5bce, 3 | BRF_GRA },           //  3 Sprite data
	{ "053-c2.c2",    0x200000, 0xec93b048, 3 | BRF_GRA },           //  4 
	{ "053-c3.c3",    0x100000, 0x0dd64965, 3 | BRF_GRA },           //  5 
	{ "053-c4.c4",    0x100000, 0x9270d954, 3 | BRF_GRA },           //  6 

	{ "053-m1.m1",    0x020000, 0x1bd9d04b, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "053-v2.v2",    0x200000, 0xa68df485, 5 | BRF_SND },           //  8 Sound data
	{ "053-v4.v4",    0x100000, 0x7bea8f66, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(wh1h, wh1h, neogeo)
STD_ROM_FN(wh1h)

struct BurnDriver BurnDrvwh1h = {
	"wh1h", "wh1", "neogeo", NULL, "1992",
	"World Heroes (set 2)\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_VSFIGHT, 0,
	NULL, wh1hRomInfo, wh1hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// World Heroes (set 3)

static struct BurnRomInfo wh1haRomDesc[] = {
	{ "053-p1_a.p1",  0x080000, 0xed29fde2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "053-p2_a.p2",  0x080000, 0x98f2b158, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "053-s1.s1",    0x020000, 0x8c2c2d6b, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "053-c1.c1",    0x200000, 0x85eb5bce, 3 | BRF_GRA },           //  3 Sprite data
	{ "053-c2.c2",    0x200000, 0xec93b048, 3 | BRF_GRA },           //  4 
	{ "053-c3.c3",    0x100000, 0x0dd64965, 3 | BRF_GRA },           //  5 
	{ "053-c4.c4",    0x100000, 0x9270d954, 3 | BRF_GRA },           //  6 

	{ "053-m1.m1",    0x020000, 0x1bd9d04b, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "053-v2.v2",    0x200000, 0xa68df485, 5 | BRF_SND },           //  8 Sound data
	{ "053-v4.v4",    0x100000, 0x7bea8f66, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(wh1ha, wh1ha, neogeo)
STD_ROM_FN(wh1ha)

struct BurnDriver BurnDrvwh1ha = {
	"wh1ha", "wh1", "neogeo", NULL, "1992",
	"World Heroes (set 3)\0", NULL, "Alpha Denshi Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPC, GBF_VSFIGHT, 0,
	NULL, wh1haRomInfo, wh1haRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// The King of Fighters '94

static struct BurnRomInfo kof94RomDesc[] = {
	{ "055-p1.p1",    0x200000, 0xf10a2042, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "055-s1.s1",    0x020000, 0x825976c1, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "055-c1.c1",    0x200000, 0xb96ef460, 3 | BRF_GRA },           //  2 Sprite data
	{ "055-c2.c2",    0x200000, 0x15e096a7, 3 | BRF_GRA },           //  3 
	{ "055-c3.c3",    0x200000, 0x54f66254, 3 | BRF_GRA },           //  4 
	{ "055-c4.c4",    0x200000, 0x0b01765f, 3 | BRF_GRA },           //  5 
	{ "055-c5.c5",    0x200000, 0xee759363, 3 | BRF_GRA },           //  6 
	{ "055-c6.c6",    0x200000, 0x498da52c, 3 | BRF_GRA },           //  7 
	{ "055-c7.c7",    0x200000, 0x62f66888, 3 | BRF_GRA },           //  8 
	{ "055-c8.c8",    0x200000, 0xfe0a235d, 3 | BRF_GRA },           //  9 

	{ "055-m1.m1",    0x020000, 0xf6e77cf5, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "055-v1.v1",    0x200000, 0x8889596d, 5 | BRF_SND },           // 11 Sound data
	{ "055-v2.v2",    0x200000, 0x25022b27, 5 | BRF_SND },           // 12 
	{ "055-v3.v3",    0x200000, 0x83cf32c0, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kof94, kof94, neogeo)
STD_ROM_FN(kof94)

struct BurnDriver BurnDrvkof94 = {
	"kof94", NULL, "neogeo", NULL, "1994",
	"The King of Fighters '94\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, FBF_KOF,
	NULL, kof94RomInfo, kof94RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Art of Fighting 2 / Ryuuko no Ken 2 (set 1)

static struct BurnRomInfo aof2RomDesc[] = {
	{ "056-p1.p1",    0x100000, 0xa3b1d021, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "056-s1.s1",    0x020000, 0x8b02638e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "056-c1.c1",    0x200000, 0x17b9cbd2, 3 | BRF_GRA },           //  2 Sprite data
	{ "056-c2.c2",    0x200000, 0x5fd76b67, 3 | BRF_GRA },           //  3 
	{ "056-c3.c3",    0x200000, 0xd2c88768, 3 | BRF_GRA },           //  4 
	{ "056-c4.c4",    0x200000, 0xdb39b883, 3 | BRF_GRA },           //  5 
	{ "056-c5.c5",    0x200000, 0xc3074137, 3 | BRF_GRA },           //  6 
	{ "056-c6.c6",    0x200000, 0x31de68d3, 3 | BRF_GRA },           //  7 
	{ "056-c7.c7",    0x200000, 0x3f36df57, 3 | BRF_GRA },           //  8 
	{ "056-c8.c8",    0x200000, 0xe546d7a8, 3 | BRF_GRA },           //  9 

	{ "056-m1.m1",    0x020000, 0xf27e9d52, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "056-v1.v1",    0x200000, 0x4628fde0, 5 | BRF_SND },           // 11 Sound data
	{ "056-v2.v2",    0x200000, 0xb710e2f2, 5 | BRF_SND },           // 12 
	{ "056-v3.v3",    0x100000, 0xd168c301, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(aof2, aof2, neogeo)
STD_ROM_FN(aof2)

struct BurnDriver BurnDrvaof2 = {
	"aof2", NULL, "neogeo", NULL, "1994",
	"Art of Fighting 2 / Ryuuko no Ken 2 (set 1)\0", NULL, "SNK", "Neo Geo",
	L"Art of Fighting 2 (set 1)\0\u9F8D\u864E\u306E\u62F3\uFF12 (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, aof2RomInfo, aof2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Art of Fighting 2 / Ryuuko no Ken 2 (set 2)

static struct BurnRomInfo aof2aRomDesc[] = {
	{ "056-p1.p1",    0x100000, 0xa3b1d021, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "056-s1.s1",    0x020000, 0x8b02638e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "056-c1.c1",    0x200000, 0x17b9cbd2, 3 | BRF_GRA },           //  2 Sprite data
	{ "056-c2.c2",    0x200000, 0x5fd76b67, 3 | BRF_GRA },           //  3 
	{ "056-c3.c3",    0x200000, 0xd2c88768, 3 | BRF_GRA },           //  4 
	{ "056-c4.c4",    0x200000, 0xdb39b883, 3 | BRF_GRA },           //  5 
	{ "056-c5.c5",    0x200000, 0xc3074137, 3 | BRF_GRA },           //  6
	{ "056-c6.c6",    0x200000, 0x31de68d3, 3 | BRF_GRA },           //  7 
	{ "056-c7.c7",    0x200000, 0x3f36df57, 3 | BRF_GRA },           //  8 
	{ "056-c8.c8",    0x200000, 0xe546d7a8, 3 | BRF_GRA },           //  9 

	{ "056-m1.m1",    0x020000, 0xf27e9d52, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "056-v1.v1",    0x200000, 0x4628fde0, 5 | BRF_SND },           // 11 Sound data
	{ "056-v2.v2",    0x200000, 0xb710e2f2, 5 | BRF_SND },           // 12 
	{ "056-v3.v3",    0x100000, 0xd168c301, 5 | BRF_SND },           // 13 

	{ "056-epr.ep1",0x080000, 0x75d6301c, 0 | BRF_ESS | BRF_PRG }, // 14
};

STDROMPICKEXT(aof2a, aof2a, neogeo)
STD_ROM_FN(aof2a)

static void aof2aCallback()
{
	BurnLoadRom(Neo68KROM, 14, 1);
}

static int aof2aInit()
{
	pNeoInitCallback = aof2aCallback;

 	return NeoInit();
}

struct BurnDriver BurnDrvaof2a = {
	"aof2a", "aof2", "neogeo", NULL, "1994",
	"Art of Fighting 2 / Ryuuko no Ken 2 (set 2)\0", NULL, "SNK", "Neo Geo",
	L"Art of Fighting 2 (set 2)\0\u9F8D\u864E\u306E\u62F3\uFF12 (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, aof2aRomInfo, aof2aRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	aof2aInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// World Heroes 2

static struct BurnRomInfo wh2RomDesc[] = {
	{ "057-p1.p1",    0x200000, 0x65a891d9, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "057-s1.s1",    0x020000, 0xfcaeb3a4, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "057-c1.c1",    0x200000, 0x21c6bb91, 3 | BRF_GRA },           //  2 Sprite data
	{ "057-c2.c2",    0x200000, 0xa3999925, 3 | BRF_GRA },           //  3 
	{ "057-c3.c3",    0x200000, 0xb725a219, 3 | BRF_GRA },           //  4 
	{ "057-c4.c4",    0x200000, 0x8d96425e, 3 | BRF_GRA },           //  5 
	{ "057-c5.c5",    0x200000, 0xb20354af, 3 | BRF_GRA },           //  6 
	{ "057-c6.c6",    0x200000, 0xb13d1de3, 3 | BRF_GRA },           //  7 

	{ "057-m1.m1",    0x020000, 0x8fa3bc77, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "057-v1.v1",    0x200000, 0x8877e301, 5 | BRF_SND },           //  9 Sound data
	{ "057-v2.v2",    0x200000, 0xc1317ff4, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(wh2, wh2, neogeo)
STD_ROM_FN(wh2)

struct BurnDriver BurnDrvwh2 = {
	"wh2", NULL, "neogeo", NULL, "1993",
	"World Heroes 2\0", NULL, "ADK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, wh2RomInfo, wh2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Fatal Fury Special / Garou Densetsu Special (set 1)

static struct BurnRomInfo fatfurspRomDesc[] = {
	{ "058-p1.p1",    0x100000, 0x2f585ba2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "058-p2.sp2",   0x080000, 0xd7c71a6b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "058-s1.s1",    0x020000, 0x2df03197, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "058-c1.c1",    0x200000, 0x044ab13c, 3 | BRF_GRA },           //  3 Sprite data
	{ "058-c2.c2",    0x200000, 0x11e6bf96, 3 | BRF_GRA },           //  4 
	{ "058-c3.c3",    0x200000, 0x6f7938d5, 3 | BRF_GRA },           //  5 
	{ "058-c4.c4",    0x200000, 0x4ad066ff, 3 | BRF_GRA },           //  6 
	{ "058-c5.c5",    0x200000, 0x49c5e0bf, 3 | BRF_GRA },           //  7 
	{ "058-c6.c6",    0x200000, 0x8ff1f43d, 3 | BRF_GRA },           //  8 

	{ "058-m1.m1",    0x020000, 0xccc5186e, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "058-v1.v1",    0x200000, 0x55d7ce84, 5 | BRF_SND },           // 10 Sound data
	{ "058-v2.v2",    0x200000, 0xee080b10, 5 | BRF_SND },           // 11 
	{ "058-v3.v3",    0x100000, 0xf9eb3d4a, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(fatfursp, fatfursp, neogeo)
STD_ROM_FN(fatfursp)

struct BurnDriver BurnDrvfatfursp = {
	"fatfursp", NULL, "neogeo", NULL, "1993",
	"Fatal Fury Special / Garou Densetsu Special (set 1)\0", NULL, "SNK", "Neo Geo",
	L"Fatal Fury Special (set 1)\0\u9913\u72FC\u4F1D\u8AAC Special (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, fatfurspRomInfo, fatfurspRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Fatal Fury Special / Garou Densetsu Special (set 2)

static struct BurnRomInfo fatfurspaRomDesc[] = {
	{ "058-p1.p1",    0x100000, 0x2f585ba2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "058-p2.sp2",   0x080000, 0xd7c71a6b, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "058-s1.s1",    0x020000, 0x2df03197, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "058-c1.c1",    0x200000, 0x044ab13c, 3 | BRF_GRA },           //  3 Sprite data
	{ "058-c2.c2",    0x200000, 0x11e6bf96, 3 | BRF_GRA },           //  4 
	{ "058-c3.c3",    0x200000, 0x6f7938d5, 3 | BRF_GRA },           //  5 
	{ "058-c4.c4",    0x200000, 0x4ad066ff, 3 | BRF_GRA },           //  6 
	{ "058-c5.c5",    0x200000, 0x49c5e0bf, 3 | BRF_GRA },           //  7 
	{ "058-c6.c6",    0x200000, 0x8ff1f43d, 3 | BRF_GRA },           //  8 

	{ "058-m1.m1",    0x020000, 0xccc5186e, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "058-v1.v1",    0x200000, 0x55d7ce84, 5 | BRF_SND },           // 10 Sound data
	{ "058-v2.v2",    0x200000, 0xee080b10, 5 | BRF_SND },           // 11 
	{ "058-v3.v3",    0x100000, 0xf9eb3d4a, 5 | BRF_SND },           // 12 
 
	{ "058-epr.ep1",0x080000, 0x9f0c1e1a, 0 | BRF_ESS | BRF_PRG }, // 13
};

STDROMPICKEXT(fatfurspa, fatfurspa, neogeo)
STD_ROM_FN(fatfurspa)

static void fatfurspaCallback()
{
	BurnLoadRom(Neo68KROM, 13, 1);
}

static int fatfurspaInit()
{
	pNeoInitCallback = fatfurspaCallback;

 	return NeoInit();
}

struct BurnDriver BurnDrvfatfurspa = {
	"fatfurspa", "fatfursp", "neogeo", NULL, "1993",
	"Fatal Fury Special / Garou Densetsu Special (set 2)\0", NULL, "SNK", "Neo Geo",
	L"Fatal Fury Special (set 2)\0\u9913\u72FC\u4F1D\u8AAC Special (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, fatfurspaRomInfo, fatfurspaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	fatfurspaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Savage Reign / Fu'un Mokushiroku - kakutou sousei

static struct BurnRomInfo savagereRomDesc[] = {
	{ "059-p1.p1",    0x200000, 0x01d4e9c0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "059-s1.s1",    0x020000, 0xe08978ca, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "059-c1.c1",    0x200000, 0x763ba611, 3 | BRF_GRA },           //  2 Sprite data
	{ "059-c2.c2",    0x200000, 0xe05e8ca6, 3 | BRF_GRA },           //  3 
	{ "059-c3.c3",    0x200000, 0x3e4eba4b, 3 | BRF_GRA },           //  4 
	{ "059-c4.c4",    0x200000, 0x3c2a3808, 3 | BRF_GRA },           //  5 
	{ "059-c5.c5",    0x200000, 0x59013f9e, 3 | BRF_GRA },           //  6 
	{ "059-c6.c6",    0x200000, 0x1c8d5def, 3 | BRF_GRA },           //  7 
	{ "059-c7.c7",    0x200000, 0xc88f7035, 3 | BRF_GRA },           //  8 
	{ "059-c8.c8",    0x200000, 0x484ce3ba, 3 | BRF_GRA },           //  9 

	{ "059-m1.m1",    0x020000, 0x29992eba, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "059-v1.v1",    0x200000, 0x530c50fd, 5 | BRF_SND },           // 11 Sound data
	{ "059-v2.v2",    0x200000, 0xeb6f1cdb, 5 | BRF_SND },           // 12 
	{ "059-v3.v3",    0x200000, 0x7038c2f9, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(savagere, savagere, neogeo)
STD_ROM_FN(savagere)

struct BurnDriver BurnDrvsavagere = {
	"savagere", NULL, "neogeo", NULL, "1995",
	"Savage Reign / Fu'un Mokushiroku - kakutou sousei\0", NULL, "SNK", "Neo Geo",
	L"Savage Reign\0\u98A8\u96F2\u9ED9\u793A\u9332 - \u683C\u95D8\u5275\u4E16\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, savagereRomInfo, savagereRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Fight Fever (set 1)

static struct BurnRomInfo fightfevRomDesc[] = {
	{ "060-p1.p1",    0x100000, 0x2a104b50, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "060-s1.s1",    0x020000, 0x7f012104, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "060-c1.c1",    0x200000, 0x8908fff9, 3 | BRF_GRA },           //  2 Sprite data
	{ "060-c2.c2",    0x200000, 0xc6649492, 3 | BRF_GRA },           //  3 
	{ "060-c3.c3",    0x200000, 0x0956b437, 3 | BRF_GRA },           //  4 
	{ "060-c4.c4",    0x200000, 0x026f3b62, 3 | BRF_GRA },           //  5 

	{ "060-m1.m1",    0x020000, 0x0b7c4e65, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "060-v1.v1",    0x200000, 0xf417c215, 5 | BRF_SND },           //  7 Sound data
	{ "060-v2.v2",    0x100000, 0xefcff7cf, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(fightfev, fightfev, neogeo)
STD_ROM_FN(fightfev)

struct BurnDriver BurnDrvfightfev = {
	"fightfev", NULL, "neogeo", NULL, "1994",
	"Fight Fever (set 1)\0", NULL, "Viccom", "Neo Geo",
	L"Fight Fever (set 1)\0\uC655\uC911\uC655 (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, fightfevRomInfo, fightfevRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Fight Fever (set 2)

static struct BurnRomInfo fightfevaRomDesc[] = {
	{ "060-p1.p1",    0x100000, 0x2a104b50, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "060-s1.s1",    0x020000, 0x7f012104, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "060-c1.c1",    0x200000, 0x8908fff9, 3 | BRF_GRA },           //  2 Sprite data
	{ "060-c2.c2",    0x200000, 0xc6649492, 3 | BRF_GRA },           //  3 
	{ "060-c3.c3",    0x200000, 0x0956b437, 3 | BRF_GRA },           //  4 
	{ "060-c4.c4",    0x200000, 0x026f3b62, 3 | BRF_GRA },           //  5 

	{ "060-m1.m1",    0x020000, 0x0b7c4e65, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "060-v1.v1",    0x200000, 0xf417c215, 5 | BRF_SND },           //  7 Sound data
	{ "060-v2.v2",    0x100000, 0xefcff7cf, 5 | BRF_SND },           //  8 
	
	{ "060-epr.sp2",  0x080000, 0x3032041b, 0 | BRF_ESS | BRF_PRG }, //  9 68K code
};

STDROMPICKEXT(fightfeva, fightfeva, neogeo)
STD_ROM_FN(fightfeva)

static void fightfevaCallback()
{
	BurnLoadRom(Neo68KROM, 9, 1);
}

static int fightfevaInit()
{
	pNeoInitCallback = fightfevaCallback;

 	return NeoInit();
}

struct BurnDriver BurnDrvfightfeva = {
	"fightfeva", "fightfev", "neogeo", NULL, "1994",
	"Fight Fever (set 2)\0", NULL, "Viccom", "Neo Geo",
	L"Fight Fever (set 2)\0\uC655\uC911\uC655 (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SCRFIGHT, 0,
	NULL, fightfevaRomInfo, fightfevaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	fightfevaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Super Sidekicks 2 - The World Championship / Tokuten Ou 2 - real fight football

static struct BurnRomInfo ssideki2RomDesc[] = {
	{ "061-p1.p1",      0x100000, 0x5969e0dc, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "061-s1.s1",      0x020000, 0x226d1b68, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "061-c1-16.c1",   0x200000, 0xa626474f, 3 | BRF_GRA },           //  2 Sprite data
	{ "061-c2-16.c2",   0x200000, 0xc3be42ae, 3 | BRF_GRA },           //  3 
	{ "061-c3-16.c3",   0x200000, 0x2a7b98b9, 3 | BRF_GRA },           //  4 
	{ "061-c4-16.c4",   0x200000, 0xc0be9a1f, 3 | BRF_GRA },           //  5 

	{ "061-m1.m1",      0x020000, 0x156f6951, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "061-v1.v1",      0x200000, 0xf081c8d3, 5 | BRF_SND },           //  7 Sound data
	{ "061-v2.v2",      0x200000, 0x7cd63302, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(ssideki2, ssideki2, neogeo)
STD_ROM_FN(ssideki2)

struct BurnDriver BurnDrvssideki2 = {
	"ssideki2", NULL, "neogeo", NULL, "1994",
	"Super Sidekicks 2 - The World Championship / Tokuten Ou 2 - real fight football\0", NULL, "SNK", "Neo Geo",
	L"Super Sidekicks 2 - the world championship\0\u5F97\u70B9\u738B\uFF12 - \u30EA\u30A2\u30EB\u30D5\u30A1\u30A4\u30C8\u30D5\u30C3\u30C8\u30DC\u30FC\u30EB\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSFOOTBALL, 0,
	NULL, ssideki2RomInfo, ssideki2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Spin Master / Miracle Adventure

static struct BurnRomInfo spinmastRomDesc[] = {
	{ "062-p1.p1",    0x100000, 0x37aba1aa, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "062-p2.sp2",   0x100000, 0xf025ab77, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "062-s1.s1",    0x020000, 0x289e2bbe, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "062-c1.c1",    0x100000, 0xa9375aa2, 3 | BRF_GRA },           //  3 Sprite data
	{ "062-c2.c2",    0x100000, 0x0e73b758, 3 | BRF_GRA },           //  4 
	{ "062-c3.c3",    0x100000, 0xdf51e465, 3 | BRF_GRA },           //  5 
	{ "062-c4.c4",    0x100000, 0x38517e90, 3 | BRF_GRA },           //  6 
	{ "062-c5.c5",    0x100000, 0x7babd692, 3 | BRF_GRA },           //  7 
	{ "062-c6.c6",    0x100000, 0xcde5ade5, 3 | BRF_GRA },           //  8 
	{ "062-c7.c7",    0x100000, 0xbb2fd7c0, 3 | BRF_GRA },           //  9 
	{ "062-c8.c8",    0x100000, 0x8d7be933, 3 | BRF_GRA },           // 10 

	{ "062-m1.m1",    0x020000, 0x76108b2f, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "062-v1.v1",    0x100000, 0xcc281aef, 5 | BRF_SND },           // 12 Sound data
};

STDROMPICKEXT(spinmast, spinmast, neogeo)
STD_ROM_FN(spinmast)

struct BurnDriver BurnDrvspinmast = {
	"spinmast", NULL, "neogeo", NULL, "1993",
	"Spin Master / Miracle Adventure\0", NULL, "Data East Corporation", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, spinmastRomInfo, spinmastRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown II / Shin Samurai Spirits - Haohmaru jigokuhen

static struct BurnRomInfo samsho2RomDesc[] = {
	{ "063-p1.p1",    0x200000, 0x22368892, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "063-s1.s1",    0x020000, 0x64a5cd66, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "063-c1.c1",    0x200000, 0x86cd307c, 3 | BRF_GRA },           //  2 Sprite data
	{ "063-c2.c2",    0x200000, 0xcdfcc4ca, 3 | BRF_GRA },           //  3 
	{ "063-c3.c3",    0x200000, 0x7a63ccc7, 3 | BRF_GRA },           //  4 
	{ "063-c4.c4",    0x200000, 0x751025ce, 3 | BRF_GRA },           //  5 
	{ "063-c5.c5",    0x200000, 0x20d3a475, 3 | BRF_GRA },           //  6 
	{ "063-c6.c6",    0x200000, 0xae4c0a88, 3 | BRF_GRA },           //  7 
	{ "063-c7.c7",    0x200000, 0x2df3cbcf, 3 | BRF_GRA },           //  8 
	{ "063-c8.c8",    0x200000, 0x1ffc6dfa, 3 | BRF_GRA },           //  9 

	{ "063-m1.m1",    0x020000, 0x56675098, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "063-v1.v1",    0x200000, 0x37703f91, 5 | BRF_SND },           // 11 Sound data
	{ "063-v2.v2",    0x200000, 0x0142bde8, 5 | BRF_SND },           // 12 
	{ "063-v3.v3",    0x200000, 0xd07fa5ca, 5 | BRF_SND },           // 13 
	{ "063-v4.v4",    0x100000, 0x24aab4bb, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(samsho2, samsho2, neogeo)
STD_ROM_FN(samsho2)

struct BurnDriver BurnDrvsamsho2 = {
	"samsho2", NULL, "neogeo", NULL, "1994",
	"Samurai Shodown II / Shin Samurai Spirits - Haohmaru jigokuhen\0", NULL, "SNK", "Neo Geo",
	L"Samurai Shodown II\0\u771F Samurai Spirits - \u8987\u738B\u4E38\u5730\u7344\u5909\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho2RomInfo, samsho2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};

// Saulabi Spirits / Jin Saulabi Tu Hon (Korean release of Samurai Shodown II)

static struct BurnRomInfo samsho2kRomDesc[] = {
	{ "063-p1-kan.p1",0x200000, 0x147cc6d7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "063-s1-kan.s1",0x020000, 0xff08f80b, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "063-c1.c1",    0x200000, 0x86cd307c, 3 | BRF_GRA },           //  2 Sprite data
	{ "063-c2.c2",    0x200000, 0xcdfcc4ca, 3 | BRF_GRA },           //  3 
	{ "063-c3.c3",    0x200000, 0x7a63ccc7, 3 | BRF_GRA },           //  4 
	{ "063-c4.c4",    0x200000, 0x751025ce, 3 | BRF_GRA },           //  5 
	{ "063-c5.c5",    0x200000, 0x20d3a475, 3 | BRF_GRA },           //  6 
	{ "063-c6.c6",    0x200000, 0xae4c0a88, 3 | BRF_GRA },           //  7 
	{ "063-c7.c7",    0x200000, 0x2df3cbcf, 3 | BRF_GRA },           //  8 
	{ "063-c8.c8",    0x200000, 0x1ffc6dfa, 3 | BRF_GRA },           //  9 

	{ "063-m1.m1",    0x020000, 0x56675098, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "063-v1.v1",    0x200000, 0x37703f91, 5 | BRF_SND },           // 11 Sound data
	{ "063-v2.v2",    0x200000, 0x0142bde8, 5 | BRF_SND },           // 12 
	{ "063-v3.v3",    0x200000, 0xd07fa5ca, 5 | BRF_SND },           // 13 
	{ "063-v4.v4",    0x100000, 0x24aab4bb, 5 | BRF_SND },           // 14 
	
	{ "063-ep1-kan.ep1", 0x080000, 0xfa32e2d8, 0 | BRF_ESS | BRF_PRG }, // 15
	{ "063-ep2-kan.ep2", 0x080000, 0x70b1a4d9, 0 | BRF_ESS | BRF_PRG }, // 16
};

STDROMPICKEXT(samsho2k, samsho2k, neogeo)
STD_ROM_FN(samsho2k)

static void samsho2kCallback()
{
	BurnLoadRom(Neo68KROM + 0x000000, 15, 1);
	BurnLoadRom(Neo68KROM + 0x080000, 16, 1);
}

static int samsho2kInit()
{
	pNeoInitCallback = samsho2kCallback;

 	return NeoInit();
}

struct BurnDriver BurnDrvsamsho2k = {
	"samsho2k", "samsho2", "neogeo", NULL, "1994",
	"Saulabi Spirits / Jin Saulabi Tu Hon (Korean release of Samurai Shodown II)\0", NULL, "SNK", "Neo Geo",
	L"Saulabi Spirits / Jin Saulabi Tu Hon (Korean release of Samurai Shodown II)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho2kRomInfo, samsho2kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsho2kInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// World Heroes 2 Jet (set 1)

static struct BurnRomInfo wh2jRomDesc[] = {
	{ "064-p1.p1",    0x200000, 0x385a2e86, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "064-s1.s1",    0x020000, 0x2a03998a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "064-c1.c1",    0x200000, 0x2ec87cea, 3 | BRF_GRA },           //  2 Sprite data
	{ "064-c2.c2",    0x200000, 0x526b81ab, 3 | BRF_GRA },           //  3 
	{ "064-c3.c3",    0x200000, 0x436d1b31, 3 | BRF_GRA },           //  4 
	{ "064-c4.c4",    0x200000, 0xf9c8dd26, 3 | BRF_GRA },           //  5 
	{ "064-c5.c5",    0x200000, 0x8e34a9f4, 3 | BRF_GRA },           //  6 
	{ "064-c6.c6",    0x200000, 0xa43e4766, 3 | BRF_GRA },           //  7 
	{ "064-c7.c7",    0x200000, 0x59d97215, 3 | BRF_GRA },           //  8 
	{ "064-c8.c8",    0x200000, 0xfc092367, 3 | BRF_GRA },           //  9 

	{ "064-m1.m1",    0x020000, 0xd2eec9d3, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "064-v1.v1",    0x200000, 0xaa277109, 5 | BRF_SND },           //  7 Sound data
	{ "064-v2.v2",    0x200000, 0xb6527edd, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(wh2j, wh2j, neogeo)
STD_ROM_FN(wh2j)

struct BurnDriver BurnDrvwh2j = {
	"wh2j", NULL, "neogeo", NULL, "1994",
	"World Heroes 2 Jet (set 1)\0", NULL, "ADK / SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, wh2jRomInfo, wh2jRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Windjammers / Flying Power Disc

static struct BurnRomInfo wjammersRomDesc[] = {
	{ "065-p1.p1",    0x100000, 0x6692c140, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "065-s1.s1",    0x020000, 0x074b5723, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "065-c1.c1",    0x100000, 0xc7650204, 3 | BRF_GRA },           //  2 Sprite data
	{ "065-c2.c2",    0x100000, 0xd9f3e71d, 3 | BRF_GRA },           //  3 
	{ "065-c3.c3",    0x100000, 0x40986386, 3 | BRF_GRA },           //  4 
	{ "065-c4.c4",    0x100000, 0x715e15ff, 3 | BRF_GRA },           //  5 

	{ "065-m1.m1",    0x020000, 0x52c23cfc, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "065-v1.v1",    0x100000, 0xce8b3698, 5 | BRF_SND },           //  7 Sound data
	{ "065-v2.v2",    0x100000, 0x659f9b96, 5 | BRF_SND },           //  8 
	{ "065-v3.v3",    0x100000, 0x39f73061, 5 | BRF_SND },           //  9 
	{ "065-v4.v4",    0x100000, 0x5dee7963, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(wjammers, wjammers, neogeo)
STD_ROM_FN(wjammers)

struct BurnDriver BurnDrvwjammers = {
	"wjammers", NULL, "neogeo", NULL, "1994",
	"Windjammers / Flying Power Disc\0", NULL, "Data East Corporation", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, wjammersRomInfo, wjammersRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Karnov's Revenge / Fighter's History Dynamite

static struct BurnRomInfo karnovrRomDesc[] = {
	{ "066-p1.p1",    0x100000, 0x8c86fd22, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "066-s1.s1",    0x020000, 0xbae5d5e5, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "066-c1.c1",    0x200000, 0x09dfe061, 3 | BRF_GRA },           //  2 Sprite data
	{ "066-c2.c2",    0x200000, 0xe0f6682a, 3 | BRF_GRA },           //  3 
	{ "066-c3.c3",    0x200000, 0xa673b4f7, 3 | BRF_GRA },           //  4 
	{ "066-c4.c4",    0x200000, 0xcb3dc5f4, 3 | BRF_GRA },           //  5 
	{ "066-c5.c5",    0x200000, 0x9a28785d, 3 | BRF_GRA },           //  6 
	{ "066-c6.c6",    0x200000, 0xc15c01ed, 3 | BRF_GRA },           //  7 

	{ "066-m1.m1",    0x020000, 0x030beae4, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "066-v1.v1",    0x200000, 0x0b7ea37a, 5 | BRF_SND },           //  9 Sound data
};

STDROMPICKEXT(karnovr, karnovr, neogeo)
STD_ROM_FN(karnovr)

struct BurnDriver BurnDrvkarnovr = {
	"karnovr", NULL, "neogeo", NULL, "1994",
	"Karnov's Revenge / Fighter's History Dynamite\0", NULL, "Data East Corporation", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, karnovrRomInfo, karnovrRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Gururin

static struct BurnRomInfo gururinRomDesc[] = {
	{ "067-p1.p1",    0x080000, 0x4cea8a49, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "067-s1.s1",    0x020000, 0xb119e1eb, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "067-c1.c1",    0x200000, 0x35866126, 3 | BRF_GRA },           //  2 Sprite data
	{ "067-c2.c2",    0x200000, 0x9db64084, 3 | BRF_GRA },           //  3 

	{ "067-m1.m1",    0x020000, 0x9e3c6328, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "067-v1.v1",    0x080000, 0xcf23afd0, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(gururin, gururin, neogeo)
STD_ROM_FN(gururin)

struct BurnDriver BurnDrvgururin = {
	"gururin", NULL, "neogeo", NULL, "1994",
	"Gururin\0", NULL, "Face", "Neo Geo",
	L"\u3050\u308B\u308A\u3093\0Gururin\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, gururinRomInfo, gururinRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Power Spikes II

static struct BurnRomInfo pspikes2RomDesc[] = {
	{ "068-pg1.p1",   0x100000, 0x105a408f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "068-sg1.s1",   0x020000, 0x18082299, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "068-c1.c1",    0x100000, 0x7f250f76, 3 | BRF_GRA },           //  2 Sprite data
	{ "068-c2.c2",    0x100000, 0x20912873, 3 | BRF_GRA },           //  3 
	{ "068-c3.c3",    0x100000, 0x4b641ba1, 3 | BRF_GRA },           //  4 
	{ "068-c4.c4",    0x100000, 0x35072596, 3 | BRF_GRA },           //  5 
	{ "068-c5.c5",    0x100000, 0x151dd624, 3 | BRF_GRA },           //  6 
	{ "068-c6.c6",    0x100000, 0xa6722604, 3 | BRF_GRA },           //  7 

	{ "068-mg1.m1",   0x020000, 0xb1c7911e, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "068-v1.v1",    0x100000, 0x2ced86df, 5 | BRF_SND },           //  9 Sound data
	{ "068-v2.v2",    0x100000, 0x970851ab, 5 | BRF_SND },           // 10 
	{ "068-v3.v3",    0x100000, 0x81ff05aa, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(pspikes2, pspikes2, neogeo)
STD_ROM_FN(pspikes2)

struct BurnDriver BurnDrvpspikes2 = {
	"pspikes2", NULL, "neogeo", NULL, "1994",
	"Power Spikes II\0", NULL, "Video System Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, pspikes2RomInfo, pspikes2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Fatal Fury 3 - Road to the Final Victory / Garou Densetsu 3 - haruka-naru tatakai

static struct BurnRomInfo fatfury3RomDesc[] = {
	{ "069-p1.p1",    0x100000, 0xa8bcfbbc, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "069-sp2.sp2",  0x200000, 0xdbe963ed, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "069-s1.s1",    0x020000, 0x0b33a800, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "069-c1.c1",    0x400000, 0xe302f93c, 3 | BRF_GRA },           //  3 Sprite data
	{ "069-c2.c2",    0x400000, 0x1053a455, 3 | BRF_GRA },           //  4 
	{ "069-c3.c3",    0x400000, 0x1c0fde2f, 3 | BRF_GRA },           //  5 
	{ "069-c4.c4",    0x400000, 0xa25fc3d0, 3 | BRF_GRA },           //  6 
	{ "069-c5.c5",    0x200000, 0xb3ec6fa6, 3 | BRF_GRA },           //  7 
	{ "069-c6.c6",    0x200000, 0x69210441, 3 | BRF_GRA },           //  8 

	{ "069-m1.m1",    0x020000, 0xfce72926, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "069-v1.v1",    0x400000, 0x2bdbd4db, 5 | BRF_SND },           // 10 Sound data
	{ "069-v2.v2",    0x400000, 0xa698a487, 5 | BRF_SND },           // 11 
	{ "069-v3.v3",    0x200000, 0x581c5304, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(fatfury3, fatfury3, neogeo)
STD_ROM_FN(fatfury3)

struct BurnDriver BurnDrvfatfury3 = {
	"fatfury3", NULL, "neogeo", NULL, "1995",
	"Fatal Fury 3 - Road to the Final Victory / Garou Densetsu 3 - haruka-naru tatakai\0", NULL, "SNK", "Neo Geo",
	L"Fatal Fury 3 - road to the final victory\0\u9913\u72FC\u4F1D\u8AAC\uFF13\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, fatfury3RomInfo, fatfury3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Panic Bomber

static struct BurnRomInfo panicbomRomDesc[] = {
	{ "073-p1.p1",    0x080000, 0xadc356ad, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "073-s1.s1",    0x020000, 0xb876de7e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "073-c1.c1",    0x100000, 0x8582e1b5, 3 | BRF_GRA },           //  2 Sprite data
	{ "073-c2.c2",    0x100000, 0xe15a093b, 3 | BRF_GRA },           //  3 

	{ "073-m1.m1",    0x020000, 0x3cdf5d88, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "073-v1.v1",    0x200000, 0x7fc86d2f, 5 | BRF_SND },           //  5 Sound data
	{ "073-v2.v2",    0x100000, 0x082adfc7, 5 | BRF_SND },           //  6 
};

STDROMPICKEXT(panicbom, panicbom, neogeo)
STD_ROM_FN(panicbom)

struct BurnDriver BurnDrvpanicbom = {
	"panicbom", NULL, "neogeo", NULL, "1994",
	"Panic Bomber\0", NULL, "Eighting / Hudson", "Neo Geo",
	L"Panic Bomber\0\u3071\u306B\u3063\u304F\u30DC\u30F3\u30D0\u30FC \u30DC\u30F3\u30D0\u30FC\u30DE\u30F3\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, panicbomRomInfo, panicbomRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Aggressors of Dark Kombat / Tsuukai GANGAN Koushinkyoku

static struct BurnRomInfo aodkRomDesc[] = {
	{ "074-p1.p1",    0x200000, 0x62369553, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "074-s1.s1",    0x020000, 0x96148d2b, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "074-c1.c1",    0x200000, 0xa0b39344, 3 | BRF_GRA },           //  2 Sprite data
	{ "074-c2.c2",    0x200000, 0x203f6074, 3 | BRF_GRA },           //  3 
	{ "074-c3.c3",    0x200000, 0x7fff4d41, 3 | BRF_GRA },           //  4 
	{ "074-c4.c4",    0x200000, 0x48db3e0a, 3 | BRF_GRA },           //  5 
	{ "074-c5.c5",    0x200000, 0xc74c5e51, 3 | BRF_GRA },           //  6 
	{ "074-c6.c6",    0x200000, 0x73e8e7e0, 3 | BRF_GRA },           //  7 
	{ "074-c7.c7",    0x200000, 0xac7daa01, 3 | BRF_GRA },           //  8 
	{ "074-c8.c8",    0x200000, 0x14e7ad71, 3 | BRF_GRA },           //  9 

	{ "074-m1.m1",    0x020000, 0x5a52a9d1, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "074-v1.v1",    0x200000, 0x7675b8fa, 5 | BRF_SND },           // 11 Sound data
	{ "074-v2.v2",    0x200000, 0xa9da86e9, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(aodk, aodk, neogeo)
STD_ROM_FN(aodk)

struct BurnDriver BurnDrvaodk = {
	"aodk", NULL, "neogeo", NULL, "1994",
	"Aggressors of Dark Kombat / Tsuukai GANGAN Koushinkyoku\0", NULL, "ADK / SNK", "Neo Geo",
	L"Aggressors of Dark Kombat\0\u75DB\u5FEB\uFF27\uFF41\uFF4E\uFF47\uFF41\uFF4E\u884C\u9032\u66F2\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, aodkRomInfo, aodkRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Aero Fighters 2 / Sonic Wings 2

static struct BurnRomInfo sonicwi2RomDesc[] = {
	{ "075-p1.p1",    0x200000, 0x92871738, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "075-s1.s1",    0x020000, 0xc9eec367, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "075-c1.c1",    0x200000, 0x3278e73e, 3 | BRF_GRA },           //  2 Sprite data
	{ "075-c2.c2",    0x200000, 0xfe6355d6, 3 | BRF_GRA },           //  3 
	{ "075-c3.c3",    0x200000, 0xc1b438f1, 3 | BRF_GRA },           //  4 
	{ "075-c4.c4",    0x200000, 0x1f777206, 3 | BRF_GRA },           //  5 

	{ "075-m1.m1",    0x020000, 0xbb828df1, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "075-v1.v1",    0x200000, 0x7577e949, 5 | BRF_SND },           //  7 Sound data
	{ "075-v2.v2",    0x100000, 0x021760cd, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(sonicwi2, sonicwi2, neogeo)
STD_ROM_FN(sonicwi2)

struct BurnDriver BurnDrvsonicwi2 = {
	"sonicwi2", NULL, "neogeo", NULL, "1994",
	"Aero Fighters 2 / Sonic Wings 2\0", NULL, "Video System Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VERSHOOT, FBF_SONICWI,
	NULL, sonicwi2RomInfo, sonicwi2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Zed Blade / Operation Ragnarok

static struct BurnRomInfo zedbladeRomDesc[] = {
	{ "076-p1.p1",    0x080000, 0xd7c1effd, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "076-s1.s1",    0x020000, 0xf4c25dd5, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "076-c1.c1",    0x200000, 0x4d9cb038, 3 | BRF_GRA },           //  2 Sprite data
	{ "076-c2.c2",    0x200000, 0x09233884, 3 | BRF_GRA },           //  3 
	{ "076-c3.c3",    0x200000, 0xd06431e3, 3 | BRF_GRA },           //  4 
	{ "076-c4.c4",    0x200000, 0x4b1c089b, 3 | BRF_GRA },           //  5 

	{ "076-m1.m1",    0x020000, 0x7b5f3d0a, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "076-v1.v1",    0x200000, 0x1a21d90c, 5 | BRF_SND },           //  7 Sound data
	{ "076-v2.v2",    0x200000, 0xb61686c3, 5 | BRF_SND },           //  8 
	{ "076-v3.v3",    0x100000, 0xb90658fa, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(zedblade, zedblade, neogeo)
STD_ROM_FN(zedblade)

struct BurnDriver BurnDrvzedblade = {
	"zedblade", NULL, "neogeo", NULL, "1994",
	"Zed Blade / Operation Ragnarok\0", NULL, "NMK", "Neo Geo",
	L"Zed Blade\0Operation Ragnarok\0\u4F5C\u6226\u540D\uFF02\u30E9\u30B0\u30CA\u30ED\u30AF\uFF02\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_HORSHOOT, 0,
	NULL, zedbladeRomInfo, zedbladeRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Galaxy Fight - Universal Warriors

static struct BurnRomInfo galaxyfgRomDesc[] = {
	{ "078-p1.p1",    0x200000, 0x45906309, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "078-s1.s1",    0x020000, 0x72f8923e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "078-c1.c1",    0x200000, 0xc890c7c0, 3 | BRF_GRA },           //  2 Sprite data
	{ "078-c2.c2",    0x200000, 0xb6d25419, 3 | BRF_GRA },           //  3 
	{ "078-c3.c3",    0x200000, 0x9d87e761, 3 | BRF_GRA },           //  4 
	{ "078-c4.c4",    0x200000, 0x765d7cb8, 3 | BRF_GRA },           //  5 
	{ "078-c5.c5",    0x200000, 0xe6b77e6a, 3 | BRF_GRA },           //  6 
	{ "078-c6.c6",    0x200000, 0xd779a181, 3 | BRF_GRA },           //  7 
	{ "078-c7.c7",    0x100000, 0x4f27d580, 3 | BRF_GRA },           //  8 
	{ "078-c8.c8",    0x100000, 0x0a7cc0d8, 3 | BRF_GRA },           //  9 

	{ "078-m1.m1",    0x020000, 0x8e9e3b10, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "078-v1.v1",    0x200000, 0xe3b735ac, 5 | BRF_SND },           // 11 Sound data
	{ "078-v2.v2",    0x200000, 0x6a8e78c2, 5 | BRF_SND },           // 12 
	{ "078-v3.v3",    0x100000, 0x70bca656, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(galaxyfg, galaxyfg, neogeo)
STD_ROM_FN(galaxyfg)

struct BurnDriver BurnDrvgalaxyfg = {
	"galaxyfg", NULL, "neogeo", NULL, "1995",
	"Galaxy Fight - Universal Warriors\0", NULL, "Sunsoft", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, galaxyfgRomInfo, galaxyfgRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Street Hoop / Street Slam / Dunk Dream

static struct BurnRomInfo strhoopRomDesc[] = {
	{ "079-p1.p1",    0x100000, 0x5e78328e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "079-s1.s1",    0x020000, 0x3ac06665, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "079-c1.c1",    0x200000, 0x0581c72a, 3 | BRF_GRA },           //  2 Sprite data
	{ "079-c2.c2",    0x200000, 0x5b9b8fb6, 3 | BRF_GRA },           //  3 
	{ "079-c3.c3",    0x200000, 0xcd65bb62, 3 | BRF_GRA },           //  4 
	{ "079-c4.c4",    0x200000, 0xa4c90213, 3 | BRF_GRA },           //  5 

	{ "079-m1.m1",    0x020000, 0xbee3455a, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "079-v1.v1",    0x200000, 0x718a2400, 5 | BRF_SND },           //  7 Sound data
	{ "079-v2.v2",    0x100000, 0x720774eb, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(strhoop, strhoop, neogeo)
STD_ROM_FN(strhoop)

struct BurnDriver BurnDrvstrhoop = {
	"strhoop", NULL, "neogeo", NULL, "1994",
	"Street Hoop / Street Slam / Dunk Dream\0", NULL, "Data East Corporation", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSMISC, 0,
	NULL, strhoopRomInfo, strhoopRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Quiz King of Fighters

static struct BurnRomInfo quizkofRomDesc[] = {
	{ "080-p1.p1",    0x100000, 0x4440315e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "080-s1.s1",    0x020000, 0xd7b86102, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "080-c1.c1",    0x200000, 0xea1d764a, 3 | BRF_GRA },           //  2 Sprite data
	{ "080-c2.c2",    0x200000, 0xd331d4a4, 3 | BRF_GRA },           //  3 
	{ "080-c3.c3",    0x200000, 0xb4851bfe, 3 | BRF_GRA },           //  4 
	{ "080-c4.c4",    0x200000, 0xca6f5460, 3 | BRF_GRA },           //  5 

	{ "080-m1.m1",    0x020000, 0xf5f44172, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "080-v1.v1",    0x200000, 0x0be18f60, 5 | BRF_SND },           //  7 Sound data
	{ "080-v2.v2",    0x200000, 0x4abde3ff, 5 | BRF_SND },           //  8 
	{ "080-v3.v3",    0x200000, 0xf02844e2, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(quizkof, quizkof, neogeo)
STD_ROM_FN(quizkof)

struct BurnDriver BurnDrvquizkof = {
	"quizkof", NULL, "neogeo", NULL, "1995",
	"Quiz King of Fighters\0", NULL, "Saurus", "Neo Geo",
	L"\u30AF\u30A4\u30BA\u30AD\u30F3\u30B0\u30AA\u30D6\u30D5\u30A1\u30A4\u30BF\u30FC\u30BA\0Quiz King of Fighters\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_QUIZ, 0,
	NULL, quizkofRomInfo, quizkofRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Quiz King of Fighters (Korea)

static struct BurnRomInfo quizkofkRomDesc[] = {
	{ "vic-080-p1.p1", 0x100000, 0x2589488e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "vic-080-s1.s1", 0x020000, 0xaf72c30f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "vic-080-c1.c1", 0x200000, 0x94d90170, 3 | BRF_GRA },           //  2 Sprite data
	{ "vic-080-c2.c2", 0x200000, 0x297f25a1, 3 | BRF_GRA },           //  3 
	{ "vic-080-c3.c3", 0x200000, 0xcf484c4f, 3 | BRF_GRA },           //  4 
	{ "vic-080-c4.c4", 0x200000, 0x36e5d997, 3 | BRF_GRA },           //  5 

	{ "vic-080-m1.m1", 0x020000, 0x4f157e9a, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "080-v1.v1",     0x200000, 0x0be18f60, 5 | BRF_SND },           //  7 Sound data
	{ "vic-080-v2.v2", 0x200000, 0x719fee5b, 5 | BRF_SND },           //  8 
	{ "vic-080-v3.v3", 0x200000, 0x64b7efde, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(quizkofk, quizkofk, neogeo)
STD_ROM_FN(quizkofk)

struct BurnDriver BurnDrvquizkofk = {
	"quizkofk", "quizkof", "neogeo", NULL, "1995",
	"Quiz King of Fighters (Korea)\0", NULL, "Saurus", "Neo Geo",
	L"\u30AF\u30A4\u30BA\u30AD\u30F3\u30B0\u30AA\u30D6\u30D5\u30A1\u30A4\u30BF\u30FC\u30BA (Korea)\0Quiz King of Fighters (Korea)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_QUIZ, 0,
	NULL, quizkofkRomInfo, quizkofkRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Super Sidekicks 3 - The Next Glory / Tokuten Ou 3 - eikoue no michi

static struct BurnRomInfo ssideki3RomDesc[] = {
	{ "081-p1.p1",    0x200000, 0x6bc27a3d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "081-s1.s1",    0x020000, 0x7626da34, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "081-c1.c1",    0x200000, 0x1fb68ebe, 3 | BRF_GRA },           //  2 Sprite data
	{ "081-c2.c2",    0x200000, 0xb28d928f, 3 | BRF_GRA },           //  3 
	{ "081-c3.c3",    0x200000, 0x3b2572e8, 3 | BRF_GRA },           //  4 
	{ "081-c4.c4",    0x200000, 0x47d26a7c, 3 | BRF_GRA },           //  5 
	{ "081-c5.c5",    0x200000, 0x17d42f0d, 3 | BRF_GRA },           //  6 
	{ "081-c6.c6",    0x200000, 0x6b53fb75, 3 | BRF_GRA },           //  7 

	{ "081-m1.m1",    0x020000, 0x82fcd863, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "081-v1.v1",    0x200000, 0x201fa1e1, 5 | BRF_SND },           //  9 Sound data
	{ "081-v2.v2",    0x200000, 0xacf29d96, 5 | BRF_SND },           // 10 
	{ "081-v3.v3",    0x200000, 0xe524e415, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(ssideki3, ssideki3, neogeo)
STD_ROM_FN(ssideki3)

struct BurnDriver BurnDrvssideki3 = {
	"ssideki3", NULL, "neogeo", NULL, "1995",
	"Super Sidekicks 3 - The Next Glory / Tokuten Ou 3 - eikoue no michi\0", NULL, "SNK", "Neo Geo",
	L"Super Sidekicks 3 - the next glory\0\u5F97\u70B9\u738B\uFF13 - \u6804\u5149\u3078\u306E\u6311\u6226\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SPORTSFOOTBALL, 0,
	NULL, ssideki3RomInfo, ssideki3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Double Dragon (Neo-Geo)

static struct BurnRomInfo doubledrRomDesc[] = {
	{ "082-p1.p1",    0x200000, 0x34ab832a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "082-s1.s1",    0x020000, 0xbef995c5, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "082-c1.c1",    0x200000, 0xb478c725, 3 | BRF_GRA },           //  2 Sprite data
	{ "082-c2.c2",    0x200000, 0x2857da32, 3 | BRF_GRA },           //  3 
	{ "082-c3.c3",    0x200000, 0x8b0d378e, 3 | BRF_GRA },           //  4 
	{ "082-c4.c4",    0x200000, 0xc7d2f596, 3 | BRF_GRA },           //  5 
	{ "082-c5.c5",    0x200000, 0xec87bff6, 3 | BRF_GRA },           //  6 
	{ "082-c6.c6",    0x200000, 0x844a8a11, 3 | BRF_GRA },           //  7 
	{ "082-c7.c7",    0x100000, 0x727c4d02, 3 | BRF_GRA },           //  8 
	{ "082-c8.c8",    0x100000, 0x69a5fa37, 3 | BRF_GRA },           //  9 

	{ "082-m1.m1",    0x020000, 0x10b144de, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "082-v1.v1",    0x200000, 0xcc1128e4, 5 | BRF_SND },           // 11 Sound data
	{ "082-v2.v2",    0x200000, 0xc3ff5554, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(doubledr, doubledr, neogeo)
STD_ROM_FN(doubledr)

struct BurnDriver BurnDrvdoubledr = {
	"doubledr", NULL, "neogeo", NULL, "1995",
	"Double Dragon (Neo-Geo)\0", NULL, "Technos", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, doubledrRomInfo, doubledrRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Puzzle Bobble / Bust-A-Move (Neo-Geo) (set 1)

static struct BurnRomInfo pbobblenRomDesc[] = {
	{ "d96-07.ep1",   0x080000, 0x6102ca14, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "d96-04.s1",    0x020000, 0x9caae538, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "068-c1.c1",    0x100000, 0x7f250f76, 3 | BRF_GRA },           //  2 Sprite data
	{ "068-c2.c2",    0x100000, 0x20912873, 3 | BRF_GRA },           //  3 
	{ "068-c3.c3",    0x100000, 0x4b641ba1, 3 | BRF_GRA },           //  4 
	{ "068-c4.c4",    0x100000, 0x35072596, 3 | BRF_GRA },           //  5 
	{ "d96-02.c5",    0x080000, 0xe89ad494, 3 | BRF_GRA },           //  6 Sprite data
	{ "d96-03.c6",    0x080000, 0x4b42d7eb, 3 | BRF_GRA },           //  7 

	{ "d96-06.m1",    0x020000, 0xf424368a, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "068-v1.v1",    0x100000, 0x2ced86df, 5 | BRF_SND },           //  9 Sound data
	{ "068-v2.v2",    0x100000, 0x970851ab, 5 | BRF_SND },           // 10 
	{ "d96-01.v3",    0x100000, 0x0840cbc4, 5 | BRF_SND },           // 11
	{ "d96-05.v4",    0x080000, 0x0a548948, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(pbobblen, pbobblen, neogeo)
STD_ROM_FN(pbobblen)

struct BurnDriver BurnDrvpbobblen = {
	"pbobblen", NULL, "neogeo", NULL, "1994",
	"Puzzle Bobble / Bust-A-Move (Neo-Geo) (set 1)\0", NULL, "Taito", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, pbobblenRomInfo, pbobblenRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Puzzle Bobble / Bust-A-Move (Neo-Geo) (bootleg)

static struct BurnRomInfo pbobblenbRomDesc[] = {
	{ "u7",           0x080000, 0xac1e9ef3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "us1",          0x020000, 0x9caae538, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "uc5",          0x080000, 0xe89ad494, 3 | BRF_GRA },           //  2 Sprite data
	{ "uc6",          0x080000, 0x4b42d7eb, 3 | BRF_GRA },           //  3 

	{ "um1",          0x020000, 0xf424368a, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "u8",           0x100000, 0x0840cbc4, 5 | BRF_SND },           //  5 Sound data
	{ "u9",           0x080000, 0x0a548948, 5 | BRF_SND },           //  6 
};

STDROMPICKEXT(pbobblenb, pbobblenb, neogeo)
STD_ROM_FN(pbobblenb)

struct BurnDriver BurnDrvpbobblenb = {
	"pbobblenb", "pbobblen", "neogeo", NULL, "1994",
	"Puzzle Bobble / Bust-A-Move (Neo-Geo) (bootleg)\0", NULL, "Taito", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, pbobblenbRomInfo, pbobblenbRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// The King of Fighters '95 (set 1)

static struct BurnRomInfo kof95RomDesc[] = {
	{ "084-p1.p1",    0x200000, 0x2cba2716, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "084-s1.s1",    0x020000, 0xde716f8a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "084-c1.c1",    0x400000, 0xfe087e32, 3 | BRF_GRA },           //  2 Sprite data
	{ "084-c2.c2",    0x400000, 0x07864e09, 3 | BRF_GRA },           //  3 
	{ "084-c3.c3",    0x400000, 0xa4e65d1b, 3 | BRF_GRA },           //  4 
	{ "084-c4.c4",    0x400000, 0xc1ace468, 3 | BRF_GRA },           //  5 
	{ "084-c5.c5",    0x200000, 0x8a2c1edc, 3 | BRF_GRA },           //  6 
	{ "084-c6.c6",    0x200000, 0xf593ac35, 3 | BRF_GRA },           //  7 
	{ "084-c7.c7",    0x100000, 0x9904025f, 3 | BRF_GRA },           //  8 
	{ "084-c8.c8",    0x100000, 0x78eb0f9b, 3 | BRF_GRA },           //  9 

	{ "084-m1.m1",    0x020000, 0x6f2d7429, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "084-v1.v1",    0x400000, 0x84861b56, 5 | BRF_SND },           // 11 Sound data
	{ "084-v2.v2",    0x200000, 0xb38a2803, 5 | BRF_SND },           // 12 
	{ "084-v3.v3",    0x100000, 0xd683a338, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kof95, kof95, neogeo)
STD_ROM_FN(kof95)

struct BurnDriver BurnDrvkof95 = {
	"kof95", NULL, "neogeo", NULL, "1995",
	"The King of Fighters '95 (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, FBF_KOF,
	NULL, kof95RomInfo, kof95RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '95 (set 2)

static struct BurnRomInfo kof95hRomDesc[] = {
	{ "084-pg1.p1",   0x200000, 0x5e54cf95, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "084-s1.s1",    0x020000, 0xde716f8a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "084-c1.c1",    0x400000, 0xfe087e32, 3 | BRF_GRA },           //  2 Sprite data
	{ "084-c2.c2",    0x400000, 0x07864e09, 3 | BRF_GRA },           //  3 
	{ "084-c3.c3",    0x400000, 0xa4e65d1b, 3 | BRF_GRA },           //  4
	{ "084-c4.c4",    0x400000, 0xc1ace468, 3 | BRF_GRA },           //  5 
	{ "084-c5.c5",    0x200000, 0x8a2c1edc, 3 | BRF_GRA },           //  6 
	{ "084-c6.c6",    0x200000, 0xf593ac35, 3 | BRF_GRA },           //  7 
	{ "084-c7.c7",    0x100000, 0x9904025f, 3 | BRF_GRA },           //  8 
	{ "084-c8.c8",    0x100000, 0x78eb0f9b, 3 | BRF_GRA },           //  9 

	{ "084-m1.m1",    0x020000, 0x6f2d7429, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "084-v1.v1",    0x400000, 0x84861b56, 5 | BRF_SND },           // 11 Sound data
	{ "084-v2.v2",    0x200000, 0xb38a2803, 5 | BRF_SND },           // 12 
	{ "084-v3.v3",    0x100000, 0xd683a338, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kof95h, kof95h, neogeo)
STD_ROM_FN(kof95h)

struct BurnDriver BurnDrvkof95h = {
	"kof95h", "kof95", "neogeo", NULL, "1995",
	"The King of Fighters '95 (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, FBF_KOF,
	NULL, kof95hRomInfo, kof95hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Tecmo World Soccer '96

static struct BurnRomInfo tws96RomDesc[] = {
	{ "086-p1.p1",    0x100000, 0x03e20ab6, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "086-s1.s1",    0x020000, 0x6f5e2b3a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "086-c1.c1",    0x400000, 0x2611bc2a, 3 | BRF_GRA },           //  2 Sprite data
	{ "086-c2.c2",    0x400000, 0x6b0d6827, 3 | BRF_GRA },           //  3 
	{ "086-c3.c3",    0x100000, 0x750ddc0c, 3 | BRF_GRA },           //  4 
	{ "086-c4.c4",    0x100000, 0x7a6e7d82, 3 | BRF_GRA },           //  5 

	{ "086-m1.m1",    0x020000, 0xcb82bc5d, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "086-v1.v1",    0x200000, 0x97bf1986, 5 | BRF_SND },           //  7 Sound data
	{ "086-v2.v2",    0x200000, 0xb7eb05df, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(tws96, tws96, neogeo)
STD_ROM_FN(tws96)

struct BurnDriver BurnDrvtws96 = {
	"tws96", NULL, "neogeo", NULL, "1996",
	"Tecmo World Soccer '96\0", NULL, "Tecmo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_SPORTSFOOTBALL, 0,
	NULL, tws96RomInfo, tws96RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown III / Samurai Spirits - Zankurou Musouken (set 1)

static struct BurnRomInfo samsho3RomDesc[] = {
	{ "087-epr.ep1",  0x080000, 0x23e09bb8, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "087-epr.ep2",  0x080000, 0x256f5302, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "087-epr.ep3",  0x080000, 0xbf2db5dd, 1 | BRF_ESS | BRF_PRG }, //  2 
	{ "087-epr.ep4",  0x080000, 0x53e60c58, 1 | BRF_ESS | BRF_PRG }, //  3 
	{ "087-p5.p5",    0x100000, 0xe86ca4af, 1 | BRF_ESS | BRF_PRG }, //  4 

	{ "087-s1.s1",    0x020000, 0x74ec7d9f, 2 | BRF_GRA },           //  5 Text layer tiles

	{ "087-c1.c1",    0x400000, 0x07a233bc, 3 | BRF_GRA },           //  6 Sprite data
	{ "087-c2.c2",    0x400000, 0x7a413592, 3 | BRF_GRA },           //  7 
	{ "087-c3.c3",    0x400000, 0x8b793796, 3 | BRF_GRA },           //  8 
	{ "087-c4.c4",    0x400000, 0x728fbf11, 3 | BRF_GRA },           //  9 
	{ "087-c5.c5",    0x400000, 0x172ab180, 3 | BRF_GRA },           // 10 
	{ "087-c6.c6",    0x400000, 0x002ff8f3, 3 | BRF_GRA },           // 11 
	{ "087-c7.c7",    0x100000, 0xae450e3d, 3 | BRF_GRA },           // 12 
	{ "087-c8.c8",    0x100000, 0xa9e82717, 3 | BRF_GRA },           // 13 

	{ "087-m1.m1",    0x020000, 0x8e6440eb, 4 | BRF_ESS | BRF_PRG }, // 14 Z80 code

	{ "087-v1.v1",    0x400000, 0x84bdd9a0, 5 | BRF_SND },           // 15 Sound data
	{ "087-v2.v2",    0x200000, 0xac0f261a, 5 | BRF_SND },           // 16
};

STDROMPICKEXT(samsho3, samsho3, neogeo)
STD_ROM_FN(samsho3)

struct BurnDriver BurnDrvsamsho3 = {
	"samsho3", NULL, "neogeo", NULL, "1995",
	"Samurai Shodown III / Samurai Spirits - Zankurou Musouken (set 1)\0", NULL, "SNK", "Neo Geo",
	L"Samurai Shodown III (set 1)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4 - \u65AC\u7D05\u90CE\u7121\u53CC\u5263 (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho3RomInfo, samsho3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown III / Samurai Spirits - Zankurou Musouken (set 2)

static struct BurnRomInfo samsho3hRomDesc[] = {
	{ "087-pg1.p1",   0x100000, 0x282a336e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "087-p2.sp2",   0x200000, 0x9bbe27e0, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "087-s1.s1",    0x020000, 0x74ec7d9f, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "087-c1.c1",    0x400000, 0x07a233bc, 3 | BRF_GRA },           //  3 Sprite data
	{ "087-c2.c2",    0x400000, 0x7a413592, 3 | BRF_GRA },           //  4 
	{ "087-c3.c3",    0x400000, 0x8b793796, 3 | BRF_GRA },           //  5 
	{ "087-c4.c4",    0x400000, 0x728fbf11, 3 | BRF_GRA },           //  6 
	{ "087-c5.c5",    0x400000, 0x172ab180, 3 | BRF_GRA },           //  7 
	{ "087-c6.c6",    0x400000, 0x002ff8f3, 3 | BRF_GRA },           //  8 
	{ "087-c7.c7",    0x100000, 0xae450e3d, 3 | BRF_GRA },           //  9 
	{ "087-c8.c8",    0x100000, 0xa9e82717, 3 | BRF_GRA },           // 10 

	{ "087-m1.m1",    0x020000, 0x8e6440eb, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "087-v1.v1",    0x400000, 0x84bdd9a0, 5 | BRF_SND },           // 12 Sound data
	{ "087-v2.v2",    0x200000, 0xac0f261a, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(samsho3h, samsho3h, neogeo)
STD_ROM_FN(samsho3h)

struct BurnDriver BurnDrvsamsho3h = {
	"samsho3h", "samsho3", "neogeo", NULL, "1995",
	"Samurai Shodown III / Samurai Spirits - Zankurou Musouken (set 2)\0", NULL, "SNK", "Neo Geo",
	L"Samurai Shodown III (set 2)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4 - \u65AC\u7D05\u90CE\u7121\u53CC\u5263 (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho3hRomInfo, samsho3hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Fighters Swords (Korean release of Samurai Shodown III)

static struct BurnRomInfo fswordsRomDesc[] = {
	{ "187-p1k.p1",   0x100000, 0xc8e7c075, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "087-p2.sp2",   0x200000, 0x9bbe27e0, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "087-s1.s1",    0x020000, 0x74ec7d9f, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "087-c1.c1",    0x400000, 0x07a233bc, 3 | BRF_GRA },           //  3 Sprite data
	{ "087-c2.c2",    0x400000, 0x7a413592, 3 | BRF_GRA },           //  4 
	{ "087-c3.c3",    0x400000, 0x8b793796, 3 | BRF_GRA },           //  5 
	{ "087-c4.c4",    0x400000, 0x728fbf11, 3 | BRF_GRA },           //  6 
	{ "087-c5.c5",    0x400000, 0x172ab180, 3 | BRF_GRA },           //  7 
	{ "087-c6.c6",    0x400000, 0x002ff8f3, 3 | BRF_GRA },           //  8 
	{ "087-c7.c7",    0x100000, 0xae450e3d, 3 | BRF_GRA },           //  9 
	{ "087-c8.c8",    0x100000, 0xa9e82717, 3 | BRF_GRA },           // 10 

	{ "087-m1.m1",    0x020000, 0x8e6440eb, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "087-v1.v1",    0x400000, 0x84bdd9a0, 5 | BRF_SND },           // 12 Sound data
	{ "087-v2.v2",    0x200000, 0xac0f261a, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(fswords, fswords, neogeo)
STD_ROM_FN(fswords)

struct BurnDriver BurnDrvfswords = {
	"fswords", "samsho3", "neogeo", NULL, "1995",
	"Fighters Swords (Korean release of Samurai Shodown III)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, fswordsRomInfo, fswordsRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Stakes Winner / Stakes Winner - GI kinzen seihae no michi

static struct BurnRomInfo stakwinRomDesc[] = {
	{ "088-p1.p1",    0x200000, 0xbd5814f6, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "088-s1.s1",    0x020000, 0x073cb208, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "088-c1.c1",    0x200000, 0x6e733421, 3 | BRF_GRA },           //  2 Sprite data
	{ "088-c2.c2",    0x200000, 0x4d865347, 3 | BRF_GRA },           //  3 
	{ "088-c3.c3",    0x200000, 0x8fa5a9eb, 3 | BRF_GRA },           //  4 
	{ "088-c4.c4",    0x200000, 0x4604f0dc, 3 | BRF_GRA },           //  5 

	{ "088-m1.m1",    0x020000, 0x2fe1f499, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "088-v1.v1",    0x200000, 0xb7785023, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(stakwin, stakwin, neogeo)
STD_ROM_FN(stakwin)

struct BurnDriver BurnDrvstakwin = {
	"stakwin", NULL, "neogeo", NULL, "1995",
	"Stakes Winner / Stakes Winner - GI kinzen seihae no michi\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_MISC, 0,
	NULL, stakwinRomInfo, stakwinRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Pulstar

static struct BurnRomInfo pulstarRomDesc[] = {
	{ "089-p1.p1",    0x100000, 0x5e5847a2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "089-p2.sp2",   0x200000, 0x028b774c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "089-s1.s1",    0x020000, 0xc79fc2c8, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "089-c1.c1",    0x400000, 0xf4e97332, 3 | BRF_GRA },           //  3 Sprite data
	{ "089-c2.c2",    0x400000, 0x836d14da, 3 | BRF_GRA },           //  4 
	{ "089-c3.c3",    0x400000, 0x913611c4, 3 | BRF_GRA },           //  5 
	{ "089-c4.c4",    0x400000, 0x44cef0e3, 3 | BRF_GRA },           //  6 
	{ "089-c5.c5",    0x400000, 0x89baa1d7, 3 | BRF_GRA },           //  7 
	{ "089-c6.c6",    0x400000, 0xb2594d56, 3 | BRF_GRA },           //  8 
	{ "089-c7.c7",    0x200000, 0x6a5618ca, 3 | BRF_GRA },           //  9 
	{ "089-c8.c8",    0x200000, 0xa223572d, 3 | BRF_GRA },           // 10 

	{ "089-m1.m1",    0x020000, 0xff3df7c7, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "089-v1.v1",    0x400000, 0x6f726ecb, 5 | BRF_SND },           // 12 Sound data
	{ "089-v2.v2",    0x400000, 0x9d2db551, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(pulstar, pulstar, neogeo)
STD_ROM_FN(pulstar)

struct BurnDriver BurnDrvpulstar = {
	"pulstar", NULL, "neogeo", NULL, "1995",
	"Pulstar\0", NULL, "Aicom", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_HORSHOOT, 0,
	NULL, pulstarRomInfo, pulstarRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// World Heroes Perfect

static struct BurnRomInfo whpRomDesc[] = {
	{ "090-p1.p1",    0x200000, 0xafaa4702, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "090-s1.s1",    0x020000, 0x174a880f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "090-c1.c1",    0x400000, 0xcd30ed9b, 3 | BRF_GRA },           //  2 Sprite data
	{ "090-c2.c2",    0x400000, 0x10eed5ee, 3 | BRF_GRA },           //  3 
	{ "064-c3.c3",    0x200000, 0x436d1b31, 3 | BRF_GRA },           //  4 
	{ "064-c4.c4",    0x200000, 0xf9c8dd26, 3 | BRF_GRA },           //  5 
	{ "064-c5.c5",    0x200000, 0x8e34a9f4, 3 | BRF_GRA },           //  6 
	{ "064-c6.c6",    0x200000, 0xa43e4766, 3 | BRF_GRA },           //  7 
	{ "064-c7.c7",    0x200000, 0x59d97215, 3 | BRF_GRA },           //  8 
	{ "064-c8.c8",    0x200000, 0xfc092367, 3 | BRF_GRA },           //  9 

	{ "090-m1.m1",    0x020000, 0x28065668, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "090-v1.v1",    0x200000, 0x30cf2709, 5 | BRF_SND },           // 11 Sound data
	{ "064-v2.v2",    0x200000, 0xb6527edd, 5 | BRF_SND },           // 12 
	{ "090-v3.v3",    0x200000, 0x1908a7ce, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(whp, whp, neogeo)
STD_ROM_FN(whp)

struct BurnDriver BurnDrvwhp = {
	"whp", NULL, "neogeo", NULL, "1995",
	"World Heroes Perfect\0", NULL, "ADK / SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, whpRomInfo, whpRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Far East of Eden - Kabuki Klash / Tengai Makyou - Shin Den

static struct BurnRomInfo kabukiklRomDesc[] = {
	{ "092-p1.p1",    0x200000, 0x28ec9b77, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "092-s1.s1",    0x020000, 0xa3d68ee2, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "092-c1.c1",    0x400000, 0x2a9fab01, 3 | BRF_GRA },           //  2 Sprite data
	{ "092-c2.c2",    0x400000, 0x6d2bac02, 3 | BRF_GRA },           //  3 
	{ "092-c3.c3",    0x400000, 0x5da735d6, 3 | BRF_GRA },           //  4 
	{ "092-c4.c4",    0x400000, 0xde07f997, 3 | BRF_GRA },           //  5 

	{ "092-m1.m1",    0x020000, 0x91957ef6, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "092-v1.v1",    0x200000, 0x69e90596, 5 | BRF_SND },           //  7 Sound data
	{ "092-v2.v2",    0x200000, 0x7abdb75d, 5 | BRF_SND },           //  8 
	{ "092-v3.v3",    0x200000, 0xeccc98d3, 5 | BRF_SND },           //  9 
	{ "092-v4.v4",    0x100000, 0xa7c9c949, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(kabukikl, kabukikl, neogeo)
STD_ROM_FN(kabukikl)

struct BurnDriver BurnDrvkabukikl = {
	"kabukikl", NULL, "neogeo", NULL, "1995",
	"Far East of Eden - Kabuki Klash / Tengai Makyou - Shin Den\0", NULL, "Hudson", "Neo Geo",
	L"Kabuki Klash - far east of eden\0\u5929\u5916\u9B54\u5883 - \u771F\u4F1D\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, kabukiklRomInfo, kabukiklRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Neo Bomberman

static struct BurnRomInfo neobombeRomDesc[] = {
	{ "093-p1.p1",    0x100000, 0xa1a71d0d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "093-s1.s1",    0x020000, 0x4b3fa119, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "093-c1.c1",    0x400000, 0xd1f328f8, 3 | BRF_GRA },           //  2 Sprite data
	{ "093-c2.c2",    0x400000, 0x82c49540, 3 | BRF_GRA },           //  3 
	{ "093-c3.c3",    0x080000, 0xe37578c5, 3 | BRF_GRA },           //  4 
	{ "093-c4.c4",    0x080000, 0x59826783, 3 | BRF_GRA },           //  5 

	{ "093-m1.m1",    0x020000, 0xe81e780b, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "093-v1.v1",    0x400000, 0x02abd4b0, 5 | BRF_SND },           //  7 Sound data
	{ "093-v2.v2",    0x200000, 0xa92b8b3d, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(neobombe, neobombe, neogeo)
STD_ROM_FN(neobombe)

struct BurnDriver BurnDrvneobombe = {
	"neobombe", NULL, "neogeo", NULL, "1997",
	"Neo Bomberman\0", NULL, "Hudson", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_MAZE, 0,
	NULL, neobombeRomInfo, neobombeRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Voltage Fighter - Gowcaizer / Choujin Gakuen Gowcaizer

static struct BurnRomInfo gowcaizrRomDesc[] = {
	{ "094-p1.p1",    0x200000, 0x33019545, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "094-s1.s1",    0x020000, 0x2f8748a2, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "094-c1.c1",    0x200000, 0x042f6af5, 3 | BRF_GRA },           //  2 Sprite data
	{ "094-c2.c2",    0x200000, 0x0fbcd046, 3 | BRF_GRA },           //  3 
	{ "094-c3.c3",    0x200000, 0x58bfbaa1, 3 | BRF_GRA },           //  4 
	{ "094-c4.c4",    0x200000, 0x9451ee73, 3 | BRF_GRA },           //  5 
	{ "094-c5.c5",    0x200000, 0xff9cf48c, 3 | BRF_GRA },           //  6 
	{ "094-c6.c6",    0x200000, 0x31bbd918, 3 | BRF_GRA },           //  7 
	{ "094-c7.c7",    0x200000, 0x2091ec04, 3 | BRF_GRA },           //  8 
	{ "094-c8.c8",    0x200000, 0xd80dd241, 3 | BRF_GRA },           //  9 

	{ "094-m1.m1",    0x020000, 0x78c851cb, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "094-v1.v1",    0x200000, 0x6c31223c, 5 | BRF_SND },           // 11 Sound data
	{ "094-v2.v2",    0x200000, 0x8edb776c, 5 | BRF_SND },           // 12 
	{ "094-v3.v3",    0x100000, 0xc63b9285, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(gowcaizr, gowcaizr, neogeo)
STD_ROM_FN(gowcaizr)

struct BurnDriver BurnDrvgowcaizr = {
	"gowcaizr", NULL, "neogeo", NULL, "1995",
	"Voltage Fighter - Gowcaizer / Choujin Gakuen Gowcaizer\0", NULL, "Technos", "Neo Geo",
	L"Voltage Fighter - Gowcaizer\0\u8D85\u4EBA\u5B66\u5712\u30B4\u30A6\u30AB\u30A4\u30B6\u30FC\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, gowcaizrRomInfo, gowcaizrRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Real Bout Fatal Fury / Real Bout Garou Densetsu

static struct BurnRomInfo rbff1RomDesc[] = {
	{ "095-p1.p1",    0x100000, 0x63b4d8ae, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "095-p2.sp2",   0x200000, 0xcc15826e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "095-s1.s1",    0x020000, 0xb6bf5e08, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "069-c1.c1",    0x400000, 0xe302f93c, 3 | BRF_GRA },           //  3 Sprite data
	{ "069-c2.c2",    0x400000, 0x1053a455, 3 | BRF_GRA },           //  4 
	{ "069-c3.c3",    0x400000, 0x1c0fde2f, 3 | BRF_GRA },           //  5 
	{ "069-c4.c4",    0x400000, 0xa25fc3d0, 3 | BRF_GRA },           //  6 
	{ "095-c5.c5",    0x400000, 0x8b9b65df, 3 | BRF_GRA },           //  7 
	{ "095-c6.c6",    0x400000, 0x3e164718, 3 | BRF_GRA },           //  8 
	{ "095-c7.c7",    0x200000, 0xca605e12, 3 | BRF_GRA },           //  9 
	{ "095-c8.c8",    0x200000, 0x4e6beb6c, 3 | BRF_GRA },           // 10 

	{ "095-m1.m1",    0x020000, 0x653492a7, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "069-v1.v1",    0x400000, 0x2bdbd4db, 5 | BRF_SND },           // 12 Sound data
	{ "069-v2.v2",    0x400000, 0xa698a487, 5 | BRF_SND },           // 13 
	{ "095-v3.v3",    0x400000, 0x189d1c6c, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(rbff1, rbff1, neogeo)
STD_ROM_FN(rbff1)

struct BurnDriver BurnDrvrbff1 = {
	"rbff1", NULL, "neogeo", NULL, "1995",
	"Real Bout Fatal Fury / Real Bout Garou Densetsu\0", NULL, "SNK", "Neo Geo",
	L"Real Bout Fatal Fury\0Real Bout \u9913\u72FC\u4F1D\u8AAC\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, rbff1RomInfo, rbff1RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Real Bout Fatal Fury / Real Bout Garou Densetsu (bug fix revision)

static struct BurnRomInfo rbff1aRomDesc[] = {
	{ "095-p1.p1",    0x100000, 0x63b4d8ae, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "095-p2.sp2",   0x200000, 0xcc15826e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "095-s1.s1",    0x020000, 0xb6bf5e08, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "069-c1.c1",    0x400000, 0xe302f93c, 3 | BRF_GRA },           //  3 Sprite data
	{ "069-c2.c2",    0x400000, 0x1053a455, 3 | BRF_GRA },           //  4 
	{ "069-c3.c3",    0x400000, 0x1c0fde2f, 3 | BRF_GRA },           //  5 
	{ "069-c4.c4",    0x400000, 0xa25fc3d0, 3 | BRF_GRA },           //  6 
	{ "095-c5.c5",    0x400000, 0x8b9b65df, 3 | BRF_GRA },           //  7 
	{ "095-c6.c6",    0x400000, 0x3e164718, 3 | BRF_GRA },           //  8 
	{ "095-c7.c7",    0x200000, 0xca605e12, 3 | BRF_GRA },           //  9 
	{ "095-c8.c8",    0x200000, 0x4e6beb6c, 3 | BRF_GRA },           // 10 

	{ "095-m1.m1",    0x020000, 0x653492a7, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "069-v1.v1",    0x400000, 0x2bdbd4db, 5 | BRF_SND },           // 12 Sound data
	{ "069-v2.v2",    0x400000, 0xa698a487, 5 | BRF_SND },           // 13 
	{ "095-v3.v3",    0x400000, 0x189d1c6c, 5 | BRF_SND },           // 14

	{ "095-epr.ep1",  0x080000, 0xbe0060a3, 0 | BRF_ESS | BRF_PRG }, // 15 68k code 
};

STDROMPICKEXT(rbff1a, rbff1a, neogeo)
STD_ROM_FN(rbff1a)

static void rbff1aCallback()
{
	BurnLoadRom(Neo68KROM, 15, 1);
}

static int rbff1aInit()
{
	pNeoInitCallback = rbff1aCallback;

 	return NeoInit();
}

struct BurnDriver BurnDrvrbff1a = {
	"rbff1a", "rbff1", "neogeo", NULL, "1995",
	"Real Bout Fatal Fury / Real Bout Garou Densetsu (bug fix revision)\0", NULL, "SNK", "Neo Geo",
	L"Real Bout Fatal Fury (bug fix revision)\0Real Bout \u9913\u72FC\u4F1D\u8AAC (bug fix revision)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, rbff1aRomInfo, rbff1aRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	rbff1aInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Art of Fighting 3 - The Path of the Warrior / Art of Fighting - Ryuuko no Ken Gaiden

static struct BurnRomInfo aof3RomDesc[] = {
	{ "096-p1.p1",    0x100000, 0x9edb420d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "096-p2.sp2",   0x200000, 0x4d5a2602, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "096-s1.s1",    0x020000, 0xcc7fd344, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "096-c1.c1",    0x400000, 0xf17b8d89, 3 | BRF_GRA },           //  3 Sprite data
	{ "096-c2.c2",    0x400000, 0x3840c508, 3 | BRF_GRA },           //  4 
	{ "096-c3.c3",    0x400000, 0x55f9ee1e, 3 | BRF_GRA },           //  5 
	{ "096-c4.c4",    0x400000, 0x585b7e47, 3 | BRF_GRA },           //  6 
	{ "096-c5.c5",    0x400000, 0xc75a753c, 3 | BRF_GRA },           //  7 
	{ "096-c6.c6",    0x400000, 0x9a9d2f7a, 3 | BRF_GRA },           //  8 
	{ "096-c7.c7",    0x200000, 0x51bd8ab2, 3 | BRF_GRA },           //  9 
	{ "096-c8.c8",    0x200000, 0x9a34f99c, 3 | BRF_GRA },           // 10 

	{ "096-m1.m1",    0x020000, 0xcb07b659, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "096-v1.v1",    0x200000, 0xe2c32074, 5 | BRF_SND },           // 12 Sound data
	{ "096-v2.v2",    0x200000, 0xa290eee7, 5 | BRF_SND },           // 13 
	{ "096-v3.v3",    0x200000, 0x199d12ea, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(aof3, aof3, neogeo)
STD_ROM_FN(aof3)

struct BurnDriver BurnDrvaof3 = {
	"aof3", NULL, "neogeo", NULL, "1996",
	"Art of Fighting 3 - The Path of the Warrior / Art of Fighting - Ryuuko no Ken Gaiden\0", NULL, "SNK", "Neo Geo",
	L"Art of Fighting 3 - the path of the warrior\0Art of Fighting - \u9F8D\u864E\u306E\u62F3\u5916\u4F1D\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, aof3RomInfo, aof3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Art of Fighting 3 - The Path of the Warrior (Korean release)

static struct BurnRomInfo aof3kRomDesc[] = {
	{ "196-p1k.p1",   0x100000, 0xa0780789, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "096-p2.sp2",   0x200000, 0x4d5a2602, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "096-s1.s1",    0x020000, 0xcc7fd344, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "096-c1.c1",    0x400000, 0xf17b8d89, 3 | BRF_GRA },           //  3 Sprite data
	{ "096-c2.c2",    0x400000, 0x3840c508, 3 | BRF_GRA },           //  4 
	{ "096-c3.c3",    0x400000, 0x55f9ee1e, 3 | BRF_GRA },           //  5 
	{ "096-c4.c4",    0x400000, 0x585b7e47, 3 | BRF_GRA },           //  6 
	{ "096-c5.c5",    0x400000, 0xc75a753c, 3 | BRF_GRA },           //  7 
	{ "096-c6.c6",    0x400000, 0x9a9d2f7a, 3 | BRF_GRA },           //  8 
	{ "096-c7.c7",    0x200000, 0x51bd8ab2, 3 | BRF_GRA },           //  9 
	{ "096-c8.c8",    0x200000, 0x9a34f99c, 3 | BRF_GRA },           // 10 

	{ "096-m1.m1",    0x020000, 0xcb07b659, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "096-v1.v1",    0x200000, 0xe2c32074, 5 | BRF_SND },           // 12 Sound data
	{ "096-v2.v2",    0x200000, 0xa290eee7, 5 | BRF_SND },           // 13 
	{ "096-v3.v3",    0x200000, 0x199d12ea, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(aof3k, aof3k, neogeo)
STD_ROM_FN(aof3k)

struct BurnDriver BurnDrvaof3k = {
	"aof3k", "aof3", "neogeo", NULL, "1996",
	"Art of Fighting 3 - The Path of the Warrior (Korean release)\0", NULL, "SNK", "Neo Geo",
	L"Art of Fighting 3 - the path of the warrior (korean release)\0Art of Fighting - \u9F8D\u864E\u306E\u62F3\u5916\u4F1D (korean release)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, aof3kRomInfo, aof3kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Aero Fighters 3 / Sonic Wings 3

static struct BurnRomInfo sonicwi3RomDesc[] = {
	{ "097-p1.p1",    0x200000, 0x0547121d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "097-s1.s1",    0x020000, 0x8dd66743, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "097-c1.c1",    0x400000, 0x33d0d589, 3 | BRF_GRA },           //  2 Sprite data
	{ "097-c2.c2",    0x400000, 0x186f8b43, 3 | BRF_GRA },           //  3 
	{ "097-c3.c3",    0x200000, 0xc339fff5, 3 | BRF_GRA },           //  4 
	{ "097-c4.c4",    0x200000, 0x84a40c6e, 3 | BRF_GRA },           //  5 

	{ "097-m1.m1",    0x020000, 0xb20e4291, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "097-v1.v1",    0x400000, 0x6f885152, 5 | BRF_SND },           //  7 Sound data
	{ "097-v2.v2",    0x200000, 0x3359e868, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(sonicwi3, sonicwi3, neogeo)
STD_ROM_FN(sonicwi3)

struct BurnDriver BurnDrvsonicwi3 = {
	"sonicwi3", NULL, "neogeo", NULL, "1995",
	"Aero Fighters 3 / Sonic Wings 3\0", NULL, "Video System Co.", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VERSHOOT, FBF_SONICWI,
	NULL, sonicwi3RomInfo, sonicwi3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Neo Turf Masters / Big Tournament Golf

static struct BurnRomInfo turfmastRomDesc[] = {
	{ "200-p1.p1",    0x200000, 0x28c83048, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "200-s1.s1",    0x020000, 0x9a5402b2, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "200-c1.c1",    0x400000, 0x8e7bf41a, 3 | BRF_GRA },           //  2 Sprite data
	{ "200-c2.c2",    0x400000, 0x5a65a8ce, 3 | BRF_GRA },           //  3 

	{ "200-m1.m1",    0x020000, 0x9994ac00, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "200-v1.v1",    0x200000, 0x00fd48d2, 5 | BRF_SND },           //  5 Sound data
	{ "200-v2.v2",    0x200000, 0x082acb31, 5 | BRF_SND },           //  6 
	{ "200-v3.v3",    0x200000, 0x7abca053, 5 | BRF_SND },           //  7 
	{ "200-v4.v4",    0x200000, 0x6c7b4902, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(turfmast, turfmast, neogeo)
STD_ROM_FN(turfmast)

struct BurnDriver BurnDrvturfmast = {
	"turfmast", NULL, "neogeo", NULL, "1996",
	"Neo Turf Masters / Big Tournament Golf\0", NULL, "Nazca", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SPORTSMISC, 0,
	NULL, turfmastRomInfo, turfmastRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug - Super Vehicle-001

static struct BurnRomInfo mslugRomDesc[] = {
	{ "201-p1.p1",    0x200000, 0x08d8daa5, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "201-s1.s1",    0x020000, 0x2f55958d, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "201-c1.c1",    0x400000, 0x72813676, 3 | BRF_GRA },           //  2 Sprite data
	{ "201-c2.c2",    0x400000, 0x96f62574, 3 | BRF_GRA },           //  3 
	{ "201-c3.c3",    0x400000, 0x5121456a, 3 | BRF_GRA },           //  4 
	{ "201-c4.c4",    0x400000, 0xf4ad59a3, 3 | BRF_GRA },           //  5 

	{ "201-m1.m1",    0x020000, 0xc28b3253, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "201-v1.v1",    0x400000, 0x23d22ed1, 5 | BRF_SND },           //  7 Sound data
	{ "201-v2.v2",    0x400000, 0x472cf9db, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(mslug, mslug, neogeo)
STD_ROM_FN(mslug)

struct BurnDriver BurnDrvmslug = {
	"mslug", NULL, "neogeo", NULL, "1996",
	"Metal Slug - Super Vehicle-001\0", NULL, "Nazca", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslugRomInfo, mslugRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Puzzle De Pon!

static struct BurnRomInfo puzzledpRomDesc[] = {
	{ "202-p1.p1",    0x080000, 0x2b61415b, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "202-s1.s1",    0x020000, 0xcd19264f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "202-c1.c1",    0x100000, 0xcc0095ef, 3 | BRF_GRA },           //  2 Sprite data
	{ "202-c2.c2",    0x100000, 0x42371307, 3 | BRF_GRA },           //  3 

	{ "202-m1.m1",    0x020000, 0x9c0291ea, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "202-v1.v1",    0x080000, 0xdebeb8fb, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(puzzledp, puzzledp, neogeo)
STD_ROM_FN(puzzledp)

struct BurnDriver BurnDrvpuzzledp = {
	"puzzledp", NULL, "neogeo", NULL, "1995",
	"Puzzle De Pon!\0", NULL, "Taito (Visco license)", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, puzzledpRomInfo, puzzledpRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Syougi No Tatsujin - Master of Syougi

static struct BurnRomInfo mosyougiRomDesc[] = {
	{ "203-p1.p1",    0x100000, 0x7ba70e2d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "203-s1.s1",    0x020000, 0xbfdc8309, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "203-c1.c1",    0x200000, 0xbba9e8c0, 3 | BRF_GRA },           //  2 Sprite data
	{ "203-c2.c2",    0x200000, 0x2574be03, 3 | BRF_GRA },           //  3 

	{ "203-m1.m1",    0x020000, 0xa602c2c2, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "203-v1.v1",    0x200000, 0xbaa2b9a5, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(mosyougi, mosyougi, neogeo)
STD_ROM_FN(mosyougi)

struct BurnDriver BurnDrvmosyougi = {
	"mosyougi", NULL, "neogeo", NULL, "1995",
	"Syougi No Tatsujin - Master of Syougi\0", NULL, "ADK / SNK", "Neo Geo",
	L"\u5C06\u68CB\u306E\u9054\u4EBA\0Master of Syougi\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, mosyougiRomInfo, mosyougiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Chibi Marukochan Deluxe Quiz

static struct BurnRomInfo marukodqRomDesc[] = {
	{ "206-p1.p1",    0x100000, 0xc33ed21e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "206-s1.s1",    0x020000, 0xf0b68780, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "206-c1.c1",    0x400000, 0x846e4e8e, 3 | BRF_GRA },           //  2 Sprite data
	{ "206-c2.c2",    0x400000, 0x1cba876d, 3 | BRF_GRA },           //  3 
	{ "206-c3.c3",    0x100000, 0x79aa2b48, 3 | BRF_GRA },           //  4 
	{ "206-c4.c4",    0x100000, 0x55e1314d, 3 | BRF_GRA },           //  5 

	{ "206-m1.m1",    0x020000, 0x0e22902e, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "206-v1.v1",    0x200000, 0x5385eca8, 5 | BRF_SND },           //  7 Sound data
	{ "206-v2.v2",    0x200000, 0xf8c55404, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(marukodq, marukodq, neogeo)
STD_ROM_FN(marukodq)

struct BurnDriver BurnDrvmarukodq = {
	"marukodq", NULL, "neogeo", NULL, "1995",
	"Chibi Marukochan Deluxe Quiz\0", NULL, "Takara", "Neo Geo",
	L"\u3061\u3073\u307E\u308B\u5B50\u3061\u3083\u3093 \u307E\u308B\u5B50\u30C7\u30E9\u30C3\u30AF\u30B9\u30AF\u30A4\u30BA\0Chibi Marukochan Deluxe Quiz\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_QUIZ, 0,
	NULL, marukodqRomInfo, marukodqRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Neo Mr. Do!

static struct BurnRomInfo neomrdoRomDesc[] = {
	{ "207-p1.p1",    0x100000, 0x334ea51e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "207-s1.s1",    0x020000, 0x6aebafce, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "207-c1.c1",    0x200000, 0xc7541b9d, 3 | BRF_GRA },           //  2 Sprite data
	{ "207-c2.c2",    0x200000, 0xf57166d2, 3 | BRF_GRA },           //  3 

	{ "207-m1.m1",    0x020000, 0xb5b74a95, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "207-v1.v1",    0x200000, 0x4143c052, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(neomrdo, neomrdo, neogeo)
STD_ROM_FN(neomrdo)

struct BurnDriver BurnDrvneomrdo = {
	"neomrdo", NULL, "neogeo", NULL, "1996",
	"Neo Mr. Do!\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_MAZE, 0,
	NULL, neomrdoRomInfo, neomrdoRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Super Dodge Ball / Kunio no Nekketsu Toukyuu Densetsu

static struct BurnRomInfo sdodgebRomDesc[] = {
	{ "208-p1.p1",    0x200000, 0x127f3d32, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "208-s1.s1",    0x020000, 0x64abd6b3, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "208-c1.c1",    0x400000, 0x93d8619b, 3 | BRF_GRA },           //  2 Sprite data
	{ "208-c2.c2",    0x400000, 0x1c737bb6, 3 | BRF_GRA },           //  3 
	{ "208-c3.c3",    0x200000, 0x14cb1703, 3 | BRF_GRA },           //  4 
	{ "208-c4.c4",    0x200000, 0xc7165f19, 3 | BRF_GRA },           //  5 

	{ "208-m1.m1",    0x020000, 0x0a5f3325, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "208-v1.v1",    0x400000, 0xe7899a24, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(sdodgeb, sdodgeb, neogeo)
STD_ROM_FN(sdodgeb)

struct BurnDriver BurnDrvsdodgeb = {
	"sdodgeb", NULL, "neogeo", NULL, "1996",
	"Super Dodge Ball / Kunio no Nekketsu Toukyuu Densetsu\0", NULL, "Technos", "Neo Geo",
	L"Super Dodge Ball\0\u304F\u306B\u304A\u306E\u71B1\u8840\u95D8\u7403\u4F1D\u8AAC\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SPORTSMISC, 0,
	NULL, sdodgebRomInfo, sdodgebRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Goal! Goal! Goal!

static struct BurnRomInfo goalx3RomDesc[] = {
	{ "209-p1.p1",    0x200000, 0x2a019a79, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "209-s1.s1",    0x020000, 0xc0eaad86, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "209-c1.c1",    0x400000, 0xb49d980e, 3 | BRF_GRA },           //  2 Sprite data
	{ "209-c2.c2",    0x400000, 0x5649b015, 3 | BRF_GRA },           //  3 
	{ "209-c3.c3",    0x100000, 0x5f91bace, 3 | BRF_GRA },           //  4 
	{ "209-c4.c4",    0x100000, 0x1e9f76f2, 3 | BRF_GRA },           //  5 

	{ "209-m1.m1",    0x020000, 0xcd758325, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "209-v1.v1",    0x200000, 0xef214212, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(goalx3, goalx3, neogeo)
STD_ROM_FN(goalx3)

struct BurnDriver BurnDrvgoalx3 = {
	"goalx3", NULL, "neogeo", NULL, "1995",
	"Goal! Goal! Goal!\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SPORTSFOOTBALL, 0,
	NULL, goalx3RomInfo, goalx3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Zintrick / Oshidashi Zentrix (hack / bootleg)

static struct BurnRomInfo zintrckbRomDesc[] = {
	{ "zin-p1.bin",   0x100000, 0x06c8fca7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "zin-s1.bin",   0x020000, 0xa7ab0e81, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "zin-c1.bin",   0x200000, 0x76aee189, 3 | BRF_GRA },           //  2 Sprite data
	{ "zin-c2.bin",   0x200000, 0x844ed4b3, 3 | BRF_GRA },           //  3 

	{ "zin-m1.bin",   0x020000, 0xfd9627ca, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "zin-v1.bin",   0x200000, 0xc09f74f1, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(zintrckb, zintrckb, neogeo)
STD_ROM_FN(zintrckb)

struct BurnDriver BurnDrvzintrckb = {
	"zintrckb", NULL, "neogeo", NULL, "1996",
	"Zintrick / Oshidashi Zentrix (hack / bootleg)\0", NULL, "hack / bootleg", "Neo Geo",
	L"Zintrick\0\u62BC\u3057\u51FA\u3057\u30B8\u30F3\u30C8\u30EA\u30C3\u30AF\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, zintrckbRomInfo, zintrckbRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Over Top

static struct BurnRomInfo overtopRomDesc[] = {
	{ "212-p1.p1",    0x200000, 0x16c063a9, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "212-s1.s1",    0x020000, 0x481d3ddc, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "212-c1.c1",    0x400000, 0x50f43087, 3 | BRF_GRA },           //  2 Sprite data
	{ "212-c2.c2",    0x400000, 0xa5b39807, 3 | BRF_GRA },           //  3 
	{ "212-c3.c3",    0x400000, 0x9252ea02, 3 | BRF_GRA },           //  4 
	{ "212-c4.c4",    0x400000, 0x5f41a699, 3 | BRF_GRA },           //  5 
	{ "212-c5.c5",    0x200000, 0xfc858bef, 3 | BRF_GRA },           //  6 
	{ "212-c6.c6",    0x200000, 0x0589c15e, 3 | BRF_GRA },           //  7 

	{ "212-m1.m1",    0x020000, 0xfcab6191, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "212-v1.v1",    0x400000, 0x013d4ef9, 5 | BRF_SND },           //  9 Sound data
};

STDROMPICKEXT(overtop, overtop, neogeo)
STD_ROM_FN(overtop)

struct BurnDriver BurnDrvovertop = {
	"overtop", NULL, "neogeo", NULL, "1996",
	"Over Top\0", NULL, "ADK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_RACING, 0,
	NULL, overtopRomInfo, overtopRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Neo Drift Out - New Technology

static struct BurnRomInfo neodriftRomDesc[] = {
	{ "213-p1.p1",    0x200000, 0xe397d798, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "213-s1.s1",    0x020000, 0xb76b61bc, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "213-c1.c1",    0x400000, 0x3edc8bd3, 3 | BRF_GRA },           //  2 Sprite data
	{ "213-c2.c2",    0x400000, 0x46ae5f16, 3 | BRF_GRA },           //  3 

	{ "213-m1.m1",    0x020000, 0x200045f1, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "213-v1.v1",    0x200000, 0xa421c076, 5 | BRF_SND },           //  5 Sound data
	{ "213-v2.v2",    0x200000, 0x233c7dd9, 5 | BRF_SND },           //  6 
};

STDROMPICKEXT(neodrift, neodrift, neogeo)
STD_ROM_FN(neodrift)

struct BurnDriver BurnDrvneodrift = {
	"neodrift", NULL, "neogeo", NULL, "1996",
	"Neo Drift Out - New Technology\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_RACING, 0,
	NULL, neodriftRomInfo, neodriftRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '96 (set 1)

static struct BurnRomInfo kof96RomDesc[] = {
	{ "214-p1.p1",    0x100000, 0x52755d74, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "214-p2.sp2",   0x200000, 0x002ccb73, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "214-s1.s1",    0x020000, 0x1254cbdb, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "214-c1.c1",    0x400000, 0x7ecf4aa2, 3 | BRF_GRA },           //  3 Sprite data
	{ "214-c2.c2",    0x400000, 0x05b54f37, 3 | BRF_GRA },           //  4 
	{ "214-c3.c3",    0x400000, 0x64989a65, 3 | BRF_GRA },           //  5 
	{ "214-c4.c4",    0x400000, 0xafbea515, 3 | BRF_GRA },           //  6 
	{ "214-c5.c5",    0x400000, 0x2a3bbd26, 3 | BRF_GRA },           //  7 
	{ "214-c6.c6",    0x400000, 0x44d30dc7, 3 | BRF_GRA },           //  8 
	{ "214-c7.c7",    0x400000, 0x3687331b, 3 | BRF_GRA },           //  9 
	{ "214-c8.c8",    0x400000, 0xfa1461ad, 3 | BRF_GRA },           // 10 

	{ "214-m1.m1",    0x020000, 0xdabc427c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "214-v1.v1",    0x400000, 0x63f7b045, 5 | BRF_SND },           // 12 Sound data
	{ "214-v2.v2",    0x400000, 0x25929059, 5 | BRF_SND },           // 13 
	{ "214-v3.v3",    0x200000, 0x92a2257d, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kof96, kof96, neogeo)
STD_ROM_FN(kof96)

struct BurnDriver BurnDrvkof96 = {
	"kof96", NULL, "neogeo", NULL, "1996",
	"The King of Fighters '96 (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof96RomInfo, kof96RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '96 (set 2)

static struct BurnRomInfo kof96hRomDesc[] = {
	{ "214-pg1.p1",   0x100000, 0xbd3757c9, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "214-p2.sp2",   0x200000, 0x002ccb73, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "214-s1.s1",    0x020000, 0x1254cbdb, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "214-c1.c1",    0x400000, 0x7ecf4aa2, 3 | BRF_GRA },           //  3 Sprite data
	{ "214-c2.c2",    0x400000, 0x05b54f37, 3 | BRF_GRA },           //  4 
	{ "214-c3.c3",    0x400000, 0x64989a65, 3 | BRF_GRA },           //  5 
	{ "214-c4.c4",    0x400000, 0xafbea515, 3 | BRF_GRA },           //  6 
	{ "214-c5.c5",    0x400000, 0x2a3bbd26, 3 | BRF_GRA },           //  7 
	{ "214-c6.c6",    0x400000, 0x44d30dc7, 3 | BRF_GRA },           //  8 
	{ "214-c7.c7",    0x400000, 0x3687331b, 3 | BRF_GRA },           //  9 
	{ "214-c8.c8",    0x400000, 0xfa1461ad, 3 | BRF_GRA },           // 10 

	{ "214-m1.m1",    0x020000, 0xdabc427c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "214-v1.v1",    0x400000, 0x63f7b045, 5 | BRF_SND },           // 12 Sound data
	{ "214-v2.v2",    0x400000, 0x25929059, 5 | BRF_SND },           // 13 
	{ "214-v3.v3",    0x200000, 0x92a2257d, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kof96h, kof96h, neogeo)
STD_ROM_FN(kof96h)

struct BurnDriver BurnDrvkof96h = {
	"kof96h", "kof96", "neogeo", NULL, "1996",
	"The King of Fighters '96 (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof96hRomInfo, kof96hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ultimate 11 - The SNK Football Championship / Tokuten Ou - Honoo no Libero, The

static struct BurnRomInfo ssideki4RomDesc[] = {
	{ "215-p1.p1",    0x200000, 0x519b4ba3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "215-s1.s1",    0x020000, 0xf0fe5c36, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "215-c1.c1",    0x400000, 0x8ff444f5, 3 | BRF_GRA },           //  2 Sprite data
	{ "215-c2.c2",    0x400000, 0x5b155037, 3 | BRF_GRA },           //  3 
	{ "215-c3.c3",    0x400000, 0x456a073a, 3 | BRF_GRA },           //  4 
	{ "215-c4.c4",    0x400000, 0x43c182e1, 3 | BRF_GRA },           //  5 
	{ "215-c5.c5",    0x200000, 0x0c6f97ec, 3 | BRF_GRA },           //  6 
	{ "215-c6.c6",    0x200000, 0x329c5e1b, 3 | BRF_GRA },           //  7 

	{ "215-m1.m1",    0x020000, 0xa932081d, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "215-v1.v1",    0x400000, 0x877d1409, 5 | BRF_SND },           //  9 Sound data
	{ "215-v2.v2",    0x200000, 0x1bfa218b, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(ssideki4, ssideki4, neogeo)
STD_ROM_FN(ssideki4)

struct BurnDriver BurnDrvssideki4 = {
	"ssideki4", NULL, "neogeo", NULL, "1996",
	"Ultimate 11 - The SNK Football Championship / Tokuten Ou - Honoo no Libero, The\0", NULL, "SNK", "Neo Geo",
	L"The Ultimate 11 - SNK football championship\0\u5F97\u70B9\u738B - \u708E\u306E\u30EA\u30D9\u30ED\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SPORTSFOOTBALL, 0,
	NULL, ssideki4RomInfo, ssideki4RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Kizuna Encounter - Super Tag Battle / Fu'un Super Tag Battle

static struct BurnRomInfo kizunaRomDesc[] = {
	{ "216-p1.p1",    0x200000, 0x75d2b3de, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "216-s1.s1",    0x020000, 0xefdc72d7, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "059-c1.c1",    0x200000, 0x763ba611, 3 | BRF_GRA },           //  2 Sprite data
	{ "059-c2.c2",    0x200000, 0xe05e8ca6, 3 | BRF_GRA },           //  3 
	{ "216-c3.c3",    0x400000, 0x665c9f16, 3 | BRF_GRA },           //  4 
	{ "216-c4.c4",    0x400000, 0x7f5d03db, 3 | BRF_GRA },           //  5 
	{ "059-c5.c5",    0x200000, 0x59013f9e, 3 | BRF_GRA },           //  6 
	{ "059-c6.c6",    0x200000, 0x1c8d5def, 3 | BRF_GRA },           //  7 
	{ "059-c7.c7",    0x200000, 0xc88f7035, 3 | BRF_GRA },           //  8 
	{ "059-c8.c8",    0x200000, 0x484ce3ba, 3 | BRF_GRA },           //  9 

	{ "216-m1.m1",    0x020000, 0x1b096820, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "059-v1.v1",    0x200000, 0x530c50fd, 5 | BRF_SND },           // 11 Sound data
	{ "216-v2.v2",    0x200000, 0x03667a8d, 5 | BRF_SND },           // 12 
	{ "059-v3.v3",    0x200000, 0x7038c2f9, 5 | BRF_SND },           // 13 
	{ "216-v4.v4",    0x200000, 0x31b99bd6, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kizuna, kizuna, neogeo)
STD_ROM_FN(kizuna)

struct BurnDriver BurnDrvkizuna = {
	"kizuna", NULL, "neogeo", NULL, "1996",
	"Kizuna Encounter - Super Tag Battle / Fu'un Super Tag Battle\0", NULL, "SNK", "Neo Geo",
	L"Kizuna Encounter - super tag battle\0\u98A8\u96F2 Super Tag Battle\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, kizunaRomInfo, kizunaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ninja Master's - haoh-ninpo-cho

static struct BurnRomInfo ninjamasRomDesc[] = {
	{ "217-p1.p1",    0x100000, 0x3e97ed69, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "217-p2.sp2",   0x200000, 0x191fca88, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "217-s1.s1",    0x020000, 0x8ff782f0, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "217-c1.c1",    0x400000, 0x5fe97bc4, 3 | BRF_GRA },           //  3 Sprite data
	{ "217-c2.c2",    0x400000, 0x886e0d66, 3 | BRF_GRA },           //  4 
	{ "217-c3.c3",    0x400000, 0x59e8525f, 3 | BRF_GRA },           //  5 
	{ "217-c4.c4",    0x400000, 0x8521add2, 3 | BRF_GRA },           //  6 
	{ "217-c5.c5",    0x400000, 0xfb1896e5, 3 | BRF_GRA },           //  7 
	{ "217-c6.c6",    0x400000, 0x1c98c54b, 3 | BRF_GRA },           //  8 
	{ "217-c7.c7",    0x400000, 0x8b0ede2e, 3 | BRF_GRA },           //  9 
	{ "217-c8.c8",    0x400000, 0xa085bb61, 3 | BRF_GRA },           // 10 

	{ "217-m1.m1",    0x020000, 0xd00fb2af, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "217-v1.v1",    0x400000, 0x1c34e013, 5 | BRF_SND },           // 12 Sound data
	{ "217-v2.v2",    0x200000, 0x22f1c681, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(ninjamas, ninjamas, neogeo)
STD_ROM_FN(ninjamas)

struct BurnDriver BurnDrvninjamas = {
	"ninjamas", NULL, "neogeo", NULL, "1996",
	"Ninja Master's - haoh-ninpo-cho\0", NULL, "ADK / SNK", "Neo Geo",
	L"Ninja master's \u8987\u738B\u5FCD\u6CD5\u5E16\0Ninja Master's haoh ninpo cho\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, ninjamasRomInfo, ninjamasRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Ragnagard / Shin-Oh-Ken

static struct BurnRomInfo ragnagrdRomDesc[] = {
	{ "218-p1.p1",    0x200000, 0xca372303, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "218-s1.s1",    0x020000, 0x7d402f9a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "218-c1.c1",    0x400000, 0xc31500a4, 3 | BRF_GRA },           //  2 Sprite data
	{ "218-c2.c2",    0x400000, 0x98aba1f9, 3 | BRF_GRA },           //  3 
	{ "218-c3.c3",    0x400000, 0x833c163a, 3 | BRF_GRA },           //  4 
	{ "218-c4.c4",    0x400000, 0xc1a30f69, 3 | BRF_GRA },           //  5 
	{ "218-c5.c5",    0x400000, 0x6b6de0ff, 3 | BRF_GRA },           //  6 
	{ "218-c6.c6",    0x400000, 0x94beefcf, 3 | BRF_GRA },           //  7 
	{ "218-c7.c7",    0x400000, 0xde6f9b28, 3 | BRF_GRA },           //  8 
	{ "218-c8.c8",    0x400000, 0xd9b311f6, 3 | BRF_GRA },           //  9 

	{ "218-m1.m1",    0x020000, 0x17028bcf, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "218-v1.v1",    0x400000, 0x61eee7f4, 5 | BRF_SND },           // 11 Sound data
	{ "218-v2.v2",    0x400000, 0x6104e20b, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(ragnagrd, ragnagrd, neogeo)
STD_ROM_FN(ragnagrd)

struct BurnDriver BurnDrvragnagrd = {
	"ragnagrd", NULL, "neogeo", NULL, "1996",
	"Ragnagard / Shin-Oh-Ken\0", NULL, "Saurus", "Neo Geo",
	L"Operation Ragnagard\0\u795E\u51F0\u62F3\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, ragnagrdRomInfo, ragnagrdRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Pleasure Goal / Futsal - 5 on 5 Mini Soccer

static struct BurnRomInfo pgoalRomDesc[] = {
	{ "219-p1.p1",    0x200000, 0x6af0e574, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "219-s1.s1",    0x020000, 0x002f3c88, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "219-c1.c1",    0x400000, 0x67fec4dc, 3 | BRF_GRA },           //  2 Sprite data
	{ "219-c2.c2",    0x400000, 0x86ed01f2, 3 | BRF_GRA },           //  3 
	{ "219-c3.c3",    0x200000, 0x5fdad0a5, 3 | BRF_GRA },           //  4 
	{ "219-c4.c4",    0x200000, 0xf57b4a1c, 3 | BRF_GRA },           //  5 

	{ "219-m1.m1",    0x020000, 0x958efdc8, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "219-v1.v1",    0x400000, 0xd0ae33d9, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(pgoal, pgoal, neogeo)
STD_ROM_FN(pgoal)

struct BurnDriver BurnDrvpgoal = {
	"pgoal", NULL, "neogeo", NULL, "1996",
	"Pleasure Goal / Futsal - 5 on 5 Mini Soccer\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SPORTSFOOTBALL, 0,
	NULL, pgoalRomInfo, pgoalRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Magical Drop II

static struct BurnRomInfo magdrop2RomDesc[] = {
	{ "221-p1.p1",    0x080000, 0x7be82353, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "221-s1.s1",    0x020000, 0x2a4063a3, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "221-c1.c1",    0x400000, 0x1f862a14, 3 | BRF_GRA },           //  2 Sprite data
	{ "221-c2.c2",    0x400000, 0x14b90536, 3 | BRF_GRA },           //  3 

	{ "221-m1.m1",    0x020000, 0xbddae628, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "221-v1.v1",    0x200000, 0x7e5e53e4, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(magdrop2, magdrop2, neogeo)
STD_ROM_FN(magdrop2)

struct BurnDriver BurnDrvmagdrop2 = {
	"magdrop2", NULL, "neogeo", NULL, "1996",
	"Magical Drop II\0", NULL, "Data East Corporation", "Neo Geo",
	L"Magical Drop II\0\u30DE\u30B8\u30AB\u30EB\u30C9\u30ED\u30C3\u30D7\uFF12\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, magdrop2RomInfo, magdrop2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown IV - Amakusa's Revenge / Samurai Spirits - Amakusa Kourin

static struct BurnRomInfo samsho4RomDesc[] = {
	{ "222-p1.p1",    0x100000, 0x1a5cb56d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "222-p2.sp2",   0x400000, 0xb023cd8b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "222-s1.s1",    0x020000, 0x8d3d3bf9, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "222-c1.c1",    0x400000, 0x68f2ed95, 3 | BRF_GRA },           //  3 Sprite data
	{ "222-c2.c2",    0x400000, 0xa6e9aff0, 3 | BRF_GRA },           //  4 
	{ "222-c3.c3",    0x400000, 0xc91b40f4, 3 | BRF_GRA },           //  5 
	{ "222-c4.c4",    0x400000, 0x359510a4, 3 | BRF_GRA },           //  6 
	{ "222-c5.c5",    0x400000, 0x9cfbb22d, 3 | BRF_GRA },           //  7 
	{ "222-c6.c6",    0x400000, 0x685efc32, 3 | BRF_GRA },           //  8 
	{ "222-c7.c7",    0x400000, 0xd0f86f0d, 3 | BRF_GRA },           //  9 
	{ "222-c8.c8",    0x400000, 0xadfc50e3, 3 | BRF_GRA },           // 10 

	{ "222-m1.m1",    0x020000, 0x7615bc1b, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "222-v1.v1",    0x400000, 0x7d6ba95f, 5 | BRF_SND },           // 12 Sound data
	{ "222-v2.v2",    0x400000, 0x6c33bb5d, 5 | BRF_SND },           // 13 
	{ "222-v3.v3",    0x200000, 0x831ea8c0, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(samsho4, samsho4, neogeo)
STD_ROM_FN(samsho4)

struct BurnDriver BurnDrvsamsho4 = {
	"samsho4", NULL, "neogeo", NULL, "1996",
	"Samurai Shodown IV - Amakusa's Revenge / Samurai Spirits - Amakusa Kourin\0", NULL, "SNK", "Neo Geo",
	L"Samurai Shodown IV - Amakusa's revenge\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4 - \u5929\u8349\u964D\u81E8\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho4RomInfo, samsho4RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Pae Wang Jeon Seol / Legend of a Warrior (Korean censored Samurai Shodown IV)

static struct BurnRomInfo samsho4kRomDesc[] = {
	{ "222-p1k.p1",   0x100000, 0x06e0a25d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "222-p2.sp2",   0x400000, 0xb023cd8b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "222-s1k.s1",   0x020000, 0xd313687d, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "222-c1.c1",    0x400000, 0x68f2ed95, 3 | BRF_GRA },           //  3 Sprite data
	{ "222-c2.c2",    0x400000, 0xa6e9aff0, 3 | BRF_GRA },           //  4 
	{ "222-c3.c3",    0x400000, 0xc91b40f4, 3 | BRF_GRA },           //  5 
	{ "222-c4.c4",    0x400000, 0x359510a4, 3 | BRF_GRA },           //  6 
	{ "222-c5.c5",    0x400000, 0x9cfbb22d, 3 | BRF_GRA },           //  7 
	{ "222-c6.c6",    0x400000, 0x685efc32, 3 | BRF_GRA },           //  8 
	{ "222-c7.c7",    0x400000, 0xd0f86f0d, 3 | BRF_GRA },           //  9 
	{ "222-c8.c8",    0x400000, 0xadfc50e3, 3 | BRF_GRA },           // 10 

	{ "222-m1.m1",    0x020000, 0x7615bc1b, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "222-v1.v1",    0x400000, 0x7d6ba95f, 5 | BRF_SND },           // 12 Sound data
	{ "222-v2.v2",    0x400000, 0x6c33bb5d, 5 | BRF_SND },           // 13 
	{ "222-v3.v3",    0x200000, 0x831ea8c0, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(samsho4k, samsho4k, neogeo)
STD_ROM_FN(samsho4k)

struct BurnDriver BurnDrvsamsho4k = {
	"samsho4k", "samsho4", "neogeo", NULL, "1996",
	"Pae Wang Jeon Seol / Legend of a Warrior (Korean censored Samurai Shodown IV)\0", NULL, "SNK", "Neo Geo",
	L"Pae Wang Jeon Seol / Legend of a Warrior (Korean censored Samurai Shodown IV)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho4kRomInfo, samsho4kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Real Bout Fatal Fury Special / Real Bout Garou Densetsu Special

static struct BurnRomInfo rbffspecRomDesc[] = {
	{ "223-p1.p1",    0x100000, 0xf84a2d1d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "223-p2.sp2",   0x400000, 0xaddd8f08, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "223-s1.s1",    0x020000, 0x7ecd6e8c, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "223-c1.c1",    0x400000, 0xebab05e2, 3 | BRF_GRA },           //  3 Sprite data
	{ "223-c2.c2",    0x400000, 0x641868c3, 3 | BRF_GRA },           //  4 
	{ "223-c3.c3",    0x400000, 0xca00191f, 3 | BRF_GRA },           //  5 
	{ "223-c4.c4",    0x400000, 0x1f23d860, 3 | BRF_GRA },           //  6 
	{ "223-c5.c5",    0x400000, 0x321e362c, 3 | BRF_GRA },           //  7 
	{ "223-c6.c6",    0x400000, 0xd8fcef90, 3 | BRF_GRA },           //  8 
	{ "223-c7.c7",    0x400000, 0xbc80dd2d, 3 | BRF_GRA },           //  9 
	{ "223-c8.c8",    0x400000, 0x5ad62102, 3 | BRF_GRA },           // 10 

	{ "223-m1.m1",    0x020000, 0x3fee46bf, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "223-v1.v1",    0x400000, 0x76673869, 5 | BRF_SND },           // 12 Sound data
	{ "223-v2.v2",    0x400000, 0x7a275acd, 5 | BRF_SND },           // 13 
	{ "223-v3.v3",    0x400000, 0x5a797fd2, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(rbffspec, rbffspec, neogeo)
STD_ROM_FN(rbffspec)

struct BurnDriver BurnDrvrbffspec = {
	"rbffspec", NULL, "neogeo", NULL, "1996",
	"Real Bout Fatal Fury Special / Real Bout Garou Densetsu Special\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, rbffspecRomInfo, rbffspecRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Real Bout Fatal Fury Special / Real Bout Garou Densetsu Special (Korea)

static struct BurnRomInfo rbffspeckRomDesc[] = {
	{ "223-p1k.p1",   0x100000, 0xb78c8391, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "223-p2.sp2",   0x400000, 0xaddd8f08, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "223-s1.s1",    0x020000, 0x7ecd6e8c, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "223-c1.c1",    0x400000, 0xebab05e2, 3 | BRF_GRA },           //  3 Sprite data
	{ "223-c2.c2",    0x400000, 0x641868c3, 3 | BRF_GRA },           //  4 
	{ "223-c3.c3",    0x400000, 0xca00191f, 3 | BRF_GRA },           //  5 
	{ "223-c4.c4",    0x400000, 0x1f23d860, 3 | BRF_GRA },           //  6 
	{ "223-c5.c5",    0x400000, 0x321e362c, 3 | BRF_GRA },           //  7 
	{ "223-c6.c6",    0x400000, 0xd8fcef90, 3 | BRF_GRA },           //  8 
	{ "223-c7.c7",    0x400000, 0xbc80dd2d, 3 | BRF_GRA },           //  9 
	{ "223-c8.c8",    0x400000, 0x5ad62102, 3 | BRF_GRA },           // 10 

	{ "223-m1.m1",    0x020000, 0x3fee46bf, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "223-v1.v1",    0x400000, 0x76673869, 5 | BRF_SND },           // 12 Sound data
	{ "223-v2.v2",    0x400000, 0x7a275acd, 5 | BRF_SND },           // 13 
	{ "223-v3.v3",    0x400000, 0x5a797fd2, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(rbffspeck, rbffspeck, neogeo)
STD_ROM_FN(rbffspeck)

struct BurnDriver BurnDrvrbffspeck = {
	"rbffspeck", "rbffspec", "neogeo", NULL, "1996",
	"Real Bout Fatal Fury Special / Real Bout Garou Densetsu Special (Korea)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, rbffspeckRomInfo, rbffspeckRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Twinkle Star Sprites

static struct BurnRomInfo twinspriRomDesc[] = {
	{ "224-p1.p1",    0x200000, 0x7697e445, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "224-s1.s1",    0x020000, 0xeeed5758, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "224-c1.c1",    0x400000, 0xf7da64ab, 3 | BRF_GRA },           //  2 Sprite data
	{ "224-c2.c2",    0x400000, 0x4c09bbfb, 3 | BRF_GRA },           //  3 
	{ "224-c3.c3",    0x100000, 0xc59e4129, 3 | BRF_GRA },           //  4 
	{ "224-c4.c4",    0x100000, 0xb5532e53, 3 | BRF_GRA },           //  5 

	{ "224-m1.m1",    0x020000, 0x364d6f96, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "224-v1.v1",    0x400000, 0xff57f088, 5 | BRF_SND },           //  7 Sound data
	{ "224-v2.v2",    0x200000, 0x7ad26599, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(twinspri, twinspri, neogeo)
STD_ROM_FN(twinspri)

struct BurnDriver BurnDrvtwinspri = {
	"twinspri", NULL, "neogeo", NULL, "1996",
	"Twinkle Star Sprites\0", NULL, "ADK", "Neo Geo",
	L"TwinkleStar Sprites\0\u30C6\u30A3\u30F3\u30AF\u30EB\u30B9\u30BF\u30FC\u30B9\u30D7\u30E9\u30A4\u30C4\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VERSHOOT, 0,
	NULL, twinspriRomInfo, twinspriRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Waku Waku 7

static struct BurnRomInfo wakuwak7RomDesc[] = {
	{ "225-p1.p1",    0x100000, 0xb14da766, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "225-p2.sp2",   0x200000, 0xfe190665, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "225-s1.s1",    0x020000, 0x71c4b4b5, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "225-c1.c1",    0x400000, 0xee4fea54, 3 | BRF_GRA },           //  3 Sprite data
	{ "225-c2.c2",    0x400000, 0x0c549e2d, 3 | BRF_GRA },           //  4 
	{ "225-c3.c3",    0x400000, 0xaf0897c0, 3 | BRF_GRA },           //  5 
	{ "225-c4.c4",    0x400000, 0x4c66527a, 3 | BRF_GRA },           //  6 
	{ "225-c5.c5",    0x400000, 0x8ecea2b5, 3 | BRF_GRA },           //  7 
	{ "225-c6.c6",    0x400000, 0x0eb11a6d, 3 | BRF_GRA },           //  8 

	{ "225-m1.m1",    0x020000, 0x0634bba6, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "225-v1.v1",    0x400000, 0x6195c6b4, 5 | BRF_SND },           // 10 Sound data
	{ "225-v2.v2",    0x400000, 0x6159c5fe, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(wakuwak7, wakuwak7, neogeo)
STD_ROM_FN(wakuwak7)

struct BurnDriver BurnDrvwakuwak7 = {
	"wakuwak7", NULL, "neogeo", NULL, "1996",
	"Waku Waku 7\0", NULL, "Sunsoft", "Neo Geo",
	L"Waku Waku 7\0\u308F\u304F\u308F\u304F\uFF17\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, wakuwak7RomInfo, wakuwak7RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Stakes Winner 2

static struct BurnRomInfo stakwin2RomDesc[] = {
	{ "227-p1.p1",    0x200000, 0xdaf101d2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "227-s1.s1",    0x020000, 0x2a8c4462, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "227-c1.c1",    0x400000, 0x7d6c2af4, 3 | BRF_GRA },           //  2 Sprite data
	{ "227-c2.c2",    0x400000, 0x7e402d39, 3 | BRF_GRA },           //  3 
	{ "227-c3.c3",    0x200000, 0x93dfd660, 3 | BRF_GRA },           //  4 
	{ "227-c4.c4",    0x200000, 0x7efea43a, 3 | BRF_GRA },           //  5 

	{ "227-m1.m1",    0x020000, 0xc8e5e0f9, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "227-v1.v1",    0x400000, 0xb8f24181, 5 | BRF_SND },           //  7 Sound data
	{ "227-v2.v2",    0x400000, 0xee39e260, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(stakwin2, stakwin2, neogeo)
STD_ROM_FN(stakwin2)

struct BurnDriver BurnDrvstakwin2 = {
	"stakwin2", NULL, "neogeo", NULL, "1996",
	"Stakes Winner 2\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_MISC, 0,
	NULL, stakwin2RomInfo, stakwin2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Ghostlop (prototype)

static struct BurnRomInfo ghostlopRomDesc[] = {
	{ "228-p1p.p1",   0x100000, 0x6033172e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "228-s1p.s1",   0x020000, 0x83c24e81, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "228-c1p.c1",   0x400000, 0xbfc99efe, 3 | BRF_GRA },           //  2 Sprite data
	{ "228-c2p.c2",   0x400000, 0x69788082, 3 | BRF_GRA },           //  3 

	{ "228-m1p.m1",   0x020000, 0xfd833b33, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "228-v1p.v1",   0x200000, 0xc603fce6, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(ghostlop, ghostlop, neogeo)
STD_ROM_FN(ghostlop)

struct BurnDriver BurnDrvghostlop = {
	"ghostlop", NULL, "neogeo", NULL, "1996",
	"Ghostlop (prototype)\0", NULL, "Data East Corporation", "Neo Geo",
	L"Ghostlop\0Ghostlop \u30B4\u30FC\u30B9\u30C8\u30ED\u30C3\u30D7\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_PROTOTYPE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, ghostlopRomInfo, ghostlopRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Breakers

static struct BurnRomInfo breakersRomDesc[] = {
	{ "230-p1.p1",    0x200000, 0xed24a6e6, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "230-s1.s1",    0x020000, 0x076fb64c, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "230-c1.c1",    0x400000, 0x68d4ae76, 3 | BRF_GRA },           //  2 Sprite data
	{ "230-c2.c2",    0x400000, 0xfdee05cd, 3 | BRF_GRA },           //  3 
	{ "230-c3.c3",    0x400000, 0x645077f3, 3 | BRF_GRA },           //  4 
	{ "230-c4.c4",    0x400000, 0x63aeb74c, 3 | BRF_GRA },           //  5 

	{ "230-m1.m1",    0x020000, 0x3951a1c1, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "230-v1.v1",    0x400000, 0x7f9ed279, 5 | BRF_SND },           //  7 Sound data
	{ "230-v2.v2",    0x400000, 0x1d43e420, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(breakers, breakers, neogeo)
STD_ROM_FN(breakers)

struct BurnDriver BurnDrvbreakers = {
	"breakers", NULL, "neogeo", NULL, "1996",
	"Breakers\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, breakersRomInfo, breakersRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Money Puzzle Exchanger / Money Idol Exchanger

static struct BurnRomInfo miexchngRomDesc[] = {
	{ "231-p1.p1",    0x080000, 0x61be1810, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "231-s1.s1",    0x020000, 0xfe0c0c53, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "231-c1.c1",    0x200000, 0x6c403ba3, 3 | BRF_GRA },           //  2 Sprite data
	{ "231-c2.c2",    0x200000, 0x554bcd9b, 3 | BRF_GRA },           //  3 
	{ "231-c3.c3",    0x100000, 0x4f6f7a63, 3 | BRF_GRA },           //  4 
	{ "231-c4.c4",    0x100000, 0x2e35e71b, 3 | BRF_GRA },           //  5 

	{ "231-m1.m1",    0x020000, 0xde41301b, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "231-v1.v1",    0x400000, 0x113fb898, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(miexchng, miexchng, neogeo)
STD_ROM_FN(miexchng)

struct BurnDriver BurnDrvmiexchng = {
	"miexchng", NULL, "neogeo", NULL, "1997",
	"Money Puzzle Exchanger / Money Idol Exchanger\0", NULL, "Face", "Neo Geo",
	L"Money Puzzle Exchanger\0\u30DE\u30CD\u30FC\u30A2\u30A4\u30C9\u30EB\u30A8\u30AF\u30B9\u30C1\u30A7\u30F3\u30B8\u30E3\u30FC\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, miexchngRomInfo, miexchngRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '97 (set 1)

static struct BurnRomInfo kof97RomDesc[] = {
	{ "232-p1.p1",    0x100000, 0x7db81ad9, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "232-p2.sp2",   0x400000, 0x158b23f6, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "232-s1.s1",    0x020000, 0x8514ecf5, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "232-c1.c1",    0x800000, 0x5f8bf0a1, 3 | BRF_GRA },           //  3 Sprite data
	{ "232-c2.c2",    0x800000, 0xe4d45c81, 3 | BRF_GRA },           //  4 
	{ "232-c3.c3",    0x800000, 0x581d6618, 3 | BRF_GRA },           //  5 
	{ "232-c4.c4",    0x800000, 0x49bb1e68, 3 | BRF_GRA },           //  6 
	{ "232-c5.c5",    0x400000, 0x34fc4e51, 3 | BRF_GRA },           //  7 
	{ "232-c6.c6",    0x400000, 0x4ff4d47b, 3 | BRF_GRA },           //  8 

	{ "232-m1.m1",    0x020000, 0x45348747, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.v1",    0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.v2",    0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.v3",    0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof97, kof97, neogeo)
STD_ROM_FN(kof97)

struct BurnDriver BurnDrvkof97 = {
	"kof97", NULL, "neogeo", NULL, "1997",
	"The King of Fighters '97 (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof97RomInfo, kof97RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '97 (set 2)

static struct BurnRomInfo kof97hRomDesc[] = {
	{ "232-pg1.p1",   0x100000, 0x5c2400b7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "232-p2.sp2",   0x400000, 0x158b23f6, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "232-s1.s1",    0x020000, 0x8514ecf5, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "232-c1.c1",    0x800000, 0x5f8bf0a1, 3 | BRF_GRA },           //  3 Sprite data
	{ "232-c2.c2",    0x800000, 0xe4d45c81, 3 | BRF_GRA },           //  4 
	{ "232-c3.c3",    0x800000, 0x581d6618, 3 | BRF_GRA },           //  5 
	{ "232-c4.c4",    0x800000, 0x49bb1e68, 3 | BRF_GRA },           //  6 
	{ "232-c5.c5",    0x400000, 0x34fc4e51, 3 | BRF_GRA },           //  7 
	{ "232-c6.c6",    0x400000, 0x4ff4d47b, 3 | BRF_GRA },           //  8 

	{ "232-m1.m1",    0x020000, 0x45348747, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.v1",    0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.v2",    0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.v3",    0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof97h, kof97h, neogeo)
STD_ROM_FN(kof97h)

struct BurnDriver BurnDrvkof97h = {
	"kof97h", "kof97", "neogeo", NULL, "1997",
	"The King of Fighters '97 (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof97hRomInfo, kof97hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '97 (Korean release)

static struct BurnRomInfo kof97kRomDesc[] = {
	{ "232-p1.p1",    0x100000, 0x7db81ad9, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "232-p2.sp2",   0x400000, 0x158b23f6, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "232-s1.s1",    0x020000, 0x8514ecf5, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "232-c1.c1",    0x800000, 0x5f8bf0a1, 3 | BRF_GRA },           //  3 Sprite data
	{ "232-c2.c2",    0x800000, 0xe4d45c81, 3 | BRF_GRA },           //  4 
	{ "232-c3.c3",    0x800000, 0x581d6618, 3 | BRF_GRA },           //  5 
	{ "232-c4.c4",    0x800000, 0x49bb1e68, 3 | BRF_GRA },           //  6 
	{ "232-c5.c5",    0x400000, 0x34fc4e51, 3 | BRF_GRA },           //  7 
	{ "232-c6.c6",    0x400000, 0x4ff4d47b, 3 | BRF_GRA },           //  8 

	{ "232-m1k.m1",   0x020000, 0xbbea9070, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.v1",    0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.v2",    0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.v3",    0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof97k, kof97k, neogeo)
STD_ROM_FN(kof97k)

struct BurnDriver BurnDrvkof97k = {
	"kof97k", "kof97", "neogeo", NULL, "1997",
	"The King of Fighters '97 (Korean release)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof97kRomInfo, kof97kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '97 Plus (bootleg)

static struct BurnRomInfo kof97plsRomDesc[] = {
	{ "kf97-p1p.bin", 0x100000, 0xc01fda46, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "kf97-p2p.bin", 0x400000, 0x5502b020, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "kf97-s1p.bin", 0x020000, 0x73254270, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "232-c1.c1",    0x800000, 0x5f8bf0a1, 3 | BRF_GRA },           //  3 Sprite data
	{ "232-c2.c2",    0x800000, 0xe4d45c81, 3 | BRF_GRA },           //  4 
	{ "232-c3.c3",    0x800000, 0x581d6618, 3 | BRF_GRA },           //  5 
	{ "232-c4.c4",    0x800000, 0x49bb1e68, 3 | BRF_GRA },           //  6 
	{ "232-c5.c5",    0x400000, 0x34fc4e51, 3 | BRF_GRA },           //  7 
	{ "232-c6.c6",    0x400000, 0x4ff4d47b, 3 | BRF_GRA },           //  8 

	{ "232-m1.m1",    0x020000, 0x45348747, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.v1",    0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.v2",    0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.v3",    0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof97pls, kof97pls, neogeo)
STD_ROM_FN(kof97pls)

struct BurnDriver BurnDrvkof97pls = {
	"kof97pls", "kof97", "neogeo", NULL, "1997",
	"The King of Fighters '97 Plus (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof97plsRomInfo, kof97plsRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Magical Drop III

static struct BurnRomInfo magdrop3RomDesc[] = {
	{ "233-p1.p1",    0x100000, 0x931e17fa, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "233-s1.s1",    0x020000, 0x7399e68a, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "233-c1.c1",    0x400000, 0x65e3f4c4, 3 | BRF_GRA },           //  2 Sprite data
	{ "233-c2.c2",    0x400000, 0x35dea6c9, 3 | BRF_GRA },           //  3 
	{ "233-c3.c3",    0x400000, 0x0ba2c502, 3 | BRF_GRA },           //  4 
	{ "233-c4.c4",    0x400000, 0x70dbbd6d, 3 | BRF_GRA },           //  5 

	{ "233-m1.m1",    0x020000, 0x5beaf34e, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "233-v1.v1",    0x400000, 0x58839298, 5 | BRF_SND },           //  7 Sound data
	{ "233-v2.v2",    0x080000, 0xd5e30df4, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(magdrop3, magdrop3, neogeo)
STD_ROM_FN(magdrop3)

struct BurnDriver BurnDrvmagdrop3 = {
	"magdrop3", NULL, "neogeo", NULL, "1997",
	"Magical Drop III\0", NULL, "Data East Corporation", "Neo Geo",
	L"Magical Drop III\0\u30DE\u30B8\u30AB\u30EB\u30C9\u30ED\u30C3\u30D7III\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, magdrop3RomInfo, magdrop3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Last Blade / Bakumatsu Roman - Gekka no Kenshi, The (set 1)

static struct BurnRomInfo lastbladRomDesc[] = {
	{ "234-p1.p1",    0x100000, 0xe123a5a3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "234-p2.sp2",   0x400000, 0x0fdc289e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "234-s1.s1",    0x020000, 0x95561412, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "234-c1.c1",    0x800000, 0x9f7e2bd3, 3 | BRF_GRA },           //  3 Sprite data
	{ "234-c2.c2",    0x800000, 0x80623d3c, 3 | BRF_GRA },           //  4 
	{ "234-c3.c3",    0x800000, 0x91ab1a30, 3 | BRF_GRA },           //  5 
	{ "234-c4.c4",    0x800000, 0x3d60b037, 3 | BRF_GRA },           //  6 
	{ "234-c5.c5",    0x400000, 0x1ba80cee, 3 | BRF_GRA },           //  7 
	{ "234-c6.c6",    0x400000, 0xbeafd091, 3 | BRF_GRA },           //  8 

	{ "234-m1.m1",    0x020000, 0x087628ea, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "234-v1.v1",    0x400000, 0xed66b76f, 5 | BRF_SND },           // 10 Sound data
	{ "234-v2.v2",    0x400000, 0xa0e7f6e2, 5 | BRF_SND },           // 11 
	{ "234-v3.v3",    0x400000, 0xa506e1e2, 5 | BRF_SND },           // 12 
	{ "234-v4.v4",    0x400000, 0x0e34157f, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(lastblad, lastblad, neogeo)
STD_ROM_FN(lastblad)

struct BurnDriver BurnDrvlastblad = {
	"lastblad", NULL, "neogeo", NULL, "1997",
	"Last Blade / Bakumatsu Roman - Gekka no Kenshi, The (set 1)\0", NULL, "SNK", "Neo Geo",
	L"The Last Blade (set 1)\0\u5E55\u672B\u6D6A\u6F2B \u6708\u83EF\u306E\u5263\u58EB (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, lastbladRomInfo, lastbladRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Last Blade / Bakumatsu Roman - Gekka no Kenshi, The (set 2)

static struct BurnRomInfo lastbladhRomDesc[] = {
	{ "234-pg1.p1",   0x100000, 0xcd01c06d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "234-p2.sp2",   0x400000, 0x0fdc289e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "234-s1.s1",    0x020000, 0x95561412, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "234-c1.c1",    0x800000, 0x9f7e2bd3, 3 | BRF_GRA },           //  3 Sprite data
	{ "234-c2.c2",    0x800000, 0x80623d3c, 3 | BRF_GRA },           //  4 
	{ "234-c3.c3",    0x800000, 0x91ab1a30, 3 | BRF_GRA },           //  5 
	{ "234-c4.c4",    0x800000, 0x3d60b037, 3 | BRF_GRA },           //  6 
	{ "234-c5.c5",    0x400000, 0x1ba80cee, 3 | BRF_GRA },           //  7 
	{ "234-c6.c6",    0x400000, 0xbeafd091, 3 | BRF_GRA },           //  8 

	{ "234-m1.m1",    0x020000, 0x087628ea, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "234-v1.v1",    0x400000, 0xed66b76f, 5 | BRF_SND },           // 10 Sound data
	{ "234-v2.v2",    0x400000, 0xa0e7f6e2, 5 | BRF_SND },           // 11 
	{ "234-v3.v3",    0x400000, 0xa506e1e2, 5 | BRF_SND },           // 12 
	{ "234-v4.v4",    0x400000, 0x0e34157f, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(lastbladh, lastbladh, neogeo)
STD_ROM_FN(lastbladh)

struct BurnDriver BurnDrvlastbladh = {
	"lastbladh", "lastblad", "neogeo", NULL, "1997",
	"Last Blade / Bakumatsu Roman - Gekka no Kenshi, The (set 2)\0", NULL, "SNK", "Neo Geo",
	L"The Last Blade (set 2)\0\u5E55\u672B\u6D6A\u6F2B \u6708\u83EF\u306E\u5263\u58EB (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, lastbladhRomInfo, lastbladhRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// The Last Soldier (Korean release of The Last Blade)

static struct BurnRomInfo lastsoldRomDesc[] = {
	{ "134-p1k.p1",   0x100000, 0x906f3065, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "234-p2.sp2",   0x400000, 0x0fdc289e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "234-s1.s1",    0x020000, 0x95561412, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "234-c1.c1",    0x800000, 0x9f7e2bd3, 3 | BRF_GRA },           //  3 Sprite data
	{ "234-c2.c2",    0x800000, 0x80623d3c, 3 | BRF_GRA },           //  4 
	{ "234-c3.c3",    0x800000, 0x91ab1a30, 3 | BRF_GRA },           //  5 
	{ "234-c4.c4",    0x800000, 0x3d60b037, 3 | BRF_GRA },           //  6 
	{ "234-c5.c5",    0x400000, 0x1ba80cee, 3 | BRF_GRA },           //  7 
	{ "234-c6.c6",    0x400000, 0xbeafd091, 3 | BRF_GRA },           //  8 

	{ "234-m1.m1",    0x020000, 0x087628ea, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "234-v1.v1",    0x400000, 0xed66b76f, 5 | BRF_SND },           // 10 Sound data
	{ "234-v2.v2",    0x400000, 0xa0e7f6e2, 5 | BRF_SND },           // 11 
	{ "234-v3.v3",    0x400000, 0xa506e1e2, 5 | BRF_SND },           // 12 
	{ "234-v4.v4",    0x400000, 0x0e34157f, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(lastsold, lastsold, neogeo)
STD_ROM_FN(lastsold)

struct BurnDriver BurnDrvlastsold = {
	"lastsold", "lastblad", "neogeo", NULL, "1997",
	"The Last Soldier (Korean release of The Last Blade)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, lastsoldRomInfo, lastsoldRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Puzzle De Pon! R!

static struct BurnRomInfo puzzldprRomDesc[] = {
	{ "235-p1.p1",    0x080000, 0xafed5de2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "235-s1.s1",    0x010000, 0x5a68d91e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "202-c1.c1",    0x100000, 0xcc0095ef, 3 | BRF_GRA },           //  2 Sprite data
	{ "202-c2.c2",    0x100000, 0x42371307, 3 | BRF_GRA },           //  3 

	{ "202-m1.m1",    0x020000, 0x9c0291ea, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "202-v1.v1",    0x080000, 0xdebeb8fb, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(puzzldpr, puzzldpr, neogeo)
STD_ROM_FN(puzzldpr)

struct BurnDriver BurnDrvpuzzldpr = {
	"puzzldpr", "puzzledp", "neogeo", NULL, "1997",
	"Puzzle De Pon! R!\0", NULL, "Taito (Visco license)", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, puzzldprRomInfo, puzzldprRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The Irritating Maze / Ultra Denryu Iraira Bou

static struct BurnRomInfo irrmazeRomDesc[] = {
	{ "236-p1.p1",    0x200000, 0x4c2ff660, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "236-s1.s1",    0x020000, 0x5d1ca640, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "236-c1.c1",    0x400000, 0xc1d47902, 3 | BRF_GRA },           //  2 Sprite data
	{ "236-c2.c2",    0x400000, 0xe15f972e, 3 | BRF_GRA },           //  3 

	{ "236-m1.m1",    0x020000, 0x880a1abd, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "236-v1.v1",    0x200000, 0x5f89c3b4, 5 | BRF_SND },           //  5 Sound data
	{ "236-v2.v2",    0x100000, 0x1e843567, 6 | BRF_SND },           //  6 

	{ "236-bios.sp1", 0x020000, 0x853e6b96, 8 | BRF_BIOS },          //  7 
};

STDROMPICKEXT(irrmaze, irrmaze, neotrackball)
STD_ROM_FN(irrmaze)

struct BurnDriver BurnDrvirrmaze = {
	"irrmaze", NULL, "neogeo", NULL, "1997",
	"The Irritating Maze / Ultra Denryu Iraira Bou\0", NULL, "SNK / Saurus", "Neo Geo",
	L"The Irritating Maze\0\u30A6\u30EB\u30C8\u30E9\u96FB\u6D41\u30A4\u30E9\u30A4\u30E9\u68D2\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_TRACKBALL, GBF_MISC, 0,
	NULL, irrmazeRomInfo, irrmazeRomName, NULL, NULL, neotrackballInputInfo, neotrackballDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Pop 'n Bounce / Gapporin

static struct BurnRomInfo popbouncRomDesc[] = {
	{ "237-p1.p1",    0x100000, 0xbe96e44f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "237-s1.s1",    0x020000, 0xb61cf595, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "237-c1.c1",    0x200000, 0xeda42d66, 3 | BRF_GRA },           //  2 Sprite data
	{ "237-c2.c2",    0x200000, 0x5e633c65, 3 | BRF_GRA },           //  3 

	{ "237-m1.m1",    0x020000, 0xd4c946dd, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "237-v1.v1",    0x200000, 0xedcb1beb, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(popbounc, popbounc, neogeo)
STD_ROM_FN(popbounc)

struct BurnDriver BurnDrvpopbounc = {
	"popbounc", NULL, "neogeo", NULL, "1997",
	"Pop 'n Bounce / Gapporin\0", NULL, "Video System Co.", "Neo Geo",
	L"Pop 'n Bounce\0\u30AC\u30C3\u30DD\u30EA\u30F3\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_PADDLE, GBF_PUZZLE, 0,
	NULL, popbouncRomInfo, popbouncRomName, NULL, NULL, neopaddleInputInfo, neopaddleDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Shock Troopers (set 1)

static struct BurnRomInfo shocktroRomDesc[] = {
	{ "238-p1.p1",    0x100000, 0x5677456f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "238-p2.sp2",   0x400000, 0x5b4a09c5, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "238-s1.s1",    0x020000, 0x1f95cedb, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "238-c1.c1",    0x400000, 0x90c6a181, 3 | BRF_GRA },           //  3 Sprite data
	{ "238-c2.c2",    0x400000, 0x888720f0, 3 | BRF_GRA },           //  4 
	{ "238-c3.c3",    0x400000, 0x2c393aa3, 3 | BRF_GRA },           //  5 
	{ "238-c4.c4",    0x400000, 0xb9e909eb, 3 | BRF_GRA },           //  6 
	{ "238-c5.c5",    0x400000, 0xc22c68eb, 3 | BRF_GRA },           //  7 
	{ "238-c6.c6",    0x400000, 0x119323cd, 3 | BRF_GRA },           //  8 
	{ "238-c7.c7",    0x400000, 0xa72ce7ed, 3 | BRF_GRA },           //  9 
	{ "238-c8.c8",    0x400000, 0x1c7c2efb, 3 | BRF_GRA },           // 10 

	{ "238-m1.m1",    0x020000, 0x075b9518, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "238-v1.v1",    0x400000, 0x260c0bef, 5 | BRF_SND },           // 12 Sound data
	{ "238-v2.v2",    0x200000, 0x4ad7d59e, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(shocktro, shocktro, neogeo)
STD_ROM_FN(shocktro)

struct BurnDriver BurnDrvshocktro = {
	"shocktro", NULL, "neogeo", NULL, "1997",
	"Shock Troopers (set 1)\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, shocktroRomInfo, shocktroRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Shock Troopers (set 2)

static struct BurnRomInfo shocktroaRomDesc[] = {
	{ "238-pg1.p1",   0x100000, 0xefedf8dc, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "238-p2.sp2",   0x400000, 0x5b4a09c5, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "238-s1.s1",    0x020000, 0x1f95cedb, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "238-c1.c1",    0x400000, 0x90c6a181, 3 | BRF_GRA },           //  3 Sprite data
	{ "238-c2.c2",    0x400000, 0x888720f0, 3 | BRF_GRA },           //  4 
	{ "238-c3.c3",    0x400000, 0x2c393aa3, 3 | BRF_GRA },           //  5 
	{ "238-c4.c4",    0x400000, 0xb9e909eb, 3 | BRF_GRA },           //  6 
	{ "238-c5.c5",    0x400000, 0xc22c68eb, 3 | BRF_GRA },           //  7 
	{ "238-c6.c6",    0x400000, 0x119323cd, 3 | BRF_GRA },           //  8 
	{ "238-c7.c7",    0x400000, 0xa72ce7ed, 3 | BRF_GRA },           //  9 
	{ "238-c8.c8",    0x400000, 0x1c7c2efb, 3 | BRF_GRA },           // 10 

	{ "238-m1.m1",    0x020000, 0x075b9518, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "238-v1.v1",    0x400000, 0x260c0bef, 5 | BRF_SND },           // 12 Sound data
	{ "238-v2.v2",    0x200000, 0x4ad7d59e, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(shocktroa, shocktroa, neogeo)
STD_ROM_FN(shocktroa)

struct BurnDriver BurnDrvshocktroa = {
	"shocktroa", "shocktro", "neogeo", NULL, "1997",
	"Shock Troopers (set 2)\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, shocktroaRomInfo, shocktroaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Blazing Star

static struct BurnRomInfo blazstarRomDesc[] = {
	{ "239-p1.p1",    0x100000, 0x183682f8, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "239-p2.sp2",   0x200000, 0x9a9f4154, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "239-s1.s1",    0x020000, 0xd56cb498, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "239-c1.c1",    0x400000, 0x84f6d584, 3 | BRF_GRA },           //  3 Sprite data
	{ "239-c2.c2",    0x400000, 0x05a0cb22, 3 | BRF_GRA },           //  4 
	{ "239-c3.c3",    0x400000, 0x5fb69c9e, 3 | BRF_GRA },           //  5 
	{ "239-c4.c4",    0x400000, 0x0be028c4, 3 | BRF_GRA },           //  6 
	{ "239-c5.c5",    0x400000, 0x74bae5f8, 3 | BRF_GRA },           //  7 
	{ "239-c6.c6",    0x400000, 0x4e0700d2, 3 | BRF_GRA },           //  8 
	{ "239-c7.c7",    0x400000, 0x010ff4fd, 3 | BRF_GRA },           //  9 
	{ "239-c8.c8",    0x400000, 0xdb60460e, 3 | BRF_GRA },           // 10 

	{ "239-m1.m1",    0x020000, 0xd31a3aea, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "239-v1.v1",    0x400000, 0x1b8d5bf7, 5 | BRF_SND },           // 12 Sound data
	{ "239-v2.v2",    0x400000, 0x74cf0a70, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(blazstar, blazstar, neogeo)
STD_ROM_FN(blazstar)

struct BurnDriver BurnDrvblazstar = {
	"blazstar", NULL, "neogeo", NULL, "1998",
	"Blazing Star\0", NULL, "Yumekobo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_HORSHOOT, 0,
	NULL, blazstarRomInfo, blazstarRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Real Bout Fatal Fury 2 - The Newcomers / Real Bout Garou Densetsu 2 - the newcomers (set 1)

static struct BurnRomInfo rbff2RomDesc[] = {
	{ "240-p1.p1",    0x100000, 0x80e41205, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "240-p2.sp2",   0x400000, 0x960aa88d, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "240-s1.s1",    0x020000, 0xda3b40de, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "240-c1.c1",    0x800000, 0xeffac504, 3 | BRF_GRA },           //  3 Sprite data
	{ "240-c2.c2",    0x800000, 0xed182d44, 3 | BRF_GRA },           //  4 
	{ "240-c3.c3",    0x800000, 0x22e0330a, 3 | BRF_GRA },           //  5 
	{ "240-c4.c4",    0x800000, 0xc19a07eb, 3 | BRF_GRA },           //  6 
	{ "240-c5.c5",    0x800000, 0x244dff5a, 3 | BRF_GRA },           //  7 
	{ "240-c6.c6",    0x800000, 0x4609e507, 3 | BRF_GRA },           //  8 

	{ "240-m1.m1",    0x040000, 0xed482791, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "240-v1.v1",    0x400000, 0xf796265a, 5 | BRF_SND },           // 10 Sound data
	{ "240-v2.v2",    0x400000, 0x2cb3f3bb, 5 | BRF_SND },           // 11 
	{ "240-v3.v3",    0x400000, 0x8fe1367a, 5 | BRF_SND },           // 12 
	{ "240-v4.v4",    0x200000, 0x996704d8, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(rbff2, rbff2, neogeo)
STD_ROM_FN(rbff2)

struct BurnDriver BurnDrvrbff2 = {
	"rbff2", NULL, "neogeo", NULL, "1998",
	"Real Bout Fatal Fury 2 - The Newcomers / Real Bout Garou Densetsu 2 - the newcomers (set 1)\0", NULL, "SNK", "Neo Geo",
	L"Real Bout Fatal Fury 2 - the newcomers (set 1)\0Real Bout \u9913\u72FC\u4F1D\u8AAC\uFF12 (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, rbff2RomInfo, rbff2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Real Bout Fatal Fury 2 - The Newcomers / Real Bout Garou Densetsu 2 - the newcomers (set 2)

static struct BurnRomInfo rbff2hRomDesc[] = {
	{ "240-pg1.p1",   0x100000, 0xb6969780, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "240-p2.sp2",   0x400000, 0x960aa88d, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "240-s1.s1",    0x020000, 0xda3b40de, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "240-c1.c1",    0x800000, 0xeffac504, 3 | BRF_GRA },           //  3 Sprite data
	{ "240-c2.c2",    0x800000, 0xed182d44, 3 | BRF_GRA },           //  4 
	{ "240-c3.c3",    0x800000, 0x22e0330a, 3 | BRF_GRA },           //  5 
	{ "240-c4.c4",    0x800000, 0xc19a07eb, 3 | BRF_GRA },           //  6 
	{ "240-c5.c5",    0x800000, 0x244dff5a, 3 | BRF_GRA },           //  7 
	{ "240-c6.c6",    0x800000, 0x4609e507, 3 | BRF_GRA },           //  8 

	{ "240-m1.m1",    0x040000, 0xed482791, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "240-v1.v1",    0x400000, 0xf796265a, 5 | BRF_SND },           // 10 Sound data
	{ "240-v2.v2",    0x400000, 0x2cb3f3bb, 5 | BRF_SND },           // 11 
	{ "240-v3.v3",    0x400000, 0x8fe1367a, 5 | BRF_SND },           // 12 
	{ "240-v4.v4",    0x200000, 0x996704d8, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(rbff2h, rbff2h, neogeo)
STD_ROM_FN(rbff2h)

struct BurnDriver BurnDrvrbff2h = {
	"rbff2h", "rbff2", "neogeo", NULL, "1998",
	"Real Bout Fatal Fury 2 - The Newcomers / Real Bout Garou Densetsu 2 - the newcomers (set 2)\0", NULL, "SNK", "Neo Geo",
	L"Real Bout Fatal Fury 2 - the newcomers (set 2)\0Real Bout \u9913\u72FC\u4F1D\u8AAC\uFF12 (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, rbff2hRomInfo, rbff2hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Real Bout Fatal Fury 2 - The Newcomers (Korean release)

static struct BurnRomInfo rbff2kRomDesc[] = {
	{ "140-p1k.p1",   0x100000, 0x965edee1, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "240-p2.sp2",   0x400000, 0x960aa88d, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "240-s1.s1",    0x020000, 0xda3b40de, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "240-c1.c1",    0x800000, 0xeffac504, 3 | BRF_GRA },           //  3 Sprite data
	{ "240-c2.c2",    0x800000, 0xed182d44, 3 | BRF_GRA },           //  4 
	{ "240-c3.c3",    0x800000, 0x22e0330a, 3 | BRF_GRA },           //  5 
	{ "240-c4.c4",    0x800000, 0xc19a07eb, 3 | BRF_GRA },           //  6 
	{ "240-c5.c5",    0x800000, 0x244dff5a, 3 | BRF_GRA },           //  7 
	{ "240-c6.c6",    0x800000, 0x4609e507, 3 | BRF_GRA },           //  8 

	{ "240-m1.m1",    0x040000, 0xed482791, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "240-v1.v1",    0x400000, 0xf796265a, 5 | BRF_SND },           // 10 Sound data
	{ "240-v2.v2",    0x400000, 0x2cb3f3bb, 5 | BRF_SND },           // 11 
	{ "240-v3.v3",    0x400000, 0x8fe1367a, 5 | BRF_SND },           // 12 
	{ "240-v4.v4",    0x200000, 0x996704d8, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(rbff2k, rbff2k, neogeo)
STD_ROM_FN(rbff2k)

struct BurnDriver BurnDrvrbff2k = {
	"rbff2k", "rbff2", "neogeo", NULL, "1998",
	"Real Bout Fatal Fury 2 - The Newcomers (Korean release)\0", NULL, "SNK", "Neo Geo",
	L"Real Bout Fatal Fury 2 - the newcomers (Korean Release)\0Real Bout \u9913\u72FC\u4F1D\u8AAC\uFF12 (Korean Release)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_FATFURY,
	NULL, rbff2kRomInfo, rbff2kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Metal Slug 2 - Super Vehicle-001/II

static struct BurnRomInfo mslug2RomDesc[] = {
	{ "241-p1.p1",    0x100000, 0x2a53c5da, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "241-p2.sp2",   0x200000, 0x38883f44, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "241-s1.s1",    0x020000, 0xf3d32f0f, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "241-c1.c1",    0x800000, 0x394b5e0d, 3 | BRF_GRA },           //  3 Sprite data
	{ "241-c2.c2",    0x800000, 0xe5806221, 3 | BRF_GRA },           //  4 
	{ "241-c3.c3",    0x800000, 0x9f6bfa6f, 3 | BRF_GRA },           //  5 
	{ "241-c4.c4",    0x800000, 0x7d3e306f, 3 | BRF_GRA },           //  6 

	{ "241-m1.m1",    0x020000, 0x94520ebd, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "241-v1.v1",    0x400000, 0x99ec20e8, 5 | BRF_SND },           //  8 Sound data
	{ "241-v2.v2",    0x400000, 0xecb16799, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(mslug2, mslug2, neogeo)
STD_ROM_FN(mslug2)

struct BurnDriver BurnDrvmslug2 = {
	"mslug2", NULL, "neogeo", NULL, "1998",
	"Metal Slug 2 - Super Vehicle-001/II\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug2RomInfo, mslug2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends

static struct BurnRomInfo kof98RomDesc[] = {
	{ "242-p1.p1",    0x200000, 0x8893df89, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "242-p2.sp2",   0x400000, 0x980aba4c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "242-s1.s1",    0x020000, 0x7f7b4805, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "242-c1.c1",    0x800000, 0xe564ecd6, 3 | BRF_GRA },           //  3 Sprite data
	{ "242-c2.c2",    0x800000, 0xbd959b60, 3 | BRF_GRA },           //  4 
	{ "242-c3.c3",    0x800000, 0x22127b4f, 3 | BRF_GRA },           //  5 
	{ "242-c4.c4",    0x800000, 0x0b4fa044, 3 | BRF_GRA },           //  6 
	{ "242-c5.c5",    0x800000, 0x9d10bed3, 3 | BRF_GRA },           //  7 
	{ "242-c6.c6",    0x800000, 0xda07b6a2, 3 | BRF_GRA },           //  8 
	{ "242-c7.c7",    0x800000, 0xf6d7a38a, 3 | BRF_GRA },           //  9 
	{ "242-c8.c8",    0x800000, 0xc823e045, 3 | BRF_GRA },           // 10 

	{ "242-m1.m1",    0x040000, 0x4ef7016b, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "242-v1.v1",    0x400000, 0xb9ea8051, 5 | BRF_SND },           // 12 Sound data
	{ "242-v2.v2",    0x400000, 0xcc11106e, 5 | BRF_SND },           // 13 
	{ "242-v3.v3",    0x400000, 0x044ea4e1, 5 | BRF_SND },           // 14 
	{ "242-v4.v4",    0x400000, 0x7985ea30, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof98, kof98, neogeo)
STD_ROM_FN(kof98)

void kof98Callback()
{
	int i, j;
	unsigned char *dst = (unsigned char *)malloc(0x100000);
	if (dst)
	{
		for (i = 0; i < 0x100000; i += 2)
		{
					 j  = i;
			if ( i&0x0000fc) j ^= 0x000100;
			if ( i&0x000002) j ^= 0x100002;
			if ( i&0x000008) j ^= 0x100002;

			if ((i&0x0c000a) == 0x08000a || (i&0x0c000a) == 0x080000) {
				if (i&0x0000fc) j ^= 0x000100;
			}

			memcpy (dst + i, Neo68KROM + j, 2);
		}

		memcpy (Neo68KROM + 0x000800, dst + 0x000800,       0x0ff800);
		memcpy (Neo68KROM + 0x100000, Neo68KROM + 0x200000, 0x400000);

		free (dst);
	}
}

static unsigned short nkof98Protection;

static void kof98Protection()
{
	// We need these writes because the BIOS vector block is actually 1024 bytes
	switch (nkof98Protection) {
		case 0x0090:
			*((unsigned int*)Neo68KROM + 0x0100) = 0x00c200fd;
			SekWriteLongROM(0x000100, 0x00c200fd);
			break;
		case 0x00f0:
			*((unsigned int*)Neo68KROM + 0x0100) = 0x4e454f2d;
			SekWriteLongROM(0x000100, 0x4e454f2d);
			break;
	}
}

void __fastcall kof98WriteByteProtection(unsigned int sekAddress, unsigned char byteValue)
{
	switch (sekAddress) {
		case 0x20aaaa: {
			nkof98Protection &= 0x00ff;
			nkof98Protection |= byteValue << 8;
			kof98Protection();
		}
		case 0x20aaab: {
			nkof98Protection &= 0xff00;
			nkof98Protection |= byteValue;
			kof98Protection();
		}
	}
}

void __fastcall kof98WriteWordProtection(unsigned int sekAddress, unsigned short wordValue)
{
	switch (sekAddress) {
		case 0x20aaaa: {
			nkof98Protection = wordValue;
			kof98Protection();
		}
	}
}

static int kof98Init()
{
	int nRet;

	pNeoInitCallback = kof98Callback;

	nRet = NeoInit();

	if (nRet == 0) {
		// Install protection handler
		SekMapHandler(5,	0x200000, 0x2ffbff, SM_WRITE);
		SekSetWriteWordHandler(5, kof98WriteWordProtection);
		SekSetWriteByteHandler(5, kof98WriteByteProtection);

		nkof98Protection = 0;
	}

	return nRet;

}
struct BurnDriver BurnDrvkof98 = {
	"kof98", NULL, "neogeo", NULL, "1998",
	"The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof98RomInfo, kof98RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof98Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (Korean board, set 1)

static struct BurnRomInfo kof98kRomDesc[] = {
	{ "242-p1.p1",    0x200000, 0x8893df89, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "242-p2.sp2",   0x400000, 0x980aba4c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "242-s1.s1",    0x020000, 0x7f7b4805, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "242-c1.c1",    0x800000, 0xe564ecd6, 3 | BRF_GRA },           //  3 Sprite data
	{ "242-c2.c2",    0x800000, 0xbd959b60, 3 | BRF_GRA },           //  4 
	{ "242-c3.c3",    0x800000, 0x22127b4f, 3 | BRF_GRA },           //  5 
	{ "242-c4.c4",    0x800000, 0x0b4fa044, 3 | BRF_GRA },           //  6 
	{ "242-c5.c5",    0x800000, 0x9d10bed3, 3 | BRF_GRA },           //  7 
	{ "242-c6.c6",    0x800000, 0xda07b6a2, 3 | BRF_GRA },           //  8 
	{ "242-c7.c7",    0x800000, 0xf6d7a38a, 3 | BRF_GRA },           //  9 
	{ "242-c8.c8",    0x800000, 0xc823e045, 3 | BRF_GRA },           // 10 

	{ "242-m1k.m1",   0x040000, 0xce12da0c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "242-v1.v1",    0x400000, 0xb9ea8051, 5 | BRF_SND },           // 12 Sound data
	{ "242-v2.v2",    0x400000, 0xcc11106e, 5 | BRF_SND },           // 13 
	{ "242-v3.v3",    0x400000, 0x044ea4e1, 5 | BRF_SND },           // 14 
	{ "242-v4.v4",    0x400000, 0x7985ea30, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof98k, kof98k, neogeo)
STD_ROM_FN(kof98k)

struct BurnDriver BurnDrvkof98k = {
	"kof98k", "kof98", "neogeo", NULL, "1998",
	"The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (Korean board, set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof98kRomInfo, kof98kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof98Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (Korean board, set 2)

static struct BurnRomInfo kof98kaRomDesc[] = {
	{ "242-p1.p1",    0x200000, 0x8893df89, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "242-p2.sp2",   0x400000, 0x980aba4c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "242-s1.s1",    0x020000, 0x7f7b4805, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "242-c1.c1",    0x800000, 0xe564ecd6, 3 | BRF_GRA },           //  3 Sprite data
	{ "242-c2.c2",    0x800000, 0xbd959b60, 3 | BRF_GRA },           //  4 
	{ "242-c3.c3",    0x800000, 0x22127b4f, 3 | BRF_GRA },           //  5 
	{ "242-c4.c4",    0x800000, 0x0b4fa044, 3 | BRF_GRA },           //  6 
	{ "242-c5.c5",    0x800000, 0x9d10bed3, 3 | BRF_GRA },           //  7 
	{ "242-c6.c6",    0x800000, 0xda07b6a2, 3 | BRF_GRA },           //  8 
	{ "242-c7.c7",    0x800000, 0xf6d7a38a, 3 | BRF_GRA },           //  9 
	{ "242-c8.c8",    0x800000, 0xc823e045, 3 | BRF_GRA },           // 10 

	{ "242-mg1k.m1",  0x040000, 0xce9fb07c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "242-v1.v1",    0x400000, 0xb9ea8051, 5 | BRF_SND },           // 12 Sound data
	{ "242-v2.v2",    0x400000, 0xcc11106e, 5 | BRF_SND },           // 13 
	{ "242-v3.v3",    0x400000, 0x044ea4e1, 5 | BRF_SND },           // 14 
	{ "242-v4.v4",    0x400000, 0x7985ea30, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof98ka, kof98ka, neogeo)
STD_ROM_FN(kof98ka)

struct BurnDriver BurnDrvkof98ka = {
	"kof98ka", "kof98", "neogeo", NULL, "1998",
	"The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (Korean board, set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof98kaRomInfo, kof98kaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof98Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (not encrypted)

static struct BurnRomInfo kof98hRomDesc[] = {
	{ "242-pn1.p1",   0x100000, 0x61ac868a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "242-p2.sp2",   0x400000, 0x980aba4c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "242-s1.s1",    0x020000, 0x7f7b4805, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "242-c1.c1",    0x800000, 0xe564ecd6, 3 | BRF_GRA },           //  3 Sprite data
	{ "242-c2.c2",    0x800000, 0xbd959b60, 3 | BRF_GRA },           //  4 
	{ "242-c3.c3",    0x800000, 0x22127b4f, 3 | BRF_GRA },           //  5 
	{ "242-c4.c4",    0x800000, 0x0b4fa044, 3 | BRF_GRA },           //  6 
	{ "242-c5.c5",    0x800000, 0x9d10bed3, 3 | BRF_GRA },           //  7 
	{ "242-c6.c6",    0x800000, 0xda07b6a2, 3 | BRF_GRA },           //  8 
	{ "242-c7.c7",    0x800000, 0xf6d7a38a, 3 | BRF_GRA },           //  9 
	{ "242-c8.c8",    0x800000, 0xc823e045, 3 | BRF_GRA },           // 10 

	{ "242-mg1.m1",   0x040000, 0x4e7a6b1b, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "242-v1.v1",    0x400000, 0xb9ea8051, 5 | BRF_SND },           // 12 Sound data
	{ "242-v2.v2",    0x400000, 0xcc11106e, 5 | BRF_SND },           // 13 
	{ "242-v3.v3",    0x400000, 0x044ea4e1, 5 | BRF_SND },           // 14 
	{ "242-v4.v4",    0x400000, 0x7985ea30, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof98h, kof98h, neogeo)
STD_ROM_FN(kof98h)

struct BurnDriver BurnDrvkof98h = {
	"kof98h", "kof98", "neogeo", NULL, "1998",
	"The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (AES cart)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof98hRomInfo, kof98hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Last Blade 2 / Bakumatsu Roman - Dai Ni Maku Gekka no Kenshi, The

static struct BurnRomInfo lastbld2RomDesc[] = {
	{ "243-pg1.p1",   0x100000, 0xaf1e6554, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "243-pg2.sp2",  0x400000, 0xadd4a30b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "243-s1.s1",    0x020000, 0xc9cd2298, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "243-c1.c1",    0x800000, 0x5839444d, 3 | BRF_GRA },           //  3 Sprite data
	{ "243-c2.c2",    0x800000, 0xdd087428, 3 | BRF_GRA },           //  4 
	{ "243-c3.c3",    0x800000, 0x6054cbe0, 3 | BRF_GRA },           //  5 
	{ "243-c4.c4",    0x800000, 0x8bd2a9d2, 3 | BRF_GRA },           //  6 
	{ "243-c5.c5",    0x800000, 0x6a503dcf, 3 | BRF_GRA },           //  7 
	{ "243-c6.c6",    0x800000, 0xec9c36d0, 3 | BRF_GRA },           //  8 

	{ "243-m1.m1",    0x020000, 0xacf12d10, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "243-v1.v1",    0x400000, 0xf7ee6fbb, 5 | BRF_SND },           // 10 Sound data
	{ "243-v2.v2",    0x400000, 0xaa9e4df6, 5 | BRF_SND },           // 11 
	{ "243-v3.v3",    0x400000, 0x4ac750b2, 5 | BRF_SND },           // 12 
	{ "243-v4.v4",    0x400000, 0xf5c64ba6, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(lastbld2, lastbld2, neogeo)
STD_ROM_FN(lastbld2)

struct BurnDriver BurnDrvlastbld2 = {
	"lastbld2", NULL, "neogeo", NULL, "1998",
	"Last Blade 2 / Bakumatsu Roman - Dai Ni Maku Gekka no Kenshi, The\0", NULL, "SNK", "Neo Geo",
	L"The Last Blade 2\0\u5E55\u672B\u6D6A\u6F2B\u7B2C\u4E8C\u5E55 - \u6708\u83EF\u306E\u5263\u58EB - \u6708\u306B\u54B2\u304F\u83EF\u3001\u6563\u308A\u3086\u304F\u82B1\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, lastbld2RomInfo, lastbld2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Neo-Geo Cup '98 - The Road to the Victory

static struct BurnRomInfo neocup98RomDesc[] = {
	{ "244-p1.p1",    0x200000, 0xf8fdb7a5, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "244-s1.s1",    0x020000, 0x9bddb697, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "244-c1.c1",    0x800000, 0xc7a62b23, 3 | BRF_GRA },           //  2 Sprite data
	{ "244-c2.c2",    0x800000, 0x33aa0f35, 3 | BRF_GRA },           //  3 

	{ "244-m1.m1",    0x020000, 0xa701b276, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "244-v1.v1",    0x400000, 0x79def46d, 5 | BRF_SND },           //  5 Sound data
	{ "244-v2.v2",    0x200000, 0xb231902f, 5 | BRF_SND },           //  6 
};

STDROMPICKEXT(neocup98, neocup98, neogeo)
STD_ROM_FN(neocup98)

struct BurnDriver BurnDrvneocup98 = {
	"neocup98", NULL, "neogeo", NULL, "1998",
	"Neo-Geo Cup '98 - The Road to the Victory\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SPORTSFOOTBALL, 0,
	NULL, neocup98RomInfo, neocup98RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Breakers Revenge

static struct BurnRomInfo breakrevRomDesc[] = {
	{ "245-p1.p1",    0x200000, 0xc828876d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "245-s1.s1",    0x020000, 0xe7660a5d, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "245-c1.c1",    0x400000, 0x68d4ae76, 3 | BRF_GRA },           //  2 Sprite data
	{ "245-c2.c2",    0x400000, 0xfdee05cd, 3 | BRF_GRA },           //  3 
	{ "245-c3.c3",    0x400000, 0x645077f3, 3 | BRF_GRA },           //  4 
	{ "245-c4.c4",    0x400000, 0x63aeb74c, 3 | BRF_GRA },           //  5 
	{ "245-c5.c5",    0x400000, 0xb5f40e7f, 3 | BRF_GRA },           //  6 
	{ "245-c6.c6",    0x400000, 0xd0337328, 3 | BRF_GRA },           //  7 

	{ "245-m1.m1",    0x020000, 0x00f31c66, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "245-v1.v1",    0x400000, 0xe255446c, 5 | BRF_SND },           //  9 Sound data
	{ "245-v2.v2",    0x400000, 0x9068198a, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(breakrev, breakrev, neogeo)
STD_ROM_FN(breakrev)

struct BurnDriver BurnDrvbreakrev = {
	"breakrev", NULL, "neogeo", NULL, "1998",
	"Breakers Revenge\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VSFIGHT, 0,
	NULL, breakrevRomInfo, breakrevRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Shock Troopers - 2nd Squad

static struct BurnRomInfo shocktr2RomDesc[] = {
	{ "246-p1.p1",    0x100000, 0x6d4b7781, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "246-p2.sp2",   0x400000, 0x72ea04c3, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "246-s1.s1",    0x020000, 0x2a360637, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "246-c1.c1",    0x800000, 0x47ac9ec5, 3 | BRF_GRA },           //  3 Sprite data
	{ "246-c2.c2",    0x800000, 0x7bcab64f, 3 | BRF_GRA },           //  4 
	{ "246-c3.c3",    0x800000, 0xdb2f73e8, 3 | BRF_GRA },           //  5 
	{ "246-c4.c4",    0x800000, 0x5503854e, 3 | BRF_GRA },           //  6 
	{ "246-c5.c5",    0x800000, 0x055b3701, 3 | BRF_GRA },           //  7 
	{ "246-c6.c6",    0x800000, 0x7e2caae1, 3 | BRF_GRA },           //  8 

	{ "246-m1.m1",    0x020000, 0xd0604ad1, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "246-v1.v1",    0x400000, 0x16986fc6, 5 | BRF_SND },           // 10 Sound data
	{ "246-v2.v2",    0x400000, 0xada41e83, 5 | BRF_SND },           // 11 
	{ "246-v3.v3",    0x200000, 0xa05ba5db, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(shocktr2, shocktr2, neogeo)
STD_ROM_FN(shocktr2)

struct BurnDriver BurnDrvshocktr2 = {
	"shocktr2", NULL, "neogeo", NULL, "1998",
	"Shock Troopers - 2nd Squad\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, shocktr2RomInfo, shocktr2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg)

static struct BurnRomInfo lans2004RomDesc[] = {
	{ "lnsq-p1.bin",  0x200000, 0xb40a879a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "lnsq-p21.bin", 0x200000, 0xecdb2d42, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "lnsq-p22.bin", 0x200000, 0xfac5e2e7, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "lnsq-s1.bin",  0x020000, 0x39e82897, 2 | BRF_GRA },           //  3 Text layer tiles

	{ "lnsq-c1.bin",  0x800000, 0xb83de59f, 3 | BRF_GRA },           //  4 Sprite data
	{ "lnsq-c2.bin",  0x800000, 0xe08969fd, 3 | BRF_GRA },           //  5 
	{ "lnsq-c3.bin",  0x800000, 0x013f2cda, 3 | BRF_GRA },           //  6 
	{ "lnsq-c4.bin",  0x800000, 0xd8c3a758, 3 | BRF_GRA },           //  7 
	{ "lnsq-c5.bin",  0x800000, 0x75500b82, 3 | BRF_GRA },           //  8 
	{ "lnsq-c6.bin",  0x800000, 0x670ac13b, 3 | BRF_GRA },           //  9 

	{ "246-m1.bin",   0x020000, 0xd0604ad1, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "lnsq-v1.bin",  0x400000, 0x4408ebc3, 5 | BRF_SND },           // 11 Sound data
	{ "lnsq-v2.bin",  0x400000, 0x3d953975, 5 | BRF_SND },           // 12 
	{ "lnsq-v3.bin",  0x200000, 0x437d1d8e, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(lans2004, lans2004, neogeo)
STD_ROM_FN(lans2004)

static void lans2004_sx_decode()
{
	int i, j, n;
	for (i = 0; i < 0x20000; i+= 0x10) {
		for (j = 0; j < 0x08; j++) {
			n = NeoTextROM[i + j + 0x20008];
			NeoTextROM[i + j + 0x20008] = NeoTextROM[i + j + 0x20000];
			NeoTextROM[i + j + 0x20000] = n;	
		}
	}
}

static void lans2004_cx_decode(int nLen)
{
	int i, j, n;
	for (i = 0; i < nLen; i+= 0x80) {
		for (j = 0; j < 0x40; j++) {
			n = NeoSpriteROM[i + j + 0x40];
			NeoSpriteROM[i + j + 0x40] = NeoSpriteROM[i + j];
			NeoSpriteROM[i + j] = n;
		}
	}
}

static void lans2004Callback()
{
	int i;
	unsigned char *dst = (unsigned char*)malloc(0x100000);
	if (dst)
	{
		static const int sec[] = { 0x3, 0x8, 0x7, 0xc, 0x1, 0xa, 0x6, 0xd };

		for (i = 0; i < 8; i++)
			memcpy (dst + i * 0x20000, Neo68KROM + sec[i] * 0x20000, 0x20000);

		memcpy (dst + 0x0bbb00, Neo68KROM + 0x045b00, 0x001710);
		memcpy (dst + 0x02fff0, Neo68KROM + 0x1a92be, 0x000010);
		memcpy (Neo68KROM, dst, 0x100000);
		free (dst);
	}

	memcpy (Neo68KROM + 0x100000, Neo68KROM + 0x200000, 0x400000);

	for (i = 0xbbb00; i < 0xbe000; i+=2) {
		if ((*((unsigned short *)(Neo68KROM + i + 0)) & 0xf2bf) == 0x42b9 &&
		     *((unsigned short *)(Neo68KROM + i + 2)) == 0x0000)
		{
			*((unsigned short *)(Neo68KROM + i + 2)) = 0x000b;
			*((unsigned short *)(Neo68KROM + i + 4)) += 0x6000;
		}
	}

	*((unsigned short *)(Neo68KROM + 0x2d15c)) = 0x000b;
	*((unsigned short *)(Neo68KROM + 0x2d15e)) = 0xbb00;
	*((unsigned short *)(Neo68KROM + 0x2d1e4)) = 0x6002;
	*((unsigned short *)(Neo68KROM + 0x2ea7e)) = 0x6002;
	*((unsigned short *)(Neo68KROM + 0xbbcd0)) = 0x6002;
	*((unsigned short *)(Neo68KROM + 0xbbdf2)) = 0x6002;
	*((unsigned short *)(Neo68KROM + 0xbbe42)) = 0x6002;

	lans2004_sx_decode();
	lans2004_cx_decode(0x800000 * 6);
}

static int lans2004Init()
{
	int nRet;

	pNeoInitCallback = lans2004Callback;

 	nRet = NeoInit();

	if (nRet == 0) {
		for (int i = 0; i < 0xa00000; i++)
			YM2610ADPCMAROM[i] = BITSWAP08(YM2610ADPCMAROM[i], 0, 1, 5, 4, 3, 2, 6, 7);
	}

	return nRet;
}

struct BurnDriver BurnDrvlans2004 = {
	"lans2004", "shocktr2", "neogeo", NULL, "1998",
	"Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, lans2004RomInfo, lans2004RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	lans2004Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Battle Flip Shot

static struct BurnRomInfo flipshotRomDesc[] = {
	{ "247-p1.p1",    0x100000, 0x95779094, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "247-s1.s1",    0x020000, 0x6300185c, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "247-c1.c1",    0x200000, 0xc9eedcb2, 3 | BRF_GRA },           //  2 Sprite data
	{ "247-c2.c2",    0x200000, 0x7d6d6e87, 3 | BRF_GRA },           //  3 

	{ "247-m1.m1",    0x020000, 0xa9fe0144, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "247-v1.v1",    0x200000, 0x42ec743d, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(flipshot, flipshot, neogeo)
STD_ROM_FN(flipshot)

struct BurnDriver BurnDrvflipshot = {
	"flipshot", NULL, "neogeo", NULL, "1998",
	"Battle Flip Shot\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_BALLPADDLE, 0,
	NULL, flipshotRomInfo, flipshotRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Puzzle Bobble 2 / Bust-A-Move Again (Neo-Geo)

static struct BurnRomInfo pbobbl2nRomDesc[] = {
	{ "248-p1.p1",    0x100000, 0x9d6c0754, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "248-s1.s1",    0x020000, 0x0a3fee41, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "248-c1.c1",    0x400000, 0xd9115327, 3 | BRF_GRA },           //  2 Sprite data
	{ "248-c2.c2",    0x400000, 0x77f9fdac, 3 | BRF_GRA },           //  3 
	{ "248-c3.c3",    0x100000, 0x8890bf7c, 3 | BRF_GRA },           //  4 
	{ "248-c4.c4",    0x100000, 0x8efead3f, 3 | BRF_GRA },           //  5 

	{ "248-m1.m1",    0x020000, 0x883097a9, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "248-v1.v1",    0x400000, 0x57fde1fa, 5 | BRF_SND },           //  7 Sound data
	{ "248-v2.v2",    0x400000, 0x4b966ef3, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(pbobbl2n, pbobbl2n, neogeo)
STD_ROM_FN(pbobbl2n)

struct BurnDriver BurnDrvpbobbl2n = {
	"pbobbl2n", NULL, "neogeo", NULL, "1999",
	"Puzzle Bobble 2 / Bust-A-Move Again (Neo-Geo)\0", NULL, "Taito (SNK license)", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, pbobbl2nRomInfo, pbobbl2nRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Captain Tomaday

static struct BurnRomInfo ctomadayRomDesc[] = {
	{ "249-p1.p1",    0x200000, 0xc9386118, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "249-s1.s1",    0x020000, 0xdc9eb372, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "249-c1.c1",    0x400000, 0x041fb8ee, 3 | BRF_GRA },           //  2 Sprite data
	{ "249-c2.c2",    0x400000, 0x74f3cdf4, 3 | BRF_GRA },           //  3 

	{ "249-m1.m1",    0x020000, 0x80328a47, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "249-v1.v1",    0x400000, 0xde7c8f27, 5 | BRF_SND },           //  5 Sound data
	{ "249-v2.v2",    0x100000, 0xc8e40119, 5 | BRF_SND },           //  6 
};

STDROMPICKEXT(ctomaday, ctomaday, neogeo)
STD_ROM_FN(ctomaday)

struct BurnDriver BurnDrvctomaday = {
	"ctomaday", NULL, "neogeo", NULL, "1999",
	"Captain Tomaday\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_VERSHOOT, 0,
	NULL, ctomadayRomInfo, ctomadayRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug X - Super Vehicle-001

static struct BurnRomInfo mslugxRomDesc[] = {
	{ "250-p1.p1",    0x100000, 0x81f1f60b, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "250-p2.ep1",   0x400000, 0x1fda2e12, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "250-s1.s1",    0x020000, 0xfb6f441d, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "250-c1.c1",    0x800000, 0x09a52c6f, 3 | BRF_GRA },           //  3 Sprite data
	{ "250-c2.c2",    0x800000, 0x31679821, 3 | BRF_GRA },           //  4 
	{ "250-c3.c3",    0x800000, 0xfd602019, 3 | BRF_GRA },           //  5 
	{ "250-c4.c4",    0x800000, 0x31354513, 3 | BRF_GRA },           //  6 
	{ "250-c5.c5",    0x800000, 0xa4b56124, 3 | BRF_GRA },           //  7 
	{ "250-c6.c6",    0x800000, 0x83e3e69d, 3 | BRF_GRA },           //  8 

	{ "250-m1.m1",    0x020000, 0xfd42a842, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "250-v1.v1",    0x400000, 0xc79ede73, 5 | BRF_SND },           // 10 Sound data
	{ "250-v2.v2",    0x400000, 0xea9aabe1, 5 | BRF_SND },           // 11 
	{ "250-v3.v3",    0x200000, 0x2ca65102, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(mslugx, mslugx, neogeo)
STD_ROM_FN(mslugx)

static void mslugxPatch()
{
	for (int i = 0; i < 0x100000 - 8; i += 2) {
		if (*((unsigned short*)(Neo68KROM + i + 0)) == 0x0243 && *((unsigned short*)(Neo68KROM + i + 2)) == 0x0001 && *((unsigned short*)(Neo68KROM + i + 4)) == 0x6600) {
			*((unsigned short*)(Neo68KROM + i + 4)) = 0x4e71;
			*((unsigned short*)(Neo68KROM + i + 6)) = 0x4e71;
		}
	}

	*((unsigned short*)(Neo68KROM + 0x3bdc)) = 0x4e71;
	*((unsigned short*)(Neo68KROM + 0x3bde)) = 0x4e71;
	*((unsigned short*)(Neo68KROM + 0x3be0)) = 0x4e71;

	*((unsigned short*)(Neo68KROM + 0x3c0c)) = 0x4e71;
	*((unsigned short*)(Neo68KROM + 0x3c0e)) = 0x4e71;
	*((unsigned short*)(Neo68KROM + 0x3c10)) = 0x4e71;

	*((unsigned short*)(Neo68KROM + 0x3c36)) = 0x4e71;
	*((unsigned short*)(Neo68KROM + 0x3c38)) = 0x4e71;
}

static int mslugxInit()
{
	pNeoInitCallback = mslugxPatch;

	return NeoInit();
}

struct BurnDriver BurnDrvmslugx = {
	"mslugx", NULL, "neogeo", NULL, "1999",
	"Metal Slug X - Super Vehicle-001\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslugxRomInfo, mslugxRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslugxInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '99 - Millennium Battle (set 1)

static struct BurnRomInfo kof99RomDesc[] = {
	{ "ka.neo-sma",   0x040000, 0x7766d09e, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "251-p1.p1",    0x400000, 0x006e4532, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "251-p2.p2",    0x400000, 0x90175f15, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "251-c1.c1",    0x800000, 0x0f9e93fe, 3 | BRF_GRA },           //  3 Sprite data
	{ "251-c2.c2",    0x800000, 0xe71e2ea3, 3 | BRF_GRA },           //  4 
	{ "251-c3.c3",    0x800000, 0x238755d2, 3 | BRF_GRA },           //  5 
	{ "251-c4.c4",    0x800000, 0x438c8b22, 3 | BRF_GRA },           //  6 
	{ "251-c5.c5",    0x800000, 0x0b0abd0a, 3 | BRF_GRA },           //  7 
	{ "251-c6.c6",    0x800000, 0x65bbf281, 3 | BRF_GRA },           //  8 
	{ "251-c7.c7",    0x800000, 0xff65f62e, 3 | BRF_GRA },           //  9 
	{ "251-c8.c8",    0x800000, 0x8d921c68, 3 | BRF_GRA },           // 10 

	{ "251-m1.m1",    0x020000, 0x5e74539c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "251-v1.v1",    0x400000, 0xef2eecc8, 5 | BRF_SND },           // 12 Sound data
	{ "251-v2.v2",    0x400000, 0x73e211ca, 5 | BRF_SND },           // 13 
	{ "251-v3.v3",    0x400000, 0x821901da, 5 | BRF_SND },           // 14 
	{ "251-v4.v4",    0x200000, 0xb49e6178, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof99, kof99, neogeo)
STD_ROM_FN(kof99)

static void kof99SMADecrypt()
{
	for (int i = 0; i < 0x800000 / 2; i++) {
		((unsigned short*)(Neo68KROM + 0x100000))[i] = BITSWAP16(((unsigned short*)(Neo68KROM + 0x100000))[i], 13, 7, 3, 0, 9, 4, 5, 6, 1, 12, 8, 14, 10, 11, 2, 15);
	}

	for (int i = 0; i < 0x0c0000 / 2; i++) {
		((unsigned short*)Neo68KROM)[i] = ((unsigned short*)Neo68KROM)[0x700000 / 2 + BITSWAP24(i, 23, 22, 21, 20, 19, 18, 11, 6, 14, 17, 16, 5, 8, 10, 12, 0, 4, 3, 2, 7, 9, 15, 13, 1)];;
	}

	for (int i = 0; i < 0x600000 / 2; i += 0x0800 / 2) {
		unsigned short nBuffer[0x0800 / 2];
		memcpy(nBuffer, &((unsigned short*)(Neo68KROM + 0x100000))[i], 0x0800);
		for (int j = 0; j < 0x0800 / 2; j++) {
			((unsigned short*)(Neo68KROM + 0x100000))[i + j] = nBuffer[BITSWAP24(j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 6, 2, 4, 9, 8, 3, 1, 7, 0, 5)];
		}
	}
}

void __fastcall kof99WriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress == 0x2ffff0) {
		static unsigned int bankoffset[64] = {
			0x100000, 0x200000, 0x300000, 0x400000,
			0x4cc000, 0x5cc000, 0x4f2000, 0x5f2000,
			0x507800, 0x607800, 0x50d000, 0x60d000,
			0x517800, 0x617800, 0x520800, 0x620800,
			0x524800, 0x624800, 0x529000, 0x629000,
			0x52e800, 0x62e800, 0x531800, 0x631800,
			0x64d000, 0x651000, 0x667000, 0x692800,
			0x688800, 0x681800, 0x699800, 0x694800,
			0x698000,	// rest not used?
		};

		// Unscramble bank number
		unsigned int nBank = bankoffset[
			((wordValue >> 14) & 0x01) |
			((wordValue >>  5) & 0x02) |
			((wordValue >>  6) & 0x04) |
			((wordValue >>  7) & 0x08) |
			((wordValue >>  8) & 0x10) |
			(wordValue & 0x20)];

		if (nBank != nNeo68KROMBank) {
			nNeo68KROMBank = nBank;
			NeoSMABankswitch();
		}
	}
}

static int kof99Init()
{
	nNeoProtectionXor = 0x00;

	return NeoSMAInit(kof99SMADecrypt, kof99WriteWordBankswitch, 0x2ffff8, 0x2ffffa);
}

struct BurnDriver BurnDrvkof99 = {
	"kof99", NULL, "neogeo", NULL, "1999",
	"The King of Fighters '99 - Millennium Battle (set 1)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SMA_PROTECTION | HARDWARE_SNK_ENCRYPTED_A, GBF_VSFIGHT, FBF_KOF,
	NULL, kof99RomInfo, kof99RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof99Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '99 - Millennium Battle (set 2)

static struct BurnRomInfo kof99hRomDesc[] = {
	{ "kc.neo-sma",   0x040000, 0x6c9d0647, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "251-p1.p1",    0x400000, 0x006e4532, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "251-pg2.p2",   0x400000, 0xd9057f51, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "251-c1.c1",    0x800000, 0x0f9e93fe, 3 | BRF_GRA },           //  3 Sprite data
	{ "251-c2.c2",    0x800000, 0xe71e2ea3, 3 | BRF_GRA },           //  4 
	{ "251-c3.c3",    0x800000, 0x238755d2, 3 | BRF_GRA },           //  5 
	{ "251-c4.c4",    0x800000, 0x438c8b22, 3 | BRF_GRA },           //  6 
	{ "251-c5.c5",    0x800000, 0x0b0abd0a, 3 | BRF_GRA },           //  7 
	{ "251-c6.c6",    0x800000, 0x65bbf281, 3 | BRF_GRA },           //  8 
	{ "251-c7.c7",    0x800000, 0xff65f62e, 3 | BRF_GRA },           //  9 
	{ "251-c8.c8",    0x800000, 0x8d921c68, 3 | BRF_GRA },           // 10 

	{ "251-m1.m1",    0x020000, 0x5e74539c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "251-v1.v1",    0x400000, 0xef2eecc8, 5 | BRF_SND },           // 12 Sound data
	{ "251-v2.v2",    0x400000, 0x73e211ca, 5 | BRF_SND },           // 13 
	{ "251-v3.v3",    0x400000, 0x821901da, 5 | BRF_SND },           // 14 
	{ "251-v4.v4",    0x200000, 0xb49e6178, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof99h, kof99h, neogeo)
STD_ROM_FN(kof99h)

struct BurnDriver BurnDrvkof99h = {
	"kof99h", "kof99", "neogeo", NULL, "1999",
	"The King of Fighters '99 - Millennium Battle (set 2)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SMA_PROTECTION | HARDWARE_SNK_ENCRYPTED_A, GBF_VSFIGHT, FBF_KOF,
	NULL, kof99hRomInfo, kof99hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof99Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '99 - Millennium Battle (earlier)

static struct BurnRomInfo kof99eRomDesc[] = {
	{ "ka.neo-sma",   0x040000, 0x7766d09e, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "251-ep1.p1",   0x200000, 0x1e8d692d, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "251-ep2.p2",   0x200000, 0xd6206e5a, 1 | BRF_ESS | BRF_PRG }, //  2 
	{ "251-ep3.p3",   0x200000, 0xd58c3ef8, 1 | BRF_ESS | BRF_PRG }, //  3 
	{ "251-ep4.p4",   0x200000, 0x52de02ae, 1 | BRF_ESS | BRF_PRG }, //  4 

	{ "251-c1.c1",    0x800000, 0x0f9e93fe, 3 | BRF_GRA },           //  5 Sprite data
	{ "251-c2.c2",    0x800000, 0xe71e2ea3, 3 | BRF_GRA },           //  6 
	{ "251-c3.c3",    0x800000, 0x238755d2, 3 | BRF_GRA },           //  7 
	{ "251-c4.c4",    0x800000, 0x438c8b22, 3 | BRF_GRA },           //  8 
	{ "251-c5.c5",    0x800000, 0x0b0abd0a, 3 | BRF_GRA },           //  9 
	{ "251-c6.c6",    0x800000, 0x65bbf281, 3 | BRF_GRA },           // 10 
	{ "251-c7.c7",    0x800000, 0xff65f62e, 3 | BRF_GRA },           // 11 
	{ "251-c8.c8",    0x800000, 0x8d921c68, 3 | BRF_GRA },           // 12 

	{ "251-m1.m1",    0x020000, 0x5e74539c, 4 | BRF_ESS | BRF_PRG }, // 13 Z80 code

	{ "251-v1.v1",    0x400000, 0xef2eecc8, 5 | BRF_SND },           // 14 Sound data
	{ "251-v2.v2",    0x400000, 0x73e211ca, 5 | BRF_SND },           // 15 
	{ "251-v3.v3",    0x400000, 0x821901da, 5 | BRF_SND },           // 16 
	{ "251-v4.v4",    0x200000, 0xb49e6178, 5 | BRF_SND },           // 17 
};

STDROMPICKEXT(kof99e, kof99e, neogeo)
STD_ROM_FN(kof99e)

struct BurnDriver BurnDrvkof99e = {
	"kof99e", "kof99", "neogeo", NULL, "1999",
	"The King of Fighters '99 - Millennium Battle (earlier)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SMA_PROTECTION | HARDWARE_SNK_ENCRYPTED_A, GBF_VSFIGHT, FBF_KOF,
	NULL, kof99eRomInfo, kof99eRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof99Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '99 - Millennium Battle (Korean release)

static struct BurnRomInfo kof99kRomDesc[] = {
	{ "152-p1.p1",    0x100000, 0xf2c7ddfa, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "152-p2.sp2",   0x400000, 0x274ef47a, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "251-c1.c1",    0x800000, 0x0f9e93fe, 3 | BRF_GRA },           //  2 Sprite data
	{ "251-c2.c2",    0x800000, 0xe71e2ea3, 3 | BRF_GRA },           //  3 
	{ "251-c3.c3",    0x800000, 0x238755d2, 3 | BRF_GRA },           //  4 
	{ "251-c4.c4",    0x800000, 0x438c8b22, 3 | BRF_GRA },           //  5 
	{ "251-c5.c5",    0x800000, 0x0b0abd0a, 3 | BRF_GRA },           //  6 
	{ "251-c6.c6",    0x800000, 0x65bbf281, 3 | BRF_GRA },           //  7 
	{ "251-c7.c7",    0x800000, 0xff65f62e, 3 | BRF_GRA },           //  8 
	{ "251-c8.c8",    0x800000, 0x8d921c68, 3 | BRF_GRA },           //  9 

	{ "251-m1.m1",    0x020000, 0x5e74539c, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "251-v1.v1",    0x400000, 0xef2eecc8, 5 | BRF_SND },           // 11 Sound data
	{ "251-v2.v2",    0x400000, 0x73e211ca, 5 | BRF_SND },           // 12 
	{ "251-v3.v3",    0x400000, 0x821901da, 5 | BRF_SND },           // 13 
	{ "251-v4.v4",    0x200000, 0xb49e6178, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kof99k, kof99k, neogeo)
STD_ROM_FN(kof99k)

int kof99kInit()
{
	nNeoProtectionXor = 0x00;
	return NeoInit();
}

struct BurnDriver BurnDrvkof99k = {
	"kof99k", "kof99", "neogeo", NULL, "1999",
	"The King of Fighters '99 - Millennium Battle (Korean release)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_A, GBF_VSFIGHT, FBF_KOF,
	NULL, kof99kRomInfo, kof99kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof99kInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '99 - Millennium Battle (prototype)

static struct BurnRomInfo kof99pRomDesc[] = {
	{ "251-p1p.p1",   0x100000, 0xf37929c4, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "251-p2p.p2",   0x400000, 0x739742ad, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "251-s1p.s1",   0x020000, 0xfb1498ed, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "251-c1p.c1",   0x800000, 0xe5d8ffa4, 3 | BRF_GRA },           //  3 Sprite data
	{ "251-c2p.c2",   0x800000, 0xd822778f, 3 | BRF_GRA },           //  4 
	{ "251-c3p.c3",   0x800000, 0xf20959e8, 3 | BRF_GRA },           //  5 
	{ "251-c4p.c4",   0x800000, 0x54ffbe9f, 3 | BRF_GRA },           //  6 
	{ "251-c5p.c5",   0x800000, 0xd87a3bbc, 3 | BRF_GRA },           //  7 
	{ "251-c6p.c6",   0x800000, 0x4d40a691, 3 | BRF_GRA },           //  8 
	{ "251-c7p.c7",   0x800000, 0xa4479a58, 3 | BRF_GRA },           //  9 
	{ "251-c8p.c8",   0x800000, 0xead513ce, 3 | BRF_GRA },           // 10 

	{ "251-m1.m1",    0x020000, 0x5e74539c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "251-v1.v1",    0x400000, 0xef2eecc8, 5 | BRF_SND },           // 12 Sound data
	{ "251-v2.v2",    0x400000, 0x73e211ca, 5 | BRF_SND },           // 13 
	{ "251-v3.v3",    0x400000, 0x821901da, 5 | BRF_SND },           // 14 
	{ "251-v4.v4",    0x200000, 0xb49e6178, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof99p, kof99p, neogeo)
STD_ROM_FN(kof99p)

struct BurnDriver BurnDrvkof99p = {
	"kof99p", "kof99", "neogeo", NULL, "1999",
	"The King of Fighters '99 - Millennium Battle (prototype)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof99pRomInfo, kof99pRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ganryu / Musashi Ganryuki

static struct BurnRomInfo ganryuRomDesc[] = {
	{ "252-p1.p1",    0x200000, 0x4b8ac4fb, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "252-c1.c1",    0x800000, 0x50ee7882, 3 | BRF_GRA },           //  1 Sprite data
	{ "252-c2.c2",    0x800000, 0x62585474, 3 | BRF_GRA },           //  2 

	{ "252-m1.m1",    0x020000, 0x30cc4099, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "252-v1.v1",    0x400000, 0xe5946733, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(ganryu, ganryu, neogeo)
STD_ROM_FN(ganryu)

int ganryuInit()
{
	nNeoProtectionXor = 0x07;
	return NeoInit();
}

struct BurnDriver BurnDrvganryu = {
	"ganryu", NULL, "neogeo", NULL, "1999",
	"Ganryu / Musashi Ganryuki\0", NULL, "Visco", "Neo Geo",
	L"Ganryu\0\u6B66\u8535\u5DCC\u6D41\u8A18\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP | HARDWARE_SNK_ENCRYPTED_A, GBF_PLATFORM, 0,
	NULL, ganryuRomInfo, ganryuRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	ganryuInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Garou - Mark of the Wolves (set 1)

static struct BurnRomInfo garouRomDesc[] = {
	{ "kf.neo-sma",   0x040000, 0x98bc93dc, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "253-ep1.p1",   0x200000, 0xea3171a4, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "253-ep2.p2",   0x200000, 0x382f704b, 1 | BRF_ESS | BRF_PRG }, //  2 
	{ "253-ep3.p3",   0x200000, 0xe395bfdd, 1 | BRF_ESS | BRF_PRG }, //  3 
	{ "253-ep4.p4",   0x200000, 0xda92c08e, 1 | BRF_ESS | BRF_PRG }, //  4 

	{ "253-c1.c1",    0x800000, 0x0603e046, 3 | BRF_GRA },           //  5 Sprite data
	{ "253-c2.c2",    0x800000, 0x0917d2a4, 3 | BRF_GRA },           //  6 
	{ "253-c3.c3",    0x800000, 0x6737c92d, 3 | BRF_GRA },           //  7 
	{ "253-c4.c4",    0x800000, 0x5ba92ec6, 3 | BRF_GRA },           //  8 
	{ "253-c5.c5",    0x800000, 0x3eab5557, 3 | BRF_GRA },           //  9 
	{ "253-c6.c6",    0x800000, 0x308d098b, 3 | BRF_GRA },           // 10 
	{ "253-c7.c7",    0x800000, 0xc0e995ae, 3 | BRF_GRA },           // 11 
	{ "253-c8.c8",    0x800000, 0x21a11303, 3 | BRF_GRA },           // 12 

	{ "253-m1.m1",    0x040000, 0x36a806be, 4 | BRF_ESS | BRF_PRG }, // 13 Z80 code

	{ "253-v1.v1",    0x400000, 0x263e388c, 5 | BRF_SND },           // 14 Sound data
	{ "253-v2.v2",    0x400000, 0x2c6bc7be, 5 | BRF_SND },           // 15 
	{ "253-v3.v3",    0x400000, 0x0425b27d, 5 | BRF_SND },           // 16 
	{ "253-v4.v4",    0x400000, 0xa54be8a9, 5 | BRF_SND },           // 17 
};

STDROMPICKEXT(garou, garou, neogeo)
STD_ROM_FN(garou)

static void garouSMADecrypt()
{
	for (int i = 0; i < 0x800000 / 2; i++) {
		((unsigned short*)(Neo68KROM + 0x100000))[i] = BITSWAP16(((unsigned short*)(Neo68KROM + 0x100000))[i], 13, 12, 14, 10, 8, 2, 3, 1, 5, 9, 11, 4, 15, 0, 6, 7);
	}

	for (int i = 0; i < 0x0C0000 / 2; i++) {
		((unsigned short*)Neo68KROM)[i] = ((unsigned short*)Neo68KROM)[0x710000 / 2 + BITSWAP24(i, 23, 22, 21, 20, 19, 18, 4, 5, 16, 14, 7, 9, 6, 13, 17, 15, 3, 1, 2, 12, 11, 8, 10, 0)];
	}

	for (int i = 0; i < 0x800000 / 2; i += 0x8000 / 2) {
		unsigned short nBuffer[0x8000 / 2];
		memcpy(nBuffer, &((unsigned short*)(Neo68KROM + 0x100000))[i], 0x8000);
		for (int j = 0; j < 0x8000 / 2; j++) {
			((unsigned short*)(Neo68KROM + 0x100000))[i + j] = nBuffer[BITSWAP24(j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 9, 4, 8, 3, 13, 6, 2, 7, 0, 12, 1, 11, 10, 5)];
		}
	}
}

void __fastcall garouWriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress == 0x2fffc0) {
		static unsigned int bankoffset[64] = {
			0x100000, 0x200000, 0x300000, 0x400000, // 00
			0x380000, 0x480000, 0x3d0000, 0x4d0000, // 04
			0x3f0000, 0x4f0000, 0x500000, 0x600000, // 08
			0x520000, 0x620000, 0x540000, 0x640000, // 12
			0x598000, 0x698000, 0x5a0000, 0x6a0000, // 16
			0x5a8000, 0x6a8000, 0x5b0000, 0x6b0000, // 20
			0x5b8000, 0x6b8000, 0x5c0000, 0x6c0000, // 24
			0x5c8000, 0x6c8000, 0x5d0000, 0x6d0000, // 28
			0x558000, 0x658000, 0x560000, 0x660000, // 32
			0x568000, 0x668000, 0x570000, 0x670000, // 36
			0x578000, 0x678000, 0x580000, 0x680000, // 40
			0x588000, 0x688000, 0x590000, 0x690000, // 44
			0x6d0000, 0x6d8000, 0x6e0000, 0x6e8000, // 48
			0x6f0000, 0x6f8000, 0x700000, // rest not used?
		};

		// Unscramble bank number
		unsigned int nBank = bankoffset[
			((wordValue >>  5) & 0x05) |
			((wordValue >>  8) & 0x02) |
			((wordValue >>  3) & 0x08) |
			((wordValue >> 10) & 0x10) |
			((wordValue >>  7) & 0x20)];

		if (nBank != nNeo68KROMBank) {
			nNeo68KROMBank = nBank;
			NeoSMABankswitch();
		}
	}
}

static int garouInit()
{
	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x06;

	return NeoSMAInit(garouSMADecrypt, garouWriteWordBankswitch, 0x2fffcc, 0x2ffff0);
}

struct BurnDriver BurnDrvgarou = {
	"garou", NULL, "neogeo", NULL, "1999",
	"Garou - Mark of the Wolves (set 1)\0", NULL, "SNK", "Neo Geo",
	L"\u9913\u72FC - mark of the wolves (set 1)\0Garou - mark of the wolves (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SMA_PROTECTION | HARDWARE_SNK_ENCRYPTED_A, GBF_VSFIGHT, 0,
	NULL, garouRomInfo, garouRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	garouInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Garou - Mark of the Wolves (set 2)

static struct BurnRomInfo garouoRomDesc[] = {
	{ "ke.neo-sma",   0x040000, 0x96c72233, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "253-p1.p1",    0x400000, 0x18ae5d7e, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "253-p2.p2",    0x400000, 0xafffa779, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "253-c1.c1",    0x800000, 0x0603e046, 3 | BRF_GRA },           //  3 Sprite data
	{ "253-c2.c2",    0x800000, 0x0917d2a4, 3 | BRF_GRA },           //  4 
	{ "253-c3.c3",    0x800000, 0x6737c92d, 3 | BRF_GRA },           //  5 
	{ "253-c4.c4",    0x800000, 0x5ba92ec6, 3 | BRF_GRA },           //  6 
	{ "253-c5.c5",    0x800000, 0x3eab5557, 3 | BRF_GRA },           //  7 
	{ "253-c6.c6",    0x800000, 0x308d098b, 3 | BRF_GRA },           //  8 
	{ "253-c7.c7",    0x800000, 0xc0e995ae, 3 | BRF_GRA },           //  9 
	{ "253-c8.c8",    0x800000, 0x21a11303, 3 | BRF_GRA },           // 10 

	{ "253-m1.m1",    0x040000, 0x36a806be, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "253-v1.v1",    0x400000, 0x263e388c, 5 | BRF_SND },           // 12 Sound data
	{ "253-v2.v2",    0x400000, 0x2c6bc7be, 5 | BRF_SND },           // 13 
	{ "253-v3.v3",    0x400000, 0x0425b27d, 5 | BRF_SND },           // 14 
	{ "253-v4.v4",    0x400000, 0xa54be8a9, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(garouo, garouo, neogeo)
STD_ROM_FN(garouo)

static void garouoSMADecrypt()
{
	for (int i = 0; i < 0x800000 / 2; i++) {
		((unsigned short*)(Neo68KROM + 0x100000))[i] = BITSWAP16(((unsigned short*)(Neo68KROM + 0x100000))[i], 14, 5, 1, 11, 7, 4, 10, 15, 3, 12, 8, 13, 0, 2, 9, 6);
	}

	for (int i = 0; i < 0x0c0000 / 2; i++) {
		((unsigned short*)Neo68KROM)[i] = ((unsigned short*)Neo68KROM)[0x7f8000 / 2 + BITSWAP24(i, 23, 22, 21, 20, 19, 18, 5, 16, 11, 2, 6, 7, 17, 3, 12, 8, 14, 4, 0, 9, 1, 10, 15, 13)];
	}

	for (int i = 0; i < 0x800000 / 2; i += 0x8000 / 2) {
		unsigned short nBuffer[0x8000 / 2];
		memcpy(nBuffer, &((unsigned short*)(Neo68KROM + 0x100000))[i], 0x8000);
		for (int j = 0; j < 0x8000 / 2; j++) {
			((unsigned short*)(Neo68KROM + 0x100000))[i + j] = nBuffer[BITSWAP24(j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 12, 8, 1, 7, 11, 3, 13, 10, 6, 9, 5, 4, 0, 2)];
		}
	}
}

void __fastcall garouoWriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress == 0x2fffc0) {
		static unsigned int bankoffset[64] = {
			0x100000, 0x200000, 0x300000, 0x400000, // 00
			0x380000, 0x480000, 0x3d0000, 0x4d0000, // 04
			0x3c8000, 0x4c8000, 0x500000, 0x600000, // 08
			0x520000, 0x620000, 0x540000, 0x640000, // 12
			0x698000, 0x798000, 0x6a0000, 0x7a0000, // 16
			0x6a8000, 0x7a8000, 0x6b0000, 0x7b0000, // 20
			0x6b8000, 0x7b8000, 0x6c0000, 0x7c0000, // 24
			0x6c8000, 0x7c8000, 0x6d0000, 0x7d0000, // 28
			0x558000, 0x658000, 0x560000, 0x660000, // 32
			0x568000, 0x668000, 0x570000, 0x670000, // 36
			0x578000, 0x678000, 0x580000, 0x680000, // 40
			0x588000, 0x688000, 0x590000, 0x690000, // 44
			0x6d8000, 0x7d8000, 0x6e0000, 0x7e0000, // 48
			0x6e8000, 0x7e8000, 0x7e8000, 0x100000, // 52
			0x100000, 0x100000, 0x100000, 0x100000, // 56
			0x100000, 0x100000, 0x100000, 0x100000, // 60
		};

		// Unscramble bank number
		unsigned int nBank = bankoffset[
			((wordValue >>  4) & 0x01) |
			((wordValue >>  7) & 0x02) |
			((wordValue >> 12) & 0x04) |
			((wordValue <<  1) & 0x08) |
			((wordValue >>  7) & 0x10) |
			((wordValue >>  8) & 0x20)];

		if (nBank != nNeo68KROMBank) {
			nNeo68KROMBank = nBank;
			NeoSMABankswitch();
		}
	}
}

static int garouoInit()
{
	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x06;

	return NeoSMAInit(garouoSMADecrypt, garouoWriteWordBankswitch, 0x2fffcc, 0x2ffff0);
}

struct BurnDriver BurnDrvgarouo = {
	"garouo", "garou", "neogeo", NULL, "1999",
	"Garou - Mark of the Wolves (set 2)\0", NULL, "SNK", "Neo Geo",
	L"\u9913\u72FC - mark of the wolves (set 2)\0Garou - mark of the wolves (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SMA_PROTECTION | HARDWARE_SNK_ENCRYPTED_A, GBF_VSFIGHT, 0,
	NULL, garouoRomInfo, garouoRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	garouoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Garou - Mark of the Wolves (prototype)

static struct BurnRomInfo garoupRomDesc[] = {
	{ "253-p1p.p1",   0x100000, 0xc72f0c16, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "253-p2p.p2",   0x400000, 0xbf8de565, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "253-s1p.s1",   0x020000, 0x779989de, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "253-c1p.c1",   0x800000, 0x5bb5d137, 3 | BRF_GRA },           //  3 Sprite data
	{ "253-c2p.c2",   0x800000, 0x5c8d2960, 3 | BRF_GRA },           //  4 
	{ "253-c3p.c3",   0x800000, 0x234d16fc, 3 | BRF_GRA },           //  5 
	{ "253-c4p.c4",   0x800000, 0xb9b5b993, 3 | BRF_GRA },           //  6 
	{ "253-c5p.c5",   0x800000, 0x722615d2, 3 | BRF_GRA },           //  7 
	{ "253-c6p.c6",   0x800000, 0x0a6fab38, 3 | BRF_GRA },           //  8 
	{ "253-c7p.c7",   0x800000, 0xd68e806f, 3 | BRF_GRA },           //  9 
	{ "253-c8p.c8",   0x800000, 0xf778fe99, 3 | BRF_GRA },           // 10 

	{ "253-m1p.m1",   0x040000, 0xbbe464f7, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "253-v1p.v1",   0x400000, 0x274f3923, 5 | BRF_SND },           // 12 Sound data
	{ "253-v2p.v2",   0x400000, 0x8f86dabe, 5 | BRF_SND },           // 13 
	{ "253-v3p.v3",   0x400000, 0x05fd06cd, 5 | BRF_SND },           // 14 
	{ "253-v4p.v4",   0x400000, 0x14984063, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(garoup, garoup, neogeo)
STD_ROM_FN(garoup)

struct BurnDriver BurnDrvgaroup = {
	"garoup", "garou", "neogeo", NULL, "1999",
	"Garou - Mark of the Wolves (prototype)\0", NULL, "SNK", "Neo Geo",
	L"\u9913\u72FC - mark of the wolves (prototype)\0Garou - mark of the wolves (prototype)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, garoupRomInfo, garoupRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Garou - Mark of the Wolves (bootleg)

static struct BurnRomInfo garoublRomDesc[] = {
	{ "garou-p1.bin", 0x0100000, 0xfd446d59, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "garou-p2.bin", 0x0400000, 0x3fb10a84, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "garou-s1.bin", 0x0020000, 0xdf720e33, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "garou-c1.bin", 0x1000000, 0xe2ac83fa, 3 | BRF_GRA },           //  3 Sprite data
	{ "garou-c2.bin", 0x1000000, 0x7c344b24, 3 | BRF_GRA },           //  4 
	{ "garou-c3.bin", 0x1000000, 0xd3aec5a6, 3 | BRF_GRA },           //  5 
	{ "garou-c4.bin", 0x1000000, 0xe02a242d, 3 | BRF_GRA },           //  6 

	{ "garou-m1.bin", 0x0080000, 0x7c51d002, 4 | BRF_ESS | BRF_PRG }, //  7 Z80 code

	{ "garou-v1.bin", 0x0400000, 0x98d736e4, 5 | BRF_SND },           //  8 Sound data
	{ "garou-v2.bin", 0x0400000, 0xeb43c03f, 5 | BRF_SND },           //  9 
	{ "garou-v3.bin", 0x0400000, 0x10a0f814, 5 | BRF_SND },           // 10 
	{ "garou-v4.bin", 0x0400000, 0x8918fdd3, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(garoubl, garoubl, neogeo)
STD_ROM_FN(garoubl)

static void garoubl_sx_decode()
{
	int i;
	for (i = 0; i < 0x020000; i++)
		NeoTextROM[0x20000 + i] = BITSWAP08(NeoTextROM[0x20000 + i], 7, 6, 0, 4, 3, 2, 1, 5);
}

static void garoublCallback()
{
	garoubl_sx_decode();
	lans2004_cx_decode(0x800000 * 8);
}

static int garoublInit()
{
	pNeoInitCallback = garoublCallback;

 	return NeoInit();
}

struct BurnDriver BurnDrvgaroubl = {
	"garoubl", "garou", "neogeo", NULL, "1999",
	"Garou - Mark of the Wolves (bootleg)\0", NULL, "bootleg", "Neo Geo",
	L"\u9913\u72FC - mark of the wolves (bootleg)\0Garou - mark of the wolves (bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, 0,
	NULL, garoublRomInfo, garoublRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	garoublInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Strikers 1945 Plus

static struct BurnRomInfo s1945pRomDesc[] = {
	{ "254-p1.p1",    0x100000, 0xff8efcff, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "254-p2.sp2",   0x400000, 0xefdfd4dd, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "254-c1.c1",    0x800000, 0xae6fc8ef, 3 | BRF_GRA },           //  2 Sprite data
	{ "254-c2.c2",    0x800000, 0x436fa176, 3 | BRF_GRA },           //  3 
	{ "254-c3.c3",    0x800000, 0xe53ff2dc, 3 | BRF_GRA },           //  4 
	{ "254-c4.c4",    0x800000, 0x818672f0, 3 | BRF_GRA },           //  5 
	{ "254-c5.c5",    0x800000, 0x4580eacd, 3 | BRF_GRA },           //  6 
	{ "254-c6.c6",    0x800000, 0xe34970fc, 3 | BRF_GRA },           //  7 
	{ "254-c7.c7",    0x800000, 0xf2323239, 3 | BRF_GRA },           //  8 
	{ "254-c8.c8",    0x800000, 0x66848c7d, 3 | BRF_GRA },           //  9 

	{ "254-m1.m1",    0x020000, 0x994b4487, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "254-v1.v1",    0x400000, 0x844f58fb, 5 | BRF_SND },           // 11 Sound data
	{ "254-v2.v2",    0x400000, 0xd9a248f0, 5 | BRF_SND },           // 12 
	{ "254-v3.v3",    0x400000, 0x0b0d2d33, 5 | BRF_SND },           // 13 
	{ "254-v4.v4",    0x400000, 0x6d13dc91, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(s1945p, s1945p, neogeo)
STD_ROM_FN(s1945p)

int s1945pInit()
{
	nNeoProtectionXor = 0x05;
	return NeoInit();
}

struct BurnDriver BurnDrvs1945p = {
	"s1945p", NULL, "neogeo", NULL, "1999",
	"Strikers 1945 Plus\0", NULL, "Psikyo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_A, GBF_VERSHOOT, 0,
	NULL, s1945pRomInfo, s1945pRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	s1945pInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Prehistoric Isle 2

static struct BurnRomInfo preisle2RomDesc[] = {
	{ "255-p1.p1",    0x100000, 0xdfa3c0f3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "255-p2.sp2",   0x400000, 0x42050b80, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "255-c1.c1",    0x800000, 0xea06000b, 3 | BRF_GRA },           //  2 Sprite data
	{ "255-c2.c2",    0x800000, 0x04e67d79, 3 | BRF_GRA },           //  3 
	{ "255-c3.c3",    0x800000, 0x60e31e08, 3 | BRF_GRA },           //  4 
	{ "255-c4.c4",    0x800000, 0x40371d69, 3 | BRF_GRA },           //  5 
	{ "255-c5.c5",    0x800000, 0x0b2e6adf, 3 | BRF_GRA },           //  6 
	{ "255-c6.c6",    0x800000, 0xb001bdd3, 3 | BRF_GRA },           //  7 

	{ "255-m1.m1",    0x020000, 0x8efd4014, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "255-v1.v1",    0x400000, 0x5a14543d, 5 | BRF_SND },           //  9 Sound data
	{ "255-v2.v2",    0x200000, 0x6610d91a, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(preisle2, preisle2, neogeo)
STD_ROM_FN(preisle2)

int preisle2Init()
{
	nNeoProtectionXor = 0x9F;
	return NeoInit();
}

struct BurnDriver BurnDrvpreisle2 = {
	"preisle2", NULL, "neogeo", NULL, "1999",
	"Prehistoric Isle 2\0", NULL, "Yumekobo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_A, GBF_HORSHOOT, 0,
	NULL, preisle2RomInfo, preisle2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	preisle2Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 3

static struct BurnRomInfo mslug3RomDesc[] = {
	{ "neo-sma",      0x040000, 0x9cd55736, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "256-pg1.p1",   0x400000, 0xb07edfd5, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "256-pg2.p2",   0x400000, 0x6097c26b, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "256-c1.c1",    0x800000, 0x5a79c34e, 3 | BRF_GRA },           //  3 Sprite data
	{ "256-c2.c2",    0x800000, 0x944c362c, 3 | BRF_GRA },           //  4 
	{ "256-c3.c3",    0x800000, 0x6e69d36f, 3 | BRF_GRA },           //  5 
	{ "256-c4.c4",    0x800000, 0xb755b4eb, 3 | BRF_GRA },           //  6 
	{ "256-c5.c5",    0x800000, 0x7aacab47, 3 | BRF_GRA },           //  7 
	{ "256-c6.c6",    0x800000, 0xc698fd5d, 3 | BRF_GRA },           //  8 
	{ "256-c7.c7",    0x800000, 0xcfceddd2, 3 | BRF_GRA },           //  9 
	{ "256-c8.c8",    0x800000, 0x4d9be34c, 3 | BRF_GRA },           // 10 

	{ "256-m1.m1",    0x080000, 0xeaeec116, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "256-v1.v1",    0x400000, 0xf2690241, 5 | BRF_SND },           // 12 Sound data
	{ "256-v2.v2",    0x400000, 0x7e2a10bd, 5 | BRF_SND },           // 13 
	{ "256-v3.v3",    0x400000, 0x0eaec17c, 5 | BRF_SND },           // 14 
	{ "256-v4.v4",    0x400000, 0x9b4b22d4, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(mslug3, mslug3, neogeo)
STD_ROM_FN(mslug3)

static void mslug3SMADecrypt()
{
	for (int i = 0; i < 0x800000 / 2; i++) {
		((unsigned short*)(Neo68KROM + 0x100000))[i] = BITSWAP16(((unsigned short*)(Neo68KROM + 0x100000))[i], 4, 11, 14, 3, 1, 13, 0, 7, 2, 8, 12, 15, 10, 9, 5, 6);
	}

	for (int i = 0; i < 0x0c0000 / 2; i++) {
		((unsigned short*)Neo68KROM)[i] = ((unsigned short*)Neo68KROM)[0x5D0000 / 2 + BITSWAP24(i, 23, 22, 21, 20, 19, 18, 15, 2, 1, 13, 3, 0, 9, 6, 16, 4, 11, 5, 7, 12, 17, 14, 10, 8)];
	}

	for (int i = 0; i < 0x800000 / 2; i += 0x010000 / 2) {
		unsigned short nBuffer[0x010000 / 2];
		memcpy(nBuffer, &((unsigned short*)(Neo68KROM + 0x100000))[i], 0x010000);
		for (int j = 0; j < 0x010000 / 2; j++) {
			((unsigned short*)(Neo68KROM + 0x100000))[i + j] = nBuffer[BITSWAP24(j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 2, 11, 0, 14, 6, 4, 13, 8, 9, 3, 10, 7, 5, 12, 1)];
		}
	}
}
void __fastcall mslug3WriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress == 0x2fffe4) {
		static unsigned int bankoffset[64] = {
			0x100000, 0x120000, 0x140000, 0x160000, // 00
			0x170000, 0x190000, 0x1b0000, 0x1d0000, // 04
			0x1e0000, 0x1f0000, 0x220000, 0x230000, // 08
			0x240000, 0x250000, 0x280000, 0x290000, // 12
			0x2a0000, 0x2b0000, 0x2e0000, 0x2f0000, // 16
			0x300000, 0x310000, 0x340000, 0x350000, // 20
			0x360000, 0x370000, 0x3a0000, 0x3b0000, // 24
			0x3c0000, 0x3d0000, 0x400000, 0x410000, // 28
			0x420000, 0x430000, 0x460000, 0x470000, // 32
			0x480000, 0x490000, 0x4c0000, 0x4d0000, // 36
			0x500000, 0x510000, 0x540000, 0x550000, // 40
			0x560000, 0x570000, 0x5a0000, 0x5b0000, // 44
			0x5c0000, // rest not used?
		};

		// Unscramble bank number
		unsigned int nBank = bankoffset[
			((wordValue >> 14) & 0x01) |
			((wordValue >> 11) & 0x02) |
			((wordValue >> 13) & 0x04) |
			((wordValue >>  3) & 0x08) |
			((wordValue <<  1) & 0x10) |
			((wordValue >>  4) & 0x20)];

		if (nBank != nNeo68KROMBank) {
			nNeo68KROMBank = nBank;
			NeoSMABankswitch();
		}
	}
}

static int mslug3Init()
{
	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0xAD;

	return NeoSMAInit(mslug3SMADecrypt, mslug3WriteWordBankswitch, 0, 0);
}

struct BurnDriver BurnDrvmslug3 = {
	"mslug3", NULL, "neogeo", NULL, "2000",
	"Metal Slug 3\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SMA_PROTECTION | HARDWARE_SNK_ENCRYPTED_A, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug3RomInfo, mslug3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug3Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 3 (not encrypted)

static struct BurnRomInfo mslug3hRomDesc[] = {
	{ "256-ph1.p1",   0x100000, 0x9c42ca85, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "256-ph2.sp2",  0x400000, 0x1f3d8ce8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "256-c1.c1",    0x800000, 0x5a79c34e, 3 | BRF_GRA },           //  2 Sprite data
	{ "256-c2.c2",    0x800000, 0x944c362c, 3 | BRF_GRA },           //  3 
	{ "256-c3.c3",    0x800000, 0x6e69d36f, 3 | BRF_GRA },           //  4 
	{ "256-c4.c4",    0x800000, 0xb755b4eb, 3 | BRF_GRA },           //  5 
	{ "256-c5.c5",    0x800000, 0x7aacab47, 3 | BRF_GRA },           //  6 
	{ "256-c6.c6",    0x800000, 0xc698fd5d, 3 | BRF_GRA },           //  7 
	{ "256-c7.c7",    0x800000, 0xcfceddd2, 3 | BRF_GRA },           //  8 
	{ "256-c8.c8",    0x800000, 0x4d9be34c, 3 | BRF_GRA },           //  9 

	{ "256-m1.m1",    0x080000, 0xeaeec116, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "256-v1.v1",    0x400000, 0xf2690241, 5 | BRF_SND },           // 11 Sound data
	{ "256-v2.v2",    0x400000, 0x7e2a10bd, 5 | BRF_SND },           // 12 
	{ "256-v3.v3",    0x400000, 0x0eaec17c, 5 | BRF_SND },           // 13 
	{ "256-v4.v4",    0x400000, 0x9b4b22d4, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(mslug3h, mslug3h, neogeo)
STD_ROM_FN(mslug3h)

int mslug3hInit()
{
	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0xAD;
	return NeoInit();
}

struct BurnDriver BurnDrvmslug3h = {
	"mslug3h", "mslug3", "neogeo", NULL, "2000",
	"Metal Slug 3 (not encrypted)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_A, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug3hRomInfo, mslug3hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug3hInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2000

static struct BurnRomInfo kof2000RomDesc[] = {
	{ "neo-sma",      0x040000, 0x71c6e6bb, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "257-p1.p1",    0x400000, 0x60947b4c, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "257-p2.p2",    0x400000, 0x1b7ec415, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "257-c1.c1",    0x800000, 0xcef1cdfa, 3 | BRF_GRA },           //  3 Sprite data
	{ "257-c2.c2",    0x800000, 0xf7bf0003, 3 | BRF_GRA },           //  4 
	{ "257-c3.c3",    0x800000, 0x101e6560, 3 | BRF_GRA },           //  5 
	{ "257-c4.c4",    0x800000, 0xbd2fc1b1, 3 | BRF_GRA },           //  6 
	{ "257-c5.c5",    0x800000, 0x89775412, 3 | BRF_GRA },           //  7 
	{ "257-c6.c6",    0x800000, 0xfa7200d5, 3 | BRF_GRA },           //  8 
	{ "257-c7.c7",    0x800000, 0x7da11fe4, 3 | BRF_GRA },           //  9 
	{ "257-c8.c8",    0x800000, 0xb1afa60b, 3 | BRF_GRA },           // 10 

	{ "257-m1.m1",    0x040000, 0x4b749113, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "257-v1.v1",    0x400000, 0x17cde847, 5 | BRF_SND },           // 12 Sound data
	{ "257-v2.v2",    0x400000, 0x1afb20ff, 5 | BRF_SND },           // 13 
	{ "257-v3.v3",    0x400000, 0x4605036a, 5 | BRF_SND },           // 14 
	{ "257-v4.v4",    0x400000, 0x764bbd6b, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kof2000, kof2000, neogeo)
STD_ROM_FN(kof2000)

static void kof2000SMADecrypt()
{
	for (int i = 0; i < 0x800000 / 2; i++) {
		((unsigned short*)(Neo68KROM + 0x100000))[i] = BITSWAP16(((unsigned short*)(Neo68KROM + 0x100000))[i], 12, 8, 11, 3, 15, 14, 7, 0, 10, 13, 6, 5, 9, 2, 1, 4);
	}

	for (int i = 0; i < 0x0c0000 / 2; i++) {
		((unsigned short*)Neo68KROM)[i] = ((unsigned short*)Neo68KROM)[0x73a000 / 2 + BITSWAP24(i, 23, 22, 21, 20, 19, 18, 8, 4, 15, 13, 3, 14, 16, 2, 6, 17, 7, 12, 10, 0, 5, 11, 1, 9)];
	}

	for (int i = 0; i < 0x63a000 / 2; i += 0x0800 / 2) {
		unsigned short nBuffer[0x0800 / 2];
		memcpy(nBuffer, &((unsigned short*)(Neo68KROM + 0x100000))[i], 0x0800);
		for (int j = 0; j < 0x0800 / 2; j++) {
			((unsigned short*)(Neo68KROM + 0x100000))[i + j] = nBuffer[BITSWAP24(j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 4, 1, 3, 8, 6, 2, 7, 0, 9, 5)];
		}
	}
}

void __fastcall kof2000WriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress == 0x2fffec) {
		static unsigned int bankoffset[64] = {
			0x100000, 0x200000, 0x300000, 0x400000, // 00
			0x4f7800, 0x5f7800, 0x4ff800, 0x5ff800, // 04
			0x507800, 0x607800, 0x50f800, 0x60f800, // 08
			0x516800, 0x616800, 0x51d800, 0x61d800, // 12
			0x524000, 0x624000, 0x623800, 0x723800, // 16
			0x626000, 0x726000, 0x628000, 0x728000, // 20
			0x62a000, 0x72a000, 0x62b800, 0x72b800, // 24
			0x62d000, 0x72d000, 0x62e800, 0x72e800, // 28
			0x718000, 0x719000, 0x71a000, 0x71a800, // 32
		};

		// Unscramble bank number
		unsigned int nBank = bankoffset[
			((wordValue >> 15) & 0x01) |
			((wordValue >> 13) & 0x02) |
			((wordValue >>  5) & 0x04) |
			((wordValue >>  6) & 0x10) |
			(wordValue & 0x28)];

		if (nBank != nNeo68KROMBank) {
			nNeo68KROMBank = nBank;
			NeoSMABankswitch();
		}
	}
}

static int kof2000Init()
{
	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x00;

	return NeoSMAInit(kof2000SMADecrypt, kof2000WriteWordBankswitch, 0x2fffd8, 0x2fffda);
}

struct BurnDriver BurnDrvkof2000 = {
	"kof2000", NULL, "neogeo", NULL, "2000",
	"The King of Fighters 2000\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_SMA_PROTECTION | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2000RomInfo, kof2000RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2000Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2000 (not encrypted)

static struct BurnRomInfo kof2000nRomDesc[] = {
	{ "257-pg1.p1",   0x100000, 0x5f809dbe, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "257-pg2.sp2",  0x400000, 0x693c2c5e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "257-c1.c1",    0x800000, 0xcef1cdfa, 3 | BRF_GRA },           //  2 Sprite data
	{ "257-c2.c2",    0x800000, 0xf7bf0003, 3 | BRF_GRA },           //  3 
	{ "257-c3.c3",    0x800000, 0x101e6560, 3 | BRF_GRA },           //  4 
	{ "257-c4.c4",    0x800000, 0xbd2fc1b1, 3 | BRF_GRA },           //  5 
	{ "257-c5.c5",    0x800000, 0x89775412, 3 | BRF_GRA },           //  6 
	{ "257-c6.c6",    0x800000, 0xfa7200d5, 3 | BRF_GRA },           //  7 
	{ "257-c7.c7",    0x800000, 0x7da11fe4, 3 | BRF_GRA },           //  8 
	{ "257-c8.c8",    0x800000, 0xb1afa60b, 3 | BRF_GRA },           //  9 

	{ "257-m1.m1",    0x040000, 0x4b749113, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "257-v1.v1",    0x400000, 0x17cde847, 5 | BRF_SND },           // 11 Sound data
	{ "257-v2.v2",    0x400000, 0x1afb20ff, 5 | BRF_SND },           // 12 
	{ "257-v3.v3",    0x400000, 0x4605036a, 5 | BRF_SND },           // 13 
	{ "257-v4.v4",    0x400000, 0x764bbd6b, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kof2000n, kof2000n, neogeo)
STD_ROM_FN(kof2000n)

int kof2000nInit()
{
	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x00;
	return NeoInit();
}

struct BurnDriver BurnDrvkof2000n = {
	"kof2000n", "kof2000", "neogeo", NULL, "2000",
	"The King of Fighters 2000 (not encrypted)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2000nRomInfo, kof2000nRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2000nInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Bang Bead

static struct BurnRomInfo bangbeadRomDesc[] = {
	{ "259-p1.p1",    0x200000, 0x88a37f8b, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "259-c1.c1",    0x800000, 0x1f537f74, 3 | BRF_GRA },           //  1 Sprite data
	{ "259-c2.c2",    0x800000, 0x0efd98ff, 3 | BRF_GRA },           //  2 

	{ "259-m1.m1",    0x020000, 0x85668ee9, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "259-v1.v1",    0x400000, 0x088eb8ab, 5 | BRF_SND },           //  4 Sound data
	{ "259-v2.v2",    0x100000, 0x97528fe9, 5 | BRF_SND },           //  5 
};

STDROMPICKEXT(bangbead, bangbead, neogeo)
STD_ROM_FN(bangbead)

int bangbeadInit()
{
	nNeoProtectionXor = 0xF8;
	return NeoInit();
}

struct BurnDriver BurnDrvbangbead = {
	"bangbead", NULL, "neogeo", NULL, "2000",
	"Bang Bead\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP | HARDWARE_SNK_ENCRYPTED_A, GBF_BALLPADDLE, 0,
	NULL, bangbeadRomInfo, bangbeadRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	bangbeadInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Bang Bead (Prototype?)

static struct BurnRomInfo bangbedpRomDesc[] = {
	{ "259-p1.bin",   0x200000, 0x88a37f8b, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "259-s1p.bin",  0x020000, 0xbb50fb2d, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "259-c1p.bin",  0x200000, 0xe3919e44, 3 | BRF_GRA },           //  2 Sprite data
	{ "259-c2p.bin",  0x200000, 0xbaf5a320, 3 | BRF_GRA },           //  3 
	{ "259-c3p.bin",  0x100000, 0xc8e52157, 3 | BRF_GRA },           //  4 
	{ "259-c4p.bin",  0x100000, 0x69fa8e60, 3 | BRF_GRA },           //  5 

	{ "259-m1.bin",   0x020000, 0x85668ee9, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "259-v1p.bin",  0x200000, 0xe97b9385, 5 | BRF_SND },           //  7 Sound data
	{ "259-v2p.bin",  0x200000, 0xb0cbd70a, 5 | BRF_SND },           //  8 
	{ "259-v3p.bin",  0x100000, 0x97528fe9, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(bangbedp, bangbedp, neogeo)
STD_ROM_FN(bangbedp)

struct BurnDriver BurnDrvbangbedp = {
	"bangbedp", "bangbead", "neogeo", NULL, "2000",
	"Bang Bead (Prototype?)\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_BALLPADDLE, 0,
	NULL, bangbedpRomInfo, bangbedpRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Nightmare in the Dark

static struct BurnRomInfo nitdRomDesc[] = {
	{ "260-p1.p1",    0x080000, 0x61361082, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "260-c1.c1",    0x800000, 0x147b0c7f, 3 | BRF_GRA },           //  1 Sprite data
	{ "260-c2.c2",    0x800000, 0xd2b04b0d, 3 | BRF_GRA },           //  2 

	{ "260-m1.m1",    0x080000, 0x6407c5e5, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "260-v1.v1",    0x400000, 0x24b0480c, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(nitd, nitd, neogeo)
STD_ROM_FN(nitd)

int nitdInit()
{
	nNeoProtectionXor = 0xFF;
	return NeoInit();
}

struct BurnDriver BurnDrvnitd = {
	"nitd", NULL, "neogeo", NULL, "2000",
	"Nightmare in the Dark\0", NULL, "Eleven / Gavaking", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_A, GBF_PLATFORM, 0,
	NULL, nitdRomInfo, nitdRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	nitdInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Nightmare in the Dark (bootleg)

static struct BurnRomInfo nitdblRomDesc[] = {
	{ "nitd-p1.bin", 0x080000, 0x1a05bd1b, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "nitd-s1.bin", 0x020000, 0xdd3bf47c, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "nitd-c1.bin", 0x200000, 0xb4353190, 3 | BRF_GRA },           //  2 Sprite data
	{ "nitd-c2.bin", 0x200000, 0x6e27511f, 3 | BRF_GRA },           //  3 
	{ "nitd-c3.bin", 0x200000, 0x472cf075, 3 | BRF_GRA },           //  4 
	{ "nitd-c4.bin", 0x200000, 0x4c3926e6, 3 | BRF_GRA },           //  5 

	{ "260-m1.m1",   0x080000, 0x6407c5e5, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "nitd-v1.bin", 0x200000, 0x79008868, 5 | BRF_SND },           //  7 Sound data
	{ "nitd-v2.bin", 0x200000, 0x728558f9, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(nitdbl, nitdbl, neogeo)
STD_ROM_FN(nitdbl)

struct BurnDriver BurnDrvnitdbl = {
	"nitdbl", "nitd", "neogeo", NULL, "2001",
	"Nightmare in the Dark (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, nitdblRomInfo, nitdblRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Zupapa!

static struct BurnRomInfo zupapaRomDesc[] = {
	{ "070-p1.p1",    0x100000, 0x5a96203e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "070-c1.c1",    0x800000, 0xf8ad02d8, 3 | BRF_GRA },           //  1 Sprite data
	{ "070-c2.c2",    0x800000, 0x70156dde, 3 | BRF_GRA },           //  2 

	{ "070-m1.m1",    0x020000, 0x5a3b3191, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "070-v1.v1",    0x200000, 0xd3a7e1ff, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(zupapa, zupapa, neogeo)
STD_ROM_FN(zupapa)

int zupapaInit()
{
	nNeoProtectionXor = 0xBD;
	return NeoInit();
}

struct BurnDriver BurnDrvzupapa = {
	"zupapa", NULL, "neogeo", NULL, "2001",
	"Zupapa!\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_A, GBF_PLATFORM, 0,
	NULL, zupapaRomInfo, zupapaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	zupapaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Sengoku 3

static struct BurnRomInfo sengoku3RomDesc[] = {
	{ "261-ph1.p1",   0x200000, 0xe0d4bc0a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "261-c1.c1",    0x800000, 0xded84d9c, 3 | BRF_GRA },           //  1 Sprite data
	{ "261-c2.c2",    0x800000, 0xb8eb4348, 3 | BRF_GRA },           //  2 
	{ "261-c3.c3",    0x800000, 0x84e2034a, 3 | BRF_GRA },           //  3 
	{ "261-c4.c4",    0x800000, 0x0b45ae53, 3 | BRF_GRA },           //  4 

	{ "261-m1.m1",    0x080000, 0x7d501c39, 4 | BRF_ESS | BRF_PRG }, //  5 Z80 code

	{ "261-v1.v1",    0x400000, 0x64c30081, 5 | BRF_SND },           //  6 Sound data
	{ "261-v2.v2",    0x400000, 0x392a9c47, 5 | BRF_SND },           //  7 
	{ "261-v3.v3",    0x400000, 0xc1a7ebe3, 5 | BRF_SND },           //  8 
	{ "261-v4.v4",    0x200000, 0x9000d085, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(sengoku3, sengoku3, neogeo)
STD_ROM_FN(sengoku3)

int sengoku3Init()
{
	nNeoProtectionXor = 0xFE;
	return NeoInit();
}

struct BurnDriver BurnDrvsengoku3 = {
	"sengoku3", NULL, "neogeo", NULL, "2001",
	"Sengoku 3\0", NULL, "SNK", "Neo Geo",
	L"Sengoku 3\0\u6226\u56FD\u4F1D\u627F\uFF12\uFF10\uFF10\uFF11\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP | HARDWARE_SNK_ENCRYPTED_A, GBF_SCRFIGHT, 0,
	NULL, sengoku3RomInfo, sengoku3RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	sengoku3Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// The King of Fighters 2001 (set 1)

static struct BurnRomInfo kof2001RomDesc[] = {
	{ "262-p1-08-e0.p1",       0x100000, 0x9381750d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "262-p2-08-e0.sp2",      0x400000, 0x8e0d8329, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "262-c1-08-e0.c1",       0x800000, 0x99cc785a, 3 | BRF_GRA },     //  2 Sprite data
	{ "262-c2-08-e0.c2",       0x800000, 0x50368cbf, 3 | BRF_GRA },     //  3 
	{ "262-c3-08-e0.c3",       0x800000, 0xfb14ff87, 3 | BRF_GRA },     //  4 
	{ "262-c4-08-e0.c4",       0x800000, 0x4397faf8, 3 | BRF_GRA },     //  5 
	{ "262-c5-08-e0.c5",       0x800000, 0x91f24be4, 3 | BRF_GRA },     //  6 
	{ "262-c6-08-e0.c6",       0x800000, 0xa31e4403, 3 | BRF_GRA },     //  7 
	{ "262-c7-08-e0.c7",       0x800000, 0x54d9d1ec, 3 | BRF_GRA },     //  8 
	{ "262-c8-08-e0.c8",       0x800000, 0x59289a6b, 3 | BRF_GRA },     //  9 

	{ "265-262-m1.m1",         0x040000, 0xa7f8119f, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "262-v1-08-e0.v1",       0x400000, 0x83d49ecf, 5 | BRF_SND },     // 11 Sound data
	{ "262-v2-08-e0.v2",       0x400000, 0x003f1843, 5 | BRF_SND },     // 12 
	{ "262-v3-08-e0.v3",       0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 13 
	{ "262-v4-08-e0.v4",       0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 14
};

STDROMPICKEXT(kof2001, kof2001, neogeo)
STD_ROM_FN(kof2001)

int kof2001Init()
{
	nNeoProtectionXor = 0x1E;
	return NeoInit();
}

struct BurnDriver BurnDrvkof2001 = {
	"kof2001", NULL, "neogeo", NULL, "2001",
	"The King of Fighters 2001 (set 1)\0", NULL, "Eolith / SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2001RomInfo, kof2001RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2001Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2001 (set 2)

static struct BurnRomInfo kof2001hRomDesc[] = {
	{ "262-pg1.p1",            0x100000, 0x2af7e741, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "262-pg2.sp2",           0x400000, 0x91eea062, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "262-c1-08-e0.c1",       0x800000, 0x99cc785a, 3 | BRF_GRA },     //  2 Sprite data
	{ "262-c2-08-e0.c2",       0x800000, 0x50368cbf, 3 | BRF_GRA },     //  3 
	{ "262-c3-08-e0.c3",       0x800000, 0xfb14ff87, 3 | BRF_GRA },     //  4 
	{ "262-c4-08-e0.c4",       0x800000, 0x4397faf8, 3 | BRF_GRA },     //  5 
	{ "262-c5-08-e0.c5",       0x800000, 0x91f24be4, 3 | BRF_GRA },     //  6 
	{ "262-c6-08-e0.c6",       0x800000, 0xa31e4403, 3 | BRF_GRA },     //  7 
	{ "262-c7-08-e0.c7",       0x800000, 0x54d9d1ec, 3 | BRF_GRA },     //  8 
	{ "262-c8-08-e0.c8",       0x800000, 0x59289a6b, 3 | BRF_GRA },     //  9 

	{ "265-262-m1.m1",         0x040000, 0xa7f8119f, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "262-v1-08-e0.v1",       0x400000, 0x83d49ecf, 5 | BRF_SND },     // 11 Sound data
	{ "262-v2-08-e0.v2",       0x400000, 0x003f1843, 5 | BRF_SND },     // 12 
	{ "262-v3-08-e0.v3",       0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 13 
	{ "262-v4-08-e0.v4",       0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 14
};

STDROMPICKEXT(kof2001h, kof2001h, neogeo)
STD_ROM_FN(kof2001h)

struct BurnDriver BurnDrvkof2001h = {
	"kof2001h", "kof2001", "neogeo", NULL, "2001",
	"The King of Fighters 2001 (set 2)\0", NULL, "Eolith / SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2001hRomInfo, kof2001hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2001Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 4 (set 1)

static struct BurnRomInfo mslug4RomDesc[] = {
	{ "263-p1.p1",    0x100000, 0x27e4def3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "263-p2.sp2",   0x400000, 0xfdb7aed8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "263-c1.c1",    0x800000, 0x84865f8a, 3 | BRF_GRA },           //  2 Sprite data
	{ "263-c2.c2",    0x800000, 0x81df97f2, 3 | BRF_GRA },           //  3 
	{ "263-c3.c3",    0x800000, 0x1a343323, 3 | BRF_GRA },           //  4 
	{ "263-c4.c4",    0x800000, 0x942cfb44, 3 | BRF_GRA },           //  5 
	{ "263-c5.c5",    0x800000, 0xa748854f, 3 | BRF_GRA },           //  6 
	{ "263-c6.c6",    0x800000, 0x5c8ba116, 3 | BRF_GRA },           //  7 

	{ "263-m1.m1",    0x020000, 0x46ac8228, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "263-v1.v1",    0x800000, 0x01e9b9cd, 5 | BRF_SND },           //  9 Sound data
	{ "263-v2.v2",    0x800000, 0x4ab2bf81, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(mslug4, mslug4, neogeo)
STD_ROM_FN(mslug4)

int mslug4Init()
{
	int nRet;

	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x31;

	nRet = NeoInit();

	if (nRet == 0) {
		for (int i = 0; i < 0x1000000 / 2; i += 8 / 2) {
			unsigned short buffer[8 / 2];
			memcpy(buffer, ((unsigned short*)YM2610ADPCMAROM) + i, 8);
			for (int j = 0; j < 8 / 2; j++) {
				((unsigned short*)YM2610ADPCMAROM)[i + j] = buffer[j ^ 2];
			}
		}
	}

	return nRet;
}

struct BurnDriver BurnDrvmslug4 = {
	"mslug4", NULL, "neogeo", NULL, "2002",
	"Metal Slug 4 (set 1)\0", NULL, "Mega", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug4RomInfo, mslug4RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug4Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 4 (set 2)

static struct BurnRomInfo mslug4hRomDesc[] = {
	{ "263-ph1.p1",   0x100000, 0xc67f5c8d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "263-ph2.sp2",  0x400000, 0xbc3ec89e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "263-c1.c1",    0x800000, 0x84865f8a, 3 | BRF_GRA },           //  2 Sprite data
	{ "263-c2.c2",    0x800000, 0x81df97f2, 3 | BRF_GRA },           //  3 
	{ "263-c3.c3",    0x800000, 0x1a343323, 3 | BRF_GRA },           //  4 
	{ "263-c4.c4",    0x800000, 0x942cfb44, 3 | BRF_GRA },           //  5 
	{ "263-c5.c5",    0x800000, 0xa748854f, 3 | BRF_GRA },           //  6 
	{ "263-c6.c6",    0x800000, 0x5c8ba116, 3 | BRF_GRA },           //  7 

	{ "263-m1.m1",    0x020000, 0x46ac8228, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "263-v1.v1",    0x800000, 0x01e9b9cd, 5 | BRF_SND },           //  9 Sound data
	{ "263-v2.v2",    0x800000, 0x4ab2bf81, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(mslug4h, mslug4h, neogeo)
STD_ROM_FN(mslug4h)

struct BurnDriver BurnDrvmslug4h = {
	"mslug4h", "mslug4", "neogeo", NULL, "2002",
	"Metal Slug 4 (set 2)\0", NULL, "Mega", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug4hRomInfo, mslug4hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug4Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 4 Plus (bootleg)

static struct BurnRomInfo ms4plusRomDesc[] = {
	{ "ms4-p1p.bin",  0x100000, 0x806a6e04, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "263-p2.sp2",   0x400000, 0xfdb7aed8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "ms4-s1p.bin",  0x020000, 0x07ff87ce, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "263-c1.c1",    0x800000, 0x84865f8a, 3 | BRF_GRA },           //  3 Sprite data
	{ "263-c2.c2",    0x800000, 0x81df97f2, 3 | BRF_GRA },           //  4 
	{ "263-c3.c3",    0x800000, 0x1a343323, 3 | BRF_GRA },           //  5 
	{ "263-c4.c4",    0x800000, 0x942cfb44, 3 | BRF_GRA },           //  6 
	{ "263-c5.c5",    0x800000, 0xa748854f, 3 | BRF_GRA },           //  7 
	{ "263-c6.c6",    0x800000, 0x5c8ba116, 3 | BRF_GRA },           //  8 

	{ "263-m1.m1",    0x020000, 0x46ac8228, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "263-v1.v1",    0x800000, 0x01e9b9cd, 5 | BRF_SND },           //  9 Sound data
	{ "263-v2.v2",    0x800000, 0x4ab2bf81, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(ms4plus, ms4plus, neogeo)
STD_ROM_FN(ms4plus)

int ms4plusInit()
{
	int nRet;

	nNeoProtectionXor = 0x31;

	nRet = NeoInit();

	if (nRet == 0) {
		for (int i = 0; i < 0x1000000 / 2; i += 8 / 2) {
			unsigned short buffer[8 / 2];
			memcpy(buffer, ((unsigned short*)YM2610ADPCMAROM) + i, 8);
			for (int j = 0; j < 8 / 2; j++) {
				((unsigned short*)YM2610ADPCMAROM)[i + j] = buffer[j ^ 2];
			}
		}
	}

	return nRet;
}

struct BurnDriver BurnDrvms4plus = {
	"ms4plus", "mslug4", "neogeo", NULL, "2002",
	"Metal Slug 4 Plus (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, ms4plusRomInfo, ms4plusRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	ms4plusInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Rage of the Dragons

static struct BurnRomInfo rotdRomDesc[] = {
	{ "264-p1.p1",    0x800000, 0xb8cc969d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "264-c1.c1",    0x800000, 0x4f148fee, 3 | BRF_GRA },           //  1 Sprite data
	{ "264-c2.c2",    0x800000, 0x7cf5ff72, 3 | BRF_GRA },           //  2 
	{ "264-c3.c3",    0x800000, 0x64d84c98, 3 | BRF_GRA },           //  3 
	{ "264-c4.c4",    0x800000, 0x2f394a95, 3 | BRF_GRA },           //  4 
	{ "264-c5.c5",    0x800000, 0x6b99b978, 3 | BRF_GRA },           //  5 
	{ "264-c6.c6",    0x800000, 0x847d5c7d, 3 | BRF_GRA },           //  6 
	{ "264-c7.c7",    0x800000, 0x231d681e, 3 | BRF_GRA },           //  7 
	{ "264-c8.c8",    0x800000, 0xc5edb5c4, 3 | BRF_GRA },           //  8 

	{ "264-m1.m1",    0x020000, 0x4dbd7b43, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "264-v1.v1",    0x800000, 0xfa005812, 5 | BRF_SND },           // 10 Sound data
	{ "264-v2.v2",    0x800000, 0xc3dc8bf0, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(rotd, rotd, neogeo)
STD_ROM_FN(rotd)

int rotdInit()
{
	int nRet;

	nNeoProtectionXor = 0x3F;

	nRet = NeoInit();

	if (nRet == 0) {
		for (int i = 0; i < 0x1000000 / 2; i += 16 / 2) {
			unsigned short buffer[16 / 2];
			memcpy(buffer, ((unsigned short*)YM2610ADPCMAROM) + i, 16);
			for (int j = 0; j < 16 / 2; j++) {
				((unsigned short*)YM2610ADPCMAROM)[i + j] = buffer[j ^ 4];
			}
		}
	}

	return nRet;
}

struct BurnDriver BurnDrvrotd = {
	"rotd", NULL, "neogeo", NULL, "2002",
	"Rage of the Dragons\0", NULL, "Evoga / Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, 0,
	NULL, rotdRomInfo, rotdRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	rotdInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// The King of Fighters 2002

static struct BurnRomInfo kof2002RomDesc[] = {
	{ "265-p1.p1",    0x100000, 0x9ede7323, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "265-p2.sp2",   0x400000, 0x327266b8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "265-c1.c1",    0x800000, 0x2b65a656, 3 | BRF_GRA },           //  2 Sprite data
	{ "265-c2.c2",    0x800000, 0xadf18983, 3 | BRF_GRA },           //  3 
	{ "265-c3.c3",    0x800000, 0x875e9fd7, 3 | BRF_GRA },           //  4 
	{ "265-c4.c4",    0x800000, 0x2da13947, 3 | BRF_GRA },           //  5 
	{ "265-c5.c5",    0x800000, 0x61bd165d, 3 | BRF_GRA },           //  6 
	{ "265-c6.c6",    0x800000, 0x03fdd1eb, 3 | BRF_GRA },           //  7 
	{ "265-c7.c7",    0x800000, 0x1a2749d8, 3 | BRF_GRA },           //  8 
	{ "265-c8.c8",    0x800000, 0xab0bb549, 3 | BRF_GRA },           //  9 

	{ "265-m1.m1",    0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "265-v1.v1",    0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 11 Sound data
	{ "265-v2.v2",    0x800000, 0xda41d6f9, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof2002, kof2002, neogeo)
STD_ROM_FN(kof2002)

static void kof2002Callback()
{
	unsigned char *dst = (unsigned char*)malloc( 0x400000 );
	if (dst) {
		unsigned int nBank[] = { 0x100000, 0x280000, 0x300000, 0x180000, 0x000000, 0x380000, 0x200000, 0x080000 };

		memcpy(dst, Neo68KROM + 0x100000, 0x400000);

		for (int i = 0; i < 0x400000 / 0x080000; i++) {
			memcpy(Neo68KROM + 0x100000 + i * 0x080000, dst + nBank[i], 0x080000);
		}

		free(dst);
	}
}

int kof2002Init()
{
	int nRet;

	nNeoProtectionXor = 0xEC;
	pNeoInitCallback = kof2002Callback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa5000, 0x000000, { 0xf9, 0xe0, 0x5d, 0xf3, 0xea, 0x92, 0xbe, 0xef } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkof2002 = {
	"kof2002", NULL, "neogeo", NULL, "2002",
	"The King of Fighters 2002\0", NULL, "Eolith / Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2002RomInfo, kof2002RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2002Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2002 Plus (bootleg set 1)

static struct BurnRomInfo kf2k2plsRomDesc[] = {
	{ "2k2-p1p.bin",  0x100000, 0x3ab03781, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "265-p2.sp2",   0x400000, 0x327266b8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "2k2-s1p.bin",  0x020000, 0x595e0006, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "265-c1.c1",    0x800000, 0x2b65a656, 3 | BRF_GRA },           //  3 Sprite data
	{ "265-c2.c2",    0x800000, 0xadf18983, 3 | BRF_GRA },           //  4 
	{ "265-c3.c3",    0x800000, 0x875e9fd7, 3 | BRF_GRA },           //  5 
	{ "265-c4.c4",    0x800000, 0x2da13947, 3 | BRF_GRA },           //  6 
	{ "265-c5.c5",    0x800000, 0x61bd165d, 3 | BRF_GRA },           //  7 
	{ "265-c6.c6",    0x800000, 0x03fdd1eb, 3 | BRF_GRA },           //  8 
	{ "265-c7.c7",    0x800000, 0x1a2749d8, 3 | BRF_GRA },           //  9 
	{ "265-c8.c8",    0x800000, 0xab0bb549, 3 | BRF_GRA },           // 10 

	{ "265-m1.m1",    0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "265-v1.v1",    0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 12 Sound data
	{ "265-v2.v2",    0x800000, 0xda41d6f9, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kf2k2pls, kf2k2pls, neogeo)
STD_ROM_FN(kf2k2pls)

struct BurnDriver BurnDrvkf2k2pls = {
	"kf2k2pls", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 Plus (bootleg set 1)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k2plsRomInfo, kf2k2plsRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2002Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2002 Plus (bootleg set 2)

static struct BurnRomInfo kf2k2plaRomDesc[] = {
	{ "2k2-p1pa.bin", 0x100000, 0x6a3a02f3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "265-p2.sp2",   0x400000, 0x327266b8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "2k2-s1pa.bin", 0x020000, 0x1a3ed064, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "265-c1.c1",    0x800000, 0x2b65a656, 3 | BRF_GRA },           //  3 Sprite data
	{ "265-c2.c2",    0x800000, 0xadf18983, 3 | BRF_GRA },           //  4 
	{ "265-c3.c3",    0x800000, 0x875e9fd7, 3 | BRF_GRA },           //  5 
	{ "265-c4.c4",    0x800000, 0x2da13947, 3 | BRF_GRA },           //  6 
	{ "265-c5.c5",    0x800000, 0x61bd165d, 3 | BRF_GRA },           //  7 
	{ "265-c6.c6",    0x800000, 0x03fdd1eb, 3 | BRF_GRA },           //  8 
	{ "265-c7.c7",    0x800000, 0x1a2749d8, 3 | BRF_GRA },           //  9 
	{ "265-c8.c8",    0x800000, 0xab0bb549, 3 | BRF_GRA },           // 10 

	{ "265-m1.m1",    0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "265-v1.v1",    0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 12 Sound data
	{ "265-v2.v2",    0x800000, 0xda41d6f9, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kf2k2pla, kf2k2pla, neogeo)
STD_ROM_FN(kf2k2pla)

struct BurnDriver BurnDrvkf2k2pla = {
	"kf2k2pla", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 Plus (bootleg set 2)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k2plaRomInfo, kf2k2plaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2002Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2002 Magic Plus (bootleg)

static struct BurnRomInfo kf2k2mpRomDesc[] = {
	{ "kf02m-p1.bin", 0x400000, 0xff7c6ec0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "kf02m-p2.bin", 0x400000, 0x91584716, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "kf02m-s1.bin", 0x020000, 0x348d6f2c, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "265-c1.c1",    0x800000, 0x2b65a656, 3 | BRF_GRA },           //  3 Sprite data
	{ "265-c2.c2",    0x800000, 0xadf18983, 3 | BRF_GRA },           //  4 
	{ "265-c3.c3",    0x800000, 0x875e9fd7, 3 | BRF_GRA },           //  5 
	{ "265-c4.c4",    0x800000, 0x2da13947, 3 | BRF_GRA },           //  6 
	{ "265-c5.c5",    0x800000, 0x61bd165d, 3 | BRF_GRA },           //  7 
	{ "265-c6.c6",    0x800000, 0x03fdd1eb, 3 | BRF_GRA },           //  8 
	{ "265-c7.c7",    0x800000, 0x1a2749d8, 3 | BRF_GRA },           //  9 
	{ "265-c8.c8",    0x800000, 0xab0bb549, 3 | BRF_GRA },           // 10 

	{ "265-m1.m1",    0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "265-v1.v1",    0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 12 Sound data
	{ "265-v2.v2",    0x800000, 0xda41d6f9, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kf2k2mp, kf2k2mp, neogeo)
STD_ROM_FN(kf2k2mp)

static void kf2k2mpCallback()
{
	int i, j;
	for (i = 0; i < 0x500000; i++)
	{
		j = (i & 0xffff00) + BITSWAP08(i, 7, 3, 4, 5, 6, 1, 2, 0);
		Neo68KROM[i] = Neo68KROM[0x300000 + j];
	}
	
	garoubl_sx_decode();
}

int kf2k2mpInit()
{
	int nRet;

	nNeoProtectionXor = 0xEC;
	pNeoInitCallback = kf2k2mpCallback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa5000, 0x000000, { 0xf9, 0xe0, 0x5d, 0xf3, 0xea, 0x92, 0xbe, 0xef } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k2mp = {
	"kf2k2mp", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 Magic Plus (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k2mpRomInfo, kf2k2mpRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k2mpInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2002 Magic Plus II (bootleg)

static struct BurnRomInfo kf2k2mp2RomDesc[] = {
	{ "k2k2m2p1.bin", 0x200000, 0x1016806c, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "k2k2m2p2.bin", 0x400000, 0x432fdf53, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "k2k2m2s1.bin", 0x020000, 0x446e74c5, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "265-c1.c1",    0x800000, 0x2b65a656, 3 | BRF_GRA },           //  3 Sprite data
	{ "265-c2.c2",    0x800000, 0xadf18983, 3 | BRF_GRA },           //  4 
	{ "265-c3.c3",    0x800000, 0x875e9fd7, 3 | BRF_GRA },           //  5 
	{ "265-c4.c4",    0x800000, 0x2da13947, 3 | BRF_GRA },           //  6 
	{ "265-c5.c5",    0x800000, 0x61bd165d, 3 | BRF_GRA },           //  7 
	{ "265-c6.c6",    0x800000, 0x03fdd1eb, 3 | BRF_GRA },           //  8 
	{ "265-c7.c7",    0x800000, 0x1a2749d8, 3 | BRF_GRA },           //  9 
	{ "265-c8.c8",    0x800000, 0xab0bb549, 3 | BRF_GRA },           // 10 

	{ "265-m1.m1",    0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "265-v1.v1",    0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 12 Sound data
	{ "265-v2.v2",    0x800000, 0xda41d6f9, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kf2k2mp2, kf2k2mp2, neogeo)
STD_ROM_FN(kf2k2mp2)

static void kf2k2mp2Callback()
{
	memcpy (Neo68KROM + 0x000000, Neo68KROM + 0x1c0000, 0x040000);
	memcpy (Neo68KROM + 0x0c0000, Neo68KROM + 0x100000, 0x040000);
	memcpy (Neo68KROM + 0x100000, Neo68KROM + 0x200000, 0x400000);

	lans2004_sx_decode();
}

int kof2km2Init()
{
	int nRet;

	nNeoProtectionXor = 0xEC;
	pNeoInitCallback = kf2k2mp2Callback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa5000, 0x000000, { 0xf9, 0xe0, 0x5d, 0xf3, 0xea, 0x92, 0xbe, 0xef } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k2mp2 = {
	"kf2k2mp2", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 Magic Plus II (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k2mp2RomInfo, kf2k2mp2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2km2Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Matrimelee / Shin Gouketsuji Ichizoku Toukon

static struct BurnRomInfo matrimRomDesc[] = {
	{ "266-p1.p1",    0x100000, 0x5d4c2dc7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "266-p2.sp2",   0x400000, 0xa14b1906, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "266-c1.c1",    0x800000, 0x505f4e30, 3 | BRF_GRA },           //  2 Sprite data
	{ "266-c2.c2",    0x800000, 0x3cb57482, 3 | BRF_GRA },           //  3 
	{ "266-c3.c3",    0x800000, 0xf1cc6ad0, 3 | BRF_GRA },           //  4 
	{ "266-c4.c4",    0x800000, 0x45b806b7, 3 | BRF_GRA },           //  5 
	{ "266-c5.c5",    0x800000, 0x9a15dd6b, 3 | BRF_GRA },           //  6 
	{ "266-c6.c6",    0x800000, 0x281cb939, 3 | BRF_GRA },           //  7 
	{ "266-c7.c7",    0x800000, 0x4b71f780, 3 | BRF_GRA },           //  8 
	{ "266-c8.c8",    0x800000, 0x29873d33, 3 | BRF_GRA },           //  9 

	{ "266-m1.m1",    0x020000, 0x456c3e6c, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "266-v1.v1",    0x800000, 0xa4f83690, 5 | BRF_SND },           // 11 Sound data
	{ "266-v2.v2",    0x800000, 0xd0f69eda, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(matrim, matrim, neogeo)
STD_ROM_FN(matrim)

int matrimInit()
{
	int nRet;

	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x6A;
	pNeoInitCallback = kof2002Callback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0x01000, 0xffce20, { 0xc4, 0x83, 0xa8, 0x5f, 0x21, 0x27, 0x64, 0xaf } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvmatrim = {
	"matrim", NULL, "neogeo", NULL, "2002",
	"Matrimelee / Shin Gouketsuji Ichizoku Toukon\0", NULL, "Noise Factory / Atlus", "Neo Geo",
	L"\u65B0\u8C6A\u8840\u5BFA\u4E00\u65CF - \u95D8\u5A5A\0Matrimelee\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_PWRINST,
	NULL, matrimRomInfo, matrimRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	matrimInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Pochi and Nyaa

static struct BurnRomInfo pnyaaRomDesc[] = {
	{ "267-p1.p1",    0x100000, 0x112fe2c0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "267-c1.c1",    0x800000, 0x5eebee65, 3 | BRF_GRA },           //  1 Sprite data
	{ "267-c2.c2",    0x800000, 0x2b67187b, 3 | BRF_GRA },           //  2 

	{ "267-m1.m1",    0x080000, 0xc7853ccd, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "267-v1.v1",    0x400000, 0xe2e8e917, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(pnyaa, pnyaa, neogeo)
STD_ROM_FN(pnyaa)

int pnyaaInit()
{
	int nRet;

	nNeoProtectionXor = 0x2E;

	nRet = NeoInit();

	if (nRet == 0) {
		for (int i = 0; i < 0x400000 / 2; i += 4 / 2) {
			unsigned short buffer[4 / 2];
			memcpy(buffer, ((unsigned short*)YM2610ADPCMAROM) + i, 4);
			for (int j = 0; j < 4 / 2; j++) {
				((unsigned short*)YM2610ADPCMAROM)[i + j] = buffer[j ^ 1];
			}
		}
	}

	return nRet;
}

struct BurnDriver BurnDrvpnyaa = {
	"pnyaa", NULL, "neogeo", NULL, "2003",
	"Pochi and Nyaa\0", NULL, "Aiky / Taito", "Neo Geo",
	L"Pochi and Nyaa\0\u30DD\u30C1\u30C3\u3068\u306B\u3083\uFF5E\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_PUZZLE, 0,
	NULL, pnyaaRomInfo, pnyaaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	pnyaaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 5

static struct BurnRomInfo mslug5RomDesc[] = {
	{ "268-p1cr.p1",   0x400000, 0xd0466792, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "268-p2cr.p2",   0x400000, 0xfbf6b61e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "268-c1c.c1",    0x800000, 0xab7c389a, 3 | BRF_GRA },           //  2 Sprite data
	{ "268-c2c.c2",    0x800000, 0x3560881b, 3 | BRF_GRA },           //  3 
	{ "268-c3c.c3",    0x800000, 0x3af955ea, 3 | BRF_GRA },           //  4 
	{ "268-c4c.c4",    0x800000, 0xc329c373, 3 | BRF_GRA },           //  5 
	{ "268-c5c.c5",    0x800000, 0x959c8177, 3 | BRF_GRA },           //  6 
	{ "268-c6c.c6",    0x800000, 0x010a831b, 3 | BRF_GRA },           //  7 
	{ "268-c7c.c7",    0x800000, 0x6d72a969, 3 | BRF_GRA },           //  8 
	{ "268-c8c.c8",    0x800000, 0x551d720e, 3 | BRF_GRA },           //  9 

	{ "268-m1.m1",     0x080000, 0x4a5a6e0e, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "268-v1c.v1",    0x800000, 0xae31d60c, 5 | BRF_SND },           // 11 Sound data
	{ "268-v2c.v2",    0x800000, 0xc40613ed, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(mslug5, mslug5, neogeo)
STD_ROM_FN(mslug5)

static void mslug5Callback()
{
	int i, j, k;
	for (i = 0; i < 0x100000; i++)
		Neo68KROM[i] ^= Neo68KROM[0x0fffe0 + (i & 0x1f)];

	for (i = 0x100000; i < 0x700000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x7fffe0 + (i & 0x1f)];

	for (i = 0x100000; i < 0x0500000; i += 4)
	{
		unsigned short rom16 = *((unsigned short *)(Neo68KROM + i + 1));
		rom16 = BITSWAP16(rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0);
		*((unsigned short *)(Neo68KROM + i + 1)) = rom16;
	}

	memcpy (Neo68KROM + 0x700000, Neo68KROM, 0x100000);

	for (i = 0; i < 0x100000 / 0x010000; i++)
	{
		j = BITSWAP08(i, 7, 6, 5, 4, 1, 0, 3, 2) * 0x010000;
		memcpy (Neo68KROM + i * 0x010000, Neo68KROM + 0x700000 + j, 0x010000);
	}

	for (i = 0x100000; i < 0x700000; i += 0x100000)
	{
		for (j = 0; j < 0x100000; j+=0x100)
		{
			k = ((j & 0xf00) ^ 0x700) + (BITSWAP08((j >> 12), 5, 4, 7, 6, 1, 0, 3, 2 ) << 12);
			memcpy (Neo68KROM + 0x700000 + j, Neo68KROM + i + k, 0x100);
		}

		memcpy (Neo68KROM + i, Neo68KROM + 0x700000, 0x100000);
	}
}

int mslug5Init()
{
	int nRet;

	nNeoProtectionXor = 0x19;
	pNeoInitCallback = mslug5Callback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0x4e001, 0xfe2cf6, { 0xc3, 0xfd, 0x81, 0xac, 0x6d, 0xe7, 0xbf, 0x9e } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvmslug5 = {
	"mslug5", NULL, "neogeo", NULL, "2003",
	"Metal Slug 5\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug5RomInfo, mslug5RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 5 (AES Cart)

static struct BurnRomInfo mslug5hRomDesc[] = {
	{ "268-p1c.p1",    0x400000, 0x3636690a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "268-p2c.p2",    0x400000, 0x8dfc47a2, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "268-c1c.c1",    0x800000, 0xab7c389a, 3 | BRF_GRA },           //  2 Sprite data
	{ "268-c2c.c2",    0x800000, 0x3560881b, 3 | BRF_GRA },           //  3 
	{ "268-c3c.c3",    0x800000, 0x3af955ea, 3 | BRF_GRA },           //  4 
	{ "268-c4c.c4",    0x800000, 0xc329c373, 3 | BRF_GRA },           //  5 
	{ "268-c5c.c5",    0x800000, 0x959c8177, 3 | BRF_GRA },           //  6 
	{ "268-c6c.c6",    0x800000, 0x010a831b, 3 | BRF_GRA },           //  7 
	{ "268-c7c.c7",    0x800000, 0x6d72a969, 3 | BRF_GRA },           //  8 
	{ "268-c8c.c8",    0x800000, 0x551d720e, 3 | BRF_GRA },           //  9 

	{ "268-m1.m1",     0x080000, 0x4a5a6e0e, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "268-v1c.v1",    0x800000, 0xae31d60c, 5 | BRF_SND },           // 11 Sound data
	{ "268-v2c.v2",    0x800000, 0xc40613ed, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(mslug5h, mslug5h, neogeo)
STD_ROM_FN(mslug5h)

struct BurnDriver BurnDrvmslug5h = {
	"mslug5h", "mslug5", "neogeo", NULL, "2003",
	"Metal Slug 5 (AES Cart)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug5hRomInfo, mslug5hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 5 (JAMMA PCB)

static struct BurnRomInfo ms5pcbRomDesc[] = {
#if !defined (ROM_VERIFY)
	{ "268-p1.p1",    0x400000,  0xd0466792, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "268-p2.p2",    0x400000,  0xfbf6b61e, 1 | BRF_ESS | BRF_PRG }, //  1 
#else
	{ "268-p1r.p1",   0x400000,  0x00000000, 1 | BRF_ESS | BRF_PRG | BRF_NODUMP }, //  0 68K code
	{ "268-p2r.p2",   0x400000,  0x00000000, 1 | BRF_ESS | BRF_PRG | BRF_NODUMP }, //  1 
#endif

	{ "268-c1.c1",    0x1000000, 0x802042e8, 3 | BRF_GRA },           //  2 Sprite data
	{ "268-c2.c2",    0x1000000, 0x3b89fb9f, 3 | BRF_GRA },           //  3 
	{ "268-c3.c3",    0x1000000, 0x0f3f59e3, 3 | BRF_GRA },           //  4 
	{ "268-c4.c4",    0x1000000, 0x3ad8435a, 3 | BRF_GRA },           //  5 

	{ "268-m1.m1",    0x080000,  0x4a5a6e0e, 4 | BRF_ESS | BRF_PRG }, // 6 Z80 code

	{ "268-v1.v1",    0x1000000, 0x8458afe5, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(ms5pcb, ms5pcb, ms5pcbBIOS)
STD_ROM_FN(ms5pcb)

struct BurnDriver BurnDrvms5pcb = {
	"ms5pcb", NULL, NULL, NULL, "2003",
	"Metal Slug 5 (JAMMA PCB)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_P32 | HARDWARE_SNK_PCB | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, ms5pcbRomInfo, ms5pcbRomName, NULL, NULL, neogeoInputInfo, ms5pcbDIPInfo,
	mslug5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 5 Plus (bootleg)

static struct BurnRomInfo ms5plusRomDesc[] = {
	{ "ms5-p1p.bin",  0x100000, 0x106b276f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "ms5-p2p.bin",  0x200000, 0xd6a458e8, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "ms5-p3p.bin",  0x200000, 0x439ec031, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "ms5-s1p.bin",  0x020000, 0x21e04432, 2 | BRF_GRA },           //  3 Text layer tiles
	
	{ "268-c1c.c1",   0x800000, 0xab7c389a, 3 | BRF_GRA },           //  4 Sprite data
	{ "268-c2c.c2",   0x800000, 0x3560881b, 3 | BRF_GRA },           //  5 
	{ "268-c3c.c3",   0x800000, 0x3af955ea, 3 | BRF_GRA },           //  6 
	{ "268-c4c.c4",   0x800000, 0xc329c373, 3 | BRF_GRA },           //  7 
	{ "268-c5c.c5",   0x800000, 0x959c8177, 3 | BRF_GRA },           //  8 
	{ "268-c6c.c6",   0x800000, 0x010a831b, 3 | BRF_GRA },           //  9 
	{ "268-c7c.c7",   0x800000, 0x6d72a969, 3 | BRF_GRA },           // 10 
	{ "268-c8c.c8",   0x800000, 0x551d720e, 3 | BRF_GRA },           // 11 

	{ "268-m1.m1",    0x080000,  0x4a5a6e0e, 4 | BRF_ESS | BRF_PRG }, // 12 Z80 code

	{ "268-v1c.v1",   0x800000, 0xae31d60c, 5 | BRF_SND },           // 13 Sound data
	{ "268-v2c.v2",   0x800000, 0xc40613ed, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(ms5plus, ms5plus, neogeo)
STD_ROM_FN(ms5plus)

void __fastcall ms5plusWriteWordBankSwitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress==0x2ffff4)
	{
		unsigned int nBank = wordValue << 16;
		if (nNeo68KROMBank != nBank)
		{
			nNeo68KROMBank = nBank;
			SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fdfff, SM_ROM);
		}
	};
}

static int ms5plusInit()
{
	int nRet;
	
	pNeoInitCallback = lans2004_sx_decode;
	pNeoBankswitchCallback = pvcMapBank;
	nNeoProtectionXor = 0x19;
	
	nRet = NeoInit();

	if (nRet == 0) {
		SekOpen(0);
		SekMapHandler(5,	0x2fe000, 0x2fffff, SM_WRITE);
		SekSetWriteWordHandler(5, ms5plusWriteWordBankSwitch);
		SekClose();

		PCM2DecryptVInfo Info = { 0x4e001, 0xfe2cf6, { 0xc3, 0xfd, 0x81, 0xac, 0x6d, 0xe7, 0xbf, 0x9e } };

		PCM2DecryptV(&Info);
	}
	
	return nRet;
}

struct BurnDriver BurnDrvms5plus = {
	"ms5plus", "mslug5", "neogeo", NULL, "2003",
	"Metal Slug 5 Plus (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, ms5plusRomInfo, ms5plusRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	ms5plusInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// SNK vs. CAPCOM SVC CHAOS (JAMMA PCB, set 1)

static struct BurnRomInfo svcpcbRomDesc[] = {
	{ "269-p1.p1",    0x2000000, 0x432cfdfc, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "269-c1.c1",    0x2000000, 0x1b608f9c, 3 | BRF_GRA },           //  1 Sprite data
	{ "269-c2.c1",    0x2000000, 0x5a95f294, 3 | BRF_GRA },           //  2 

	{ "269-m1.m1",    0x080000,  0xf6819d00, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "269-v1.v1",    0x800000,  0xc659b34c, 5 | BRF_SND },           //  5 Sound data
	{ "269-v2.v1",    0x800000,  0xdd903835, 5 | BRF_SND },           //  6 
};

STDROMPICKEXT(svcpcb, svcpcb, svcpcbBIOS)
STD_ROM_FN(svcpcb)

static void svcCallback()
{
	int i, j, k;
	for (i = 0; i < 0x100000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x0fffe0 + (i & 0x1f)];

	for (i = 0x100000; i < 0x800000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x7fffe0 + (i & 0x1f)];

	for (i = 0x100000; i < 0x0600000; i+=4)
	{
		unsigned short rom16 = *((unsigned short *)(Neo68KROM + i + 1));
		rom16 = BITSWAP16(rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0);
		*((unsigned short *)(Neo68KROM + i + 1)) = rom16;
	}

	memcpy (Neo68KROM + 0x700000, Neo68KROM, 0x100000);

	for (i = 0; i < 0x0100000 / 0x10000; i++)
	{
		j = BITSWAP08(i, 7, 6, 5, 4, 2, 3, 0, 1);
		memcpy (Neo68KROM + i * 0x010000, Neo68KROM + 0x700000 + j * 0x010000, 0x010000);
	}

	for (i = 0x100000; i < 0x700000; i += 0x100000)
	{
		for (j = 0; j < 0x100000; j+= 0x100)
		{
			k  = BITSWAP08(j >> 12, 4, 5, 6, 7, 1, 0, 3, 2 ) << 12;
			k |= (j & 0x00f00) ^ 0x00a00;

			memcpy (Neo68KROM + 0x700000 + j, Neo68KROM + i + k, 0x100);
		}

		memcpy (Neo68KROM + i, Neo68KROM + 0x700000, 0x100000);
	}
}

int svcpcbInit()
{
	int nRet;

	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x57;
	pNeoInitCallback = svcCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xc2000, 0xffac28, { 0xc3, 0xfd, 0x81, 0xac, 0x6d, 0xe7, 0xbf, 0x9e } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvsvcpcb = {
	"svcpcb", NULL, NULL, NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS (JAMMA PCB, set 1)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_PCB | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcpcbRomInfo, svcpcbRomName, NULL, NULL, neogeoInputInfo, svcpcbDIPInfo,
	svcpcbInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// SNK vs. CAPCOM SVC CHAOS (JAMMA PCB, set 2)

static struct BurnRomInfo svcpcbaRomDesc[] = {
	{ "269-p1a.p1",   0x400000,  0x38e2005e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "269-p2a.p1",   0x400000,  0x6d13797c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "269-c1a.c1",   0x1000000, 0xe64d2b0c, 3 | BRF_GRA },           //  2 Sprite data
	{ "269-c2a.c2",   0x1000000, 0x249089c2, 3 | BRF_GRA },           //  3 
	{ "269-c3a.c3",   0x1000000, 0xd32f2fab, 3 | BRF_GRA },           //  4 
	{ "269-c4a.c4",   0x1000000, 0xbf77e878, 3 | BRF_GRA },           //  5 

	{ "269-m1.m1",    0x080000,  0xf6819d00, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "269-v1a.v1",   0x1000000, 0xa6af4753, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(svcpcba, svcpcba, svcpcbBIOS)
STD_ROM_FN(svcpcba)

struct BurnDriver BurnDrvsvcpcba = {
	"svcpcba", "svcpcb", NULL, NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS (JAMMA PCB, set 2)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_PCB | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcpcbaRomInfo, svcpcbaRomName, NULL, NULL, neogeoInputInfo, svcpcbDIPInfo,
	svcpcbInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// SNK vs. CAPCOM SVC CHAOS

static struct BurnRomInfo svcRomDesc[] = {
	{ "269-p1.p1",    0x400000, 0x38e2005e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "269-p2.p2",    0x400000, 0x6d13797c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "269-c1r.c1",   0x800000, 0x887b4068, 3 | BRF_GRA },           //  2 Sprite data
	{ "269-c2r.c2",   0x800000, 0x4e8903e4, 3 | BRF_GRA },           //  3 
	{ "269-c3r.c3",   0x800000, 0x7d9c55b0, 3 | BRF_GRA },           //  4 
	{ "269-c4r.c4",   0x800000, 0x8acb5bb6, 3 | BRF_GRA },           //  5 
	{ "269-c5r.c5",   0x800000, 0x097a4157, 3 | BRF_GRA },           //  6 
	{ "269-c6r.c6",   0x800000, 0xe19df344, 3 | BRF_GRA },           //  7 
	{ "269-c7r.c7",   0x800000, 0xd8f0340b, 3 | BRF_GRA },           //  8 
	{ "269-c8r.c8",   0x800000, 0x2570b71b, 3 | BRF_GRA },           //  9 

	{ "269-m1.m1",    0x080000,  0xf6819d00, 4 | BRF_ESS | BRF_PRG }, //  10 Z80 code

	{ "269-v1.v1",    0x800000, 0xc659b34c, 5 | BRF_SND },           // 11 Sound data
	{ "269-v2.v2",    0x800000, 0xdd903835, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(svc, svc, neogeo)
STD_ROM_FN(svc)

struct BurnDriver BurnDrvsvc = {
	"svc", NULL, "neogeo", NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcRomInfo, svcRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	svcpcbInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// SNK vs. CAPCOM SVC CHAOS (bootleg)

static struct BurnRomInfo svcbootRomDesc[] = {
	{ "svc-p1.bin",   0x800000, 0x0348f162, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "svc-s1.bin",   0x020000, 0x70b44df1, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "svc-c1.bin",   0x800000, 0xa7826b89, 3 | BRF_GRA },           //  2 Sprite data
	{ "svc-c2.bin",   0x800000, 0xed3c2089, 3 | BRF_GRA },           //  3 
	{ "svc-c3.bin",   0x800000, 0x71ed8063, 3 | BRF_GRA },           //  4 
	{ "svc-c4.bin",   0x800000, 0x250bde2d, 3 | BRF_GRA },           //  5 
	{ "svc-c5.bin",   0x800000, 0x9817c082, 3 | BRF_GRA },           //  6 
	{ "svc-c6.bin",   0x800000, 0x2bc0307f, 3 | BRF_GRA },           //  7 
	{ "svc-c7.bin",   0x800000, 0x4358d7b9, 3 | BRF_GRA },           //  8 
	{ "svc-c8.bin",   0x800000, 0x366deee5, 3 | BRF_GRA },           //  9 

	{ "svc-m1.bin",   0x020000, 0x804328c3, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "svc-v2.bin",   0x400000, 0xb5097287, 5 | BRF_SND },           // 11 Sound data
	{ "svc-v1.bin",   0x400000, 0xbd3a391f, 5 | BRF_SND },           // 12 
	{ "svc-v4.bin",   0x400000, 0x33fc0b37, 5 | BRF_SND },           // 13 
	{ "svc-v3.bin",   0x400000, 0xaa9849a0, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(svcboot, svcboot, neogeo)
STD_ROM_FN(svcboot)

void DoPerm(int g) // 0 - cthd2003, 1 - svcboot
{
	static int idx[ 2 ][ 16 ] = {
		{ 0, 1, 2, 3, 3, 4, 4, 5, 0, 1, 2, 3, 3, 4, 4, 5 }, // 0
		{ 0, 1, 0, 1, 2, 3, 2, 3, 3, 4, 3, 4, 4, 5, 4, 5 }, // 1
	};

	static int tbl[ 6 ][ 4 ] = {
		{ 3, 0, 1, 2 },
		{ 2, 3, 0, 1 },
		{ 1, 2, 3, 0 },
		{ 0, 1, 2, 3 },
		{ 3, 2, 1, 0 },
		{ 3, 0, 2, 1 },
	};

	int i, j, k, *b;
	unsigned char dst[0x800];

	for (i = 0; i < 0x4000000 >> 11; i++)
	{
		for(j = 0; j < 0x800 >> 7; j++)
		{
			b = tbl[idx[g][(i >> (5 ^ g)) & 0x0f]];
			k = BITSWAP08(j, 7, 6, 5, 4, b[3], b[2], b[1], b[0]);

			memcpy (dst + (j << 7), NeoSpriteROM + (i << 11) + (k << 7), 0x80);
		}

		memcpy (NeoSpriteROM + (i << 11), dst, 0x800);
	}
}

static void svcboot_sx_decode()
{
	for (int i = 0; i < 0x20000 / 2; i++) {
		int n = NeoTextROM[0x20000 + i];
		NeoTextROM[0x20000 + i] = NeoTextROM[0x30000 + i];
		NeoTextROM[0x30000 + i] = n;
	}
}

static void svcboot_decode()
{
	DoPerm(1);

	for (int i = 0; i < 0x20000 / 2; i++) {
		int n = NeoZ80ROM[i];
		NeoZ80ROM[i] = NeoZ80ROM[0x10000 + i];
		NeoZ80ROM[0x10000 + i] = n;
	}
}

static void svcbootCallback()
{
	int i, j, k;
	for (i = 0x100000; i < 0x800000; i+=0x100000)
	{
		memcpy (Neo68KROM, Neo68KROM + i, 0x100000);

		for (j = 0; j < 0x100000; j++)
		{
			k  = (j & 0xfff00) + BITSWAP08(j, 7, 2, 1, 4, 3, 6, 5, 0);
			Neo68KROM[i + j] = Neo68KROM[k];
		}
	}

	memcpy (Neo68KROM, Neo68KROM + 0x700000, 0x100000);

	svcboot_sx_decode();
	svcboot_decode();
}


int svcbootInit()
{
	int nRet;

	pNeoInitCallback = svcbootCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		BurnByteswap(YM2610ADPCMAROM, 0x1000000);
	}

	return nRet;
}

struct BurnDriver BurnDrvsvcboot = {
	"svcboot", "svc", "neogeo", NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcbootRomInfo, svcbootRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	svcbootInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 1)

static struct BurnRomInfo svcplusRomDesc[] = {
	{ "svc-p2p.bin",  0x200000, 0x50c0e2b7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "svc-p3p.bin",  0x200000, 0x58cdc293, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "svc-p1p.bin",  0x200000, 0xa194d842, 1 | BRF_ESS | BRF_PRG }, //  2

	{ "svc-s1p.bin",  0x020000, 0x73344711, 2 | BRF_GRA },           //  3 Text layer tiles

	{ "svc-c1.bin",   0x800000, 0xa7826b89, 3 | BRF_GRA },           //  4 Sprite data
	{ "svc-c2.bin",   0x800000, 0xed3c2089, 3 | BRF_GRA },           //  5 
	{ "svc-c3.bin",   0x800000, 0x71ed8063, 3 | BRF_GRA },           //  6 
	{ "svc-c4.bin",   0x800000, 0x250bde2d, 3 | BRF_GRA },           //  7 
	{ "svc-c5.bin",   0x800000, 0x9817c082, 3 | BRF_GRA },           //  8 
	{ "svc-c6.bin",   0x800000, 0x2bc0307f, 3 | BRF_GRA },           //  9 
	{ "svc-c7.bin",   0x800000, 0x4358d7b9, 3 | BRF_GRA },           // 10 
	{ "svc-c8.bin",   0x800000, 0x366deee5, 3 | BRF_GRA },           // 11 

	{ "svc-m1.bin",   0x020000, 0x804328c3, 4 | BRF_ESS | BRF_PRG }, // 12 Z80 code

	{ "svc-v2.bin",   0x400000, 0xb5097287, 5 | BRF_SND },           // 13 Sound data
	{ "svc-v1.bin",   0x400000, 0xbd3a391f, 5 | BRF_SND },           // 14 
	{ "svc-v4.bin",   0x400000, 0x33fc0b37, 5 | BRF_SND },           // 15 
	{ "svc-v3.bin",   0x400000, 0xaa9849a0, 5 | BRF_SND },           // 16 
};

STDROMPICKEXT(svcplus, svcplus, neogeo)
STD_ROM_FN(svcplus)

static void svcplusCallback()
{
	int i, j, k;
	unsigned char *dst = (unsigned char*)malloc(0x100000);

	if (dst)
	{
		for (i = 0; i < 0x600000; i+= 0x100000)
		{
			for (j = 0; j < 0x100000; j++)
			{
				k = BITSWAP24(j, 23,22,21,20,1,2,3,16,15,14,13,12,11,10,9,8,7,6,5,4,17,18,19,0);
				k ^= 0x0e000e;
	
				dst[j] = Neo68KROM[i + k];
			}

			memcpy (Neo68KROM + i, dst, 0x100000);
		}

		memcpy (Neo68KROM + 0x100000, Neo68KROM, 0x500000);
		memcpy (Neo68KROM, dst, 0x100000);

		free (dst);
	}

	*((unsigned short*)(Neo68KROM + 0xf8016)) = 0x33c1; // Patch protected address

	lans2004_sx_decode();
	svcboot_decode();
}

int svcplusInit()
{
	int nRet;

	pNeoInitCallback = svcplusCallback;

	nRet = NeoInit();

	if (nRet == 0) {
		BurnByteswap(YM2610ADPCMAROM, 0x1000000);
	}

	return nRet;
}

struct BurnDriver BurnDrvsvcplus = {
	"svcplus", "svc", "neogeo", NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 1)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcplusRomInfo, svcplusRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	svcplusInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 2)

static struct BurnRomInfo svcplusaRomDesc[] = {
	{ "svc-p2pl.bin", 0x400000, 0x7231ace2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "svc-p1pl.bin", 0x200000, 0x16b44144, 1 | BRF_ESS | BRF_PRG }, //  1

	{ "svc-s1pl.bin", 0x020000, 0xca3c735e, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "svc-c1.bin",   0x800000, 0xa7826b89, 3 | BRF_GRA },           //  3 Sprite data
	{ "svc-c2.bin",   0x800000, 0xed3c2089, 3 | BRF_GRA },           //  4 
	{ "svc-c3.bin",   0x800000, 0x71ed8063, 3 | BRF_GRA },           //  5 
	{ "svc-c4.bin",   0x800000, 0x250bde2d, 3 | BRF_GRA },           //  6 
	{ "svc-c5.bin",   0x800000, 0x9817c082, 3 | BRF_GRA },           //  7 
	{ "svc-c6.bin",   0x800000, 0x2bc0307f, 3 | BRF_GRA },           //  8 
	{ "svc-c7.bin",   0x800000, 0x4358d7b9, 3 | BRF_GRA },           //  9 
	{ "svc-c8.bin",   0x800000, 0x366deee5, 3 | BRF_GRA },           // 10 

	{ "svc-m1.bin",   0x020000, 0x804328c3, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "svc-v2.bin",   0x400000, 0xb5097287, 5 | BRF_SND },           // 12 Sound data
	{ "svc-v1.bin",   0x400000, 0xbd3a391f, 5 | BRF_SND },           // 13 
	{ "svc-v4.bin",   0x400000, 0x33fc0b37, 5 | BRF_SND },           // 14 
	{ "svc-v3.bin",   0x400000, 0xaa9849a0, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(svcplusa, svcplusa, neogeo)
STD_ROM_FN(svcplusa)

static void svcplusaCallback()
{
	unsigned char *dst = (unsigned char*)malloc(0x100000);
	if (dst)
	{
		memcpy (dst, Neo68KROM + 0x500000, 0x100000);
		memcpy (Neo68KROM + 0x100000, Neo68KROM, 0x500000);
		memcpy (Neo68KROM, dst, 0x100000);
		free (dst);
	}

	*((unsigned short*)(Neo68KROM + 0xf8016)) = 0x33c1; // Patch protected address

	svcboot_sx_decode();
	svcboot_decode();
}

int svcplusaInit()
{
	int nRet;

	pNeoInitCallback = svcplusaCallback;

	nRet = NeoInit();

	if (nRet == 0) {
		BurnByteswap(YM2610ADPCMAROM, 0x1000000);
	}

	return nRet;
}

struct BurnDriver BurnDrvsvcplusa = {
	"svcplusa", "svc", "neogeo", NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS Plus (bootleg set 2)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcplusaRomInfo, svcplusaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	svcplusaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// SNK vs. CAPCOM SVC CHAOS Super Plus (bootleg)

static struct BurnRomInfo svcsplusRomDesc[] = {
	{ "svc-p1sp.bin", 0x400000, 0x2601902f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "svc-p2sp.bin", 0x400000, 0x0ca13305, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "svc-s1sp.bin", 0x020000, 0x233d6439, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "svc-c1.bin",   0x800000, 0xa7826b89, 3 | BRF_GRA },           //  3 Sprite data
	{ "svc-c2.bin",   0x800000, 0xed3c2089, 3 | BRF_GRA },           //  4 
	{ "svc-c3.bin",   0x800000, 0x71ed8063, 3 | BRF_GRA },           //  5 
	{ "svc-c4.bin",   0x800000, 0x250bde2d, 3 | BRF_GRA },           //  6 
	{ "svc-c5.bin",   0x800000, 0x9817c082, 3 | BRF_GRA },           //  7 
	{ "svc-c6.bin",   0x800000, 0x2bc0307f, 3 | BRF_GRA },           //  8 
	{ "svc-c7.bin",   0x800000, 0x4358d7b9, 3 | BRF_GRA },           //  9 
	{ "svc-c8.bin",   0x800000, 0x366deee5, 3 | BRF_GRA },           // 10 

	{ "svc-m1.bin",   0x020000, 0x804328c3, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "svc-v2.bin",   0x400000, 0xb5097287, 5 | BRF_SND },           // 12 Sound data
	{ "svc-v1.bin",   0x400000, 0xbd3a391f, 5 | BRF_SND },           // 13 
	{ "svc-v4.bin",   0x400000, 0x33fc0b37, 5 | BRF_SND },           // 14 
	{ "svc-v3.bin",   0x400000, 0xaa9849a0, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(svcsplus, svcsplus, neogeo)
STD_ROM_FN(svcsplus)

static void svcsplus_sx_decode()
{
	for (int i = 0; i < 0x20000; i++)
		NeoTextROM[0x20000 + i] = BITSWAP08(NeoTextROM[0x20000 + i], 7, 6, 0, 4, 3, 2, 1, 5);
}

static void svcsplusCallback()
{
	int i, j, k;
	for (i = 0x100000; i < 0x800000; i+=0x10000)
	{
		memcpy (Neo68KROM, Neo68KROM + i, 0x10000);

		for (j = 0; j < 0x10000; j++)
		{
			k = BITSWAP16(j, 1, 9, 10, 12, 11, 13, 14, 5, 4, 2, 8, 7, 3, 6, 15, 0);
        		Neo68KROM[i + j] = Neo68KROM[k];
		}
	}

	memcpy (Neo68KROM, Neo68KROM + 0x600000, 0x100000);

	*((unsigned short*)(Neo68KROM + 0x9e90)) = 0x000f; // Enable S. Plus
	*((unsigned short*)(Neo68KROM + 0x9e92)) = 0xc9c0;
	*((unsigned short*)(Neo68KROM + 0xa10c)) = 0x4eb9; // Enable boss icons
	*((unsigned short*)(Neo68KROM + 0xa10e)) = 0x000e;
	*((unsigned short*)(Neo68KROM + 0xa110)) = 0x9750;

	svcsplus_sx_decode();
	svcboot_decode();
}

int svcsplusInit()
{
	int nRet;

	pNeoInitCallback = svcsplusCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		BurnByteswap(YM2610ADPCMAROM, 0x1000000);
	}

	return nRet;
}

struct BurnDriver BurnDrvsvcsplus = {
	"svcsplus", "svc", "neogeo", NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS Super Plus (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcsplusRomInfo, svcsplusRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	svcsplusInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown V / Samurai Spirits Zero (set 1)

static struct BurnRomInfo samsho5RomDesc[] = {
	{ "270-p1.p1",    0x400000, 0x4a2a09e6, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "270-p2.sp2",   0x400000, 0xe0c74c85, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "270-c1.c1",    0x800000, 0x14ffffac, 3 | BRF_GRA },           //  2 Sprite data
	{ "270-c2.c2",    0x800000, 0x401f7299, 3 | BRF_GRA },           //  3 
	{ "270-c3.c3",    0x800000, 0x838f0260, 3 | BRF_GRA },           //  4 
	{ "270-c4.c4",    0x800000, 0x041560a5, 3 | BRF_GRA },           //  5 
	{ "270-c5.c5",    0x800000, 0xbd30b52d, 3 | BRF_GRA },           //  6 
	{ "270-c6.c6",    0x800000, 0x86a69c70, 3 | BRF_GRA },           //  7 
	{ "270-c7.c7",    0x800000, 0xd28fbc3c, 3 | BRF_GRA },           //  8 
	{ "270-c8.c8",    0x800000, 0x02c530a6, 3 | BRF_GRA },           //  9 

	{ "270-m1.m1",    0x080000, 0x49c9901a, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "270-v1.v1",    0x800000, 0x62e434eb, 5 | BRF_SND },           // 11 Sound data
	{ "270-v2.v2",    0x800000, 0x180f3c9a, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(samsho5, samsho5, neogeo)
STD_ROM_FN(samsho5)

static void samsho5Callback()
{
	int i;
	unsigned char *dst = (unsigned char *)malloc(0x800000);
	if (dst)
	{
		static const int sec[16] = {
			0x00, 0x01, 0x0e, 0x0d, 0x0a, 0x03, 0x04, 0x09,
			0x06, 0x0f, 0x0c, 0x05, 0x02, 0x0b, 0x08, 0x07
		};

		memcpy (dst, Neo68KROM, 0x800000);

		for (i = 0; i < 0x800000 / 0x080000; i++)
		{
			memcpy (Neo68KROM + i * 0x080000, dst + sec[i] * 0x080000, 0x080000);
		}

		free(dst);
	}
}

int samsho5Init()
{
	int nRet;

	nNeoTextROMSize = 0x020000;
	nNeoProtectionXor = 0x0F;
	pNeoInitCallback = samsho5Callback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0x0a000, 0xfeb2c0, { 0xcb, 0x29, 0x7d, 0x43, 0xd2, 0x3a, 0xc2, 0xb4 } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvsamsho5 = {
	"samsho5", NULL, "neogeo", NULL, "2003",
	"Samurai Shodown V / Samurai Spirits Zero (set 1)\0", NULL, "Yuki Enterprise / SNK Playmore", "Neo Geo",
	L"Samurai Shodown V (set 1)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho5RomInfo, samsho5RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsho5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown V / Samurai Spirits Zero (set 2)

static struct BurnRomInfo samsho5hRomDesc[] = {
	{ "270-p1c.p1",   0x400000, 0xbf956089, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "270-p2c.sp2",  0x400000, 0x943a6b1d, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "270-c1.c1",    0x800000, 0x14ffffac, 3 | BRF_GRA },           //  2 Sprite data
	{ "270-c2.c2",    0x800000, 0x401f7299, 3 | BRF_GRA },           //  3 
	{ "270-c3.c3",    0x800000, 0x838f0260, 3 | BRF_GRA },           //  4 
	{ "270-c4.c4",    0x800000, 0x041560a5, 3 | BRF_GRA },           //  5 
	{ "270-c5.c5",    0x800000, 0xbd30b52d, 3 | BRF_GRA },           //  6 
	{ "270-c6.c6",    0x800000, 0x86a69c70, 3 | BRF_GRA },           //  7 
	{ "270-c7.c7",    0x800000, 0xd28fbc3c, 3 | BRF_GRA },           //  8 
	{ "270-c8.c8",    0x800000, 0x02c530a6, 3 | BRF_GRA },           //  9 

	{ "270-m1.m1",    0x080000, 0x49c9901a, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "270-v1.v1",    0x800000, 0x62e434eb, 5 | BRF_SND },           // 11 Sound data
	{ "270-v2.v2",    0x800000, 0x180f3c9a, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(samsho5h, samsho5h, neogeo)
STD_ROM_FN(samsho5h)

struct BurnDriver BurnDrvsamsho5h = {
	"samsho5h", "samsho5", "neogeo", NULL, "2003",
	"Samurai Shodown V / Samurai Spirits Zero (set 2)\0", NULL, "Yuki Enterprise / SNK Playmore", "Neo Geo",
	L"Samurai Shodown V (set 2)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho5hRomInfo, samsho5hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsho5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown V / Samurai Spirits Zero (bootleg)

static struct BurnRomInfo samsho5bRomDesc[] = {
	{ "ssv-p2.bin",   0x400000, 0x5023067f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "ssv-p1.bin",   0x400000, 0xb6cbe386, 1 | BRF_ESS | BRF_PRG }, //  1 
	
	{ "ssv-s1.bin",   0x020000, 0x70f667d0, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "ssv-c1.bin",   0x1000000, 0x9c564a01, 3 | BRF_GRA },           //  3 Sprite data
	{ "ssv-c2.bin",   0x1000000, 0x4b73b8da, 3 | BRF_GRA },           //  4 
	{ "ssv-c3.bin",   0x1000000, 0x029f9bb5, 3 | BRF_GRA },           //  5 
	{ "ssv-c4.bin",   0x1000000, 0x75722430, 3 | BRF_GRA },           //  6 

	{ "ssv-m1.bin",   0x020000, 0x18114fb1, 4 | BRF_ESS | BRF_PRG }, // 7 Z80 code (decrypted)

	{ "ssv-v1.bin",   0x400000, 0xa3609761, 5 | BRF_SND },           // 8 Sound data
	{ "ssv-v2.bin",   0x400000, 0xcbd6ebd5, 5 | BRF_SND },           // 9 
	{ "ssv-v3.bin",   0x400000, 0x6f1c2703, 5 | BRF_SND },           // 10
	{ "ssv-v4.bin",   0x400000, 0x5020c055, 5 | BRF_SND },           // 11
};

STDROMPICKEXT(samsho5b, samsho5b, neogeo)
STD_ROM_FN(samsho5b)

static void samsho5b_sx_decode()
{
	unsigned char *Buf = (unsigned char*)malloc(0x20000);
	if (Buf) {
		memcpy(Buf, NeoTextROM + 0x20000, 0x20000);
		
		for (int i = 0; i < 0x20000; i += 0x10) {
			memcpy(&NeoTextROM[0x20000 + i + 0], &Buf[i + 8], 8);
			memcpy(&NeoTextROM[0x20000 + i + 8], &Buf[i + 0], 8);
		}
		
		free(Buf);
	}
}

static void samsho5b_vx_decode()
{
	for (int i = 0; i < 0x400000 * 4; i++)
		YM2610ADPCMAROM[i] = BITSWAP08(YM2610ADPCMAROM[i], 0, 1, 5, 4, 3, 2, 6, 7);
}

static void samsho5bCallback()
{
	int i, j, k;
	unsigned char *dst = (unsigned char *)malloc(0x100000);

	if (dst)
	{
		for (i = 0; i < 0x800000; i+=0x100000)
		{
			for (j = 0; j < 0x100000; j++)
			{
				k = BITSWAP08((j & 0xff), 7, 6, 5, 4, 1, 2, 3, 0);
				k += (j & 0xfffff00);
				k ^= 0x0c000a;
	
				dst[j] = Neo68KROM[i + k];
			}
	
			memcpy (Neo68KROM + i, dst, 0x100000);
		}
	
		memcpy (Neo68KROM + 0x100000, Neo68KROM, 0x700000);
		memcpy (Neo68KROM, dst, 0x100000);
	
		free (dst);
	}
	
	samsho5b_sx_decode();
	lans2004_cx_decode(0x1000000 * 4);
	
}

int samsho5bInit()
{
	int nRet;
	
	pNeoInitCallback = samsho5bCallback;

	nRet = NeoInit();
	
	if (nRet == 0) {
		samsho5b_vx_decode();
	}
	
	return nRet;
}

struct BurnDriver BurnDrvsamsho5b = {
	"samsho5b", "samsho5", "neogeo", NULL, "2003",
	"Samurai Shodown V / Samurai Spirits Zero (bootleg)\0", NULL, "bootleg", "Neo Geo",
	L"Samurai Shodown V (bootleg)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 (bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsho5bRomInfo, samsho5bRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsho5bInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2003 (Japan, JAMMA PCB)

static struct BurnRomInfo kf2k3pcbRomDesc[] = {
	{ "271-p1.p1",    0x0400000, 0xb9da070c, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "271-p2.p2",    0x0400000, 0xda3118c4, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "271-p3.p3",    0x0100000, 0x5cefd0d2, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "271-c1.c1",    0x1000000, 0xf5ebb327, 3 | BRF_GRA },           //  3 Sprite data
	{ "271-c2.c2",    0x1000000, 0x2be21620, 3 | BRF_GRA },           //  4 
	{ "271-c3.c3",    0x1000000, 0xddded4ff, 3 | BRF_GRA },           //  5 
	{ "271-c4.c4",    0x1000000, 0xd85521e6, 3 | BRF_GRA },           //  6 
	{ "271-c5.c5",    0x1000000, 0x18aa3540, 3 | BRF_GRA },           //  7 
	{ "271-c6.c6",    0x1000000, 0x1c40de87, 3 | BRF_GRA },           //  8 

	{ "271-m1.m1",    0x0080000, 0xd6bcf2bc, 4 | BRF_ESS | BRF_ESS }, //  9 Z80 code

	{ "271-v1.v1",    0x1000000, 0x1d96154b, 5 | BRF_SND },           // 10 Sound data
};

STDROMPICKEXT(kf2k3pcb, kf2k3pcb, kf2k3pcbBIOS)
STD_ROM_FN(kf2k3pcb)

void kf2k3pcb_bios_decode()
{
	int i, j;
	static const int address[0x40] = {
		0x04,0x0a,0x04,0x0a,0x04,0x0a,0x04,0x0a,
		0x0a,0x04,0x0a,0x04,0x0a,0x04,0x0a,0x04,
		0x09,0x07,0x09,0x07,0x09,0x07,0x09,0x07,
		0x09,0x09,0x04,0x04,0x09,0x09,0x04,0x04,
		0x0b,0x0d,0x0b,0x0d,0x03,0x05,0x03,0x05,
		0x0e,0x0e,0x03,0x03,0x0e,0x0e,0x03,0x03,
		0x03,0x05,0x0b,0x0d,0x03,0x05,0x0b,0x0d,
		0x04,0x00,0x04,0x00,0x0e,0x0a,0x0e,0x0a
	};

	extern unsigned char *Neo68KBIOS;
	unsigned short *src = (unsigned short*)Neo68KBIOS;
	unsigned short *dst = (unsigned short*)malloc( 0x80000 );

	for (i = 0; i < 0x80000 / 2; i++) {
				  j  = i;
		if ( i & 0x00020) j ^= 0x0010;
		if (~i & 0x00010) j ^= 0x0040;
				  j ^= 0x00a0;
		if ( i & 0x00004) j ^= 0x0080;
		if ( i & 0x00200) j ^= 0x0100;
		if (~i & 0x02000) j ^= 0x0400;
		if (~i & 0x10000) j ^= 0x1000;
		if ( i & 0x02000) j ^= 0x8000;
				  j ^= address[((i >> 1) & 0x38) | (i & 7)];

				     dst[i]  = src[j];
		if (dst[i] & 0x0004) dst[i] ^= 0x0001;
		if (dst[i] & 0x0010) dst[i] ^= 0x0002;
		if (dst[i] & 0x0020) dst[i] ^= 0x0008;
	}

	memcpy (src, dst, 0x80000);

	free (dst);
}

static void kf2k3pcbCallback()
{
	int i, j, k;
	for (i = 0; i < 0x100000; i++)
	        Neo68KROM[0x800000 + i] ^= Neo68KROM[0x100002 | i];

	for (i = 0x100000; i < 0x800000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x7fffe0 + (i & 0x1f)];

	for (i = 0x100000; i < 0x700000; i += 4) {
		unsigned short rom16 = *((unsigned short *)(Neo68KROM + i + 1));
		rom16 = BITSWAP16(rom16, 15, 14, 13, 12, 4, 5, 6, 7, 8, 9, 10, 11, 3, 2, 1, 0);
		*((unsigned short *)(Neo68KROM + i + 1)) = rom16;
	}

	memcpy (Neo68KROM + 0x700000, Neo68KROM, 0x100000);

	for (i = 0; i < 0x0100000 / 0x10000; i++) {
		j = BITSWAP08(i, 7, 6, 5, 4, 1, 0, 3, 2);
		memcpy (Neo68KROM + i * 0x010000, Neo68KROM + 0x700000 + j * 0x010000, 0x010000);
	}

	memcpy (Neo68KROM + 0x200000, Neo68KROM + 0x100000, 0x600000);

	for (i = 0x200000; i < 0x900000; i += 0x100000)
	{
		for (j = 0; j < 0x100000; j += 0x100)
		{
			k  = (j & 0xf00) ^ 0x00300;
			k |= BITSWAP08(j >> 12, 4, 5, 6, 7, 1, 0, 3, 2 ) << 12;

			memcpy (Neo68KROM + 0x100000 + j, Neo68KROM + i + k, 0x100);
		}

		memcpy (Neo68KROM + i, Neo68KROM + 0x100000, 0x100000);
	}
}

int kf2k3pcbInit()
{
	int nRet;

	nNeoTextROMSize = 0x100000;
	nNeoProtectionXor = 0x9D;
	pNeoInitCallback = kf2k3pcbCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa7001, 0xff14ea, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
		
		// M1 has additional swap
		for (int i = 0; i < 0x80000; i++) {
			NeoZ80ROM[i] = BITSWAP08(NeoZ80ROM[i], 5, 6, 1, 4, 3, 0, 7, 2);
		}
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k3pcb = {
	"kf2k3pcb", NULL, NULL, NULL, "2003",
	"The King of Fighters 2003 (Japan, JAMMA PCB)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_PCB | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k3pcbRomInfo, kf2k3pcbRomName, NULL, NULL, neogeoInputInfo, kf2k3pcbDIPInfo,
	kf2k3pcbInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2003 (World / US, MVS)

static struct BurnRomInfo kof2003RomDesc[] = {
	{ "271-p1c.p1",   0x400000, 0x530ecc14, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "271-p2c.p2",   0x400000, 0xfd568da9, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "271-p3c.p3",   0x100000, 0xaec5b4a9, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "271-c1c.c1",   0x800000, 0xb1dc25d0, 3 | BRF_GRA },           //  3 Sprite data
	{ "271-c2c.c2",   0x800000, 0xd5362437, 3 | BRF_GRA },           //  4 
	{ "271-c3c.c3",   0x800000, 0x0a1fbeab, 3 | BRF_GRA },           //  5 
	{ "271-c4c.c4",   0x800000, 0x87b19a0c, 3 | BRF_GRA },           //  6 
	{ "271-c5c.c5",   0x800000, 0x704ea371, 3 | BRF_GRA },           //  7 
	{ "271-c6c.c6",   0x800000, 0x20a1164c, 3 | BRF_GRA },           //  8 
	{ "271-c7c.c7",   0x800000, 0x189aba7f, 3 | BRF_GRA },           //  9 
	{ "271-c8c.c8",   0x800000, 0x20ec4fdc, 3 | BRF_GRA },           // 10 

	{ "271-m1c.m1",   0x080000, 0xf5515629, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "271-v1c.v1",   0x800000, 0xffa3f8c7, 5 | BRF_SND },           // 12 Sound data
	{ "271-v2c.v2",   0x800000, 0x5382c7d1, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kof2003, kof2003, neogeo)
STD_ROM_FN(kof2003)

static void kof2003Callback()
{
	int i, j, k;
	for (i = 0; i < 0x100000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x0fffe0 + (i & 0x1f)];

	for (i = 0; i < 0x100000; i++)
	        Neo68KROM[0x800000 + i] ^= Neo68KROM[0x100002 | i];

	for (i = 0x100000; i < 0x800000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x7fffe0 + (i & 0x1f)];

	for (i = 0x100000; i < 0x800000; i += 4) {
		unsigned short rom16 = *((unsigned short *)(Neo68KROM + i + 1));
		rom16 = BITSWAP16(rom16, 15, 14, 13, 12, 5, 4, 7, 6, 9, 8, 11, 10, 3, 2, 1, 0);
		*((unsigned short *)(Neo68KROM + i + 1)) = rom16;
	}

	memcpy (Neo68KROM + 0x700000, Neo68KROM, 0x100000);

	for (i = 0; i < 0x0100000 / 0x10000; i++) {
		j = BITSWAP08(i, 7, 6, 5, 4, 0, 1, 2, 3);
		memcpy (Neo68KROM + i * 0x010000, Neo68KROM + 0x700000 + j * 0x010000, 0x010000);
	}

	memcpy (Neo68KROM + 0x200000, Neo68KROM + 0x100000, 0x600000);

	for (i = 0x200000; i < 0x900000; i += 0x100000)
	{
		for (j = 0; j < 0x100000; j += 0x100)
		{
			k  = (j & 0xf00) ^ 0x00800;
			k |= BITSWAP08(j >> 12, 4, 5, 6, 7, 1, 0, 3, 2 ) << 12;

			memcpy (Neo68KROM + 0x100000 + j, Neo68KROM + i + k, 0x100);
		}

		memcpy (Neo68KROM + i, Neo68KROM + 0x100000, 0x100000);
	}
}

int kof2003Init()
{
	int nRet;

	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x9D;
	pNeoInitCallback = kof2003Callback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa7001, 0xff14ea, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkof2003 = {
	"kof2003", NULL, "neogeo", NULL, "2003",
	"The King of Fighters 2003 (World / US, MVS)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2003RomInfo, kof2003RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2003Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2003 (bootleg set 1)

static struct BurnRomInfo kf2k3blRomDesc[] = {
	{ "2k3-p1.bin",   0x400000, 0x92ed6ee3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "2k3-p2.bin",   0x400000, 0x5d3d8bb3, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "2k3-s1.bin",   0x020000, 0x482c48a5, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "271-c1c.c1",   0x800000, 0xb1dc25d0, 3 | BRF_GRA },           //  3 Sprite data
	{ "271-c2c.c2",   0x800000, 0xd5362437, 3 | BRF_GRA },           //  4 
	{ "271-c3c.c3",   0x800000, 0x0a1fbeab, 3 | BRF_GRA },           //  5 
	{ "271-c4c.c4",   0x800000, 0x87b19a0c, 3 | BRF_GRA },           //  6 
	{ "271-c5c.c5",   0x800000, 0x704ea371, 3 | BRF_GRA },           //  7 
	{ "271-c6c.c6",   0x800000, 0x20a1164c, 3 | BRF_GRA },           //  8 
	{ "271-c7c.c7",   0x800000, 0x189aba7f, 3 | BRF_GRA },           //  9 
	{ "271-c8c.c8",   0x800000, 0x20ec4fdc, 3 | BRF_GRA },           // 10 

	{ "2k3-m1.bin",   0x020000, 0x3a4969ff, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "271-v1c.v1",   0x800000, 0xffa3f8c7, 5 | BRF_SND },           // 12 Sound data
	{ "271-v2c.v2",   0x800000, 0x5382c7d1, 5 | BRF_SND },           // 13
};

STDROMPICKEXT(kf2k3bl, kf2k3bl, neogeo)
STD_ROM_FN(kf2k3bl)

unsigned char __fastcall kf2k3blReadByteProtection(unsigned int)
{
	return CartRAM[0x1ff2];
}

static void kf2k3blCallback()
{
	memcpy (Neo68KROM + 0x100000, Neo68KROM, 0x700000);
	memcpy (Neo68KROM, Neo68KROM + 0x700000, 0x100000);

	lans2004_sx_decode();
}

int kf2k3blInit()
{
	int nRet;

	nNeoProtectionXor = 0x9D;
	pNeoInitCallback = kf2k3blCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		SekMapHandler(6,    0x058196,    0x058197,  SM_READ);
		SekSetReadByteHandler(6,  kf2k3blReadByteProtection);
		
		PCM2DecryptVInfo Info = { 0xa7001, 0xff14ea, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k3bl = {
	"kf2k3bl", "kof2003", "neogeo", NULL, "2003",
	"The King of Fighters 2003 (bootleg set 1)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k3blRomInfo, kf2k3blRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k3blInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2003 (bootleg set 2)

static struct BurnRomInfo kf2k3blaRomDesc[] = {
	{ "2k3-p1bl.bin", 0x100000, 0x4ea414dd, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "2k3-p3bl.bin", 0x400000, 0x370acbff, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "2k3-p2bl.bin", 0x200000, 0x9c04fc52, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "2k3-s1.bin",   0x020000, 0x482c48a5, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "271-c1c.c1",   0x800000, 0xb1dc25d0, 3 | BRF_GRA },           //  3 Sprite data
	{ "271-c2c.c2",   0x800000, 0xd5362437, 3 | BRF_GRA },           //  4 
	{ "271-c3c.c3",   0x800000, 0x0a1fbeab, 3 | BRF_GRA },           //  5 
	{ "271-c4c.c4",   0x800000, 0x87b19a0c, 3 | BRF_GRA },           //  6 
	{ "271-c5c.c5",   0x800000, 0x704ea371, 3 | BRF_GRA },           //  7 
	{ "271-c6c.c6",   0x800000, 0x20a1164c, 3 | BRF_GRA },           //  8 
	{ "271-c7c.c7",   0x800000, 0x189aba7f, 3 | BRF_GRA },           //  9 
	{ "271-c8c.c8",   0x800000, 0x20ec4fdc, 3 | BRF_GRA },           // 10 

	{ "2k3-m1.bin",   0x020000, 0x3a4969ff, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "271-v1c.v1",   0x800000, 0xffa3f8c7, 5 | BRF_SND },           // 12 Sound data
	{ "271-v2c.v2",   0x800000, 0x5382c7d1, 5 | BRF_SND },           // 13
};

STDROMPICKEXT(kf2k3bla, kf2k3bla, neogeo)
STD_ROM_FN(kf2k3bla)

void kf2k3blaCallback()
{
	int i, j, k;
	unsigned char *dst = (unsigned char *)malloc(0x100000);
	if (dst)
	{
		for (i = 0; i < 0x700000; i += 0x100000)
		{
			memcpy(dst, Neo68KROM + i, 0x100000);

			for (j = 0; j < 0x100000; j++)
			{
				k = BITSWAP24(j, 23,22,21,20,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0);
				Neo68KROM[i + j] = dst[k];
			}
		}
	
		free (dst);
	}

	// patched by Altera protection chip
	*((unsigned short*)(Neo68KROM + 0x0f38ac)) = 0x4e75;

	lans2004_sx_decode();
}

void __fastcall kf2k3blaWriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	*((unsigned short*)(CartRAM + (sekAddress & 0x1ffe))) = wordValue;

	if (sekAddress == 0x2ffff2)
	{
		unsigned int nBank = ((CartRAM[0x1ff3] << 16) | (CartRAM[0x1ff2] << 8) | CartRAM[0x1ff0]) + 0x100000;

		if (nBank != nNeo68KROMBank)
		{
			nNeo68KROMBank = nBank;
			SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fdfff, SM_ROM);
		}
	}
}

int kf2k3blaInit()
{
	int nRet;

	nNeoProtectionXor = 0x9D;
	pNeoInitCallback = kf2k3blaCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		// Install bankswitch handler
		SekMapHandler(5,         0x2ffff0, 0x2fffff, SM_WRITE);
		SekSetWriteWordHandler(5, kf2k3blaWriteWordBankswitch);
		
		PCM2DecryptVInfo Info = { 0xa7001, 0xff14ea, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k3bla = {
	"kf2k3bla", "kof2003", "neogeo", NULL, "2003",
	"The King of Fighters 2003 (bootleg set 2)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k3blaRomInfo, kf2k3blaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k3blaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2004 Plus / Hero (The King of Fighters 2003 bootleg)

static struct BurnRomInfo kf2k3plRomDesc[] = {
	{ "2k3-p1pl.bin", 0x100000, 0x07b84112, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "2k3-p3bl.bin", 0x400000, 0x370acbff, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "2k3-p2bl.bin", 0x200000, 0x9c04fc52, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "2k3-s1pl.bin", 0x020000, 0xad548a36, 2 | BRF_GRA },           //  3 Text layer tiles

	{ "271-c1c.c1",   0x800000, 0xb1dc25d0, 3 | BRF_GRA },           //  3 Sprite data
	{ "271-c2c.c2",   0x800000, 0xd5362437, 3 | BRF_GRA },           //  4 
	{ "271-c3c.c3",   0x800000, 0x0a1fbeab, 3 | BRF_GRA },           //  5 
	{ "271-c4c.c4",   0x800000, 0x87b19a0c, 3 | BRF_GRA },           //  6 
	{ "271-c5c.c5",   0x800000, 0x704ea371, 3 | BRF_GRA },           //  7 
	{ "271-c6c.c6",   0x800000, 0x20a1164c, 3 | BRF_GRA },           //  8 
	{ "271-c7c.c7",   0x800000, 0x189aba7f, 3 | BRF_GRA },           //  9 
	{ "271-c8c.c8",   0x800000, 0x20ec4fdc, 3 | BRF_GRA },           // 10 

	{ "2k3-m1.bin",   0x020000, 0x3a4969ff, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "271-v1c.v1",   0x800000, 0xffa3f8c7, 5 | BRF_SND },           // 12 Sound data
	{ "271-v2c.v2",   0x800000, 0x5382c7d1, 5 | BRF_SND },           // 13
};

STDROMPICKEXT(kf2k3pl, kf2k3pl, neogeo)
STD_ROM_FN(kf2k3pl)

struct BurnDriver BurnDrvkf2k3pl = {
	"kf2k3pl", "kof2003", "neogeo", NULL, "2003",
	"The King of Fighters 2004 Plus / Hero (The King of Fighters 2003 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k3plRomInfo, kf2k3plRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k3blaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2004 Ultra Plus (The King of Fighters 2003 bootleg)

static struct BurnRomInfo kf2k3uplRomDesc[] = {
	{ "2k3-p1up.bin", 0x800000, 0x87294c01, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "2k3-s1up.bin", 0x020000, 0xe5708c0c, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "271-c1c.c1",   0x800000, 0xb1dc25d0, 3 | BRF_GRA },           //  2 Sprite data
	{ "271-c2c.c2",   0x800000, 0xd5362437, 3 | BRF_GRA },           //  3 
	{ "271-c3c.c3",   0x800000, 0x0a1fbeab, 3 | BRF_GRA },           //  4 
	{ "271-c4c.c4",   0x800000, 0x87b19a0c, 3 | BRF_GRA },           //  5 
	{ "271-c5c.c5",   0x800000, 0x704ea371, 3 | BRF_GRA },           //  6 
	{ "271-c6c.c6",   0x800000, 0x20a1164c, 3 | BRF_GRA },           //  7 
	{ "271-c7c.c7",   0x800000, 0x189aba7f, 3 | BRF_GRA },           //  8 
	{ "271-c8c.c8",   0x800000, 0x20ec4fdc, 3 | BRF_GRA },           //  9 

	{ "2k3-m1.bin",   0x020000, 0x3a4969ff, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "271-v1c.v1",   0x800000, 0xffa3f8c7, 5 | BRF_SND },           // 11 Sound data
	{ "271-v2c.v2",   0x800000, 0x5382c7d1, 5 | BRF_SND },           // 12
};

STDROMPICKEXT(kf2k3upl, kf2k3upl, neogeo)
STD_ROM_FN(kf2k3upl)

void kf2k3uplCallback()
{
	int i, j;
	memcpy (Neo68KROM + 0x100000, Neo68KROM, 0x600000);
	memcpy (Neo68KROM, Neo68KROM + 0x700000, 0x100000);

	for(i = 0; i < 0x2000; i++) {
		j = (i & 0x1f00) | BITSWAP08(i & 0xff, 7, 1, 5, 4, 3, 2, 6, 0) ;
		Neo68KROM[0xfe000 + i] = Neo68KROM[0xd0610 + j];
	}

	garoubl_sx_decode();
}

int kof2k3uplInit()
{
	int nRet;

	nNeoProtectionXor = 0x9D;
	pNeoInitCallback = kf2k3uplCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		SekMapHandler(6,    0x058196,    0x058197,  SM_READ);
		SekSetReadByteHandler(6,  kf2k3blReadByteProtection);
		
		PCM2DecryptVInfo Info = { 0xa7001, 0xff14ea, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k3upl = {
	"kf2k3upl", "kof2003", "neogeo", NULL, "2003",
	"The King of Fighters 2004 Ultra Plus (The King of Fighters 2003 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k3uplRomInfo, kf2k3uplRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2k3uplInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown V Special / Samurai Spirits Zero Special (set 1, uncensored)

static struct BurnRomInfo samsh5spRomDesc[] = {
	{ "272-p1.p1",    0x400000, 0xfb7a6bba, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "272-p2.sp2",   0x400000, 0x63492ea6, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "272-c1.c1",    0x800000, 0x4f97661a, 3 | BRF_GRA },           //  2 Sprite data
	{ "272-c2.c2",    0x800000, 0xa3afda4f, 3 | BRF_GRA },           //  3 
	{ "272-c3.c3",    0x800000, 0x8c3c7502, 3 | BRF_GRA },           //  4 
	{ "272-c4.c4",    0x800000, 0x32d5e2e2, 3 | BRF_GRA },           //  5 
	{ "272-c5.c5",    0x800000, 0x6ce085bc, 3 | BRF_GRA },           //  6 
	{ "272-c6.c6",    0x800000, 0x05c8dc8e, 3 | BRF_GRA },           //  7 
	{ "272-c7.c7",    0x800000, 0x1417b742, 3 | BRF_GRA },           //  8 
	{ "272-c8.c8",    0x800000, 0xd49773cd, 3 | BRF_GRA },           //  9 

	{ "272-m1.m1",    0x080000, 0xadeebf40, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "272-v1.v1",    0x800000, 0x76a94127, 5 | BRF_SND },           // 11 Sound data
	{ "272-v2.v2",    0x800000, 0x4ba507f1, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(samsh5sp, samsh5sp, neogeo)
STD_ROM_FN(samsh5sp)

static void samsh5spCallback()
{
	int i;
	unsigned char *dst = (unsigned char *)malloc(0x800000);
	if (dst)
	{
		static const int sec[16] = {
			0x00, 0x01, 0x0a, 0x09, 0x0c, 0x0b, 0x0e, 0x05,
			0x02, 0x0d, 0x08, 0x0f, 0x04, 0x07, 0x06, 0x03
		};

		memcpy (dst, Neo68KROM, 0x800000);

		for (i = 0; i < 0x800000 / 0x080000; i++) {
			memcpy (Neo68KROM + i * 0x080000, dst + sec[i] * 0x080000, 0x080000);
		}

		free(dst);
	}
}

static int samsh5spInit()
{
	int nRet;

	nNeoTextROMSize = 0x20000;
	nNeoProtectionXor = 0x0D;
	pNeoInitCallback = samsh5spCallback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0x02000, 0xffb440, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvsamsh5sp = {
	"samsh5sp", NULL, "neogeo", NULL, "2003",
	"Samurai Shodown V Special / Samurai Spirits Zero Special (set 1, uncensored)\0", NULL, "Yuki Enterprise / SNK Playmore", "Neo Geo",
	L"Samurai Shodown V Special (set 1, uncensored)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 Special (set 1, uncensored)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsh5spRomInfo, samsh5spRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsh5spInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown V Special / Samurai Spirits Zero Special (set 2, censored)

static struct BurnRomInfo samsh5sphRomDesc[] = {
	{ "272-p1c.p1",   0x400000, 0x9291794d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "272-p2c.sp2",  0x400000, 0xfa1a7dd8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "272-c1.c1",    0x800000, 0x4f97661a, 3 | BRF_GRA },           //  2 Sprite data
	{ "272-c2.c2",    0x800000, 0xa3afda4f, 3 | BRF_GRA },           //  3 
	{ "272-c3.c3",    0x800000, 0x8c3c7502, 3 | BRF_GRA },           //  4 
	{ "272-c4.c4",    0x800000, 0x32d5e2e2, 3 | BRF_GRA },           //  5 
	{ "272-c5.c5",    0x800000, 0x6ce085bc, 3 | BRF_GRA },           //  6 
	{ "272-c6.c6",    0x800000, 0x05c8dc8e, 3 | BRF_GRA },           //  7 
	{ "272-c7.c7",    0x800000, 0x1417b742, 3 | BRF_GRA },           //  8 
	{ "272-c8.c8",    0x800000, 0xd49773cd, 3 | BRF_GRA },           //  9 

	{ "272-m1.m1",    0x080000, 0xadeebf40, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "272-v1.v1",    0x800000, 0x76a94127, 5 | BRF_SND },           // 11 Sound data
	{ "272-v2.v2",    0x800000, 0x4ba507f1, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(samsh5sph, samsh5sph, neogeo)
STD_ROM_FN(samsh5sph)

struct BurnDriver BurnDrvsamsh5sph = {
	"samsh5sph", "samsh5sp", "neogeo", NULL, "2003",
	"Samurai Shodown V Special / Samurai Spirits Zero Special (set 2, censored)\0", NULL, "Yuki Enterprise / SNK Playmore", "Neo Geo",
	L"Samurai Shodown V Special (set 2, censored)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 Special (set 2, censored)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsh5sphRomInfo, samsh5sphRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsh5spInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Samurai Shodown V Special / Samurai Spirits Zero Special (set 3, less censored)

static struct BurnRomInfo samsh5spnRomDesc[] = {
	{ "272-p1ca.p1",  0x400000, 0xc30a08dd, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "272-p2ca.sp2", 0x400000, 0xbd64a518, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "272-c1.c1",    0x800000, 0x4f97661a, 3 | BRF_GRA },           //  2 Sprite data
	{ "272-c2.c2",    0x800000, 0xa3afda4f, 3 | BRF_GRA },           //  3 
	{ "272-c3.c3",    0x800000, 0x8c3c7502, 3 | BRF_GRA },           //  4 
	{ "272-c4.c4",    0x800000, 0x32d5e2e2, 3 | BRF_GRA },           //  5 
	{ "272-c5.c5",    0x800000, 0x6ce085bc, 3 | BRF_GRA },           //  6 
	{ "272-c6.c6",    0x800000, 0x05c8dc8e, 3 | BRF_GRA },           //  7 
	{ "272-c7.c7",    0x800000, 0x1417b742, 3 | BRF_GRA },           //  8 
	{ "272-c8.c8",    0x800000, 0xd49773cd, 3 | BRF_GRA },           //  9 

	{ "272-m1.m1",    0x080000, 0xadeebf40, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "272-v1.v1",    0x800000, 0x76a94127, 5 | BRF_SND },           // 11 Sound data
	{ "272-v2.v2",    0x800000, 0x4ba507f1, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(samsh5spn, samsh5spn, neogeo)
STD_ROM_FN(samsh5spn)

struct BurnDriver BurnDrvsamsh5spn = {
	"samsh5spn", "samsh5sp", "neogeo", NULL, "2003",
	"Samurai Shodown V Special / Samurai Spirits Zero Special (set 3, less censored)\0", NULL, "Yuki Enterprise / SNK Playmore", "Neo Geo",
	L"Samurai Shodown V Special (set 3, less censored)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 Special (set 3, less censored)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsh5spnRomInfo, samsh5spnRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsh5spInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Jockey Grand Prix (set 1)

static struct BurnRomInfo jockeygpRomDesc[] = {
	{ "008-epr.p1",   0x100000, 0x2fb7f388, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "008-c1.c1",    0x800000, 0xa9acbf18, 3 | BRF_GRA },           //  1 Sprite data
	{ "008-c2.c2",    0x800000, 0x6289eef9, 3 | BRF_GRA },           //  2 

	{ "008-mg1.m1",   0x080000, 0xd163c690, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "008-v1.v1",    0x200000, 0x443eadba, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(jockeygp, jockeygp, neogeo)
STD_ROM_FN(jockeygp)

int jockeygpInit()
{
	nNeoProtectionXor = 0xAC;
	return NeoInit();
}

struct BurnDriver BurnDrvjockeygp = {
	"jockeygp", NULL, "neogeo", NULL, "2001",
	"Jockey Grand Prix (set 1)\0", NULL, "Sun Amusement / BrezzaSoft", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_GAMBLING | HARDWARE_SNK_ENCRYPTED_M1, GBF_MISC, 0,
	NULL, jockeygpRomInfo, jockeygpRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	jockeygpInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Jockey Grand Prix (set 2)

static struct BurnRomInfo jockeygpaRomDesc[] = {
	{ "008-epr_a.p1", 0x100000, 0xb8f35532, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	
	{ "008-c1.c1",    0x800000, 0xa9acbf18, 3 | BRF_GRA },           //  1 Sprite data
	{ "008-c2.c2",    0x800000, 0x6289eef9, 3 | BRF_GRA },           //  2 

	{ "008-mg1.m1",   0x080000, 0xd163c690, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "008-v1.v1",    0x200000, 0x443eadba, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(jockeygpa, jockeygpa, neogeo)
STD_ROM_FN(jockeygpa)

struct BurnDriver BurnDrvjockeygpa = {
	"jockeygpa", "jockeygp", "neogeo", NULL, "2001",
	"Jockey Grand Prix (set 2)\0", NULL, "Sun Amusement / BrezzaSoft", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_GAMBLING | HARDWARE_SNK_ENCRYPTED_M1, GBF_MISC, 0,
	NULL, jockeygpaRomInfo, jockeygpaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	jockeygpInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// V-Liner (set 1)

static struct BurnRomInfo vlinerRomDesc[] = {
	{ "epr.p1",       0x080000, 0x72a2c043, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "s-1.s1",       0x020000, 0x972d8c31, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "c-1.c1",       0x080000, 0x5118f7c0, 3 | BRF_GRA },           //  2 Sprite data
	{ "c-2.c2",       0x080000, 0xefe9b33e, 3 | BRF_GRA },           //  3 

	{ "m-1.m1",       0x010000, 0x9b92b7d1, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code
};

STDROMPICKEXT(vliner, vliner, neogeo)
STD_ROM_FN(vliner)

struct BurnDriver BurnDrvvliner = {
	"vliner", NULL, "neogeo", NULL, "2001",
	"V-Liner (set 1)\0", NULL, "Dyna / BreezaSoft", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_GAMBLING, GBF_CASINO, 0,
	NULL, vlinerRomInfo, vlinerRomName, NULL, NULL, vlinerInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// V-Liner (set 2)

static struct BurnRomInfo vlineroRomDesc[] = {
	{ "epr_54.p1",    0x080000, 0x172efc18, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "s-1.s1",       0x020000, 0x972d8c31, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "c-1.c1",       0x080000, 0x5118f7c0, 3 | BRF_GRA },           //  2 Sprite data
	{ "c-2.c2",       0x080000, 0xefe9b33e, 3 | BRF_GRA },           //  3 

	{ "m-1.m1",       0x010000, 0x9b92b7d1, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code
};

STDROMPICKEXT(vlinero, vlinero, neogeo)
STD_ROM_FN(vlinero)

struct BurnDriver BurnDrvvlinero = {
	"vlinero", "vliner", "neogeo", NULL, "2001",
	"V-Liner (set 2)\0", NULL, "Dyna / BreezaSoft", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_GAMBLING, GBF_CASINO, 0,
	NULL, vlineroRomInfo, vlineroRomName, NULL, NULL, vlinerInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ironclad (Prototype)

static struct BurnRomInfo ironcladRomDesc[] = {
	{ "220-p1p.p1",   0x200000, 0xce37e3a0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "220-s1p.p1",   0x020000, 0x372fe217, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "220-c1p.c1",   0x400000, 0x9aa2b7dc, 3 | BRF_GRA },           //  2 Sprite data
	{ "220-c2p.c2",   0x400000, 0x8a2ad708, 3 | BRF_GRA },           //  3 
	{ "220-c3p.c3",   0x400000, 0xd67fb15a, 3 | BRF_GRA },           //  4 
	{ "220-c4p.c4",   0x400000, 0xe73ea38b, 3 | BRF_GRA },           //  5 

	{ "220-m1p.m1",   0x020000, 0x3a08bb63, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "220-v1p.v1",   0x400000, 0x8f30a215, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(ironclad, ironclad, neogeo)
STD_ROM_FN(ironclad)

struct BurnDriver BurnDrvironclad = {
	"ironclad", NULL, "neogeo", NULL, "1996",
	"Ironclad (Prototype)\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_PROTOTYPE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_HORSHOOT, 0,
	NULL, ironcladRomInfo, ironcladRomName, NULL, NULL, neogeoInputInfo, neogeousaDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ironclad (Prototype, older)

static struct BurnRomInfo ironcladoRomDesc[] = {
	{ "220-p1po.p1",  0x200000, 0x62a942c6, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "220-s1p.s1",   0x020000, 0x372fe217, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "220-c1p.c1",   0x400000, 0x9aa2b7dc, 3 | BRF_GRA },           //  2 Sprite data
	{ "220-c2p.c2",   0x400000, 0x8a2ad708, 3 | BRF_GRA },           //  3 
	{ "220-c3p.c3",   0x400000, 0xd67fb15a, 3 | BRF_GRA },           //  4 
	{ "220-c4p.c4",   0x400000, 0xe73ea38b, 3 | BRF_GRA },           //  5 

	{ "220-m1p.m1",   0x020000, 0x3a08bb63, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "220-v1p.v1",   0x400000, 0x8f30a215, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(ironclado, ironclado, neogeo)
STD_ROM_FN(ironclado)

struct BurnDriver BurnDrvironclado = {
	"ironclado", "ironclad", "neogeo", NULL, "1996",
	"Ironclad (Prototype, older)\0", NULL, "Saurus", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_HORSHOOT, 0,
	NULL, ironcladoRomInfo, ironcladoRomName, NULL, NULL, neogeoInputInfo, neogeousaDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// King of Gladiator (The King of Fighters '97 bootleg)

static struct BurnRomInfo kogRomDesc[] = {
	{ "5232-p1.bin",  0x200000, 0xd2413ec6, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "232-p2.sp2",   0x400000, 0x158b23f6, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "5232-s1.bin",  0x020000, 0x0bef69da, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "5232-c1a.bin", 0x800000, 0x4eab9b0a, 3 | BRF_GRA },           //  3 Sprite data
	{ "5232-c2a.bin", 0x800000, 0x697f8fd0, 3 | BRF_GRA },           //  4 
	{ "5232-c1b.bin", 0x800000, 0x1143fdf3, 3 | BRF_GRA },           //  5 
	{ "5232-c2b.bin", 0x800000, 0xea82cf8f, 3 | BRF_GRA },           //  6 
	{ "5232-c3.bin",  0x400000, 0xabd1be07, 3 | BRF_GRA },           //  7 
	{ "5232-c4.bin",  0x400000, 0xd2bd967b, 3 | BRF_GRA },           //  8 

	{ "232-m1.m1",    0x020000, 0x45348747, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.v1",    0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.v2",    0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.v3",    0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kog, kog, neogeo)
STD_ROM_FN(kog)

// This is actually set by a jumper on the PCB
unsigned short __fastcall KogReadWord(unsigned int)
{
	extern int nBIOS;
	if (nBIOS == 5 || nBIOS == 6 || nBIOS == 7 || nBIOS == 8 || nBIOS == 10 || nBIOS == 11) {
		return 0xff00;
	} else {
		return 0xff01;
	}
}

static void kogCallback()
{
	int i;
	unsigned char *dst = (unsigned char *)malloc( 0x100000 );

	if (dst)
	{
		static const int sec[] = { 0x3, 0x8, 0x7, 0xc, 0x1, 0xa, 0x6, 0xd };
	
		for (i = 0; i < 0x100000 / 0x020000; i++)
			memcpy (dst + i * 0x020000, Neo68KROM + sec[i] * 0x020000, 0x020000);

		memcpy (dst + 0x090000, Neo68KROM + 0x040000, 0x004000);
		memcpy (Neo68KROM, dst, 0x100000);
		free (dst);
	}

	for (i = 0x90000; i < 0x94000; i+=2) {
		if ((*((unsigned short *)(Neo68KROM + i + 0)) & 0xf2bf) == 0x42b9 && *((unsigned short *)(Neo68KROM + i + 2)) == 0)
			*((unsigned short *)(Neo68KROM + i + 2)) = 0x0009;

		if (*((unsigned short *)(Neo68KROM + i + 0)) == 0x4eb8) {
			*((unsigned short *)(Neo68KROM + i + 0))  = 0x6100;
			*((unsigned short *)(Neo68KROM + i + 2)) += 0xfffe - (i & 0xfffe);
		}
	}

	memcpy (Neo68KROM + 0x0007a6, Neo68KROM + 0x0907a6, 0x000006);
	memcpy (Neo68KROM + 0x0007c6, Neo68KROM + 0x0907c6, 0x000006);
	memcpy (Neo68KROM + 0x0007e6, Neo68KROM + 0x0907e6, 0x000006);
	memcpy (Neo68KROM + 0x100000, Neo68KROM + 0x200000, 0x400000);

	*((unsigned short *)(Neo68KROM + 0x924ac)) = 0x0009;
	*((unsigned short *)(Neo68KROM + 0x9251c)) = 0x0009;

	lans2004_sx_decode();
	lans2004_cx_decode(0x800000 * 5);
}

static int kogInit()
{
 	int nRet;

	nBurnCPUSpeedAdjust = 0x010a;	// fix garbage on intro
	pNeoInitCallback = kogCallback;

 	nRet = NeoInit();
 
	if (nRet == 0) {
		// Install jumper that controls title screen language
		SekOpen(0);
		SekMapHandler(5, 0x0FFFFE, 0x0FFFFF, SM_READ);
		SekSetReadWordHandler(5, KogReadWord);
		SekClose();
	}

	return nRet;
}

struct BurnDriver BurnDrvkog = {
	"kog", "kof97", "neogeo", NULL, "1997",
	"King of Gladiator (The King of Fighters '97 bootleg)\0", NULL, "bootleg", "Neo Geo",
	L"King of Gladiator (bootleg)\0\u683C\u6597\u4E4B\u738B\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kogRomInfo, kogRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kogInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg)

static struct BurnRomInfo cthd2003RomDesc[] = {
	{ "5003-p1.bin",  0x100000, 0xbb7602c1, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "5003-p2.bin",  0x400000, 0xadc1c22b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "5003-s1.bin",  0x020000, 0x5ba29aab, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "5003-c1.bin",  0x800000, 0x68f54b67, 3 | BRF_GRA },           //  3 Sprite data
	{ "5003-c2.bin",  0x800000, 0x2f8849d5, 3 | BRF_GRA },           //  4 
	{ "5003-c3.bin",  0x800000, 0xac4aff71, 3 | BRF_GRA },           //  5 
	{ "5003-c4.bin",  0x800000, 0xafef5d66, 3 | BRF_GRA },           //  6 
	{ "5003-c5.bin",  0x800000, 0xc7c1ae50, 3 | BRF_GRA },           //  7 
	{ "5003-c6.bin",  0x800000, 0x613197f9, 3 | BRF_GRA },           //  8 
	{ "5003-c7.bin",  0x800000, 0x64ddfe0f, 3 | BRF_GRA },           //  9 
	{ "5003-c8.bin",  0x800000, 0x917a1439, 3 | BRF_GRA },           // 10 

	{ "5003-m1.bin",  0x020000, 0x1a8c274b, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "262-v1-08-e0.v1",    0x400000, 0x83d49ecf, 5 | BRF_SND },     // 12 Sound data
	{ "262-v2-08-e0.v2",    0x400000, 0x003f1843, 5 | BRF_SND },     // 13 
	{ "262-v3-08-e0.v3",    0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 14 
	{ "262-v4-08-e0.v4",    0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 15 
};

STDROMPICKEXT(cthd2003, cthd2003, neogeo)
STD_ROM_FN(cthd2003)

static void cthd2003Bankswitch(unsigned int nBank)
{
	static unsigned int cthd2003_banks[8] =	{
		0x200000, 0x100000, 0x200000, 0x100000, 0x200000, 0x100000, 0x400000, 0x300000,
	};

	nBank = cthd2003_banks[nBank & 7];

	if (nBank != nNeo68KROMBank) {
		nNeo68KROMBank = nBank;
		SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fffff, SM_ROM);
	}
}

void __fastcall cthd2003WriteByteBankswitch(unsigned int sekAddress, unsigned char byteValue)
{
	if (sekAddress == 0x2ffff0) {
		cthd2003Bankswitch(byteValue);
	}
}

void __fastcall cthd2003WriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress == 0x2ffff0) {
		cthd2003Bankswitch(wordValue);
	}
}
static void cthd2003_decode()
{
	int i, n;

	// fix garbage on s1 layer over everything
	*((unsigned short*)(Neo68KROM + 0xf415a)) = 0x4ef9;
	*((unsigned short*)(Neo68KROM + 0xf415c)) = 0x000f;
	*((unsigned short*)(Neo68KROM + 0xf415e)) = 0x4cf2;

	// Fix corruption in attract mode before title screen
	for (i = 0x1ae290; i < 0x1ae8d0; i+=2) {
		*((unsigned short*)(Neo68KROM + i)) = 0x0000;
	}

	// Fix for title page
	for (i = 0x1f8ef0; i < 0x1fa1f0; i += 4) {
		*((unsigned short*)(Neo68KROM + i + 0)) -= 0x7000;
		*((unsigned short*)(Neo68KROM + i + 2)) -= 0x0010;
	}

	// Fix for green dots on title page
	for (i = 0xac500; i < 0xac520; i+=2) {
		*((unsigned short*)(Neo68KROM + i)) = 0xffff;
	}

	// Fix for blanks as screen change level end clear
	*((unsigned short*)(Neo68KROM + 0x991d0)) = 0xdd03;
	*((unsigned short*)(Neo68KROM + 0x99306)) = 0xdd03;
	*((unsigned short*)(Neo68KROM + 0x99354)) = 0xdd03;
	*((unsigned short*)(Neo68KROM + 0x9943e)) = 0xdd03;

	// Swap bits 15 & 16 in the address of the Z80 ROM
	for (i = 0; i < 0x10000 / 2; i++) {
		n = NeoZ80ROM[0x08000 + i];
		NeoZ80ROM[0x08000 + i] = NeoZ80ROM[0x10000 + i];
		NeoZ80ROM[0x10000 + i] = n;
	}

	DoPerm(0);
}

static void cthd2003Callback()
{
	int i, n;
	for (i = 0; i < 0x8000; i++)
	{
		n = NeoTextROM[0x28000 + i];
		NeoTextROM[0x28000 + i] = NeoTextROM[0x30000 + i];
		NeoTextROM[0x30000 + i] = n;
	}

	cthd2003_decode();
}

static int cthd2003Init()
{
 	int nRet;

	pNeoInitCallback = cthd2003Callback;
	pNeoBankswitchCallback = NeoMapBank;
	nBurnCPUSpeedAdjust = 0x010a;

 	nRet = NeoInit();

	if (nRet == 0) {
		SekOpen(0);

		// Install bankswitch handler
		SekMapHandler(4,	0x200000, 0x2fffff, SM_WRITE);
		SekSetWriteWordHandler(4, cthd2003WriteWordBankswitch);
		SekSetWriteByteHandler(4, cthd2003WriteByteBankswitch);

		cthd2003Bankswitch(0);

		SekClose();
	}

	return nRet;
}

struct BurnDriver BurnDrvcthd2003 = {
	"cthd2003", "kof2001", "neogeo", NULL, "2003",
	"Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg)\0", NULL, "bootleg", "Neo Geo",
	L"Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg)\0\u81E5\u864E\u85CF\u9F8D Crouching Tiger Hidden Dragon 2003\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, cthd2003RomInfo, cthd2003RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	cthd2003Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Crouching Tiger Hidden Dragon 2003 Super Plus (The King of Fighters 2001 bootleg)

static struct BurnRomInfo ct2k3spRomDesc[] = {
	{ "5003-p1sp.bin",  0x100000, 0xab5c4de0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "5003-p2.bin",    0x400000, 0xadc1c22b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "5003-s1sp.bin",  0x040000, 0x6c355ab4, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "5003-c1.bin",    0x800000, 0x68f54b67, 3 | BRF_GRA },           //  3 Sprite data
	{ "5003-c2.bin",    0x800000, 0x2f8849d5, 3 | BRF_GRA },           //  4 
	{ "5003-c3.bin",    0x800000, 0xac4aff71, 3 | BRF_GRA },           //  5 
	{ "5003-c4.bin",    0x800000, 0xafef5d66, 3 | BRF_GRA },           //  6 
	{ "5003-c5.bin",    0x800000, 0xc7c1ae50, 3 | BRF_GRA },           //  7 
	{ "5003-c6.bin",    0x800000, 0x613197f9, 3 | BRF_GRA },           //  8 
	{ "5003-c7.bin",    0x800000, 0x64ddfe0f, 3 | BRF_GRA },           //  9 
	{ "5003-c8.bin",    0x800000, 0x917a1439, 3 | BRF_GRA },           // 10 

	{ "5003-m1.bin",    0x020000, 0x1a8c274b, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "262-v1-08-e0.v1",      0x400000, 0x83d49ecf, 5 | BRF_SND },     // 12 Sound data
	{ "262-v2-08-e0.v2",      0x400000, 0x003f1843, 5 | BRF_SND },     // 13 
	{ "262-v3-08-e0.v3",      0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 14 
	{ "262-v4-08-e0.v4",      0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 15 
};

STDROMPICKEXT(ct2k3sp, ct2k3sp, neogeo)
STD_ROM_FN(ct2k3sp)

static void ct2k3spCallback()
{
	int i, j;
	unsigned char *dst = (unsigned char *)malloc( 0x040000 );
	if (dst)
	{
		for (i = 0; i < 0x040000; i++)
		{
			j = BITSWAP24(i, 23,22,21,20,19,18,17,3,0,1,4,2,13,14,15,16,5,6,11,10,9,8,7,12);
			dst[i] = NeoTextROM[0x020000 + j];
		}

		memcpy (NeoTextROM + 0x020000, dst, 0x040000 );

		free (dst);
	}

	cthd2003_decode();
}

static int ct2k3spInit()
{
 	int nRet;

	nBurnCPUSpeedAdjust = 0x010a;
	pNeoInitCallback = ct2k3spCallback;
	pNeoBankswitchCallback = NeoMapBank;

 	nRet = NeoInit();

	if (nRet == 0) {
		SekOpen(0);

		// Install bankswitch handler
		SekMapHandler(4,	0x200000, 0x2fffff, SM_WRITE);
		SekSetWriteWordHandler(4, cthd2003WriteWordBankswitch);
		SekSetWriteByteHandler(4, cthd2003WriteByteBankswitch);

		cthd2003Bankswitch(0);

		SekClose();
	}

	return nRet;
}

struct BurnDriver BurnDrvct2k3sp = {
	"ct2k3sp", "kof2001", "neogeo", NULL, "2003",
	"Crouching Tiger Hidden Dragon 2003 Super Plus (The King of Fighters 2001 bootleg)\0", NULL, "bootleg", "Neo Geo",
	L"Crouching Tiger Hidden Dragon 2003 Super Plus (The King of Fighters 2001 bootleg)\0\u81E5\u864E\u85CF\u9F8D Crouching Tiger Hidden Dragon 2003 Super Plus\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, ct2k3spRomInfo, ct2k3spRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	ct2k3spInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Crouching Tiger Hidden Dragon 2003 Super Plus alternate (The King of Fighters 2001 bootleg)

static struct BurnRomInfo ct2k3saRomDesc[] = {
	{ "5003-p1sa.bin",  0x100000, 0x013a509d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "5003-p2.bin",    0x400000, 0xadc1c22b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "5003-s1sa.bin",  0x020000, 0x4e1f7eae, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "5003-c1.bin",    0x800000, 0x68f54b67, 3 | BRF_GRA },           //  3 Sprite data
	{ "5003-c2.bin",    0x800000, 0x2f8849d5, 3 | BRF_GRA },           //  4 
	{ "5003-c3.bin",    0x800000, 0xac4aff71, 3 | BRF_GRA },           //  5 
	{ "5003-c4.bin",    0x800000, 0xafef5d66, 3 | BRF_GRA },           //  6 
	{ "5003-c5.bin",    0x800000, 0xc7c1ae50, 3 | BRF_GRA },           //  7 
	{ "5003-c6.bin",    0x800000, 0x613197f9, 3 | BRF_GRA },           //  8 
	{ "5003-c7.bin",    0x800000, 0x64ddfe0f, 3 | BRF_GRA },           //  9 
	{ "5003-c8.bin",    0x800000, 0x917a1439, 3 | BRF_GRA },           // 10 

	{ "5003-m1.bin",    0x020000, 0x1a8c274b, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "262-v1-08-e0.v1",      0x400000, 0x83d49ecf, 5 | BRF_SND },     // 12 Sound data
	{ "262-v2-08-e0.v2",      0x400000, 0x003f1843, 5 | BRF_SND },     // 13 
	{ "262-v3-08-e0.v3",      0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 14 
	{ "262-v4-08-e0.v4",      0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 15 
};

STDROMPICKEXT(ct2k3sa, ct2k3sa, neogeo)
STD_ROM_FN(ct2k3sa)

static int ct2k3saInit()
{
	nBurnCPUSpeedAdjust = 0x010d;
	pNeoInitCallback = cthd2003_decode;
 	return NeoInit();
}

struct BurnDriver BurnDrvct2k3sa = {
	"ct2k3sa", "kof2001", "neogeo", NULL, "2003",
	"Crouching Tiger Hidden Dragon 2003 Super Plus alternate (The King of Fighters 2001 bootleg)\0", NULL, "bootleg", "Neo Geo",
	L"Crouching Tiger Hidden Dragon 2003 Super Plus alternate (The King of Fighters 2001 bootleg)\0\u81E5\u864E\u85CF\u9F8D Crouching Tiger Hidden Dragon 2003 Super Plus\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, ct2k3saRomInfo, ct2k3saRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	ct2k3saInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters Special Edition 2004 (The King of Fighters 2002 bootleg)

static struct BurnRomInfo kof2k4seRomDesc[] = {
	{ "k2k4s-p1.bin", 0x400000, 0xe6c50566, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "k2k4s-p2.bin", 0x080000, 0x21a84084, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "k2k4s-p3.bin", 0x080000, 0xfebb484e, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "k2k4s-s1.bin", 0x020000, 0xa3c9b2d8, 2 | BRF_GRA },           //  3 Text layer tiles

	{ "k2k4s-c4.bin", 0x800000, 0x7a050288, 3 | BRF_GRA },           //  4 Sprite data
	{ "k2k4s-c8.bin", 0x800000, 0xe924afcf, 3 | BRF_GRA },           //  5 
	{ "k2k4s-c3.bin", 0x800000, 0x959fad0b, 3 | BRF_GRA },           //  6 
	{ "k2k4s-c7.bin", 0x800000, 0xefe6a468, 3 | BRF_GRA },           //  7 
	{ "k2k4s-c2.bin", 0x800000, 0x74bba7c6, 3 | BRF_GRA },           //  8 
	{ "k2k4s-c6.bin", 0x800000, 0xe20d2216, 3 | BRF_GRA },           //  9 
	{ "k2k4s-c1.bin", 0x800000, 0xfa705b2b, 3 | BRF_GRA },           // 10 
	{ "k2k4s-c5.bin", 0x800000, 0x2c912ff9, 3 | BRF_GRA },           // 11 

	{ "k2k4s-m1.bin", 0x020000, 0x5a47d9ad, 4 | BRF_ESS | BRF_PRG }, // 12 Z80 code

	{ "k2k4s-v2.bin", 0x800000, 0xe4ddfb3f, 5 | BRF_SND },           // 13 Sound data
	{ "k2k4s-v1.bin", 0x800000, 0xb887d287, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kof2k4se, kof2k4se, neogeo)
STD_ROM_FN(kof2k4se)

static void kof2k4seCallback()
{
	unsigned char *dst = (unsigned char*)malloc(0x100000);

	if (dst)
	{
		memcpy (dst,                  Neo68KROM + 0x000000, 0x100000);
		memcpy (Neo68KROM + 0x000000, Neo68KROM + 0x400000, 0x100000);
		memcpy (Neo68KROM + 0x400000, dst,                  0x100000);
		memcpy (dst,                  Neo68KROM + 0x100000, 0x100000);
		memcpy (Neo68KROM + 0x100000, Neo68KROM + 0x300000, 0x100000);
		memcpy (Neo68KROM + 0x300000, dst,                  0x100000);

		free (dst);
	}
}

static int kof2k4seInit()
{
	pNeoInitCallback = kof2k4seCallback;

	return NeoInit();
}

struct BurnDriver BurnDrvkof2k4se = {
	"kof2k4se", "kof2002", "neogeo", NULL, "2004",
	"The King of Fighters Special Edition 2004 (The King of Fighters 2002 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2k4seRomInfo, kof2k4seRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2k4seInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 10th Anniversary (The King of Fighters 2002 bootleg)

static struct BurnRomInfo kof10thRomDesc[] = {
	{ "kf10-p1.bin",  0x800000, 0xb1fd0c43, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "kf10-c1a.bin", 0x400000, 0x3bbc0364, 3 | BRF_GRA },           //  1 Sprite data
	{ "kf10-c2a.bin", 0x400000, 0x91230075, 3 | BRF_GRA },           //  2 
	{ "kf10-c1b.bin", 0x400000, 0xb5abfc28, 3 | BRF_GRA },           //  3 
	{ "kf10-c2b.bin", 0x400000, 0x6cc4c6e1, 3 | BRF_GRA },           //  4 
	{ "kf10-c3a.bin", 0x400000, 0x5b3d4a16, 3 | BRF_GRA },           //  5 
	{ "kf10-c4a.bin", 0x400000, 0xc6f3419b, 3 | BRF_GRA },           //  6 
	{ "kf10-c3b.bin", 0x400000, 0x9d2bba19, 3 | BRF_GRA },           //  7 
	{ "kf10-c4b.bin", 0x400000, 0x5a4050cb, 3 | BRF_GRA },           //  8 
	{ "kf10-c5a.bin", 0x400000, 0xa289d1e1, 3 | BRF_GRA },           //  9 
	{ "kf10-c6a.bin", 0x400000, 0xe6494b5d, 3 | BRF_GRA },           // 10 
	{ "kf10-c5b.bin", 0x400000, 0x404fff02, 3 | BRF_GRA },           // 11 
	{ "kf10-c6b.bin", 0x400000, 0xf2ccfc9e, 3 | BRF_GRA },           // 12 
	{ "kf10-c7a.bin", 0x400000, 0xbe79c5a8, 3 | BRF_GRA },           // 13 
	{ "kf10-c8a.bin", 0x400000, 0xa5952ca4, 3 | BRF_GRA },           // 14 
	{ "kf10-c7b.bin", 0x400000, 0x3fdb3542, 3 | BRF_GRA },           // 15 
	{ "kf10-c8b.bin", 0x400000, 0x661b7a52, 3 | BRF_GRA },           // 16 

	{ "kf10-m1.bin",  0x020000, 0xf6fab859, 4 | BRF_ESS | BRF_PRG }, // 17 Z80 code

	{ "kf10-v1.bin",  0x800000, 0x0fc9a58d, 5 | BRF_SND },           // 18 Sound data
	{ "kf10-v2.bin",  0x800000, 0xb8c475a4, 5 | BRF_SND },           // 19 
};

STDROMPICKEXT(kof10th, kof10th, neogeo)
STD_ROM_FN(kof10th)

static unsigned char* kof10thExtraRAMA,  *kof10thExtraRAMB;

void kof10thBankswitch(unsigned int nBank)
{
	nBank = 0x100000 + ((nBank & 7) << 20);
	if (nBank >= 0x700000) {
		nBank = 0x100000;
	}

	if (nBank != nNeo68KROMBank) {
		nNeo68KROMBank = nBank;
		SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fdfff, SM_ROM);
	}
}

// Text data extraction, game does this on the fly!
void __fastcall kof10thWriteWordCustom(unsigned int sekAddress, unsigned short wordValue)
{
	if (kof10thExtraRAMB[0x1ffc]) {
		NeoUpdateTextOne(0x20000 + ((sekAddress >> 1) & 0x1ffff), BITSWAP08(wordValue, 3, 2, 1, 5, 7, 6, 0, 4));
	} else {
		*(unsigned short*)(kof10thExtraRAMA + (sekAddress & 0x1fffe)) = wordValue;
	}
}

// 68K Bankswitch and RAM Handlers
void __fastcall kof10thWriteByteBankswitch(unsigned int sekAddress, unsigned char byteValue)
{
	sekAddress ^= 1;
	if (sekAddress == 0x2ffff0)
		kof10thBankswitch(byteValue);

	kof10thExtraRAMB[sekAddress & 0x1fff] = byteValue;
}

void __fastcall kof10thWriteWordBankswitch(unsigned int sekAddress, unsigned short wordValue)
{
	if (sekAddress == 0x2ffff0)
		kof10thBankswitch(wordValue);
	else if (sekAddress == 0x2ffff8 && *(unsigned short*)(kof10thExtraRAMB + 0x1ff8) != wordValue)
		SekMapMemory(Neo68KROM + ((wordValue & 1) ? 0x710000 : 0x010000) , 0x010000, 0x0dffff, SM_ROM);

	*(unsigned short*)(kof10thExtraRAMB + (sekAddress & 0x01ffe)) = wordValue;
}

static void kof10thCallback()
{
	int i, j, k;
	unsigned char *dst = (unsigned char*)malloc(0x100000);
	if (dst)
	{
		for (i = 0; i < 0x800000; i+=0x100000)
		{
			for (j = 0; j < 0x100000; j++)
			{
				k = BITSWAP24(j,23,22,21,20,19,18,17,16,15,14,13,12,11,2,9,8,7,1,5,4,3,10,6,0);
				dst[k] = Neo68KROM[i + j];
			}
			memcpy (Neo68KROM + i, dst, 0x100000);
		}

		memcpy (Neo68KROM + 0x100000, Neo68KROM, 0x700000);
		memcpy (Neo68KROM, dst, 0x100000);
		free (dst);
	}

	// Altera protection chip patches these over P ROM
	*((unsigned short *)(Neo68KROM + 0x0124)) = 0x000d; // Enables XOR for RAM moves, forces SoftDIPs, and USA region
	*((unsigned short *)(Neo68KROM + 0x0126)) = 0xf7a8;

	*((unsigned short *)(Neo68KROM + 0x8bf4)) = 0x4ef9; // Run code to change "S" data
	*((unsigned short *)(Neo68KROM + 0x8bf6)) = 0x000d;
	*((unsigned short *)(Neo68KROM + 0x8bf8)) = 0xf980;
}

void kof10thMapBank()
{
	SekMapMemory(Neo68KROM + nNeo68KROMBank, 0x200000, 0x2fdfff, SM_ROM);
}

int kof10thScan(int nAction, int* pnMin)
{
	struct BurnArea ba;
	
	if (pnMin) {												// Return minimum compatible version
		*pnMin =  0x029523;
	}

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= kof10thExtraRAMA;
		ba.nLen		= 0x00020000;
		ba.nAddress	= 0;
		ba.szName	= "Extra RAM A";
		BurnAcb(&ba);

		ba.Data		= kof10thExtraRAMB;
		ba.nLen		= 0x00002000;
		ba.nAddress	= 0;
		ba.szName	= "Extra RAM B";
		BurnAcb(&ba);
	}
	return 0;
}

static int kof10thInit()
{
	int nRet;

	pNeoInitCallback = kof10thCallback;
	pNeoBankswitchCallback = kof10thMapBank;
	pNeoScanCallback = kof10thScan;

	nRet = NeoInit();

	if (nRet == 0) {
		kof10thExtraRAMA = Neo68KROM + 0x7e0000;
		kof10thExtraRAMB = Neo68KROM + 0x1fe000;
		memset(kof10thExtraRAMA, 0, 0x20000);
		memset(kof10thExtraRAMB, 0, 0x02000);

		SekOpen(0);

		// Install RAM handlers
		SekMapMemory(kof10thExtraRAMA, 0x0e0000, 0x0fffff, SM_ROM); // Text RAM on range 0x0e0000 - 0x0fffff
		SekMapMemory(kof10thExtraRAMB, 0x2fe000, 0x2fffff, SM_ROM); // Extra 68K RAM

		// Install bankswitch and text handlers
		SekMapHandler(4,	0x2fe000, 0x2fffff, SM_WRITE);
		SekSetWriteWordHandler(4, kof10thWriteWordBankswitch);
		SekSetWriteByteHandler(4, kof10thWriteByteBankswitch);

		SekMapHandler(5,	0x200000, 0x23ffff, SM_WRITE);
		SekSetWriteWordHandler(5,     kof10thWriteWordCustom);

		// Set bank
		nNeo68KROMBank = 1;
		kof10thBankswitch(0);

		SekClose();
	}

	return nRet;
}

struct BurnDriver BurnDrvkof10th = {
	"kof10th", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 10th Anniversary (The King of Fighters 2002 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof10thRomInfo, kof10thRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof10thInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 10th Anniversary 2005 Unique (The King of Fighters 2002 bootleg)

static struct BurnRomInfo kf2k5uniRomDesc[] = {
	{ "5006-p2a.bin", 0x400000, 0xced883a2, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "5006-p1.bin",  0x400000, 0x72c39c46, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "5006-s1.bin",  0x020000, 0x91f8c544, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "kf10-c1a.bin", 0x400000, 0x3bbc0364, 3 | BRF_GRA },           //  3 Sprite data
	{ "kf10-c2a.bin", 0x400000, 0x91230075, 3 | BRF_GRA },           //  4 
	{ "kf10-c1b.bin", 0x400000, 0xb5abfc28, 3 | BRF_GRA },           //  5 
	{ "kf10-c2b.bin", 0x400000, 0x6cc4c6e1, 3 | BRF_GRA },           //  6 
	{ "kf10-c3a.bin", 0x400000, 0x5b3d4a16, 3 | BRF_GRA },           //  7 
	{ "kf10-c4a.bin", 0x400000, 0xc6f3419b, 3 | BRF_GRA },           //  8 
	{ "kf10-c3b.bin", 0x400000, 0x9d2bba19, 3 | BRF_GRA },           //  9 
	{ "kf10-c4b.bin", 0x400000, 0x5a4050cb, 3 | BRF_GRA },           // 10 
	{ "kf10-c5a.bin", 0x400000, 0xa289d1e1, 3 | BRF_GRA },           // 11 
	{ "kf10-c6a.bin", 0x400000, 0xe6494b5d, 3 | BRF_GRA },           // 12 
	{ "kf10-c5b.bin", 0x400000, 0x404fff02, 3 | BRF_GRA },           // 13 
	{ "kf10-c6b.bin", 0x400000, 0xf2ccfc9e, 3 | BRF_GRA },           // 14 
	{ "kf10-c7a.bin", 0x400000, 0xbe79c5a8, 3 | BRF_GRA },           // 15 
	{ "kf10-c8a.bin", 0x400000, 0xa5952ca4, 3 | BRF_GRA },           // 16 
	{ "kf10-c7b.bin", 0x400000, 0x3fdb3542, 3 | BRF_GRA },           // 17 
	{ "kf10-c8b.bin", 0x400000, 0x661b7a52, 3 | BRF_GRA },           // 18 

	{ "5006-m1.bin",  0x020000, 0x9050bfe7, 4 | BRF_ESS | BRF_PRG }, // 19 Z80 code

	{ "kf10-v1.bin",  0x800000, 0x0fc9a58d, 5 | BRF_SND },           // 20 Sound data
	{ "kf10-v2.bin",  0x800000, 0xb8c475a4, 5 | BRF_SND },           // 21 
};

STDROMPICKEXT(kf2k5uni, kf2k5uni, neogeo)
STD_ROM_FN(kf2k5uni)

static void kf2k5uniCallback()
{
	int i, j, k;
	unsigned char dst[0x80];

	for (i = 0; i < 0x800000; i+=0x80)
	{
		for (j = 0; j < 0x80; j++)
		{
			k = BITSWAP08(j, 7, 3, 4, 5, 6, 1, 2, 0);
			dst[j] = Neo68KROM[i + k];
		}
		memcpy (Neo68KROM + i, dst, 0x80);
	}

	memcpy(Neo68KROM, Neo68KROM + 0x600000, 0x100000);

	for (i = 0; i < 0x30000; i++)
		NeoZ80ROM[i] = BITSWAP08(NeoZ80ROM[i], 4, 5, 6, 7, 0, 1, 2, 3);

	for (i = 0; i < 0x20000; i++)
		NeoTextROM[0x20000 + i] = BITSWAP08(NeoTextROM[0x20000 + i], 4, 5, 6, 7, 0, 1, 2, 3);
}

static int kf2k5uniInit()
{
	pNeoInitCallback = kf2k5uniCallback;

	return NeoInit();
}

struct BurnDriver BurnDrvkf2k5uni = {
	"kf2k5uni", "kof2002", "neogeo", NULL, "2004",
	"The King of Fighters 10th Anniversary 2005 Unique (The King of Fighters 2002 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k5uniRomInfo, kf2k5uniRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k5uniInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg)

static struct BurnRomInfo kf10thepRomDesc[] = {
	{ "5008-p1.bin",  0x200000, 0xbf5469ba, 1 | BRF_PRG | BRF_OPT }, //  0 68K code
	{ "5008-p2.bin",  0x400000, 0xa649ec38, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "5008-p3.bin",  0x200000, 0xe629e13c, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "5008-s1.bin",  0x020000, 0x92410064, 2 | BRF_GRA },           //  3 Text layer tiles

	{ "kf10-c1a.bin", 0x400000, 0x3bbc0364, 3 | BRF_GRA },           //  4 Sprite data
	{ "kf10-c2a.bin", 0x400000, 0x91230075, 3 | BRF_GRA },           //  5 
	{ "kf10-c1b.bin", 0x400000, 0xb5abfc28, 3 | BRF_GRA },           //  6 
	{ "kf10-c2b.bin", 0x400000, 0x6cc4c6e1, 3 | BRF_GRA },           //  7 
	{ "kf10-c3a.bin", 0x400000, 0x5b3d4a16, 3 | BRF_GRA },           //  8 
	{ "kf10-c4a.bin", 0x400000, 0xc6f3419b, 3 | BRF_GRA },           //  9 
	{ "kf10-c3b.bin", 0x400000, 0x9d2bba19, 3 | BRF_GRA },           // 10 
	{ "kf10-c4b.bin", 0x400000, 0x5a4050cb, 3 | BRF_GRA },           // 11 
	{ "kf10-c5a.bin", 0x400000, 0xa289d1e1, 3 | BRF_GRA },           // 12 
	{ "kf10-c6a.bin", 0x400000, 0xe6494b5d, 3 | BRF_GRA },           // 13 
	{ "kf10-c5b.bin", 0x400000, 0x404fff02, 3 | BRF_GRA },           // 14 
	{ "kf10-c6b.bin", 0x400000, 0xf2ccfc9e, 3 | BRF_GRA },           // 15 
	{ "kf10-c7a.bin", 0x400000, 0xbe79c5a8, 3 | BRF_GRA },           // 16 
	{ "kf10-c8a.bin", 0x400000, 0xa5952ca4, 3 | BRF_GRA },           // 17 
	{ "5008-c7b.bin", 0x400000, 0x33604ef0, 3 | BRF_GRA },           // 18 
	{ "5008-c8b.bin", 0x400000, 0x51f6a8f8, 3 | BRF_GRA },           // 19 

	{ "5008-m1.bin",  0x020000, 0x5a47d9ad, 4 | BRF_ESS | BRF_PRG }, // 20 Z80 code

	{ "kf10-v1.bin",  0x800000, 0x0fc9a58d, 5 | BRF_SND },           // 21 Sound data
	{ "kf10-v2.bin",  0x800000, 0xb8c475a4, 5 | BRF_SND },           // 22 
};

STDROMPICKEXT(kf10thep, kf10thep, neogeo)
STD_ROM_FN(kf10thep)

static void kf10thepCallback()
{
	int i;
	unsigned char *dst = (unsigned char*)malloc(0x100000);
	if (dst)
	{
		unsigned int sec[8] = { 0x3, 0x8, 0x7, 0xC, 0x1, 0xA, 0x6, 0xD };

		for (i = 0; i < 8; i++)
			memcpy (dst + i * 0x20000, Neo68KROM + sec[i] * 0x20000, 0x20000);

		memcpy (dst + 0x0002e0, Neo68KROM + 0x0402e0, 0x06a);
		memcpy (dst + 0x0f92bc, Neo68KROM + 0x0492bc, 0xb9e);
		memcpy (Neo68KROM, dst, 0x100000);
		free (dst);
	}

	memcpy (Neo68KROM + 0x100000, Neo68KROM + 0x200000, 0x600000);

	for (i = 0xf92bc; i < 0xf9e58; i += 2)
	{
		if ((*((unsigned short*)(Neo68KROM + i + 0)) & 0xffbf) == 0x4eb9 && *((unsigned short*)(Neo68KROM + i + 2)) == 0x0000)
		{
			*((unsigned short*)(Neo68KROM + i + 2)) = 0x000f;
 		}
	}

	*((unsigned short *)(Neo68KROM + 0x00342)) = 0x000f;

	lans2004_sx_decode();
}

static int kf10thepInit()
{
	pNeoInitCallback = kf10thepCallback;

	return NeoInit();
}

struct BurnDriver BurnDrvkf10thep = {
	"kf10thep", "kof2002", "neogeo", NULL, "2005",
	"The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kf10thepRomInfo, kf10thepRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf10thepInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 6 (Metal Slug 3 bootleg)

static struct BurnRomInfo mslug3b6RomDesc[] = {
	{ "299-p1.bin",   0x200000, 0x5f2fe228, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "299-p2.bin",   0x400000, 0x193fa835, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "299-s1.bin",   0x020000, 0x6f8b9635, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "256-c1.c1",    0x800000, 0x5a79c34e, 3 | BRF_GRA },           //  3 Sprite data
	{ "256-c2.c2",    0x800000, 0x944c362c, 3 | BRF_GRA },           //  4 
	{ "256-c3.c3",    0x800000, 0x6e69d36f, 3 | BRF_GRA },           //  5 
	{ "256-c4.c4",    0x800000, 0xb755b4eb, 3 | BRF_GRA },           //  6 
	{ "256-c5.c5",    0x800000, 0x7aacab47, 3 | BRF_GRA },           //  7 
	{ "256-c6.c6",    0x800000, 0xc698fd5d, 3 | BRF_GRA },           //  8 
	{ "256-c7.c7",    0x800000, 0xcfceddd2, 3 | BRF_GRA },           //  9 
	{ "256-c8.c8",    0x800000, 0x4d9be34c, 3 | BRF_GRA },           // 10 

	{ "256-m1.m1",    0x080000, 0xeaeec116, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "256-v1.v1",    0x400000, 0xf2690241, 5 | BRF_SND },           // 12 Sound data
	{ "256-v2.v2",    0x400000, 0x7e2a10bd, 5 | BRF_SND },           // 13 
	{ "256-v3.v3",    0x400000, 0x0eaec17c, 5 | BRF_SND },           // 14 
	{ "256-v4.v4",    0x400000, 0x9b4b22d4, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(mslug3b6, mslug3b6, neogeo)
STD_ROM_FN(mslug3b6)

static void mslug3b6Callback()
{
	memcpy(Neo68KROM, Neo68KROM + 0x100000, 0x500000);

	garoubl_sx_decode();
}

static int mslug3b6Init()
{
	pNeoInitCallback = mslug3b6Callback;
	nNeoProtectionXor = 0xAD;

 	return NeoInit();
}

struct BurnDriver BurnDrvmslug3b6 = {
	"mslug3b6", "mslug3", "neogeo", NULL, "2000",
	"Metal Slug 6 (Metal Slug 3 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_A, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug3b6RomInfo, mslug3b6RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug3b6Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Digger Man

static struct BurnRomInfo diggermaRomDesc[] = {
	{ "dig-p1.bin",   0x080000, 0xeda433d7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "dig-s1.bin",   0x010000, 0x75a88c1f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "dig-c1.bin",   0x080000, 0x3db0a4ed, 3 | BRF_GRA },           //  2 Sprite data
	{ "dig-c2.bin",   0x080000, 0x3e632161, 3 | BRF_GRA },           //  3 

	{ "dig-m1.bin",   0x010000, 0x833cdf1b, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "dig-v1.bin",   0x080000, 0xee15bda4, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(diggerma, diggerma, neogeo)
STD_ROM_FN(diggerma)

struct BurnDriver BurnDrvdiggerma = {
	"diggerma", NULL, "neogeo", NULL, "2000",
	"Digger Man\0", NULL, "Kyle Hodgetts", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_MAZE, 0,
	NULL, diggermaRomInfo, diggermaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// -----------------------------------------------------------------------------
// Pre-decrypted ROMsets

// The King of Fighters '99 - Millennium Battle (decrypted C)

static struct BurnRomInfo kof99ndRomDesc[] = {
	{ "152-p1.bin",   0x100000, 0xf2c7ddfa, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "152-p2.bin",   0x400000, 0x274ef47a, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "251-c1_decrypted.bin", 0x800000, 0xb3d88546, 3 | BRF_GRA },		 //  2 Sprite data
	{ "251-c2_decrypted.bin", 0x800000, 0x915c8634, 3 | BRF_GRA },		 //  3
	{ "251-c3_decrypted.bin", 0x800000, 0xb047c9d5, 3 | BRF_GRA },		 //  4
	{ "251-c4_decrypted.bin", 0x800000, 0x6bc8e4b1, 3 | BRF_GRA },		 //  5
	{ "251-c5_decrypted.bin", 0x800000, 0x9746268c, 3 | BRF_GRA },		 //  6
	{ "251-c6_decrypted.bin", 0x800000, 0x238b3e71, 3 | BRF_GRA },		 //  7
	{ "251-c7_decrypted.bin", 0x800000, 0x2f68fdeb, 3 | BRF_GRA },		 //  8
	{ "251-c8_decrypted.bin", 0x800000, 0x4c2fad1e, 3 | BRF_GRA },		 //  9 

	{ "251-m1.bin",   0x020000, 0x5e74539c, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "251-v1.bin",   0x400000, 0xef2eecc8, 5 | BRF_SND },           // 11 Sound data
	{ "251-v2.bin",   0x400000, 0x73e211ca, 5 | BRF_SND },           // 12 
	{ "251-v3.bin",   0x400000, 0x821901da, 5 | BRF_SND },           // 13 
	{ "251-v4.bin",   0x200000, 0xb49e6178, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kof99nd, kof99nd, neogeo)
STD_ROM_FN(kof99nd)

struct BurnDriverX BurnDrvkof99nd = {
	"kof99nd", "kof99", "neogeo", NULL, "1999",
	"The King of Fighters '99 - Millennium Battle (decrypted C)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof99ndRomInfo, kof99ndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Ganryu / Musashi ganryunki (decrypted C)

static struct BurnRomInfo ganryunRomDesc[] = {
	{ "252-p1.bin",   0x200000, 0x4b8ac4fb, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "252-c1_decrypted.bin", 0x800000, 0x6bf7605b, 3 | BRF_GRA },           //  1 Sprite data
	{ "252-c2_decrypted.bin", 0x800000, 0x596792ce, 3 | BRF_GRA },           //  2 

	{ "252-m1.bin",   0x020000, 0x30cc4099, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "252-v1.bin",   0x400000, 0xe5946733, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(ganryun, ganryun, neogeo)
STD_ROM_FN(ganryun)

struct BurnDriverX BurnDrvganryun = {
	"ganryun", "ganryu", "neogeo", NULL, "1999",
	"Ganryu / Musashi Ganryuki (decrypted C)\0", NULL, "Visco", "Neo Geo",
	L"Ganryu\0\u6B66\u8535\u5DCC\u6D41\u8A18 (decrypted C)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_PLATFORM, 0,
	NULL, ganryunRomInfo, ganryunRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Garou - Mark of the Wolves (decrypted C)

static struct BurnRomInfo garounRomDesc[] = {
	{ "253-sma.kf",   0x040000, 0x98bc93dc, 9 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "253-ep1.p1",   0x200000, 0xea3171a4, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "253-ep2.p2",   0x200000, 0x382f704b, 1 | BRF_ESS | BRF_PRG }, //  2 
	{ "253-ep3.p3",   0x200000, 0xe395bfdd, 1 | BRF_ESS | BRF_PRG }, //  3 
	{ "253-ep4.p4",   0x200000, 0xda92c08e, 1 | BRF_ESS | BRF_PRG }, //  4 

	{ "253-c1_decrypted.bin", 0x800000, 0x497be3f2, 3 | BRF_GRA },           //  5 Sprite data
	{ "253-c2_decrypted.bin", 0x800000, 0x6a9e95ca, 3 | BRF_GRA },           //  6
	{ "253-c3_decrypted.bin", 0x800000, 0x39373d2f, 3 | BRF_GRA },           //  7
	{ "253-c4_decrypted.bin", 0x800000, 0x4de23f6c, 3 | BRF_GRA },           //  8
	{ "253-c5_decrypted.bin", 0x800000, 0x16634ba5, 3 | BRF_GRA },           //  9
	{ "253-c6_decrypted.bin", 0x800000, 0x95671ffd, 3 | BRF_GRA },           // 10
	{ "253-c7_decrypted.bin", 0x800000, 0xe36ce77f, 3 | BRF_GRA },           // 11
	{ "253-c8_decrypted.bin", 0x800000, 0xddbd1096, 3 | BRF_GRA },           // 12

	{ "253-m1.bin",   0x040000, 0x36a806be, 4 | BRF_ESS | BRF_PRG }, // 13 Z80 code

	{ "253-v1.bin",   0x400000, 0x263e388c, 5 | BRF_SND },           // 14 Sound data
	{ "253-v2.bin",   0x400000, 0x2c6bc7be, 5 | BRF_SND },           // 15 
	{ "253-v3.bin",   0x400000, 0x0425b27d, 5 | BRF_SND },           // 16 
	{ "253-v4.bin",   0x400000, 0xa54be8a9, 5 | BRF_SND },           // 17 
};

STDROMPICKEXT(garoun, garoun, neogeo)
STD_ROM_FN(garoun)

struct BurnDriverX BurnDrvgaroun = {
	"garoun", "garou", "neogeo", NULL, "1999",
	"Garou - Mark of the Wolves (decrypted C)\0", NULL, "SNK", "Neo Geo",
	L"\u9913\u72FC - mark of the wolves (set 1)\0Garou - mark of the wolves (decrypted C)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SMA_PROTECTION, GBF_VSFIGHT, 0,
	NULL, garounRomInfo, garounRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	garouInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Strikers 1945 Plus (decrypted C)

static struct BurnRomInfo s1945pnRomDesc[] = {
	{ "254-p1.bin",   0x100000, 0xff8efcff, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "254-p2.bin",   0x400000, 0xefdfd4dd, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "254-c1_decrypted.bin", 0x800000, 0x7b6902f9, 3 | BRF_GRA },		 //  2 Sprite data
	{ "254-c2_decrypted.bin", 0x800000, 0x51bd4252, 3 | BRF_GRA },		 //  3
	{ "254-c3_decrypted.bin", 0x800000, 0xa38993e4, 3 | BRF_GRA },		 //  4
	{ "254-c4_decrypted.bin", 0x800000, 0xd5696530, 3 | BRF_GRA },		 //  5
	{ "254-c5_decrypted.bin", 0x800000, 0x28764bd6, 3 | BRF_GRA },		 //  6
	{ "254-c6_decrypted.bin", 0x800000, 0x9931bdf1, 3 | BRF_GRA },		 //  7
	{ "254-c7_decrypted.bin", 0x800000, 0x6e12afcd, 3 | BRF_GRA },		 //  8
	{ "254-c8_decrypted.bin", 0x800000, 0x178d4684, 3 | BRF_GRA },		 //  9 

	{ "254-m1.bin",   0x020000, 0x994b4487, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "254-v1.bin",   0x400000, 0x844f58fb, 5 | BRF_SND },           // 11 Sound data
	{ "254-v2.bin",   0x400000, 0xd9a248f0, 5 | BRF_SND },           // 12 
	{ "254-v3.bin",   0x400000, 0x0b0d2d33, 5 | BRF_SND },           // 13 
	{ "254-v4.bin",   0x400000, 0x6d13dc91, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(s1945pn, s1945pn, neogeo)
STD_ROM_FN(s1945pn)

struct BurnDriverX BurnDrvs1945pn = {
	"s1945pn", "s1945p", "neogeo", NULL, "1999",
	"Strikers 1945 Plus (decrypted C)\0", NULL, "Psikyo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VERSHOOT, 0,
	NULL, s1945pnRomInfo, s1945pnRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Prehistoric Isle 2 (decrypted C)

static struct BurnRomInfo preisl2nRomDesc[] = {
	{ "255-p1.bin",   0x100000, 0xdfa3c0f3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "255-p2.bin",   0x400000, 0x42050b80, 1 | BRF_ESS | BRF_PRG }, //  1 
 
	{ "255-c1_decrypted.bin",  0x800000, 0x50fd785e, 3 | BRF_GRA },		 //  2 Sprite data
	{ "255-c2_decrypted.bin",  0x800000, 0xab913f1e, 3 | BRF_GRA },		 //  3
	{ "255-c3_decrypted.bin",  0x800000, 0xbc0ee75c, 3 | BRF_GRA },		 //  4
	{ "255-c4_decrypted.bin",  0x800000, 0x29908823, 3 | BRF_GRA },		 //  5
	{ "255-c5_decrypted.bin",  0x800000, 0x83c56bca, 3 | BRF_GRA },		 //  6
	{ "255-c6_decrypted.bin",  0x800000, 0x59e0e805, 3 | BRF_GRA },		 //  7

	{ "255-m1.bin",   0x020000, 0x8efd4014, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "255-v1.bin",   0x400000, 0x5a14543d, 5 | BRF_SND },           //  9 Sound data
	{ "255-v2.bin",   0x200000, 0x6610d91a, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(preisl2n, preisl2n, neogeo)
STD_ROM_FN(preisl2n)

struct BurnDriverX BurnDrvpreisl2n = {
	"preisl2n", "preisle2", "neogeo", NULL, "1999",
	"Prehistoric Isle 2 (decrypted C)\0", NULL, "Yumekobo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_HORSHOOT, 0,
	NULL, preisl2nRomInfo, preisl2nRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Metal Slug 3 (not encrypted)

static struct BurnRomInfo mslug3hdRomDesc[] = {
	{ "256-ph1.bin",  0x100000, 0x9c42ca85, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "256-ph2.bin",  0x400000, 0x1f3d8ce8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "256-c1_decrypted.bin",  0x800000, 0x3540398c, 3 | BRF_GRA },		 //  2 Sprite data
	{ "256-c2_decrypted.bin",  0x800000, 0xbdd220f0, 3 | BRF_GRA },		 //  3
	{ "256-c3_decrypted.bin",  0x800000, 0xbfaade82, 3 | BRF_GRA },		 //  4
	{ "256-c4_decrypted.bin",  0x800000, 0x1463add6, 3 | BRF_GRA },		 //  5
	{ "256-c5_decrypted.bin",  0x800000, 0x48ca7f28, 3 | BRF_GRA },		 //  6
	{ "256-c6_decrypted.bin",  0x800000, 0x806eb36f, 3 | BRF_GRA },		 //  7
	{ "256-c7_decrypted.bin",  0x800000, 0x9395b809, 3 | BRF_GRA },		 //  8
	{ "256-c8_decrypted.bin",  0x800000, 0xa369f9d4, 3 | BRF_GRA },		 //  9

	{ "256-m1.bin",   0x080000, 0xeaeec116, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "256-v1.bin",   0x400000, 0xf2690241, 5 | BRF_SND },           // 11 Sound data
	{ "256-v2.bin",   0x400000, 0x7e2a10bd, 5 | BRF_SND },           // 12 
	{ "256-v3.bin",   0x400000, 0x0eaec17c, 5 | BRF_SND },           // 13 
	{ "256-v4.bin",   0x400000, 0x9b4b22d4, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(mslug3hd, mslug3hd, neogeo)
STD_ROM_FN(mslug3hd)

struct BurnDriverX BurnDrvmslug3hd = {
	"mslug3hd", "mslug3", "neogeo", NULL, "2000",
	"Metal Slug 3 (not encrypted, decrpyted C)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug3hdRomInfo, mslug3hdRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug3hInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// The King of Fighters 2000 (non encrypted P, decrypted C)

static struct BurnRomInfo kof2kndRomDesc[] = {
	{ "257-p1n.bin",  0x100000, 0x5f809dbe, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "257-p2n.bin",  0x400000, 0x693c2c5e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "257-c1_decrypted.bin", 0x800000, 0xabcdd424, 3 | BRF_GRA },		 //  2 Sprite data
	{ "257-c2_decrypted.bin", 0x800000, 0xcda33778, 3 | BRF_GRA },		 //  3
	{ "257-c3_decrypted.bin", 0x800000, 0x087fb15b, 3 | BRF_GRA },		 //  4
	{ "257-c4_decrypted.bin", 0x800000, 0xfe9dfde4, 3 | BRF_GRA },		 //  5
	{ "257-c5_decrypted.bin", 0x800000, 0x03ee4bf4, 3 | BRF_GRA },		 //  6
	{ "257-c6_decrypted.bin", 0x800000, 0x8599cc5b, 3 | BRF_GRA },		 //  7
	{ "257-c7_decrypted.bin", 0x800000, 0x71dfc3e2, 3 | BRF_GRA },		 //  8
	{ "257-c8_decrypted.bin", 0x800000, 0x0fa30e5f, 3 | BRF_GRA },		 //  9
 
	{ "257-m1.bin",   0x040000, 0x4b749113, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "257-v1.bin",   0x400000, 0x17cde847, 5 | BRF_SND },           // 11 Sound data
	{ "257-v2.bin",   0x400000, 0x1afb20ff, 5 | BRF_SND },           // 12 
	{ "257-v3.bin",   0x400000, 0x4605036a, 5 | BRF_SND },           // 13 
	{ "257-v4.bin",   0x400000, 0x764bbd6b, 5 | BRF_SND },           // 14 
};

STDROMPICKEXT(kof2knd, kof2knd, neogeo)
STD_ROM_FN(kof2knd)

struct BurnDriverX BurnDrvkof2knd = {
	"kof2knd", "kof2000", "neogeo", NULL, "2000",
	"The King of Fighters 2000 (non encrypted P, decrypted C)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2kndRomInfo, kof2kndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2000nInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Bang Bead (decrypted C)

static struct BurnRomInfo bngbeadnRomDesc[] = {
	{ "259-p1.bin",   0x200000, 0x88a37f8b, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "259-c1_decrypted.bin",   0x800000, 0xc50a91c3, 3 | BRF_GRA },		 //  1 Sprite data
	{ "259-c2_decrypted.bin",   0x800000, 0x820ab36e, 3 | BRF_GRA },		 //  2

	{ "259-m1.bin",   0x020000, 0x85668ee9, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "259-v1.bin",   0x400000, 0x088eb8ab, 5 | BRF_SND },           //  4 Sound data
	{ "259-v2.bin",   0x100000, 0x97528fe9, 5 | BRF_SND },           //  5 
};

STDROMPICKEXT(bngbeadn, bngbeadn, neogeo)
STD_ROM_FN(bngbeadn)

struct BurnDriverX BurnDrvbngbeadn = {
	"bngbeadn", "bangbead", "neogeo", NULL, "2000",
	"Bang Bead (decrypted C)\0", NULL, "Visco", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_BALLPADDLE, 0,
	NULL, bngbeadnRomInfo, bngbeadnRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Nightmare in the Dark (decrypted C)

static struct BurnRomInfo nitdnRomDesc[] = {
	{ "260-p1.bin",   0x080000, 0x61361082, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "260-c1_decrypted.bin", 0x800000, 0xf3ff4953, 3 | BRF_GRA },		 //  1 Sprite data
	{ "260-c2_decrypted.bin", 0x800000, 0xf1e49faa, 3 | BRF_GRA },		 //  2

	{ "260-m1.bin",   0x080000, 0x6407c5e5, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "260-v1.bin",   0x400000, 0x24b0480c, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(nitdn, nitdn, neogeo)
STD_ROM_FN(nitdn)

struct BurnDriverX BurnDrvnitdn = {
	"nitdn", "nitd", "neogeo", NULL, "2000",
	"Nightmare in the Dark (decrypted C)\0", NULL, "Eleven / Gavaking", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, nitdnRomInfo, nitdnRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	nitdInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};

// Zupapa! (decrypted C)

static struct BurnRomInfo zupapanRomDesc[] = {
	{ "070-p1.bin",   0x100000, 0x5a96203e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "070-c1_decrypted.bin", 0x800000, 0x65d73348, 3 | BRF_GRA },		 //  1 Sprite data
	{ "070-c2_decrypted.bin", 0x800000, 0xc498708f, 3 | BRF_GRA },		 //  2

	{ "070-m1.bin",   0x020000, 0x5a3b3191, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "070-v1.bin",   0x200000, 0xd3a7e1ff, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(zupapan, zupapan, neogeo)
STD_ROM_FN(zupapan)

struct BurnDriverX BurnDrvzupapan = {
	"zupapan", "zupapa", "neogeo", NULL, "2001",
	"Zupapa! (decrypted C)\0", NULL, "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PLATFORM, 0,
	NULL, zupapanRomInfo, zupapanRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Sengoku 3 (decrypted C)

static struct BurnRomInfo sngoku3nRomDesc[] = {
	{ "261-p1.bin",   0x200000, 0xe0d4bc0a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "261-c1_decrypted.bin", 0x800000, 0x9af7cbca, 3 | BRF_GRA },		 //  1 Sprite data
	{ "261-c2_decrypted.bin", 0x800000, 0x2a1f874d, 3 | BRF_GRA },		 //  2
	{ "261-c3_decrypted.bin", 0x800000, 0x5403adb5, 3 | BRF_GRA },		 //  3
	{ "261-c4_decrypted.bin", 0x800000, 0x18926df6, 3 | BRF_GRA },		 //  4

	{ "261-m1.bin",   0x080000, 0x7d501c39, 4 | BRF_ESS | BRF_PRG }, //  5 Z80 code

	{ "261-v1.bin",   0x400000, 0x64c30081, 5 | BRF_SND },           //  6 Sound data
	{ "261-v2.bin",   0x400000, 0x392a9c47, 5 | BRF_SND },           //  7 
	{ "261-v3.bin",   0x400000, 0xc1a7ebe3, 5 | BRF_SND },           //  8 
	{ "261-v4.bin",   0x200000, 0x9000d085, 5 | BRF_SND },           //  9 
};

STDROMPICKEXT(sngoku3n, sngoku3n, neogeo)
STD_ROM_FN(sngoku3n)

struct BurnDriverX BurnDrvsngoku3n = {
	"sngoku3n", "sengoku3", "neogeo", NULL, "2001",
	"Sengoku 3 (decrypted C)\0", NULL, "SNK", "Neo Geo",
	L"Sengoku 3 (decrypted C)\0\u6226\u56FD\u4F1D\u627F\uFF12\uFF10\uFF10\uFF11 (decrypted C)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_SWAPP, GBF_SCRFIGHT, 0,
	NULL, sngoku3nRomInfo, sngoku3nRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// The King of Fighters 2001 (decrypted C)

static struct BurnRomInfo kof2k1ndRomDesc[] = {
	{ "262-p1.bin",      0x100000, 0x9381750d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "262-p2.bin",      0x400000, 0x8e0d8329, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "262-c1_decrypted.bin",    0x800000, 0x103225b1, 3 | BRF_GRA },	    //  2 Sprite data
	{ "262-c2_decrypted.bin",    0x800000, 0xf9d05d99, 3 | BRF_GRA },	    //  3
	{ "262-c3_decrypted.bin",    0x800000, 0x4c7ec427, 3 | BRF_GRA },	    //  4
	{ "262-c4_decrypted.bin",    0x800000, 0x1d237aa6, 3 | BRF_GRA },	    //  5
	{ "262-c5_decrypted.bin",    0x800000, 0xc2256db5, 3 | BRF_GRA },	    //  6
	{ "262-c6_decrypted.bin",    0x800000, 0x8d6565a9, 3 | BRF_GRA },	    //  7
	{ "262-c7_decrypted.bin",    0x800000, 0xd1408776, 3 | BRF_GRA },	    //  8
	{ "262-c8_decrypted.bin",    0x800000, 0x954d0e16, 3 | BRF_GRA },	    //  9

	{ "265-262-m1.bin",          0x040000, 0xa7f8119f, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "262-v1-08-e0.bin",      0x400000, 0x83d49ecf, 5 | BRF_SND },     // 11 Sound data
	{ "262-v2-08-e0.bin",      0x400000, 0x003f1843, 5 | BRF_SND },     // 12 
	{ "262-v3-08-e0.bin",      0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 13 
	{ "262-v4-08-e0.bin",      0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 14 
};

STDROMPICKEXT(kof2k1nd, kof2k1nd, neogeo)
STD_ROM_FN(kof2k1nd)

struct BurnDriverX BurnDrvkof2k1nd = {
	"kof2k1nd", "kof2001", "neogeo", NULL, "2001",
	"The King of Fighters 2001 (decrypted C)\0", NULL, "Eolith / SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2k1ndRomInfo, kof2k1ndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2001Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Metal Slug 4 (decrypted C)

static struct BurnRomInfo mslug4ndRomDesc[] = {
	{ "263-p1.bin",   0x100000, 0x27e4def3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "263-p2.bin",   0x400000, 0xfdb7aed8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "263-c1_decrypted.bin",  0x800000, 0xa75ffcde, 3 | BRF_GRA },		 //  2 Sprite data
	{ "263-c2_decrypted.bin",  0x800000, 0x5ab0d12b, 3 | BRF_GRA },		 //  3
	{ "263-c3_decrypted.bin",  0x800000, 0x61af560c, 3 | BRF_GRA },		 //  4
	{ "263-c4_decrypted.bin",  0x800000, 0xf2c544fd, 3 | BRF_GRA },		 //  5
	{ "263-c5_decrypted.bin",  0x800000, 0x84c66c44, 3 | BRF_GRA },		 //  6
	{ "263-c6_decrypted.bin",  0x800000, 0x5ed018ab, 3 | BRF_GRA },		 //  7

	{ "263-m1.bin",   0x020000, 0x46ac8228, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "263-v1.bin",   0x800000, 0x01e9b9cd, 5 | BRF_SND },           //  9 Sound data
	{ "263-v2.bin",   0x800000, 0x4ab2bf81, 5 | BRF_SND },           // 10 
};

STDROMPICKEXT(mslug4nd, mslug4nd, neogeo)
STD_ROM_FN(mslug4nd)

struct BurnDriverX BurnDrvmslug4nd = {
	"mslug4nd", "mslug4", "neogeo", NULL, "2002",
	"Metal Slug 4 (decrypted C)\0", NULL, "Mega", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug4ndRomInfo, mslug4ndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug4Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Rage of the Dragons (decrypted C)

static struct BurnRomInfo rotdndRomDesc[] = {
	{ "264-p1.bin",   0x800000, 0xb8cc969d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "264-c1_decrypted.bin",  0x800000, 0xec9d18c0, 3 | BRF_GRA },		 //  1 Sprite data
	{ "264-c2_decrypted.bin",  0x800000, 0xb1069066, 3 | BRF_GRA },		 //  2
	{ "264-c3_decrypted.bin",  0x800000, 0x7e636d49, 3 | BRF_GRA },		 //  3
	{ "264-c4_decrypted.bin",  0x800000, 0x76892fda, 3 | BRF_GRA },		 //  4
	{ "264-c5_decrypted.bin",  0x800000, 0x469061bc, 3 | BRF_GRA },		 //  5
	{ "264-c6_decrypted.bin",  0x800000, 0x2200220a, 3 | BRF_GRA },		 //  6
	{ "264-c7_decrypted.bin",  0x800000, 0xedda4baf, 3 | BRF_GRA },		 //  7
	{ "264-c8_decrypted.bin",  0x800000, 0x82b1ba22, 3 | BRF_GRA },		 //  8

	{ "264-m1.bin",   0x020000, 0x4dbd7b43, 4 | BRF_ESS | BRF_PRG }, // 9 Z80 code

	{ "264-v1.bin",   0x800000, 0xfa005812, 5 | BRF_SND },           // 10 Sound data
	{ "264-v2.bin",   0x800000, 0xc3dc8bf0, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(rotdnd, rotdnd, neogeo)
STD_ROM_FN(rotdnd)

struct BurnDriverX BurnDrvrotdnd = {
	"rotdnd", "rotd", "neogeo", NULL, "2002",
	"Rage of the Dragons (decrypted C)\0", NULL, "Evoga / Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, 0,
	NULL, rotdndRomInfo, rotdndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	rotdInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};

// The King of Fighters 2002 (decrypted C)

static struct BurnRomInfo kof2k2ndRomDesc[] = {
	{ "265-p1.bin",   0x100000, 0x9ede7323, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "265-p2.bin",   0x400000, 0x327266b8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "265-c1_decrypted.bin", 0x800000, 0x7efa6ef7, 3 | BRF_GRA },		 //  2 Sprite data
	{ "265-c2_decrypted.bin", 0x800000, 0xaa82948b, 3 | BRF_GRA },		 //  3
	{ "265-c3_decrypted.bin", 0x800000, 0x959fad0b, 3 | BRF_GRA },		 //  4
	{ "265-c4_decrypted.bin", 0x800000, 0xefe6a468, 3 | BRF_GRA },		 //  5
	{ "265-c5_decrypted.bin", 0x800000, 0x74bba7c6, 3 | BRF_GRA },		 //  6
	{ "265-c6_decrypted.bin", 0x800000, 0xe20d2216, 3 | BRF_GRA },		 //  7
	{ "265-c7_decrypted.bin", 0x800000, 0x8a5b561c, 3 | BRF_GRA },		 //  8
	{ "265-c8_decrypted.bin", 0x800000, 0xbef667a3, 3 | BRF_GRA },		 //  9 

	{ "265-m1.bin",   0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "265-v1.bin",   0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 11 Sound data
	{ "265-v2.bin",   0x800000, 0xda41d6f9, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof2k2nd, kof2k2nd, neogeo)
STD_ROM_FN(kof2k2nd)

struct BurnDriverX BurnDrvkof2k2nd = {
	"kof2k2nd", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 (decrypted C)\0", NULL, "Eolith / Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2k2ndRomInfo, kof2k2ndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2002Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Matrimelee / Shin Gouketsuji Ichizoku Toukon (decrypted C)

static struct BurnRomInfo matrimndRomDesc[] = {
	{ "266-p1.bin",   0x100000, 0x5d4c2dc7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "266-p2.bin",   0x400000, 0xa14b1906, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "266-c1_decrypted.bin",  0x800000, 0x0ab54c54, 3 | BRF_GRA },		 //  2 Sprite data
	{ "266-c2_decrypted.bin",  0x800000, 0xb834dd89, 3 | BRF_GRA },		 //  3
	{ "266-c3_decrypted.bin",  0x800000, 0xf31d68d4, 3 | BRF_GRA },		 //  4
	{ "266-c4_decrypted.bin",  0x800000, 0x58cfb6e9, 3 | BRF_GRA },		 //  5
	{ "266-c5_decrypted.bin",  0x800000, 0xf3c0e3fd, 3 | BRF_GRA },		 //  6
	{ "266-c6_decrypted.bin",  0x800000, 0xb53c8dcf, 3 | BRF_GRA },		 //  7
	{ "266-c7_decrypted.bin",  0x800000, 0x3c1fb02a, 3 | BRF_GRA },		 //  8
	{ "266-c8_decrypted.bin",  0x800000, 0x5ee31f80, 3 | BRF_GRA },		 //  9

	{ "266-m1.bin",   0x020000, 0x456c3e6c, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "266-v1.bin",   0x800000, 0xa4f83690, 5 | BRF_SND },           // 11 Sound data
	{ "266-v2.bin",   0x800000, 0xd0f69eda, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(matrimnd, matrimnd, neogeo)
STD_ROM_FN(matrimnd)

struct BurnDriverX BurnDrvmatrimnd = {
	"matrimnd", "matrim", "neogeo", NULL, "2002",
	"Matrimelee / Shin Gouketsuji Ichizoku Toukon (decrypted C)\0", NULL, "Noise Factory / Atlus", "Neo Geo",
	L"\u65B0\u8C6A\u8840\u5BFA\u4E00\u65CF - \u95D8\u5A5A\0Matrimelee (decrypted C)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_PWRINST,
	NULL, matrimndRomInfo, matrimndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	matrimInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};

// Pochi and Nyaa (decrypted C)

static struct BurnRomInfo pnyaandRomDesc[] = {
	{ "267-p1.bin",   0x100000, 0x112fe2c0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "267-c1_decrypted.bin",   0x800000, 0x75fa4f27, 3 | BRF_GRA },           //  1 Sprite data
	{ "267-c2_decrypted.bin",   0x800000, 0x02345689, 3 | BRF_GRA },           //  2 

	{ "267-m1.bin",   0x080000, 0xc7853ccd, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "267-v1.bin",   0x400000, 0xe2e8e917, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(pnyaand, pnyaand, neogeo)
STD_ROM_FN(pnyaand)

int pnyaandInit()
{
	int nRet;
	
	nRet = NeoInit();

	if (nRet == 0) {
		for (int i = 0; i < 0x400000 / 2; i += 4 / 2) {
			unsigned short buffer[4 / 2];
			memcpy(buffer, ((unsigned short*)YM2610ADPCMAROM) + i, 4);
			for (int j = 0; j < 4 / 2; j++) {
				((unsigned short*)YM2610ADPCMAROM)[i + j] = buffer[j ^ 1];
			}
		}
	}

	return nRet;
}

struct BurnDriverX BurnDrvpnyaand = {
	"pnyaand", "pnyaa", "neogeo", NULL, "2003",
	"Pochi and Nyaa (decrypted C)\0", NULL, "Aiky / Taito", "Neo Geo",
	L"Pochi and Nyaa (decrypted C)\0\u30DD\u30C1\u30C3\u3068\u306B\u3083\uFF5E\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_PUZZLE, 0,
	NULL, pnyaandRomInfo, pnyaandRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	pnyaandInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Metal Slug 5 (decrypted C)

static struct BurnRomInfo mslug5ndRomDesc[] = {
	{ "268-p1cr.bin", 0x400000, 0xd0466792, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "268-p2cr.bin", 0x400000, 0xfbf6b61e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "268-c1_decrypted.bin",  0x800000, 0xe8239365, 3 | BRF_GRA },		 //  2 Sprite data
	{ "268-c2_decrypted.bin",  0x800000, 0x89b21d4c, 3 | BRF_GRA },		 //  3
	{ "268-c3_decrypted.bin",  0x800000, 0x3cda13a0, 3 | BRF_GRA },		 //  4
	{ "268-c4_decrypted.bin",  0x800000, 0x9c00160d, 3 | BRF_GRA },		 //  5
	{ "268-c5_decrypted.bin",  0x800000, 0x38754256, 3 | BRF_GRA },		 //  6
	{ "268-c6_decrypted.bin",  0x800000, 0x59d33e9c, 3 | BRF_GRA },		 //  7
	{ "268-c7_decrypted.bin",  0x800000, 0xc9f8c357, 3 | BRF_GRA },		 //  8
	{ "268-c8_decrypted.bin",  0x800000, 0xfafc3eb9, 3 | BRF_GRA },		 //  9

	{ "268-m1.bin",   0x080000, 0x4a5a6e0e, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "268-v1c.bin",  0x800000, 0xae31d60c, 5 | BRF_SND },           // 11 Sound data
	{ "268-v2c.bin",  0x800000, 0xc40613ed, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(mslug5nd, mslug5nd, neogeo)
STD_ROM_FN(mslug5nd)

struct BurnDriverX BurnDrvmslug5nd = {
	"mslug5nd", "mslug5", "neogeo", NULL, "2003",
	"Metal Slug 5 (decrypted C)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug5ndRomInfo, mslug5ndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Metal Slug 5 (JAMMA PCB, decrypted C)

static struct BurnRomInfo ms5pcbndRomDesc[] = {
	{ "268-p1.bin",   0x400000,  0xd0466792, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "268-p2.bin",   0x400000,  0xfbf6b61e, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "268-c1_decrypted.bin",   0x1000000, 0x996debed, 3 | BRF_GRA },           //  2 Sprite data
	{ "268-c2_decrypted.bin",   0x1000000, 0x960a9764, 3 | BRF_GRA },           //  3 
	{ "268-c3_decrypted.bin",   0x1000000, 0xf699994f, 3 | BRF_GRA },           //  4 
	{ "268-c4_decrypted.bin",   0x1000000, 0x49e902e8, 3 | BRF_GRA },           //  5 

	{ "268-m1.bin",   0x080000,  0x4a5a6e0e, 4 | BRF_ESS | BRF_PRG }, // 6 Z80 code

	{ "268-v1.bin",   0x1000000, 0x8458afe5, 5 | BRF_SND },           //  7 Sound data
};

STDROMPICKEXT(ms5pcbnd, ms5pcbnd, ms5pcbBIOS)
STD_ROM_FN(ms5pcbnd)

struct BurnDriverX BurnDrvms5pcbnd = {
	"ms5pcbnd", "ms5pcb", NULL, NULL, "2003",
	"Metal Slug 5 (JAMMA PCB, decrypted C)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_P32 | HARDWARE_SNK_PCB | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, ms5pcbndRomInfo, ms5pcbndRomName, NULL, NULL, neogeoInputInfo, ms5pcbDIPInfo,
	mslug5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// SNK vs. CAPCOM SVC CHAOS (decrypted C)

static struct BurnRomInfo svcndRomDesc[] = {
	{ "269-p1.bin",   0x400000, 0x38e2005e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "269-p2.bin",   0x400000, 0x6d13797c, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "269-c1_decrypted.bin",  0x800000, 0x465d473b, 3 | BRF_GRA },		 //  2 Sprite data
	{ "269-c2_decrypted.bin",  0x800000, 0x3eb28f78, 3 | BRF_GRA },		 //  3
	{ "269-c3_decrypted.bin",  0x800000, 0xf4d4ab2b, 3 | BRF_GRA },		 //  4
	{ "269-c4_decrypted.bin",  0x800000, 0xa69d523a, 3 | BRF_GRA },		 //  5
	{ "269-c5_decrypted.bin",  0x800000, 0xba2a7892, 3 | BRF_GRA },		 //  6
	{ "269-c6_decrypted.bin",  0x800000, 0x37371ca1, 3 | BRF_GRA },		 //  7
	{ "269-c7_decrypted.bin",  0x800000, 0x5595b6cc, 3 | BRF_GRA },		 //  8
	{ "269-c8_decrypted.bin",  0x800000, 0xb17dfcf9, 3 | BRF_GRA },		 //  9

	{ "269-m1.bin",   0x080000,  0xf6819d00, 4 | BRF_ESS | BRF_PRG }, //  10 Z80 code

	{ "269-v1.bin",   0x800000, 0xc659b34c, 5 | BRF_SND },           // 11 Sound data
	{ "269-v2.bin",   0x800000, 0xdd903835, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(svcnd, svcnd, neogeo)
STD_ROM_FN(svcnd)

struct BurnDriverX BurnDrvsvcnd = {
	"svcnd", "svc", "neogeo", NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS (decrypted C)\0", NULL, "Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcndRomInfo, svcndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	svcpcbInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// SNK vs. CAPCOM SVC CHAOS (JAMMA PCB, set 1, decrypted C)

static struct BurnRomInfo svcpcbndRomDesc[] = {
	{ "269-p1.bin",   0x2000000, 0x432cfdfc, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "269-c1_decrypted.bin",   0x2000000, 0x382ce01f, 3 | BRF_GRA },           //  1 Sprite data
	{ "269-c2_decrypted.bin",   0x2000000, 0x88ad01ec, 3 | BRF_GRA },           //  2 

	{ "269-m1.bin",   0x080000,  0xf6819d00, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "269-v1.bin",   0x800000,  0xc659b34c, 5 | BRF_SND },           //  4 Sound data
	{ "269-v2.bin",   0x800000,  0xdd903835, 5 | BRF_SND },           //  5 
};

STDROMPICKEXT(svcpcbnd, svcpcbnd, svcpcbBIOS)
STD_ROM_FN(svcpcbnd)

int svcpcbndInit()
{
	int nRet;

	nNeoTextROMSize = 0x080000;
	pNeoInitCallback = svcCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xc2000, 0xffac28, { 0xc3, 0xfd, 0x81, 0xac, 0x6d, 0xe7, 0xbf, 0x9e } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriverX BurnDrvsvcpcbnd = {
	"svcpcbnd", "svcpcb", NULL, NULL, "2003",
	"SNK vs. CAPCOM SVC CHAOS (JAMMA PCB, set 1, decrypted C)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_PCB | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF | FBF_SF,
	NULL, svcpcbndRomInfo, svcpcbndRomName, NULL, NULL, neogeoInputInfo, svcpcbDIPInfo,
	svcpcbndInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Samurai Shodown V / Samurai Spirits Zero (decrypted C)

static struct BurnRomInfo samsh5ndRomDesc[] = {
	{ "270-p1.bin",   0x400000, 0x4a2a09e6, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "270-p2.bin",   0x400000, 0xe0c74c85, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "270-c1_decrypted.bin",   0x800000, 0x9adec562, 3 | BRF_GRA },		 //  2 Sprite data
	{ "270-c2_decrypted.bin",   0x800000, 0xac0309e5, 3 | BRF_GRA },		 //  3
	{ "270-c3_decrypted.bin",   0x800000, 0x82db9dae, 3 | BRF_GRA },		 //  4
	{ "270-c4_decrypted.bin",   0x800000, 0xf8041153, 3 | BRF_GRA },		 //  5
	{ "270-c5_decrypted.bin",   0x800000, 0xe689d62d, 3 | BRF_GRA },		 //  6
	{ "270-c6_decrypted.bin",   0x800000, 0xa993bdcf, 3 | BRF_GRA },		 //  7
	{ "270-c7_decrypted.bin",   0x800000, 0x707d56a0, 3 | BRF_GRA },		 //  8
	{ "270-c8_decrypted.bin",   0x800000, 0xf5903adc, 3 | BRF_GRA },		 //  9

	{ "270-m1.bin",   0x080000, 0x49c9901a, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "270-v1.bin",   0x800000, 0x62e434eb, 5 | BRF_SND },           // 11 Sound data
	{ "270-v2.bin",   0x800000, 0x180f3c9a, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(samsh5nd, samsh5nd, neogeo)
STD_ROM_FN(samsh5nd)

struct BurnDriverX BurnDrvsamsh5nd = {
	"samsh5nd", "samsho5", "neogeo", NULL, "2003",
	"Samurai Shodown V / Samurai Spirits Zero (decrypted C)\0", NULL, "Yuki Enterprise / SNK Playmore", "Neo Geo",
	L"Samurai Shodown V (decrypted C)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 (decrypted C)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, samsh5ndRomInfo, samsh5ndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsho5Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// The King of Fighters 2003 (decrypted C)

static struct BurnRomInfo kof2k3ndRomDesc[] = {
	{ "271-p1c.bin",  0x400000, 0x530ecc14, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "271-p2c.bin",  0x400000, 0xfd568da9, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "271-p3c.bin",  0x100000, 0xaec5b4a9, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "271-c1_decrypted.bin", 0x800000, 0xe42fc226, 3 | BRF_GRA },		 //  3 Sprite data
	{ "271-c2_decrypted.bin", 0x800000, 0x1b5e3b58, 3 | BRF_GRA },		 //  4
	{ "271-c3_decrypted.bin", 0x800000, 0xd334fdd9, 3 | BRF_GRA },		 //  5
	{ "271-c4_decrypted.bin", 0x800000, 0x0d457699, 3 | BRF_GRA },		 //  6
	{ "271-c5_decrypted.bin", 0x800000, 0x8a91aae4, 3 | BRF_GRA },		 //  7
	{ "271-c6_decrypted.bin", 0x800000, 0x9f8674b8, 3 | BRF_GRA },		 //  8
	{ "271-c7_decrypted.bin", 0x800000, 0x8ee6b43c, 3 | BRF_GRA },		 //  9
	{ "271-c8_decrypted.bin", 0x800000, 0x6d8d2d60, 3 | BRF_GRA },		 // 10 

	{ "271-m1c.bin",  0x080000, 0xf5515629, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "271-v1c.bin",  0x800000, 0xffa3f8c7, 5 | BRF_SND },           // 12 Sound data
	{ "271-v2c.bin",  0x800000, 0x5382c7d1, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kof2k3nd, kof2k3nd, neogeo)
STD_ROM_FN(kof2k3nd)

struct BurnDriverX BurnDrvkof2k3nd = {
	"kof2k3nd", "kof2003", "neogeo", NULL, "2003",
	"The King of Fighters 2003 (decrypted C)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2k3ndRomInfo, kof2k3ndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2003Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// The King of Fighters 2003 (Japan, JAMMA PCB, decrypted C)

static struct BurnRomInfo k2k3pcbdRomDesc[] = {
	{ "271-p1.bin",   0x0400000, 0xb9da070c, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "271-p2.bin",   0x0400000, 0xda3118c4, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "271-p3.bin",   0x0100000, 0x5cefd0d2, 1 | BRF_ESS | BRF_PRG }, //  2 

	{ "271-c1_decrypted.bin",   0x1000000, 0xc29acd28, 3 | BRF_GRA },           //  3 Sprite data
	{ "271-c2_decrypted.bin",   0x1000000, 0x328e80b1, 3 | BRF_GRA },           //  4 
	{ "271-c3_decrypted.bin",   0x1000000, 0x020a11f1, 3 | BRF_GRA },           //  5 
	{ "271-c4_decrypted.bin",   0x1000000, 0x991b5ed2, 3 | BRF_GRA },           //  6 
	{ "271-c5_decrypted.bin",   0x1000000, 0xc2de8b66, 3 | BRF_GRA },           //  7 
	{ "271-c6_decrypted.bin",   0x1000000, 0x3ff750db, 3 | BRF_GRA },           //  8 

	{ "271-m1.bin",   0x0080000, 0xd6bcf2bc, 4 | BRF_ESS | BRF_ESS }, //  9 Z80 code

	{ "271-v1.bin",   0x1000000, 0x1d96154b, 5 | BRF_SND },           // 10 Sound data
};

STDROMPICKEXT(k2k3pcbd, k2k3pcbd, kf2k3pcbBIOS)
STD_ROM_FN(k2k3pcbd)

int k2k3pcbdInit()
{
	int nRet;

	nNeoTextROMSize = 0x100000;
	pNeoInitCallback = kf2k3pcbCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa7001, 0xff14ea, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
		
		// M1 has additional swap
		for (int i = 0; i < 0x80000; i++) {
			NeoZ80ROM[i] = BITSWAP08(NeoZ80ROM[i], 5, 6, 1, 4, 3, 0, 7, 2);
		}
	}

	return nRet;
}

struct BurnDriverX BurnDrvk2k3pcbd = {
	"k2k3pcbd", "kf2k3pcb", NULL, NULL, "2003",
	"The King of Fighters 2003 (Japan, JAMMA PCB, decrypted C)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_PCB | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, k2k3pcbdRomInfo, k2k3pcbdRomName, NULL, NULL, neogeoInputInfo, kf2k3pcbDIPInfo,
	k2k3pcbdInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Samurai Shodown V Special / Samurai Spirits Zero Special (decrypted C)

static struct BurnRomInfo ss5shndRomDesc[] = {
	{ "272-p1.bin",   0x400000, 0xfb7a6bba, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "272-p2.bin",   0x400000, 0x63492ea6, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "272-c1_decrypted.bin",  0x800000, 0x8548097e, 3 | BRF_GRA },		 //  2 Sprite data
	{ "272-c2_decrypted.bin",  0x800000, 0x8c1b48d0, 3 | BRF_GRA },		 //  3
	{ "272-c3_decrypted.bin",  0x800000, 0x96ddb28c, 3 | BRF_GRA },		 //  4
	{ "272-c4_decrypted.bin",  0x800000, 0x99ef7a0a, 3 | BRF_GRA },		 //  5
	{ "272-c5_decrypted.bin",  0x800000, 0x772e8b1e, 3 | BRF_GRA },		 //  6
	{ "272-c6_decrypted.bin",  0x800000, 0x5fff21fc, 3 | BRF_GRA },		 //  7
	{ "272-c7_decrypted.bin",  0x800000, 0x9ac56a0e, 3 | BRF_GRA },		 //  8
	{ "272-c8_decrypted.bin",  0x800000, 0xcfde7aff, 3 | BRF_GRA },		 //  9
 
	{ "272-m1.bin",   0x080000, 0xadeebf40, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "272-v1.bin",   0x800000, 0x76a94127, 5 | BRF_SND },           // 11 Sound data
	{ "272-v2.bin",   0x800000, 0x4ba507f1, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(ss5shnd, ss5shnd, neogeo)
STD_ROM_FN(ss5shnd)

struct BurnDriverX BurnDrvss5shnd = {
	"ss5shnd", "samsh5sp", "neogeo", NULL, "2003",
	"Samurai Shodown V Special / Samurai Spirits Zero Special (decrypted C)\0", NULL, "Yuki Enterprise / SNK Playmore", "Neo Geo",
	L"Samurai Shodown V Special (decrypted C)\0\u30B5\u30E0\u30E9\u30A4\u30B9\u30D4\u30EA\u30C3\u30C4\u96F6 Special (decrypted C)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_SAMSHO,
	NULL, ss5shndRomInfo, ss5shndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	samsh5spInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Jockey Grand Prix (decrypted C)

static struct BurnRomInfo jckygpndRomDesc[] = {
	{ "jgp-p1.bin",   0x100000, 0x2fb7f388, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "jgp-c1_decrypted.bin",   0x800000, 0xea259019, 3 | BRF_GRA },           //  1 Sprite data
	{ "jgp-c2_decrypted.bin",   0x800000, 0x97f270be, 3 | BRF_GRA },           //  2 

	{ "jgp-m1.bin",   0x080000, 0xd163c690, 4 | BRF_ESS | BRF_PRG }, //  3 Z80 code

	{ "jgp-v1.bin",   0x200000, 0x443eadba, 5 | BRF_SND },           //  4 Sound data
};

STDROMPICKEXT(jckygpnd, jckygpnd, neogeo)
STD_ROM_FN(jckygpnd)

struct BurnDriverX BurnDrvjckygpnd = {
	"jckygpnd", "jockeygp", "neogeo", NULL, "2001",
	"Jockey Grand Prix (decrypted C)\0", NULL, "Sun Amusement / BrezzaSoft", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_GAMBLING | HARDWARE_SNK_ENCRYPTED_M1, GBF_MISC, 0,
	NULL, jckygpndRomInfo, jckygpndRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	jockeygpInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// -----------------------------------------------------------------------------
// Kawaks Hacks/bootlegs


// The King of Fighters 2002 (bootleg)

static struct BurnRomInfo kof2002bRomDesc[] = {
	{ "265-p1.p1",    0x100000, 0x9ede7323, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "2k2-p2.bin",   0x400000, 0x6dbee4df, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "2k2-s1.bin",   0x020000, 0x2255f5bf, 2 | BRF_GRA },		 //  2 Text data

	{ "2k2-c1.bin",   0x800000, 0xf25d3d66, 3 | BRF_GRA },		 //  3 Sprite data
	{ "2k2-c2.bin",   0x800000, 0xe3e66f1d, 3 | BRF_GRA },		 //  4
	{ "2k2-c3.bin",   0x800000, 0x8732fa30, 3 | BRF_GRA },		 //  5
	{ "2k2-c4.bin",   0x800000, 0x0989fd40, 3 | BRF_GRA },		 //  6
	{ "2k2-c5.bin",   0x800000, 0x60635cd2, 3 | BRF_GRA },		 //  7
	{ "2k2-c6.bin",   0x800000, 0xbd736824, 3 | BRF_GRA },		 //  8
	{ "2k2-c7.bin",   0x800000, 0x2da8d8cf, 3 | BRF_GRA },		 //  9
	{ "2k2-c8.bin",   0x800000, 0x2048404a, 3 | BRF_GRA },		 // 10

	{ "265-m1.m1",    0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "265-v1.v1",    0x800000, 0x15e8f3f5, 5 | BRF_SND },		 // 12 Sound data
	{ "265-v2.v2",    0x800000, 0xda41d6f9, 5 | BRF_SND },		 // 13
};

STDROMPICKEXT(kof2002b, kof2002b, neogeo)
STD_ROM_FN(kof2002b)

void kof2002b_gfx_decrypt(unsigned char *src, int nLen)
{
	int i, j, k, *m;
	int tbl[8][6] = {
		{ 1, 2, 6, 7, 8, 0 },
		{ 2, 6, 7, 8, 0, 1 },
		{ 6, 7, 8, 0, 1, 2 },
		{ 7, 8, 0, 1, 2, 6 },
		{ 8, 0, 1, 2, 6, 7 },
		{ 8, 7, 6, 2, 1, 0 },
		{ 8, 7, 6, 0, 1, 2 },
		{ 1, 2, 6, 7, 0, 8 }
	};

	unsigned char dst[0x10000];

	for (i = 0; i < nLen; i+= 0x10000)
	{
		memcpy (dst, src + i, 0x10000);

		for (j = 0; j < 0x10000 / 0x80; j++)
		{
			m = tbl[(j >> 3) & 7];
			k = BITSWAP16(j, 15, 14, 13, 12, 11, 10, 9, m[5], m[4], m[3], 5, 4, 3, m[2], m[1], m[0]);
			memcpy (src + i + k * 0x80, dst + j * 0x80, 0x80);
		}
	}
}

static void kof2002bCallback()
{
	kof2002Callback();
	kof2002b_gfx_decrypt(NeoSpriteROM, 0x4000000);
	kof2002b_gfx_decrypt(NeoTextROM + 0x020000, 0x020000);
}

int kof2002bInit()
{
	int nRet;

	pNeoInitCallback = kof2002bCallback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa5000, 0x000000, { 0xf9, 0xe0, 0x5d, 0xf3, 0xea, 0x92, 0xbe, 0xef } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkof2002b = {
	"kof2002b", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2002bRomInfo, kof2002bRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2002bInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Matrimelee / Shin Gouketsuji Ichizoku Toukon (bootleg)

static struct BurnRomInfo matrimblRomDesc[] = {
	{ "266-p1.p1",    0x100000, 0x5d4c2dc7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "266-p2.sp2",   0x400000, 0xa14b1906, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "mart-c1.bin",  0x800000, 0xa5595656, 3 | BRF_GRA },		 //  2 Sprite data
	{ "mart-c2.bin",  0x800000, 0xc5f7c300, 3 | BRF_GRA },		 //  3
	{ "mart-c3.bin",  0x800000, 0x574efd7d, 3 | BRF_GRA },		 //  4
	{ "mart-c4.bin",  0x800000, 0x109d54d9, 3 | BRF_GRA },		 //  5
	{ "mart-c5.bin",  0x800000, 0x15c9e882, 3 | BRF_GRA },		 //  6
	{ "mart-c6.bin",  0x800000, 0x77497b97, 3 | BRF_GRA },		 //  7
	{ "mart-c7.bin",  0x800000, 0xab481bb6, 3 | BRF_GRA },		 //  8
	{ "mart-c8.bin",  0x800000, 0x906cf267, 3 | BRF_GRA },		 //  9

	{ "mart-m1.bin",  0x020000, 0x3ea96ab1, 4 | BRF_ESS | BRF_PRG }, // 10 Z80 code

	{ "mart-v1.bin",  0x400000, 0x352b0a07, 5 | BRF_SND },		 // 11 Sound data
	{ "mart-v2.bin",  0x400000, 0x1e9bd59e, 5 | BRF_SND },		 // 12
	{ "mart-v3.bin",  0x400000, 0xe8362fcc, 5 | BRF_SND },		 // 13
	{ "mart-v4.bin",  0x400000, 0xc8c79b19, 5 | BRF_SND },		 // 14 
};

STDROMPICKEXT(matrimbl, matrimbl, neogeo)
STD_ROM_FN(matrimbl)

static void matrimblCallback()
{
	int i, j;
	unsigned char *dst = (unsigned char *)malloc( 0x020000 );

	if (dst) 
	{
		memcpy(dst, NeoZ80ROM, 0x020000);

		for(i = 0; i < 0x020000; i++) {
					 j  = i;
			if (i & 0x00001) j ^= 0x00800;
			if (i & 0x00002) j ^= 0x02000;
			if (i & 0x00800) j ^= 0x10800;
			if (i & 0x10000) j ^= 0x00800;
			NeoZ80ROM[j] = dst[i];
		}
		free (dst);
	}

	kof2002Callback();
	DoPerm(0);
	NeoExtractSData(NeoSpriteROM, NeoTextROM + 0x020000, 0x4000000, 0x080000);
}

int matrimblInit()
{
	int nRet;

	nNeoTextROMSize = 0x080000;
	pNeoInitCallback = matrimblCallback;

	nRet = NeoInit();

	if (nRet == 0) {
		BurnByteswap(YM2610ADPCMAROM + 0x400000, 0x400000);
		BurnByteswap(YM2610ADPCMAROM + 0xc00000, 0x400000);
	}

	return nRet;
}

struct BurnDriver BurnDrvmatrimbl = {
	"matrimbl", "matrim", "neogeo", NULL, "2002",
	"Matrimelee / Shin Gouketsuji Ichizoku Toukon (bootleg)\0", NULL, "bootleg", "Neo Geo",
	L"\u65B0\u8C6A\u8840\u5BFA\u4E00\u65CF - \u95D8\u5A5A\0Matrimelee (bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ALTERNATE_TEXT, GBF_VSFIGHT, FBF_PWRINST,
	NULL, matrimblRomInfo, matrimblRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	matrimblInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	320, 224, 4, 3
};


// Zintrick / Oshidashi Zentrix (Neo CD conversion)

static struct BurnRomInfo zintrkcdRomDesc[] = {
	{ "211-p1.bin",   0x100000, 0x9a0bfe0a, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "211-s1.bin",   0x020000, 0x56d16afa, 2 | BRF_GRA },		 //  1 Text data

	{ "zin-c1.bin",   0x200000, 0x76aee189, 3 | BRF_GRA },		 //  2 Sprite data
	{ "zin-c2.bin",   0x200000, 0x844ed4b3, 3 | BRF_GRA },		 //  3

	{ "211-m1.bin",   0x010000, 0xfcae1407, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "211-v1.bin",   0x100000, 0x781439da, 5 | BRF_SND },		 //  5 Sound data
};

STDROMPICKEXT(zintrkcd, zintrkcd, neogeo)
STD_ROM_FN(zintrkcd)

struct BurnDriver BurnDrvzintrkcd = {
	"zintrkcd", "zintrckb", "neogeo", NULL, "1996",
	"Zintrick / Oshidashi Zentrix (Neo CD conversion)\0", NULL, "hack", "Neo Geo",
	L"Zintrick (Neo CD conversion)\0\u62BC\u3057\u51FA\u3057\u30B8\u30F3\u30C8\u30EA\u30C3\u30AF (Neo CD conversion)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, zintrkcdRomInfo, zintrkcdRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Last Hope (Neo CD conversion)

static struct BurnRomInfo lhcdbRomDesc[] = {
	{ "300-p1ch.bin", 0x100000, 0x0f7405d7, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code

	{ "300-s1ch.bin", 0x020000, 0x298495d6, 2 | BRF_GRA },		 //  1 Text data

	{ "300-c1ch.bin", 0x400000, 0x554e6b73, 3 | BRF_GRA },		 //  2 Sprite data
	{ "300-c2ch.bin", 0x400000, 0x7c84b0fc, 3 | BRF_GRA },		 //  3
	{ "300-c3ch.bin", 0x400000, 0x28ec7555, 3 | BRF_GRA },		 //  4	
	{ "300-c4ch.bin", 0x400000, 0x8b7c236b, 3 | BRF_GRA },		 //  5	

	{ "300-m1ch.bin", 0x020000, 0xd9f6c153, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "300-v1ch.bin", 0x400000, 0xde563ec3, 5 | BRF_SND },		 //  7 Sound data
	{ "300-v2ch.bin", 0x400000, 0x93478033, 5 | BRF_SND },		 //  8
	{ "300-v3ch.bin", 0x400000, 0xe0fc99ca, 5 | BRF_SND },		 //  9
};

STDROMPICKEXT(lhcdb, lhcdb, neogeo)
STD_ROM_FN(lhcdb)

struct BurnDriver BurnDrvlhcdb = {
	"lhcdb", NULL, "neogeo", NULL, "2007",
	"Last Hope CD Beta (Neo CD conversion)\0", "Imperfect graphics", "NG:Dev.Team", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_PUZZLE, 0,
	NULL, lhcdbRomInfo, lhcdbRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2003 (set 2)

static struct BurnRomInfo kof2003hRomDesc[] = {
	{ "271-p1k.p1",           0x400000, 0xd0d0ae3e, 1 | BRF_ESS | BRF_PRG }, //  0 68K Code
	{ "271-p2k.p2",           0x400000, 0xfb3f79d9, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "271-p3k.p3",           0x100000, 0x232702ad, 1 | BRF_ESS | BRF_PRG }, //  2

	{ "271-c1k.c1",           0x800000, 0xefb9dd24, 3 | BRF_GRA },		 //  3 Sprite data
	{ "271-c2k.c2",           0x800000, 0x3fb90447, 3 | BRF_GRA },		 //  4
	{ "271-c3k.c3",           0x800000, 0x27950f28, 3 | BRF_GRA },		 //  5
	{ "271-c4k.c4",           0x800000, 0x735177f8, 3 | BRF_GRA },		 //  6
	{ "271-c5k.c5",           0x800000, 0xa51b7c0f, 3 | BRF_GRA },		 //  7
	{ "271-c6k.c6",           0x800000, 0xd5cae4e0, 3 | BRF_GRA },		 //  8
	{ "271-c7k.c7",           0x800000, 0xe65ae2d0, 3 | BRF_GRA },		 //  9
	{ "271-c8k.c8",           0x800000, 0x312f528c, 3 | BRF_GRA },		 // 10

	{ "271-m1k.m1",           0x080000, 0x48d9affe, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code
	
	{ "271-v1c.v1",           0x800000, 0xffa3f8c7, 5 | BRF_SND },		 // 12 Sound data
	{ "271-v2c.v2",           0x800000, 0x5382c7d1, 5 | BRF_SND },		 // 13
}; 

STDROMPICKEXT(kof2003h, kof2003h, neogeo) 
STD_ROM_FN(kof2003h)

static void kof2003hCallback()
{
	int i, j, k;
	for (i = 0; i < 0x100000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x0fffe0 + (i & 0x1f)];

	for (i = 0; i < 0x100000; i++)
	        Neo68KROM[0x800000 + i] ^= Neo68KROM[0x100002 | i];

	for (i = 0x100000; i < 0x800000; i++)
		Neo68KROM[i] ^= ~Neo68KROM[0x7fffe0 + (i & 0x1f)];

	for (i = 0x100000; i < 0x800000; i += 4) {
		unsigned short rom16 = *((unsigned short *)(Neo68KROM + i + 1));
		rom16 = BITSWAP16(rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0);
		*((unsigned short *)(Neo68KROM + i + 1)) = rom16;
	}

	memcpy (Neo68KROM + 0x700000, Neo68KROM, 0x100000);

	for (i = 0; i < 0x0100000 / 0x10000; i++) {
		j = BITSWAP08(i, 7, 6, 5, 4, 1, 0, 3, 2);
		memcpy (Neo68KROM + i * 0x010000, Neo68KROM + 0x700000 + j * 0x010000, 0x010000);
	}

	memcpy (Neo68KROM + 0x200000, Neo68KROM + 0x100000, 0x600000);

	for (i = 0x200000; i < 0x900000; i += 0x100000)
	{
		for (j = 0; j < 0x100000; j += 0x100)
		{
			k  = (j & 0xf00) ^ 0x00400;
			k |= BITSWAP08(j >> 12, 6, 7, 4, 5, 0, 1, 2, 3) << 12;

			memcpy (Neo68KROM + 0x100000 + j, Neo68KROM + i + k, 0x100);
		}

		memcpy (Neo68KROM + i, Neo68KROM + 0x100000, 0x100000);
	}
}

static int kof2003hInit()
{
	int nRet;
	
	nNeoTextROMSize = 0x080000;
	nNeoProtectionXor = 0x9D;
	pNeoInitCallback = kof2003hCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa7001, 0xff14ea, { 0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62 } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkof2003h = {
	"kof2003h", "kof2003", "neogeo", NULL, "2003",
	"The King of Fighters 2003 (set 2, AES cart)\0", NULL, "SNK Playmore", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ALTERNATE_TEXT | HARDWARE_SNK_P32 | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kof2003hRomInfo, kof2003hRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2003hInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// -----------------------------------------------------------------------------
// Various hacks/bootlegs/homebrew

// The King of Fighters '96 (bootleg / hack)

static struct BurnRomInfo kof96epRomDesc[] = {
	{ "214-p1.bin",   0x100000, 0x52755d74, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "214-p2.bin",   0x200000, 0x002ccb73, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "214-s1.bin",   0x020000, 0x1254cbdb, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "214-c1.bin",   0x400000, 0x7ecf4aa2, 3 | BRF_GRA },           //  3 Sprite data
	{ "214-c2.bin",   0x400000, 0x05b54f37, 3 | BRF_GRA },           //  4 
	{ "214-c3.bin",   0x400000, 0x64989a65, 3 | BRF_GRA },           //  5 
	{ "214-c4.bin",   0x400000, 0xafbea515, 3 | BRF_GRA },           //  6 
	{ "214-c5.bin",   0x400000, 0x2a3bbd26, 3 | BRF_GRA },           //  7 
	{ "214-c6.bin",   0x400000, 0x44d30dc7, 3 | BRF_GRA },           //  8 
	{ "214-c7.bin",   0x400000, 0x3687331b, 3 | BRF_GRA },           //  9 
	{ "214-c8.bin",   0x400000, 0xfa1461ad, 3 | BRF_GRA },           // 10 

	{ "214-m1.bin",   0x020000, 0xdabc427c, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "214-v1.bin",   0x400000, 0x63f7b045, 5 | BRF_SND },           // 12 Sound data
	{ "214-v2.bin",   0x400000, 0x25929059, 5 | BRF_SND },           // 13 
	{ "214-v3.bin",   0x200000, 0x92a2257d, 5 | BRF_SND },           // 14 
	
	{ "kof96m_ep1.rom",0x080000, 0xe5f2fb43, 0 | BRF_ESS | BRF_PRG }, // 15
};

STDROMPICKEXT(kof96ep, kof96ep, neogeo)
STD_ROM_FN(kof96ep)

static void kof96epCallback()
{
	unsigned char *pTemp = (unsigned char*)malloc(0x80000);
	
	if (pTemp) {
		BurnLoadRom(pTemp, 15, 1);
		
		for (unsigned int i = 0; i < 0x80000; i++) {
			if (pTemp[i] - Neo68KROM[i] == 8) pTemp[i] = Neo68KROM[i];
		}
		
		memcpy(Neo68KROM, pTemp, 0x80000);
		
		free(pTemp);
	}
}

static int kof96epInit()
{
	pNeoInitCallback = kof96epCallback;

 	return NeoInit();
}

struct BurnDriver BurnDrvkof96ep = {
	"kof96ep", "kof96", "neogeo", NULL, "1996",
	"The King of Fighters '96 (bootleg / hack)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof96epRomInfo, kof96epRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof96epInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '97 Plus 2003 (bootleg / hack)

static struct BurnRomInfo kof97plaRomDesc[] = {
	{ "kf97-p1a.bin", 0x100000, 0x035911c5, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "232-p2.bin",   0x400000, 0x158b23f6, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "kf97-s1a.bin", 0x020000, 0x4ee2149a, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "232-c1.bin",   0x800000, 0x5f8bf0a1, 3 | BRF_GRA },           //  3 Sprite data
	{ "232-c2.bin",   0x800000, 0xe4d45c81, 3 | BRF_GRA },           //  4 
	{ "232-c3.bin",   0x800000, 0x581d6618, 3 | BRF_GRA },           //  5 
	{ "232-c4.bin",   0x800000, 0x49bb1e68, 3 | BRF_GRA },           //  6 
	{ "232-c5.bin",   0x400000, 0x34fc4e51, 3 | BRF_GRA },           //  7 
	{ "232-c6.bin",   0x400000, 0x4ff4d47b, 3 | BRF_GRA },           //  8 

	{ "232-m1.bin",   0x020000, 0x45348747, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.bin",   0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.bin",   0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.bin",   0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof97pla, kof97pla, neogeo)
STD_ROM_FN(kof97pla)

static void kof97plaCallback()
{
	lans2004_sx_decode();
}

int kof97plaInit()
{
	pNeoInitCallback = kof97plaCallback;

	return NeoInit();
}

struct BurnDriver BurnDrvkof97pla = {
	"kof97pla", "kof97", "neogeo", NULL, "2003",
	"The King of Fighters '97 Plus 2003 (bootleg / hack)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof97plaRomInfo, kof97plaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof97plaInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2001 Plus (set 1, bootleg / hack)

static struct BurnRomInfo kf2k1plsRomDesc[] = {
	{ "2k1-p1p.bin",      0x100000, 0x758529a7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "262-pg2.bin",      0x400000, 0x91eea062, 1 | BRF_ESS | BRF_PRG }, //  1
	
	{ "2k1-s1p.bin",      0x020000, 0x088657e6, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "262-c1-08-e0.bin", 0x800000, 0x99cc785a, 3 | BRF_GRA },     //  3 Sprite data
	{ "262-c2-08-e0.bin", 0x800000, 0x50368cbf, 3 | BRF_GRA },     //  4 
	{ "262-c3-08-e0.bin", 0x800000, 0xfb14ff87, 3 | BRF_GRA },     //  5 
	{ "262-c4-08-e0.bin", 0x800000, 0x4397faf8, 3 | BRF_GRA },     //  6 
	{ "262-c5-08-e0.bin", 0x800000, 0x91f24be4, 3 | BRF_GRA },     //  7 
	{ "262-c6-08-e0.bin", 0x800000, 0xa31e4403, 3 | BRF_GRA },     //  8 
	{ "262-c7-08-e0.bin", 0x800000, 0x54d9d1ec, 3 | BRF_GRA },     //  9 
	{ "262-c8-08-e0.bin", 0x800000, 0x59289a6b, 3 | BRF_GRA },     //  10 

	{ "265-262-m1.bin",   0x040000, 0xa7f8119f, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "262-v1-08-e0.bin", 0x400000, 0x83d49ecf, 5 | BRF_SND },     // 12 Sound data
	{ "262-v2-08-e0.bin", 0x400000, 0x003f1843, 5 | BRF_SND },     // 13 
	{ "262-v3-08-e0.bin", 0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 14 
	{ "262-v4-08-e0.bin", 0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 15 
};

STDROMPICKEXT(kf2k1pls, kf2k1pls, neogeo)
STD_ROM_FN(kf2k1pls)

int kf2k1plsInit()
{
	nNeoProtectionXor = 0x1e;

	return NeoInit();
}

struct BurnDriver BurnDrvkf2k1pls = {
	"kf2k1pls", "kof2001", "neogeo", NULL, "2002",
	"The King of Fighters 2001 Plus (set 1, bootleg / hack)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k1plsRomInfo, kf2k1plsRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k1plsInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2001 Plus (set 2, bootleg / hack)

static struct BurnRomInfo kf2k1paRomDesc[] = {
	{ "2k1-p1a.bin",      0x100000, 0xf8a71b6f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "262-pg2.bin",      0x400000, 0x91eea062, 1 | BRF_ESS | BRF_PRG }, //  1
	
	{ "2k1-s1a.bin",      0x020000, 0x50986eeb, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "262-c1-08-e0.bin", 0x800000, 0x99cc785a, 3 | BRF_GRA },     //  3 Sprite data
	{ "262-c2-08-e0.bin", 0x800000, 0x50368cbf, 3 | BRF_GRA },     //  4 
	{ "262-c3-08-e0.bin", 0x800000, 0xfb14ff87, 3 | BRF_GRA },     //  5 
	{ "262-c4-08-e0.bin", 0x800000, 0x4397faf8, 3 | BRF_GRA },     //  6 
	{ "262-c5-08-e0.bin", 0x800000, 0x91f24be4, 3 | BRF_GRA },     //  7 
	{ "262-c6-08-e0.bin", 0x800000, 0xa31e4403, 3 | BRF_GRA },     //  8 
	{ "262-c7-08-e0.bin", 0x800000, 0x54d9d1ec, 3 | BRF_GRA },     //  9 
	{ "262-c8-08-e0.bin", 0x800000, 0x59289a6b, 3 | BRF_GRA },     //  10 

	{ "265-262-m1.bin",   0x040000, 0xa7f8119f, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "262-v1-08-e0.bin", 0x400000, 0x83d49ecf, 5 | BRF_SND },     // 12 Sound data
	{ "262-v2-08-e0.bin", 0x400000, 0x003f1843, 5 | BRF_SND },     // 13 
	{ "262-v3-08-e0.bin", 0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 14 
	{ "262-v4-08-e0.bin", 0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 15 
};

STDROMPICKEXT(kf2k1pa, kf2k1pa, neogeo)
STD_ROM_FN(kf2k1pa)

static void kf2k1paCallback()
{
	for (int i = 0; i < 0x20000; i++) {
		NeoTextROM[i + 0x20000] = BITSWAP08(NeoTextROM[i + 0x20000], 3, 2, 4, 5, 1, 6, 0, 7);
	}	
}

int kf2k1paInit()
{
	nNeoProtectionXor = 0x1e;
	pNeoInitCallback = kf2k1paCallback;

	return NeoInit();
}

struct BurnDriver BurnDrvkf2k1pa = {
	"kf2k1pa", "kof2001", "neogeo", NULL, "2002",
	"The King of Fighters 2001 Plus (set 2, bootleg / hack)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k1paRomInfo, kf2k1paRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k1paInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg set 2)

static struct BurnRomInfo cthd2k3aRomDesc[] = {
	{ "5003-p1a.bin", 0x100000, 0x1185fe39, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "5003-p2a.bin", 0x400000, 0xea71faf7, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "5003-s1a.bin", 0x020000, 0x174ccffd, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "5003-c1a.bin", 0x800000, 0xdc90c563, 3 | BRF_GRA },           //  3 Sprite data
	{ "5003-c2a.bin", 0x800000, 0x7b08d331, 3 | BRF_GRA },           //  4 
	{ "5003-c3.bin",  0x800000, 0xac4aff71, 3 | BRF_GRA },           //  5 
	{ "5003-c4.bin",  0x800000, 0xafef5d66, 3 | BRF_GRA },           //  6 
	{ "5003-c5.bin",  0x800000, 0xc7c1ae50, 3 | BRF_GRA },           //  7 
	{ "5003-c6.bin",  0x800000, 0x613197f9, 3 | BRF_GRA },           //  8 
	{ "5003-c7a.bin", 0x800000, 0xad2d72b3, 3 | BRF_GRA },           //  9 
	{ "5003-c8a.bin", 0x800000, 0x8c3fc1b5, 3 | BRF_GRA },           // 10 

	{ "5003-m1a.bin", 0x020000, 0xa37cc447, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "262-v1-08-e0.bin",   0x400000, 0x83d49ecf, 5 | BRF_SND },     // 12 Sound data
	{ "5003-v2a.bin",       0x400000, 0x2b498449, 5 | BRF_SND },     // 13 
	{ "262-v3-08-e0.bin",   0x400000, 0x2ae38dbe, 5 | BRF_SND },     // 14 
	{ "262-v4-08-e0.bin",   0x400000, 0x26ec4dd9, 5 | BRF_SND },     // 15 
};

STDROMPICKEXT(cthd2k3a, cthd2k3a, neogeo)
STD_ROM_FN(cthd2k3a)

static void cthd2k3aCallback()
{
	int i, n;
	
	// P-ROM Encryption
	unsigned char nBank[] = { 0x06, 0x02, 0x04, 0x05, 0x01, 0x03, 0x00, 0x07, 0x27, 0x0E, 0x1C, 0x15, 0x1B, 0x17, 0x0A, 0x0F, 0x16, 0x14, 0x23, 0x0B, 0x22, 0x26, 0x08, 0x24, 0x21, 0x13, 0x1A, 0x0C, 0x19, 0x1D, 0x25, 0x10, 0x09, 0x20, 0x18, 0x1F, 0x1E, 0x12, 0x0D, 0x11 };
	
	unsigned char *pTemp = (unsigned char*)malloc(0x500000);
	if (pTemp) {
		for (i = 0; i < 0x500000 / 0x20000; i++) {
			memcpy(pTemp + (i * 0x20000), Neo68KROM + (nBank[i] * 0x20000), 0x20000);
		}
		memcpy(Neo68KROM, pTemp, 0x500000);
		free(pTemp);
	}
	
	unsigned short *Rom = (unsigned short*)Neo68KROM;
	Rom[0xed00e / 2] = 0x4e71;
	Rom[0xed394 / 2] = 0x4e71;
	Rom[0xa2b7e / 2] = 0x4e71;
	
	// Text ROM
	for (i = 0; i < 0x8000; i++) {
		n = NeoTextROM[0x28000 + i];
		NeoTextROM[0x28000 + i] = NeoTextROM[0x30000 + i];
		NeoTextROM[0x30000 + i] = n;
	}
	
	// Swap bits 15 & 16 in the address of the Z80 ROM
	for (i = 0; i < 0x10000 / 2; i++) {
		n = NeoZ80ROM[0x08000 + i];
		NeoZ80ROM[0x08000 + i] = NeoZ80ROM[0x10000 + i];
		NeoZ80ROM[0x10000 + i] = n;
	}

	DoPerm(0);
}

int cthd2k3aInit()
{
	pNeoInitCallback = cthd2k3aCallback;
	
	return NeoInit();
}

struct BurnDriver BurnDrvcthd2k3a = {
	"cthd2k3a", "kof2001", "neogeo", NULL, "2003",
	"Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg set 2)\0", NULL, "bootleg", "Neo Geo",
	L"Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg set 2)\0\u81E5\u864E\u85CF\u9F8D Crouching Tiger Hidden Dragon 2003\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, cthd2k3aRomInfo, cthd2k3aRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	cthd2k3aInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2002 Plus (bootleg set 3)

static struct BurnRomInfo kf2k2plbRomDesc[] = {
	{ "2k2-p1p.bin",  0x100000, 0x3ab03781, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "265-p2.bin",   0x400000, 0x327266b8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "2k2-s1pb.bin", 0x020000, 0x2072d5e9, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "265-c1.bin",   0x800000, 0x2b65a656, 3 | BRF_GRA },           //  3 Sprite data
	{ "265-c2.bin",   0x800000, 0xadf18983, 3 | BRF_GRA },           //  4 
	{ "265-c3.bin",   0x800000, 0x875e9fd7, 3 | BRF_GRA },           //  5 
	{ "265-c4.bin",   0x800000, 0x2da13947, 3 | BRF_GRA },           //  6 
	{ "265-c5.bin",   0x800000, 0x61bd165d, 3 | BRF_GRA },           //  7 
	{ "265-c6.bin",   0x800000, 0x03fdd1eb, 3 | BRF_GRA },           //  8 
	{ "265-c7.bin",   0x800000, 0x1a2749d8, 3 | BRF_GRA },           //  9 
	{ "265-c8.bin",   0x800000, 0xab0bb549, 3 | BRF_GRA },           // 10 

	{ "265-m1.bin",   0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "265-v1.bin",   0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 12 Sound data
	{ "265-v2.bin",   0x800000, 0xda41d6f9, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kf2k2plb, kf2k2plb, neogeo)
STD_ROM_FN(kf2k2plb)

struct BurnDriver BurnDrvkf2k2plb = {
	"kf2k2plb", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 Plus (bootleg set 3)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k2plbRomInfo, kf2k2plbRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kof2002Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2002 Super (bootleg)

static struct BurnRomInfo kf2k2plcRomDesc[] = {
	{ "2k2-p1pc.bin", 0x100000, 0xebedae17, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "265-p2.bin",   0x400000, 0x327266b8, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "2k2-s1pc.bin", 0x020000, 0xfecbb589, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "265-c1.bin",   0x800000, 0x2b65a656, 3 | BRF_GRA },           //  3 Sprite data
	{ "265-c2.bin",   0x800000, 0xadf18983, 3 | BRF_GRA },           //  4 
	{ "265-c3.bin",   0x800000, 0x875e9fd7, 3 | BRF_GRA },           //  5 
	{ "265-c4.bin",   0x800000, 0x2da13947, 3 | BRF_GRA },           //  6 
	{ "265-c5.bin",   0x800000, 0x61bd165d, 3 | BRF_GRA },           //  7 
	{ "265-c6.bin",   0x800000, 0x03fdd1eb, 3 | BRF_GRA },           //  8 
	{ "265-c7.bin",   0x800000, 0x1a2749d8, 3 | BRF_GRA },           //  9 
	{ "265-c8.bin",   0x800000, 0xab0bb549, 3 | BRF_GRA },           // 10 

	{ "265-m1.bin",   0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "265-v1.bin",   0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 12 Sound data
	{ "265-v2.bin",   0x800000, 0xda41d6f9, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kf2k2plc, kf2k2plc, neogeo)
STD_ROM_FN(kf2k2plc)

static void kf2k2plcCallback()
{
	kof2002Callback();
	
	garoubl_sx_decode();
}

int kf2k2plcInit()
{
	int nRet;

	nNeoProtectionXor = 0xEC;
	pNeoInitCallback = kf2k2plcCallback;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa5000, 0x000000, { 0xf9, 0xe0, 0x5d, 0xf3, 0xea, 0x92, 0xbe, 0xef } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k2plc = {
	"kf2k2plc", "kof2002", "neogeo", NULL, "2002",
	"The King of Fighters 2002 Super (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k2plcRomInfo, kf2k2plcRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k2plcInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters Special Edition 2004 Plus (The King of Fighters 2002 bootleg)

static struct BurnRomInfo kf2k4plsRomDesc[] = {
	{ "k2k4s-p1p.bin", 0x200000, 0xed97c684, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "k2k4s-p1.bin",  0x400000, 0xe6c50566, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "k2k4s-s1p.bin", 0x020000, 0xcc0ab564, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "k2k4s-c4.bin",  0x800000, 0x7a050288, 3 | BRF_GRA },           //  3 Sprite data
	{ "k2k4s-c8.bin",  0x800000, 0xe924afcf, 3 | BRF_GRA },           //  4 
	{ "k2k4s-c3.bin",  0x800000, 0x959fad0b, 3 | BRF_GRA },           //  5 
	{ "k2k4s-c7.bin",  0x800000, 0xefe6a468, 3 | BRF_GRA },           //  6 
	{ "k2k4s-c2.bin",  0x800000, 0x74bba7c6, 3 | BRF_GRA },           //  7 
	{ "k2k4s-c6.bin",  0x800000, 0xe20d2216, 3 | BRF_GRA },           //  8 
	{ "k2k4s-c1.bin",  0x800000, 0xfa705b2b, 3 | BRF_GRA },           //  9 
	{ "k2k4s-c5.bin",  0x800000, 0x2c912ff9, 3 | BRF_GRA },           // 10 

	{ "k2k4s-m1.bin",  0x020000, 0x5a47d9ad, 4 | BRF_ESS | BRF_PRG }, // 11 Z80 code

	{ "k2k4s-v2.bin",  0x800000, 0xe4ddfb3f, 5 | BRF_SND },           // 12 Sound data
	{ "k2k4s-v1.bin",  0x800000, 0xb887d287, 5 | BRF_SND },           // 13 
};

STDROMPICKEXT(kf2k4pls, kf2k4pls, neogeo)
STD_ROM_FN(kf2k4pls)

static void kf2k4plsCallback()
{
	unsigned char *pTemp = (unsigned char*)malloc(0x600000);
	
	if (pTemp) {
		memcpy(pTemp, Neo68KROM, 0x600000);
		memcpy(Neo68KROM + 0x000000, pTemp + 0x000000, 0x100000);
		memcpy(Neo68KROM + 0x500000, pTemp + 0x100000, 0x100000);
		memcpy(Neo68KROM + 0x400000, pTemp + 0x200000, 0x100000);
		memcpy(Neo68KROM + 0x300000, pTemp + 0x300000, 0x100000);
		memcpy(Neo68KROM + 0x200000, pTemp + 0x400000, 0x100000);
		memcpy(Neo68KROM + 0x100000, pTemp + 0x500000, 0x100000);
		
		memset(pTemp, 0, 0x600000);
		memcpy(pTemp + 0x000000, Neo68KROM + 0x000000, 0x100000);
		memcpy(pTemp + 0x100000, Neo68KROM + 0x500000, 0x100000);
		memcpy(Neo68KROM + 0x000000, pTemp + 0x1a0000, 0x020000);
		memcpy(Neo68KROM + 0x020000, pTemp + 0x080000, 0x020000);
		memcpy(Neo68KROM + 0x040000, pTemp + 0x140000, 0x020000);
		memcpy(Neo68KROM + 0x060000, pTemp + 0x000000, 0x020000);
		memcpy(Neo68KROM + 0x080000, pTemp + 0x180000, 0x020000);
		memcpy(Neo68KROM + 0x0a0000, pTemp + 0x0a0000, 0x020000);
		memcpy(Neo68KROM + 0x0c0000, pTemp + 0x100000, 0x020000);
		memcpy(Neo68KROM + 0x0e0000, pTemp + 0x040000, 0x020000);
		
		free(pTemp);
	}
	
	lans2004_sx_decode();
}

int kf2k4plsInit()
{
	pNeoInitCallback = kf2k4plsCallback;

	return NeoInit();
}

struct BurnDriver BurnDrvkf2k4pls = {
	"kf2k4pls", "kof2002", "neogeo", NULL, "2004",
	"The King of Fighters Special Edition 2004 Plus (The King of Fighters 2002 bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k4plsRomInfo, kf2k4plsRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k4plsInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Metal Slug 5 (bootleg)

static struct BurnRomInfo mslug5bRomDesc[] = {
	{ "ms5-p1b.bin",   0x500000, 0xd05853ea, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "268-c1c.bin",   0x800000, 0xab7c389a, 3 | BRF_GRA },           //  1 Sprite data
	{ "268-c2c.bin",   0x800000, 0x3560881b, 3 | BRF_GRA },           //  2 
	{ "268-c3c.bin",   0x800000, 0x3af955ea, 3 | BRF_GRA },           //  3 
	{ "268-c4c.bin",   0x800000, 0xc329c373, 3 | BRF_GRA },           //  4 
	{ "268-c5c.bin",   0x800000, 0x959c8177, 3 | BRF_GRA },           //  5 
	{ "268-c6c.bin",   0x800000, 0x010a831b, 3 | BRF_GRA },           //  6 
	{ "268-c7c.bin",   0x800000, 0x6d72a969, 3 | BRF_GRA },           //  7 
	{ "268-c8c.bin",   0x800000, 0x551d720e, 3 | BRF_GRA },           //  8 

	{ "268-m1h.bin",   0x080000, 0x4a5a6e0e, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "268-v1c.bin",   0x800000, 0xae31d60c, 5 | BRF_SND },           // 10 Sound data
	{ "268-v2c.bin",   0x800000, 0xc40613ed, 5 | BRF_SND },           // 11 
};

STDROMPICKEXT(mslug5b, mslug5b, neogeo)
STD_ROM_FN(mslug5b)

static void mslug5bCallback()
{
	unsigned char *pTemp = (unsigned char*)malloc(0x500000);
	
	if (pTemp) {
		memcpy(pTemp, Neo68KROM, 0x500000);
		memset(Neo68KROM, 0, 0x500000);
		memcpy(Neo68KROM + 0x000000, pTemp + 0x000000, 0x100000);
		memcpy(Neo68KROM + 0x100000, pTemp + 0x100000, 0x100000);
		memcpy(Neo68KROM + 0x300000, pTemp + 0x200000, 0x100000);
		memcpy(Neo68KROM + 0x200000, pTemp + 0x300000, 0x100000);
		memcpy(Neo68KROM + 0x400000, pTemp + 0x400000, 0x100000);
			
		free(pTemp);
	}
}

int mslug5bInit()
{
	int nRet;

	nNeoProtectionXor = 0x19;
	pNeoInitCallback = mslug5bCallback;

	nRet = NeoPVCInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0x4e001, 0xfe2cf6, { 0xc3, 0xfd, 0x81, 0xac, 0x6d, 0xe7, 0xbf, 0x9e } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvmslug5b = {
	"mslug5b", "mslug5", "neogeo", NULL, "2003",
	"Metal Slug 5 (bootleg)\0", NULL, "bootleg", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_B | HARDWARE_SNK_ENCRYPTED_M1, GBF_PLATFORM, FBF_MSLUG,
	NULL, mslug5bRomInfo, mslug5bRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	mslug5bInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '96 (Chinese Edition ver 1.0, hack)

static struct BurnRomInfo kof96cnRomDesc[] = {
	{ "214cn-p1.bin", 0x100000, 0xa8c25f0a, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "214cn-p2.bin", 0x200000, 0x0edface5, 1 | BRF_ESS | BRF_PRG }, //  1 
	{ "214cn-p3.bin", 0x500000, 0x8724a441, 1 | BRF_ESS | BRF_PRG }, //  2

	{ "214cn-s1.bin", 0x020000, 0xf917527a, 2 | BRF_GRA },           //  3 Text layer tiles

	{ "214-c1.bin",   0x400000, 0x7ecf4aa2, 3 | BRF_GRA },           //  4 Sprite data
	{ "214-c2.bin",   0x400000, 0x05b54f37, 3 | BRF_GRA },           //  5 
	{ "214-c3.bin",   0x400000, 0x64989a65, 3 | BRF_GRA },           //  6 
	{ "214-c4.bin",   0x400000, 0xafbea515, 3 | BRF_GRA },           //  7 
	{ "214-c5.bin",   0x400000, 0x2a3bbd26, 3 | BRF_GRA },           //  8 
	{ "214-c6.bin",   0x400000, 0x44d30dc7, 3 | BRF_GRA },           //  9 
	{ "214-c7.bin",   0x400000, 0x3687331b, 3 | BRF_GRA },           // 10 
	{ "214-c8.bin",   0x400000, 0xfa1461ad, 3 | BRF_GRA },           // 11 
	{ "214cn-c9.bin", 0x400000, 0xa9f811d2, 3 | BRF_GRA },           // 12
	{ "214cn-c10.bin",0x400000, 0x1147406a, 3 | BRF_GRA },           // 13 

	{ "214-m1.bin",   0x020000, 0xdabc427c, 4 | BRF_ESS | BRF_PRG }, // 14 Z80 code

	{ "214-v1.bin",   0x400000, 0x63f7b045, 5 | BRF_SND },           // 15 Sound data
	{ "214-v2.bin",   0x400000, 0x25929059, 5 | BRF_SND },           // 16 
	{ "214-v3.bin",   0x200000, 0x92a2257d, 5 | BRF_SND },           // 17 
};

STDROMPICKEXT(kof96cn, kof96cn, neogeo)
STD_ROM_FN(kof96cn)

struct BurnDriver BurnDrvkof96cn = {
	"kof96cn", "kof96", "neogeo", NULL, "200?",
	"The King of Fighters '96 (Chinese Edition ver 1.0, hack)\0", NULL, "hack", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof96cnRomInfo, kof96cnRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '96 (Anniversary Edition, EGHT hack)

static struct BurnRomInfo kof96aeRomDesc[] = {
	{ "214ae-p1.bin", 0x100000, 0x47660e7c, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "214ae-p2.bin", 0x400000, 0x824ff3eb, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "214ae-s1.bin", 0x020000, 0x203b3aaf, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "214ae-c1.bin", 0x400000, 0xb308c1f9, 3 | BRF_GRA },           //  3 Sprite data
	{ "214ae-c2.bin", 0x400000, 0x220a92b2, 3 | BRF_GRA },           //  4 
	{ "214-c3.bin",   0x400000, 0x64989a65, 3 | BRF_GRA },           //  5 
	{ "214-c4.bin",   0x400000, 0xafbea515, 3 | BRF_GRA },           //  6 
	{ "214ae-c5.bin", 0x400000, 0x6e03cb0c, 3 | BRF_GRA },           //  7 
	{ "214ae-c6.bin", 0x400000, 0xa90bc55d, 3 | BRF_GRA },           //  8 
	{ "214ae-c7.bin", 0x400000, 0x08021fd6, 3 | BRF_GRA },           //  9 
	{ "214ae-c8.bin", 0x400000, 0xbbd21b08, 3 | BRF_GRA },           // 10 
	{ "214ae-c9.bin", 0x400000, 0x57e5bf80, 3 | BRF_GRA },           // 11
	{ "214ae-c10.bin",0x400000, 0xcbe44c81, 3 | BRF_GRA },           // 12 
	{ "214ae-c11.bin",0x400000, 0x5c7ddc8b, 3 | BRF_GRA },           // 13
	{ "214ae-c12.bin",0x400000, 0x48d8b574, 3 | BRF_GRA },           // 14 

	{ "214ae-m1.bin", 0x020000, 0xc9dd0348, 4 | BRF_ESS | BRF_PRG }, // 15 Z80 code

	{ "214-v1.bin",   0x400000, 0x63f7b045, 5 | BRF_SND },           // 16 Sound data
	{ "214-v2.bin",   0x400000, 0x25929059, 5 | BRF_SND },           // 17 
	{ "214ae-v3.bin", 0x400000, 0x143923a7, 5 | BRF_SND },           // 18 
};

STDROMPICKEXT(kof96ae, kof96ae, neogeo)
STD_ROM_FN(kof96ae)

struct BurnDriver BurnDrvkof96ae = {
	"kof96ae", "kof96", "neogeo", NULL, "2007",
	"The King of Fighters '96 (Anniversary Edition, EGHT hack)\0", NULL, "hack", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof96aeRomInfo, kof96aeRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '97 (10th Anniversary Chinese Edition, EGHT hack)

static struct BurnRomInfo kof97cnRomDesc[] = {
	{ "232cn-p1.bin", 0x100000, 0x082933c8, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "232cn-p2.bin", 0x600000, 0xd22d3ca3, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "232cn-s1.bin", 0x020000, 0xde5ad278, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "232cn-c1.bin", 0x800000, 0x8c3d2c0c, 3 | BRF_GRA },           //  3 Sprite data
	{ "232cn-c2.bin", 0x800000, 0x43ce69ae, 3 | BRF_GRA },           //  4 
	{ "232-c3.bin",   0x800000, 0x581d6618, 3 | BRF_GRA },           //  5 
	{ "232-c4.bin",   0x800000, 0x49bb1e68, 3 | BRF_GRA },           //  6 
	{ "232-c5.bin",   0x400000, 0x34fc4e51, 3 | BRF_GRA },           //  7 
	{ "232-c6.bin",   0x400000, 0x4ff4d47b, 3 | BRF_GRA },           //  8 

	{ "232-m1.bin",   0x020000, 0x45348747, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.bin",   0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.bin",   0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.bin",   0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof97cn, kof97cn, neogeo)
STD_ROM_FN(kof97cn)

struct BurnDriver BurnDrvkof97cn = {
	"kof97cn", "kof97", "neogeo", NULL, "2007",
	"The King of Fighters '97 (10th Anniversary Chinese Edition, EGHT hack)\0", NULL, "hack", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof97cnRomInfo, kof97cnRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '97 - Final Battle (hack)

static struct BurnRomInfo kof97xtRomDesc[] = {
	{ "232xt-p1.bin", 0x100000, 0x2e4f4e3b, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "232xt-p2.bin", 0x400000, 0x6d4503ce, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "232xt-s1.bin", 0x020000, 0xd6fe166f, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "232xt-c1.bin", 0x800000, 0xd504bf4a, 3 | BRF_GRA },           //  3 Sprite data
	{ "232xt-c2.bin", 0x800000, 0x942ea708, 3 | BRF_GRA },           //  4 
	{ "232-c3.bin",   0x800000, 0x581d6618, 3 | BRF_GRA },           //  5 
	{ "232-c4.bin",   0x800000, 0x49bb1e68, 3 | BRF_GRA },           //  6 
	{ "232xt-c5.bin", 0x800000, 0xc1bd2375, 3 | BRF_GRA },           //  7 
	{ "232xt-c6.bin", 0x800000, 0xa7c8506a, 3 | BRF_GRA },           //  8 

	{ "232-m1.bin",   0x020000, 0x45348747, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "232-v1.bin",   0x400000, 0x22a2b5b5, 5 | BRF_SND },           // 10 Sound data
	{ "232-v2.bin",   0x400000, 0x2304e744, 5 | BRF_SND },           // 11 
	{ "232-v3.bin",   0x400000, 0x759eb954, 5 | BRF_SND },           // 12 
};

STDROMPICKEXT(kof97xt, kof97xt, neogeo)
STD_ROM_FN(kof97xt)

struct BurnDriver BurnDrvkof97xt = {
	"kof97xt", "kof97", "neogeo", NULL, "2007",
	"The King of Fighters '97 - Final Battle (hack)\0", NULL, "hack", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof97xtRomInfo, kof97xtRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters '98 (Anniversary Edition, EGHT hack)

static struct BurnRomInfo kof98aeRomDesc[] = {
	{ "242-p1ae.bin", 0x100000, 0xc9188c66, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "242-p2ae.bin", 0x600000, 0x609fac6b, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "242-s1ae.bin", 0x020000, 0xf1fee5c0, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "242-c1ae.bin", 0x800000, 0x0319cfc9, 3 | BRF_GRA },           //  3 Sprite data
	{ "242-c2ae.bin", 0x800000, 0x553f6714, 3 | BRF_GRA },           //  4 
	{ "242-c3.bin",   0x800000, 0x22127b4f, 3 | BRF_GRA },           //  5 
	{ "242-c4.bin",   0x800000, 0x0b4fa044, 3 | BRF_GRA },           //  6 
	{ "242-c5ae.bin", 0x800000, 0x71641718, 3 | BRF_GRA },           //  7 
	{ "242-c6ae.bin", 0x800000, 0x982ba2b3, 3 | BRF_GRA },           //  8 
	{ "242-c7ae.bin", 0x800000, 0x8d495552, 3 | BRF_GRA },           //  9 
	{ "242-c8ae.bin", 0x800000, 0x8bfc3417, 3 | BRF_GRA },           // 10 
	{ "242-c9ae.bin", 0x800000, 0x128256d1, 3 | BRF_GRA },           // 11
	{ "242-c91ae.bin",0x800000, 0x4bd8412d, 3 | BRF_GRA },           // 12
	{ "242-c92ae.bin",0x800000, 0xe893277f, 3 | BRF_GRA },           // 13
	{ "242-c93ae.bin",0x800000, 0xf165589e, 3 | BRF_GRA },           // 14

	{ "242-m1ae.bin", 0x040000, 0x9ade0528, 4 | BRF_ESS | BRF_PRG }, // 15 Z80 code

	{ "242-v1.bin",   0x400000, 0xb9ea8051, 5 | BRF_SND },           // 16 Sound data
	{ "242-v2.bin",   0x400000, 0xcc11106e, 5 | BRF_SND },           // 17 
	{ "242-v3.bin",   0x400000, 0x044ea4e1, 5 | BRF_SND },           // 18 
	{ "242-v4.bin",   0x400000, 0x7985ea30, 5 | BRF_SND },           // 19 
	{ "242-v5ae.bin", 0x400000, 0xafdd9660, 5 | BRF_SND },           // 20
};

STDROMPICKEXT(kof98ae, kof98ae, neogeo)
STD_ROM_FN(kof98ae)

struct BurnDriver BurnDrvkof98ae = {
	"kof98ae", "kof98", "neogeo", NULL, "2007",
	"The King of Fighters '98 (Anniversary Edition, EGHT hack)\0", NULL, "hack", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE, GBF_VSFIGHT, FBF_KOF,
	NULL, kof98aeRomInfo, kof98aeRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// The King of Fighters 2002 (PlayStation 2 ver 0.4, EGHT hack)

static struct BurnRomInfo kf2k2ps2RomDesc[] = {
	{ "265ps2-p1.bin",   0x100000, 0x336c4ca8, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "265ps2-p2.bin",   0x500000, 0xcb0032bf, 1 | BRF_ESS | BRF_PRG }, //  1 

	{ "265ps2-s1.bin",   0x020000, 0x714ade47, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "265-c1_decrypted.bin", 0x800000, 0x7efa6ef7, 3 | BRF_GRA },           //  3 Sprite data
	{ "265-c2_decrypted.bin", 0x800000, 0xaa82948b, 3 | BRF_GRA },           //  4 
	{ "265-c3_decrypted.bin", 0x800000, 0x959fad0b, 3 | BRF_GRA },           //  5 
	{ "265-c4_decrypted.bin", 0x800000, 0xefe6a468, 3 | BRF_GRA },           //  6 
	{ "265-c5_decrypted.bin", 0x800000, 0x74bba7c6, 3 | BRF_GRA },           //  7 
	{ "265-c6_decrypted.bin", 0x800000, 0xe20d2216, 3 | BRF_GRA },           //  8 
	{ "265ps2-c7.bin",   0x800000, 0x1b1d35fb, 3 | BRF_GRA },           //  9 
	{ "265ps2-c8.bin",   0x800000, 0xa5e35d11, 3 | BRF_GRA },           // 10 
	{ "265ps2-c9.bin",   0x800000, 0xaa8bbc97, 3 | BRF_GRA },           // 11
	{ "265ps2-c10.bin",  0x800000, 0x9832713d, 3 | BRF_GRA },           // 12 

	{ "265-m1.bin",      0x020000, 0x85aaa632, 4 | BRF_ESS | BRF_PRG }, // 13 Z80 code

	{ "265-v1.bin",      0x800000, 0x15e8f3f5, 5 | BRF_SND },           // 14 Sound data
	{ "265-v2.bin",      0x800000, 0xda41d6f9, 5 | BRF_SND },           // 15 
};

STDROMPICKEXT(kf2k2ps2, kf2k2ps2, neogeo)
STD_ROM_FN(kf2k2ps2)

int kf2k2ps2Init()
{
	int nRet;

	nRet = NeoInit();

	if (nRet == 0) {
		PCM2DecryptVInfo Info = { 0xa5000, 0x000000, { 0xf9, 0xe0, 0x5d, 0xf3, 0xea, 0x92, 0xbe, 0xef } };

		PCM2DecryptV(&Info);
	}

	return nRet;
}

struct BurnDriver BurnDrvkf2k2ps2 = {
	"kf2k2ps2", "kof2002", "neogeo", NULL, "2007",
	"The King of Fighters 2002 (PlayStation 2 ver 0.4, EGHT hack)\0", NULL, "hack", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HACK, 2, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_MVSCARTRIDGE | HARDWARE_SNK_ENCRYPTED_M1, GBF_VSFIGHT, FBF_KOF,
	NULL, kf2k2ps2RomInfo, kf2k2ps2RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	kf2k2ps2Init, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Neo Frog Feast

static struct BurnRomInfo ngfrogRomDesc[] = {
	{ "frg-p1.bin",   0x080000, 0xc26848da, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "frg-s1.bin",   0x020000, 0x88f4cd13, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "frg-c1.bin",   0x100000, 0x7b2f9fb3, 3 | BRF_GRA },           //  2 Sprite data
	{ "frg-c2.bin",   0x100000, 0xa7f18a6f, 3 | BRF_GRA },           //  3 
	{ "frg-c3.bin",   0x100000, 0x956bac74, 3 | BRF_GRA },           //  4 
	{ "frg-c4.bin",   0x100000, 0x956bac74, 3 | BRF_GRA },           //  5 

	{ "frg-m1.bin",   0x020000, 0xc825b484, 4 | BRF_ESS | BRF_PRG }, //  6 Z80 code

	{ "frg-v1.bin",   0x100000, 0x2639da11, 5 | BRF_SND },           //  7 Sound data
	{ "frg-v2.bin",   0x100000, 0xe5eda58b, 5 | BRF_SND },           //  8 
};

STDROMPICKEXT(ngfrog, ngfrog, neogeo)
STD_ROM_FN(ngfrog)

struct BurnDriver BurnDrvngfrog = {
	"ngfrog", NULL, "neogeo", NULL, "2006",
	"Frog Feast (Neo Geo)\0", NULL, "Rastersoft", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, ngfrogRomInfo, ngfrogRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Columns (Neo Geo)

static struct BurnRomInfo columnsnRomDesc[] = {
	{ "2000-p1.bin",  0x080000, 0xc6c9888d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "2000-s1.bin",  0x020000, 0x089a28a8, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "2000-c1.bin",  0x080000, 0x9a579cab, 3 | BRF_GRA },           //  2 Sprite data
	{ "2000-c2.bin",  0x080000, 0x91d14043, 3 | BRF_GRA },           //  3 

	{ "nnp-m1.rom",   0x010000, 0x7669de64, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "nnp-v1.rom",   0x080000, 0x504bf849, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(columnsn, columnsn, neogeo)
STD_ROM_FN(columnsn)

struct BurnDriver BurnDrvcolumnsn = {
	"columnsn", NULL, "neogeo", NULL, "????",
	"Columns (Neo Geo)\0", NULL, "homebrew", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_PUZZLE, 0,
	NULL, columnsnRomInfo, columnsnRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Poker Night

static struct BurnRomInfo poknightRomDesc[] = {
	{ "2003-p1.bin",  0x080000, 0x750421ee, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "2003-s1.bin",  0x020000, 0xa26d2f09, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "2003-c1.bin",  0x080000, 0x5fb2b761, 3 | BRF_GRA },           //  2 Sprite data
	{ "2003-c2.bin",  0x080000, 0xf2b570e3, 3 | BRF_GRA },           //  3 

	{ "nnp-m1.rom",   0x010000, 0x7669de64, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "nnp-v1.rom",   0x080000, 0x504bf849, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(poknight, poknight, neogeo)
STD_ROM_FN(poknight)

struct BurnDriver BurnDrvpoknight = {
	"poknight", NULL, "neogeo", NULL, "????",
	"Poker Night\0", NULL, "Jeff Kurtz", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_CASINO, 0,
	NULL, poknightRomInfo, poknightRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Neo No Panepon (beta)

static struct BurnRomInfo neonoponRomDesc[] = {
	{ "nnp-p1.rom",   0x080000, 0x8a792271, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "nnp-s1.rom",   0x010000, 0x93e6fe7d, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "nnp-c1.rom",   0x100000, 0x4a718ae3, 3 | BRF_GRA },           //  2 Sprite data
	{ "nnp-c2.rom",   0x100000, 0x0e2cbc25, 3 | BRF_GRA },           //  3 

	{ "nnp-m1.rom",   0x010000, 0x7669de64, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "nnp-v1.rom",   0x080000, 0x504bf849, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(neonopon, neonopon, neogeo)
STD_ROM_FN(neonopon)

struct BurnDriver BurnDrvneonopon = {
	"neonopon", NULL, "neogeo", NULL, "????",
	"Neo No Panepon (beta)\0", NULL, "blastar@gmx.net", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_PUZZLE, 0,
	NULL, neonoponRomInfo, neonoponRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Neo Pong

static struct BurnRomInfo neopongRomDesc[] = {
	{ "pong_p1.rom",  0x020000, 0x9f35e29d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "pong_s1.rom",  0x020000, 0xcd19264f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "pong_c1.rom",  0x100000, 0xf5a57d1a, 3 | BRF_GRA },           //  2 Sprite data
	{ "pong_c2.rom",  0x100000, 0xaffde64e, 3 | BRF_GRA },           //  3 

	{ "pong_m1.rom",  0x020000, 0x78c851cb, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "pong_v1.rom",  0x200000, 0x6c31223c, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(neopong, neopong, neogeo)
STD_ROM_FN(neopong)

struct BurnDriver BurnDrvneopong = {
	"neopong", NULL, "neogeo", NULL, "2002",
	"Neo Pong (ver 1.1)\0", NULL, "Neo Dev Corporation", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_BALLPADDLE, 0,
	NULL, neopongRomInfo, neopongRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Neo Pong (v1.0)

static struct BurnRomInfo neopongaRomDesc[] = {
	{ "np10_p1.rom",  0x020000, 0x31b724d7, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "pong_s1.rom",  0x020000, 0xcd19264f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "np10_c1.rom",  0x100000, 0xd7587282, 3 | BRF_GRA },           //  2 Sprite data
	{ "np10_c2.rom",  0x100000, 0xfefc9d06, 3 | BRF_GRA },           //  3 

	{ "np10_m1.rom",  0x020000, 0x9c0291ea, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "np10_v1.rom",  0x080000, 0xdebeb8fb, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(neoponga, neoponga, neogeo)
STD_ROM_FN(neoponga)

struct BurnDriverD BurnDrvneoponga = {
	"neoponga", "neopong", "neogeo", NULL, "2002",
	"Neo Pong (ver 1.0)\0", NULL, "Neo Dev Corporation", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_BALLPADDLE, 0,
	NULL, neopongaRomInfo, neopongaRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Jonas Indiana and the Lost Temple of RA (20050717)

static struct BurnRomInfo ltorb1RomDesc[] = {
	{ "nnp-p1.rom",   0x040000, 0x353f6b05, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "nnp-s1.rom",   0x010000, 0x8224d8b7, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "nnp-c1.rom",   0x080000, 0x8b607198, 3 | BRF_GRA },           //  2 Sprite data
	{ "nnp-c2.rom",   0x080000, 0x6b96b59c, 3 | BRF_GRA },           //  3 

	{ "nnp-m1.rom",   0x010000, 0x6847fd52, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "nnp-v1.rom",   0x080000, 0x0329c2e7, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(ltorb1, ltorb1, neogeo)
STD_ROM_FN(ltorb1)

struct BurnDriver BurnDrvltorb1 = {
	"ltorb1", NULL, "neogeo", NULL, "2005",
	"Jonas Indiana and the Lost Temple of RA (20050717)\0", NULL, "blastar@gmx.net", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_PLATFORM, 0,
	NULL, ltorb1RomInfo, ltorb1RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Codename - Blut Engel (2006-01-19)

static struct BurnRomInfo cnbeRomDesc[] = {
	{ "cnbe-p1.bin",  0x080000, 0x7ece4186, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "cnbe-s1.bin",  0x010000, 0x40a4698f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "cnbe-c1.bin",  0x100000, 0x0f4fea6f, 3 | BRF_GRA },           //  2 Sprite data
	{ "cnbe-c2.bin",  0x100000, 0x3d5fc36f, 3 | BRF_GRA },           //  3 

	{ "cnbe-m1.bin",  0x020000, 0xa5821c9c, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "cnbe-v1.bin",  0x080000, 0x70191764, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(cnbe, cnbe, neogeo)
STD_ROM_FN(cnbe)

struct BurnDriver BurnDrvcnbe = {
	"cnbe", NULL, "neogeo", NULL, "2006",
	"Codename - Blut Engel (2006-01-19)\0", NULL, "blastar@gmx.net", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_VERSHOOT, 0,
	NULL, cnbeRomInfo, cnbeRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// NGEM2K (beta 2006-01-18)

static struct BurnRomInfo ngem2kRomDesc[] = {
	{ "ngem-p1.bin",  0x080000, 0xf2c42fe3, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "ngem-s1.bin",  0x010000, 0xfb3c02d2, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "ngem-c1.bin",  0x100000, 0xa8e2ab47, 3 | BRF_GRA },           //  2 Sprite data
	{ "ngem-c2.bin",  0x100000, 0xa738ea1c, 3 | BRF_GRA },           //  3 

	{ "ngem-m1.bin",  0x020000, 0xb1f44a15, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "ngem-v1.bin",  0x080000, 0xc32199f0, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(ngem2k, ngem2k, neogeo)
STD_ROM_FN(ngem2k)

struct BurnDriver BurnDrvngem2k = {
	"ngem2k", NULL, "neogeo", NULL, "2006",
	"NGEM2K (beta 2006-01-18)\0", NULL, "homebrew", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HOMEBREW, 2, HARDWARE_SNK_NEOGEO, GBF_PUZZLE, 0,
	NULL, ngem2kRomInfo, ngem2kRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Idol Mahjong - final romance 2 (CD to MVS Conversion)

static struct BurnRomInfo fr2chRomDesc[] = {
	{ "098-p1ch.bin", 0x080000, 0x9aa8cee0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "098-s1ch.bin", 0x020000, 0x764ac7aa, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "098-c1ch.bin", 0x100000, 0x6158cf4a, 3 | BRF_GRA },           //  2 Sprite data
	{ "098-c2ch.bin", 0x100000, 0x93a809a3, 3 | BRF_GRA },           //  3 
	{ "098-c3ch.bin", 0x100000, 0xfafa3381, 3 | BRF_GRA },           //  4
	{ "098-c4ch.bin", 0x100000, 0x9895e23f, 3 | BRF_GRA },           //  5 
	{ "098-c5ch.bin", 0x100000, 0xeeaaa818, 3 | BRF_GRA },           //  6 
	{ "098-c6ch.bin", 0x100000, 0xf3d9a190, 3 | BRF_GRA },           //  7 

	{ "098-m1ch.bin", 0x010000, 0xa455fa31, 4 | BRF_ESS | BRF_PRG }, //  8 Z80 code

	{ "098-v1ch.bin", 0x100000, 0x92e175f0, 5 | BRF_SND },           //  9 Sound data
};

STDROMPICKEXT(fr2ch, fr2ch, neogeo)
STD_ROM_FN(fr2ch)

static void fr2chCallback()
{
	UINT16 *src = (UINT16*)Neo68KROM;
	UINT8 *rom = Neo68KROM;

	int i;
	UINT8 data[16] = {
		0x49, 0x46, 0x41, 0x4E, 0x20, 0x4C, 0x4F, 0x52,
		0x41, 0x4D, 0x43, 0x4E, 0x20, 0x45, 0x20, 0x32
	};

	// change jsr to C004DA
	src[0x01AF8 >> 1] = 0x04DA; // C00552 (Not used?)
	src[0x01BF6 >> 1] = 0x04DA; // C0056A (fixes crash)
	src[0x01ED8 >> 1] = 0x04DA; // C00570 (Not used?)
	src[0x1C384 >> 1] = 0x04DA; // C00552 (fixes crash) 

	// 0x001C06 - this routine can cause a loop/freeze
	src[0x01C06 >> 1] = 0x4E75;

	// can cause bugs
	// Move text for credit + coin info (Thanks to Kanyero)
	memcpy (NeoTextROM + 0x20000, NeoTextROM + 0x20600, 0x140);

	// optional
	// Hack in the proper identification (see setup menu [F2])
	for (i = 0; i < 0x10; i++)
	{
		rom[0x3A6 + i] = rom[0x61E + i] = rom[0x896 + i] = data[i];
	}
	for (i = 0; i < 0x20; i += 4)
	{
		src[(0x40 + i + 2) >> 1] = 0x0426;
	}

	// Album Fix
	src[0x1C382 >> 1] = 0x0008; // C00552
	src[0x1C384 >> 1] = 0x0000;
	src[0x80000 >> 1] = 0x33FC;
	src[0x80002 >> 1] = 0x0001;
	src[0x80004 >> 1] = 0x0020;
	src[0x80006 >> 1] = 0x0002;
	src[0x80008 >> 1] = 0x4E75;
}

static int fr2chInit()
{
	pNeoInitCallback = fr2chCallback;

	return NeoInit();
}

struct BurnDriver BurnDrvfr2ch = {
	"fr2ch", NULL, "neogeo", NULL, "1995",
	"Idol Mahjong - final romance 2 (CD to MVS Conversion)\0", NULL, "hack", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HACK, 2, HARDWARE_SNK_NEOGEO, GBF_MAHJONG, 0,
	NULL, fr2chRomInfo, fr2chRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	fr2chInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Shadow of the Beast (Neo Geo demo)

static struct BurnRomInfo beastRomDesc[] = {
	{ "2002-p1.bin",  0x080000, 0x1f9e62a4, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "2002-s1.bin",  0x020000, 0x05a417f5, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "2002-c1.bin",  0x080000, 0xe1974741, 3 | BRF_GRA },           //  2 Sprite data
	{ "2002-c2.bin",  0x080000, 0x9f5b9cf6, 3 | BRF_GRA },           //  3 

	{ "nnp-m1.rom",   0x010000, 0x7669de64, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "nnp-v1.rom",   0x080000, 0x504bf849, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(beast, beast, neogeo)
STD_ROM_FN(beast)

struct BurnDriver BurnDrvbeast = {
	"beast", NULL, "neogeo", NULL, "????",
	"Shadow of the Beast (Neo Geo demo)\0", NULL, "Jeff Kurtz", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, beastRomInfo, beastRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};


// Chip n Dale (Intro demo)

static struct BurnRomInfo cndiRomDesc[] = {
	{ "cnd-p1.bin",   0x800000, 0x2df9a41d, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "cnd-s1.bin",   0x010000, 0x63b8b25e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "cnd-c1.bin",   0x400000, 0x55bd32fc, 3 | BRF_GRA },           //  2 Sprite data
	{ "cnd-c2.bin",   0x400000, 0xbbb43733, 3 | BRF_GRA },           //  3 
	{ "cnd-c3.bin",   0x400000, 0x715e0318, 3 | BRF_GRA },           //  4
	{ "cnd-c4.bin",   0x400000, 0x016f3c7e, 3 | BRF_GRA },           //  5 
	{ "cnd-c5.bin",   0x400000, 0x28c6cb8c, 3 | BRF_GRA },           //  6 
	{ "cnd-c6.bin",   0x400000, 0x9c89503f, 3 | BRF_GRA },           //  7 
	{ "cnd-c7.bin",   0x400000, 0x074dd407, 3 | BRF_GRA },           //  8
	{ "cnd-c8.bin",   0x400000, 0x5ddb7db5, 3 | BRF_GRA },           //  9 
	
	{ "cnd-m1.bin",   0x002000, 0xae120a59, 4 | BRF_ESS | BRF_PRG }, //  10 Z80 code

	{ "cnd-v1.bin",   0x092000, 0x91d45537, 5 | BRF_SND },           //  11 Sound data
};

STDROMPICKEXT(cndi, cndi, neogeo)
STD_ROM_FN(cndi)

struct BurnDriverD BurnDrvcndi = {
	"cndi", NULL, "neogeo", NULL, "2009",
	"Chip n Dale (Intro demo)\0", NULL, "Demo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, cndiRomInfo, cndiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// DarkWing Duck (Intro demo)

static struct BurnRomInfo dwiRomDesc[] = {
	{ "dwi-p1.bin",   0x800000, 0x8e3f2ea8, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "dwi-s1.bin",   0x010000, 0x3dc5c1ef, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "dwi-c1.bin",   0x400000, 0x2b8bda94, 3 | BRF_GRA },           //  2 Sprite data
	{ "dwi-c2.bin",   0x400000, 0x5345e4de, 3 | BRF_GRA },           //  3 
	{ "dwi-c3.bin",   0x400000, 0xc6185f14, 3 | BRF_GRA },           //  4
	{ "dwi-c4.bin",   0x400000, 0xc9e59701, 3 | BRF_GRA },           //  5 
	{ "dwi-c5.bin",   0x400000, 0x42dd1c90, 3 | BRF_GRA },           //  6 
	{ "dwi-c6.bin",   0x400000, 0x9fa6d651, 3 | BRF_GRA },           //  7 
	{ "dwi-c7.bin",   0x400000, 0x5cfa6886, 3 | BRF_GRA },           //  8
	{ "dwi-c8.bin",   0x400000, 0x59c37871, 3 | BRF_GRA },           //  9 

	{ "dwi-m1.bin",   0x002000, 0x2249e065, 4 | BRF_ESS | BRF_PRG }, //  10 Z80 code

	{ "dwi-v1.bin",   0x09b000, 0x13bb4ce4, 5 | BRF_SND },           //  11 Sound data
};

STDROMPICKEXT(dwi, dwi, neogeo)
STD_ROM_FN(dwi)

struct BurnDriverD BurnDrvdwi = {
	"dwi", NULL, "neogeo", NULL, "2009",
	"DarkWing Duck (Intro demo)\0", NULL, "Demo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, dwiRomInfo, dwiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Duck Tales (Intro demo)

static struct BurnRomInfo duckiRomDesc[] = {
	{ "duck-p1.bin",  0x800000, 0xf975711c, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "duck-s1.bin",  0x010000, 0x63b8b25e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "duck-c1.bin",  0x400000, 0x02adabba, 3 | BRF_GRA },           //  2 Sprite data
	{ "duck-c2.bin",  0x400000, 0x00bd87ac, 3 | BRF_GRA },           //  3 
	{ "duck-c3.bin",  0x400000, 0x263307c8, 3 | BRF_GRA },           //  4
	{ "duck-c4.bin",  0x400000, 0xb8330c1e, 3 | BRF_GRA },           //  5 
	{ "duck-c5.bin",  0x400000, 0xce99fcb8, 3 | BRF_GRA },           //  6 
	{ "duck-c6.bin",  0x400000, 0xa6d30781, 3 | BRF_GRA },           //  7 
	{ "duck-c7.bin",  0x400000, 0xeda15323, 3 | BRF_GRA },           //  8
	{ "duck-c8.bin",  0x400000, 0x400fd4cd, 3 | BRF_GRA },           //  9 

	{ "duck-m1.bin",  0x002000, 0x7761d993, 4 | BRF_ESS | BRF_PRG }, //  10 Z80 code

	{ "duck-v1.bin",  0x090200, 0x24b716ee, 5 | BRF_SND },           //  11 Sound data
};

STDROMPICKEXT(ducki, ducki, neogeo)
STD_ROM_FN(ducki)

struct BurnDriverD BurnDrvducki = {
	"ducki", NULL, "neogeo", NULL, "2009",
	"Duck Tales (Intro demo)\0", NULL, "Demo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, duckiRomInfo, duckiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Ghost Busters (Intro demo)

static struct BurnRomInfo ghostbiRomDesc[] = {
	{ "ghostb-p1.bin",  0x100000, 0xaf69ef5f, 1 | BRF_ESS | BRF_PRG }, //  0 68K code
	{ "ghostb-p2.bin",  0x400000, 0xf57e393f, 1 | BRF_ESS | BRF_PRG }, //  1 68K code

	{ "ghostb-s1.bin",  0x010000, 0x63b8b25e, 2 | BRF_GRA },           //  2 Text layer tiles

	{ "ghostb-c1.bin",  0x400000, 0x4281fecd, 3 | BRF_GRA },           //  3 Sprite data
	{ "ghostb-c2.bin",  0x400000, 0x0f07018c, 3 | BRF_GRA },           //  4 
	{ "ghostb-c3.bin",  0x400000, 0xa4589b2a, 3 | BRF_GRA },           //  5
	{ "ghostb-c4.bin",  0x400000, 0x2a72bd04, 3 | BRF_GRA },           //  6 
	{ "ghostb-c5.bin",  0x400000, 0xad3b72f9, 3 | BRF_GRA },           //  7 
	{ "ghostb-c6.bin",  0x400000, 0x963e0941, 3 | BRF_GRA },           //  8 

	{ "ghostb-m1.bin",  0x002000, 0x6624e642, 4 | BRF_ESS | BRF_PRG }, //  9 Z80 code

	{ "ghostb-v1.bin",  0x094a00, 0x44864761, 5 | BRF_SND },           //  10 Sound data
};

STDROMPICKEXT(ghostbi, ghostbi, neogeo)
STD_ROM_FN(ghostbi)

struct BurnDriverD BurnDrvghostbi = {
	"ghostbi", NULL, "neogeo", NULL, "2009",
	"Ghost Busters (Intro demo)\0", NULL, "Demo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, ghostbiRomInfo, ghostbiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Robocop (Intro demo)

static struct BurnRomInfo robocopiRomDesc[] = {
	{ "robo-p1.bin",  0x800000, 0xadcb4fe0, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "robo-s1.bin",  0x010000, 0x63b8b25e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "robo-c1.bin",  0x400000, 0x8cf565a0, 3 | BRF_GRA },           //  2 Sprite data
	{ "robo-c2.bin",  0x400000, 0x76ef561d, 3 | BRF_GRA },           //  3 
	{ "robo-c3.bin",  0x400000, 0x7166790c, 3 | BRF_GRA },           //  4
	{ "robo-c4.bin",  0x400000, 0xc0cdcbfc, 3 | BRF_GRA },           //  5 
	{ "robo-c5.bin",  0x400000, 0xa5ddc363, 3 | BRF_GRA },           //  6 
	{ "robo-c6.bin",  0x400000, 0x1fcc8fce, 3 | BRF_GRA },           //  7 
	{ "robo-c7.bin",  0x400000, 0xbe9f9da2, 3 | BRF_GRA },           //  8
	{ "robo-c8.bin",  0x400000, 0x7cff633a, 3 | BRF_GRA },           //  9 

	{ "robo-m1.bin",  0x002000, 0xc4d35e83, 4 | BRF_ESS | BRF_PRG }, //  10 Z80 code

	{ "robo-v1.bin",  0x096000, 0xecb284a9, 5 | BRF_SND },           //  11 Sound data
};

STDROMPICKEXT(robocopi, robocopi, neogeo)
STD_ROM_FN(robocopi)

struct BurnDriverD BurnDrvrobocopi = {
	"robocopi", NULL, "neogeo", NULL, "2009",
	"Robocop (Intro demo)\0", NULL, "Demo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, robocopiRomInfo, robocopiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Teenage Mutant Ninja Turtles (Intro demo)

static struct BurnRomInfo tmntiRomDesc[] = {
	{ "tmnt-p1.bin",  0x800000, 0x2d5066ff, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "tmnt-s1.bin",  0x010000, 0x3dc5c1ef, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "tmnt-c1.bin",  0x400000, 0xae1735c1, 3 | BRF_GRA },           //  2 Sprite data
	{ "tmnt-c2.bin",  0x400000, 0x344281b5, 3 | BRF_GRA },           //  3 
	{ "tmnt-c3.bin",  0x400000, 0x1058a648, 3 | BRF_GRA },           //  4
	{ "tmnt-c4.bin",  0x400000, 0x1c6ba1b0, 3 | BRF_GRA },           //  5 
	{ "tmnt-c5.bin",  0x400000, 0x73a89fb0, 3 | BRF_GRA },           //  6 
	{ "tmnt-c6.bin",  0x400000, 0xa139246a, 3 | BRF_GRA },           //  7 
	{ "tmnt-c7.bin",  0x400000, 0xed957842, 3 | BRF_GRA },           //  8
	{ "tmnt-c8.bin",  0x400000, 0x8c15d91b, 3 | BRF_GRA },           //  9 

	{ "tmnt-m1.bin",  0x002000, 0x953542ef, 4 | BRF_ESS | BRF_PRG }, //  10 Z80 code

	{ "tmnt-v1.bin",  0x094900, 0x273640c3, 5 | BRF_SND },           //  11 Sound data
};

STDROMPICKEXT(tmnti, tmnti, neogeo)
STD_ROM_FN(tmnti)

struct BurnDriverD BurnDrvtmnti = {
	"tmnti", NULL, "neogeo", NULL, "2009",
	"Teenage Mutant Ninja Turtles (Intro demo)\0", NULL, "Demo", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, tmntiRomInfo, tmntiRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Neo System Check (ver 1.0b)

static struct BurnRomInfo syscheckRomDesc[] = {
	{ "sys-p1.rom",   0x080000, 0xa4adbe1e, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "sys-s1.rom",   0x010000, 0x4774f28e, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "sys-c1.rom",   0x080000, 0x75660aac, 3 | BRF_GRA },           //  2 Sprite data
	{ "sys-c2.rom",   0x080000, 0x75660aac, 3 | BRF_GRA },           //  3 

	{ "sys-m1.rom",   0x010000, 0x7669de64, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "sys-v1.rom",   0x080000, 0x504bf849, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(syscheck, syscheck, neogeo)
STD_ROM_FN(syscheck)

struct BurnDriver BurnDrvsyscheck = {
	"syscheck", NULL, "neogeo", NULL, "????",
	"Neo System Check (ver 1.0b)\0", NULL, "blastar@gmx.net", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, syscheckRomInfo, syscheckRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Neo Demo
static struct BurnRomInfo neodemoRomDesc[] = {
	{ "demo-p1.bin",  0x080000, 0xfbfce2a4, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "demo-s1.bin",  0x020000, 0xcd19264f, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "demo-c1.bin",  0x100000, 0x870b28ea, 3 | BRF_GRA },           //  2 Sprite data
	{ "demo-c2.bin",  0x100000, 0x1ecf8128, 3 | BRF_GRA },           //  3 

	{ "202-m1.bin",   0x020000, 0x9c0291ea, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "202-v1.bin",   0x080000, 0xdebeb8fb, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(neodemo, neodemo, neogeo)
STD_ROM_FN(neodemo)

struct BurnDriver BurnDrvneodemo = {
	"neodemo", NULL, "neogeo", NULL, "2002",
	"Chaos Demo (Neo Geo)\0", NULL, "Chaos", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, neodemoRomInfo, neodemoRomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Neo 2500 Demo

static struct BurnRomInfo neo2500RomDesc[] = {
	{ "2500-p1.bin",  0x080000, 0x2321c0d8, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "2500-s1.bin",  0x010000, 0x1a37925c, 2 | BRF_GRA },           //  1 Text layer tiles

	{ "2500-c1.bin",  0x080000, 0x8c304b56, 3 | BRF_GRA },           //  2 Sprite data
	{ "2500-c2.bin",  0x080000, 0x9a0758d6, 3 | BRF_GRA },           //  3 

	{ "202-m1.bin",   0x020000, 0x9c0291ea, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "202-v1.bin",   0x080000, 0xdebeb8fb, 5 | BRF_SND },           //  5 Sound data
};

STDROMPICKEXT(neo2500, neo2500, neogeo)
STD_ROM_FN(neo2500)

struct BurnDriver BurnDrvneo2500 = {
	"neo2500", NULL, "neogeo", NULL, "2004",
	"Neo 2500 Demo\0", NULL, "blastar@gmx.net", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 2, HARDWARE_SNK_NEOGEO, GBF_MISC, 0,
	NULL, neo2500RomInfo, neo2500RomName, NULL, NULL, neogeoInputInfo, neogeoDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};

// Neo Geo 4 player test cartridge

static struct BurnRomInfo fourplayRomDesc[] = {
	{ "4play_p1.rom", 0x100000, 0x3C3D741B, 1 | BRF_ESS | BRF_PRG }, //  0 68K code

	{ "4play_s1.rom", 0x020000, 0xDE828076, 2 | BRF_GRA },			 //  1 Text data

	{ "4play_c1.rom", 0x020000, 0x7EE8CDCD, 3 | BRF_GRA },			 //  2 Sprite data
	{ "4play_c2.rom", 0x020000, 0x7EE8CDCD, 3 | BRF_GRA },			 //  3

	{ "4play_m1.rom", 0x020000, 0x5132580E, 4 | BRF_ESS | BRF_PRG }, //  4 Z80 code

	{ "4play_v1.rom", 0x020000, 0x7EE8CDCD, 5 | BRF_SND },			 //  5 Sound data
};

STDROMPICKEXT(fourplay, fourplay, neogeo)
STD_ROM_FN(fourplay)

struct BurnDriver BurnDrv4play = {
	"4play", NULL, "neogeo", NULL, "???",
	"4 player input test\0", "4 player  input test cartridge", "SNK", "Neo Geo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_DEMO, 4, HARDWARE_SNK_NEOGEO | HARDWARE_SNK_4_JOYSTICKS, GBF_MISC, 0,
	NULL, fourplayRomInfo, fourplayRomName, NULL, NULL, neodualInputInfo, neodualDIPInfo,
	NeoInit, NeoExit, NeoFrame, NeoRender, NeoScan, &NeoRecalcPalette, 0x1000,
	304, 224, 4, 3
};
