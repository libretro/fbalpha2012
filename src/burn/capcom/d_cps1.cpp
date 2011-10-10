// FB Alpha CPS-1 & CPS Changer driver module

// Maintained by Creamymami && KOF2112

#include "cps.h"
#include "bitswap.h"

//----------------------------------------------------------------------------------------------
// Game inputs

static struct BurnInputInfo NTFOInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , {CpsInp018+0}, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , {CpsInp018+4}, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , {CpsInp001+3}, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , {CpsInp001+2}, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , {CpsInp001+1}, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , {CpsInp001+0}, "p1 right"  },
	{"P1 Shot"          , BIT_DIGITAL  , {CpsInp001+4}, "p1 fire 1" },
	{"P1 Special"       , BIT_DIGITAL  , {CpsInp001+5}, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , {CpsInp018+1}, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , {CpsInp018+5}, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , {CpsInp000+3}, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , {CpsInp000+2}, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , {CpsInp000+1}, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , {CpsInp000+0}, "p2 right"  },
	{"P2 Shot"          , BIT_DIGITAL  , {CpsInp000+4}, "p2 fire 1" },
	{"P2 Special"       , BIT_DIGITAL  , {CpsInp000+5}, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , {&CpsReset},   "reset"     },
	{"Service"          , BIT_DIGITAL  , {CpsInp018+2}, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, {&Cpi01A}    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, {&Cpi01C}    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, {&Cpi01E}    , "dip"       },
};

STDINPUTINFO(NTFO)

static struct BurnInputInfo wonders3InputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Button 1"      , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Button 2"      , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Button 1"      , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Button 2"      , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wonders3)

static struct BurnInputInfo captcommInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"P4 Coin"          , BIT_DIGITAL  , CpsInp179+6, "p4 coin"   },
	{"P4 Start"         , BIT_DIGITAL  , CpsInp179+7, "p4 start"  },
	{"P4 Up"            , BIT_DIGITAL  , CpsInp179+3, "p4 up"     },
	{"P4 Down"          , BIT_DIGITAL  , CpsInp179+2, "p4 down"   },
	{"P4 Left"          , BIT_DIGITAL  , CpsInp179+1, "p4 left"   },
	{"P4 Right"         , BIT_DIGITAL  , CpsInp179+0, "p4 right"  },
	{"P4 Attack"        , BIT_DIGITAL  , CpsInp179+4, "p4 fire 1" },
	{"P4 Jump"          , BIT_DIGITAL  , CpsInp179+5, "p4 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(captcomm)

static struct BurnInputInfo cawingInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(cawing)

static struct BurnInputInfo cworld2jInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Answer 1"      , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Answer 2"      , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Answer 3"      , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Answer 4"      , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Answer 1"      , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Answer 2"      , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Answer 3"      , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Answer 4"      , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(cworld2j)

static struct BurnInputInfo dinoInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0 , "p1 coin"  },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4 , "p1 start" },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3 , "p1 up"    },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2 , "p1 down"  },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1 , "p1 left"  },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0 , "p1 right" },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4 , "p1 fire 1"},
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5 , "p1 fire 2"},

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1 , "p2 coin"  },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5 , "p2 start" },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3 , "p2 up"    },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2 , "p2 down"  },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1 , "p2 left"  },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0 , "p2 right" },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4 , "p2 fire 1"},
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5 , "p2 fire 2"},

	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc001+6, "p3 coin"  },
	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},

	{"Reset"            , BIT_DIGITAL  , &CpsReset   ,  "reset"   },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6 ,  "diag"    },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2 ,  "service" },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E     ,  "dip"     },
};

STDINPUTINFO(dino)

static struct BurnInputInfo dynwarInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack Left"   , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Attack Right"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack Left"   , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Attack Right"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(dynwar)

static struct BurnInputInfo ffightInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(ffight)

#define A(a, b, c, d) {a, b, (unsigned char*)(c), d}

static struct BurnInputInfo forgottnInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL,    CpsInp018+0, "p1 coin"  },
	{"P1 Start"         , BIT_DIGITAL,    CpsInp018+4, "p1 start" },
	{"P1 Up"            , BIT_DIGITAL,    CpsInp001+3, "p1 up"    },
	{"P1 Down"          , BIT_DIGITAL,    CpsInp001+2, "p1 down"  },
	{"P1 Left"          , BIT_DIGITAL,    CpsInp001+1, "p1 left"  },
	{"P1 Right"         , BIT_DIGITAL,    CpsInp001+0, "p1 right" },
	{"P1 Attack"        , BIT_DIGITAL,    CpsInp001+4, "p1 fire 1"},
	A("P1 Turn"         , BIT_ANALOG_REL, &CpsInp055,  "p1 z-axis"),

	{"P2 Coin"          , BIT_DIGITAL,    CpsInp018+1, "p2 coin"  },
	{"P2 Start"         , BIT_DIGITAL,    CpsInp018+5, "p2 start" },
	{"P2 Up"            , BIT_DIGITAL,    CpsInp000+3, "p2 up"    },
	{"P2 Down"          , BIT_DIGITAL,    CpsInp000+2, "p2 down"  },
	{"P2 Left"          , BIT_DIGITAL,    CpsInp000+1, "p2 left"  },
	{"P2 Right"         , BIT_DIGITAL,    CpsInp000+0, "p2 right" },
	{"P2 Attack"        , BIT_DIGITAL,    CpsInp000+4, "p2 fire 1"},
	A("P2 Turn"         , BIT_ANALOG_REL, &CpsInp05d,  "p2 z-axis"),

	{"Reset"            , BIT_DIGITAL,    &CpsReset,   "reset"    },
	{"Service"          , BIT_DIGITAL,    CpsInp018+2, "service"  },

	{"Dip A"            , BIT_DIPSWITCH,  &Cpi01A    , "dip"      },
	{"Dip B"            , BIT_DIPSWITCH,  &Cpi01C    , "dip"      },
	{"Dip C"            , BIT_DIPSWITCH,  &Cpi01E    , "dip"      },
};

#undef A

STDINPUTINFO(forgottn)

static struct BurnInputInfo ghoulsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(ghouls)

static struct BurnInputInfo knightsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(knights)

static struct BurnInputInfo kodInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(kod)

static struct BurnInputInfo megamanInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Select"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Select"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(megaman)

static struct BurnInputInfo mercsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Shot"          , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Special"       , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(mercs)

static struct BurnInputInfo mswordInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(msword)

static struct BurnInputInfo mtwinsInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(mtwins)

static struct BurnInputInfo nemoInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(nemo)

static struct BurnInputInfo pang3InputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot 1"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Shot 2"        , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot 1"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Shot 2"        , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
};

STDINPUTINFO(pang3)

static struct BurnInputInfo pang3nInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot 1"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Shot 2"        , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot 1"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Shot 2"        , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(pang3n)

static struct BurnInputInfo pnickjInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Turn 1"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Turn 2"        , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Turn 1"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Turn 2"        , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(pnickj)

static struct BurnInputInfo punisherInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(punisher)

static struct BurnInputInfo qadInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Answer 1"      , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Answer 2"      , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Answer 3"      , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Answer 4"      , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Answer 1"      , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Answer 2"      , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Answer 3"      , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Answer 4"      , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(qad)

static struct BurnInputInfo sf2InputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp177+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp177+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp177+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp177+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp177+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp177+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(sf2)

static struct BurnInputInfo Sf2ueInputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp1fd+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp1fd+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp1fd+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp1fd+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp1fd+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp1fd+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2ue)

static struct BurnInputInfo Sf2m1InputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp012+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp012+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp012+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp012+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp012+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp012+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2m1)

static struct BurnInputInfo Sf2m3InputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp029+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp029+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp011+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp011+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp011+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp011+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp011+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp011+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp011+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp186+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp186+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp186+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp029+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp029+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp010+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp010+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp010+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp010+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp010+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp010+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp010+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp186+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp186+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp186+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp029+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp029+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2m3)

static struct BurnInputInfo Sf2yycInputList[] = {
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp019+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp019+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp176+0, "p1 fire 4" },
	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp176+1, "p1 fire 5" },
	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp176+2, "p1 fire 6" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp019+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp019+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp176+4, "p2 fire 4" },
	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp176+5, "p2 fire 5" },
	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp176+6, "p2 fire 6" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp019+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp019+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(Sf2yyc)

static struct BurnInputInfo SfzchInputList[] =
{
	{"P1 Pause"         , BIT_DIGITAL  , CpsInp018+2, "p1 select" },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Weak Punch"    , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Medium Punch"  , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Strong Punch"  , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
 	{"P1 Weak Kick"     , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },
 	{"P1 Medium Kick"   , BIT_DIGITAL  , CpsInp018+0, "p1 fire 5" },
 	{"P1 Strong Kick"   , BIT_DIGITAL  , CpsInp018+6, "p1 fire 6" },

 	{"P2 Pause"         , BIT_DIGITAL  , CpsInp018+3, "p2 select" },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Weak Punch"    , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Medium Punch"  , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Strong Punch"  , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
 	{"P2 Weak Kick"     , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },
 	{"P2 Medium Kick"   , BIT_DIGITAL  , CpsInp018+1, "p2 fire 5" },
 	{"P2 Strong Kick"   , BIT_DIGITAL  , CpsInp018+7, "p2 fire 6" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
};

STDINPUTINFO(Sfzch)

static struct BurnInputInfo slammastInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Pin"           , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Pin"           , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc001+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},
 	{"P3 Pin"           , BIT_DIGITAL  , CpsInp001+7 , "p3 fire 3"},

 	{"P4 Coin"          , BIT_DIGITAL  , CpsInpc003+6, "p4 coin"  },
 	{"P4 Start"         , BIT_DIGITAL  , CpsInpc003+7, "p4 start" },
 	{"P4 Up"            , BIT_DIGITAL  , CpsInpc003+3, "p4 up"    },
 	{"P4 Down"          , BIT_DIGITAL  , CpsInpc003+2, "p4 down"  },
 	{"P4 Left"          , BIT_DIGITAL  , CpsInpc003+1, "p4 left"  },
 	{"P4 Right"         , BIT_DIGITAL  , CpsInpc003+0, "p4 right" },
 	{"P4 Attack"        , BIT_DIGITAL  , CpsInpc003+4, "p4 fire 1"},
 	{"P4 Jump"          , BIT_DIGITAL  , CpsInpc003+5, "p4 fire 2"},
 	{"P4 Pin"           , BIT_DIGITAL  , CpsInp000+7 , "p4 fire 3"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset   , "reset"    },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6 , "diag"     },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2 , "service"  },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E     , "dip"      },
};

STDINPUTINFO(slammast)

static struct BurnInputInfo striderInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(strider)

static struct BurnInputInfo unsquadInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },

};

STDINPUTINFO(unsquad)

static struct BurnInputInfo varthInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Bomb"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Bomb"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(varth)

static struct BurnInputInfo willowInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
 	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(willow)

static struct BurnInputInfo wofInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc001+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wof)

static struct BurnInputInfo cawingbInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp008+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp008+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Shot"          , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Fire 3"        , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp008+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp008+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Shot"          , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Fire 3"        , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp008+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp008+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(cawingb)

static struct BurnInputInfo dinohInputList[]=
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0 , "p1 coin"  },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4 , "p1 start" },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3 , "p1 up"    },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2 , "p1 down"  },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1 , "p1 left"  },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0 , "p1 right" },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4 , "p1 fire 1"},
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5 , "p1 fire 2"},
	{"P1 Bomb"          , BIT_DIGITAL  , CpsInp001+6 , "p1 fire 3"},

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1 , "p2 coin"  },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5 , "p2 start" },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3 , "p2 up"    },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2 , "p2 down"  },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1 , "p2 left"  },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0 , "p2 right" },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4 , "p2 fire 1"},
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5 , "p2 fire 2"},
	{"P2 Bomb"          , BIT_DIGITAL  , CpsInp000+6 , "p2 fire 3"},

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6,  "p3 coin"  },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7,  "p3 start" },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3,  "p3 up"    },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2,  "p3 down"  },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1,  "p3 left"  },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0,  "p3 right" },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4,  "p3 fire 1"},
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5,  "p3 fire 2"},

	{"Reset"            , BIT_DIGITAL  , &CpsReset   ,  "reset"   },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6 ,  "diag"    },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2 ,  "service" },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(dinoh)

static struct BurnInputInfo dinohbInputList[]=
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0 , "p1 coin"  },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4 , "p1 start" },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3 , "p1 up"    },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2 , "p1 down"  },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1 , "p1 left"  },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0 , "p1 right" },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4 , "p1 fire 1"},
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5 , "p1 fire 2"},
	{"P1 Bomb"          , BIT_DIGITAL  , CpsInp001+6 , "p1 fire 3"},

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1 , "p2 coin"  },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5 , "p2 start" },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3 , "p2 up"    },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2 , "p2 down"  },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1 , "p2 left"  },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0 , "p2 right" },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4 , "p2 fire 1"},
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5 , "p2 fire 2"},
	{"P2 Bomb"          , BIT_DIGITAL  , CpsInp000+6 , "p2 fire 3"},

	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc001+6, "p3 coin"  },
	{"P3 Start"         , BIT_DIGITAL  , CpsInpc001+7, "p3 start" },
	{"P3 Up"            , BIT_DIGITAL  , CpsInpc001+3, "p3 up"    },
	{"P3 Down"          , BIT_DIGITAL  , CpsInpc001+2, "p3 down"  },
	{"P3 Left"          , BIT_DIGITAL  , CpsInpc001+1, "p3 left"  },
	{"P3 Right"         , BIT_DIGITAL  , CpsInpc001+0, "p3 right" },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc001+4, "p3 fire 1"},
	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc001+5, "p3 fire 2"},

	{"Reset"            , BIT_DIGITAL  , &CpsReset   ,  "reset"   },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6 ,  "diag"    },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2 ,  "service" },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E     ,  "dip"     },
};

STDINPUTINFO(dinohb)

static struct BurnInputInfo knightshInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Super"         , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Super"         , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(knightsh)

static struct BurnInputInfo kodhInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Super"         , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Super"         , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"P3 Coin"          , BIT_DIGITAL  , CpsInpc000+6, "p3 coin"   },
	{"P3 Start"         , BIT_DIGITAL  , CpsInpc000+7, "p3 start"  },
	{"P3 Up"            , BIT_DIGITAL  , CpsInpc000+3, "p3 up"     },
	{"P3 Down"          , BIT_DIGITAL  , CpsInpc000+2, "p3 down"   },
	{"P3 Left"          , BIT_DIGITAL  , CpsInpc000+1, "p3 left"   },
	{"P3 Right"         , BIT_DIGITAL  , CpsInpc000+0, "p3 right"  },
	{"P3 Attack"        , BIT_DIGITAL  , CpsInpc000+4, "p3 fire 1" },
	{"P3 Jump"          , BIT_DIGITAL  , CpsInpc000+5, "p3 fire 2" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(kodh)

static struct BurnInputInfo punisherhInputList[] =
{
	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
	{"P1 Super"         , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
	{"P2 Super"         , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
};

STDINPUTINFO(punisherh)

static struct BurnInputInfo wofhInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp006+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp006+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp006+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp006+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp007+6, "p3 coin"  }, // doesn't work
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp007+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp007+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp007+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp007+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp007+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp007+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp007+5, "p3 fire 2"},
 	{"P3 Fire"          , BIT_DIGITAL  , CpsInp007+6, "p3 fire 3"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp006+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp006+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wofh)

static struct BurnInputInfo wof3jsInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wof3js)

static struct BurnInputInfo wof3sjInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp008+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp008+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp008+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp008+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp009+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp009+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp009+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp009+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp009+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp009+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp009+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp009+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp008+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp008+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wof3sj)

static struct BurnInputInfo wofchInputList[] =
{
	{"P1 Pause"         , BIT_DIGITAL  , CpsInp018+2, "p1 select" },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Special"       , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },
 	{"P1 Fire 4"        , BIT_DIGITAL  , CpsInp001+7, "p1 fire 4" },
 	{"P1 Fire 5"        , BIT_DIGITAL  , CpsInp018+0, "p1 fire 5" },
 	{"P1 Fire 6"        , BIT_DIGITAL  , CpsInp018+6, "p1 fire 6" },

 	{"P2 Pause"         , BIT_DIGITAL  , CpsInp018+3, "p2 select" },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Special"       , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },
 	{"P2 Fire 4"        , BIT_DIGITAL  , CpsInp000+7, "p2 fire 4" },
 	{"P2 Fire 5"        , BIT_DIGITAL  , CpsInp018+1, "p2 fire 5" },
 	{"P2 Fire 6"        , BIT_DIGITAL  , CpsInp018+7, "p2 fire 6" },

 	{"Reset"            , BIT_DIGITAL  , &CpsReset  , "reset"     },
};

STDINPUTINFO(wofch)

static struct BurnInputInfo wofhfhInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },
 	{"P1 Fire"          , BIT_DIGITAL  , CpsInp001+6, "p1 fire 3" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },
 	{"P2 Fire"          , BIT_DIGITAL  , CpsInp000+6, "p2 fire 3" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp177+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp177+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp177+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp177+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp177+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp177+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp177+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp177+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
	{"Dip A"            , BIT_DIPSWITCH, &Cpi01A    , "dip"       },
	{"Dip B"            , BIT_DIPSWITCH, &Cpi01C    , "dip"       },
	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wofhfh)

static struct BurnInputInfo wofsjInputList[]=
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp008+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp008+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp008+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp008+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp009+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp009+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp009+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp009+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp009+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp009+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp009+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp009+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp008+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp008+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wofsj)

static struct BurnInputInfo wofsjbInputList[] =
{
 	{"P1 Coin"          , BIT_DIGITAL  , CpsInp018+0, "p1 coin"   },
 	{"P1 Start"         , BIT_DIGITAL  , CpsInp018+4, "p1 start"  },
 	{"P1 Up"            , BIT_DIGITAL  , CpsInp001+3, "p1 up"     },
 	{"P1 Down"          , BIT_DIGITAL  , CpsInp001+2, "p1 down"   },
 	{"P1 Left"          , BIT_DIGITAL  , CpsInp001+1, "p1 left"   },
 	{"P1 Right"         , BIT_DIGITAL  , CpsInp001+0, "p1 right"  },
 	{"P1 Attack"        , BIT_DIGITAL  , CpsInp001+4, "p1 fire 1" },
 	{"P1 Jump"          , BIT_DIGITAL  , CpsInp001+5, "p1 fire 2" },

 	{"P2 Coin"          , BIT_DIGITAL  , CpsInp018+1, "p2 coin"   },
 	{"P2 Start"         , BIT_DIGITAL  , CpsInp018+5, "p2 start"  },
 	{"P2 Up"            , BIT_DIGITAL  , CpsInp000+3, "p2 up"     },
 	{"P2 Down"          , BIT_DIGITAL  , CpsInp000+2, "p2 down"   },
 	{"P2 Left"          , BIT_DIGITAL  , CpsInp000+1, "p2 left"   },
 	{"P2 Right"         , BIT_DIGITAL  , CpsInp000+0, "p2 right"  },
 	{"P2 Attack"        , BIT_DIGITAL  , CpsInp000+4, "p2 fire 1" },
 	{"P2 Jump"          , BIT_DIGITAL  , CpsInp000+5, "p2 fire 2" },

 	{"P3 Coin"          , BIT_DIGITAL  , CpsInp01b+6, "p3 coin"  },
 	{"P3 Start"         , BIT_DIGITAL  , CpsInp01b+7, "p3 start" },
 	{"P3 Up"            , BIT_DIGITAL  , CpsInp01b+3, "p3 up"    },
 	{"P3 Down"          , BIT_DIGITAL  , CpsInp01b+2, "p3 down"  },
 	{"P3 Left"          , BIT_DIGITAL  , CpsInp01b+1, "p3 left"  },
 	{"P3 Right"         , BIT_DIGITAL  , CpsInp01b+0, "p3 right" },
 	{"P3 Attack"        , BIT_DIGITAL  , CpsInp01b+4, "p3 fire 1"},
 	{"P3 Jump"          , BIT_DIGITAL  , CpsInp01b+5, "p3 fire 2"},

 	{"Reset"            , BIT_DIGITAL  , &CpsReset,   "reset"     },
 	{"Diagnostic"       , BIT_DIGITAL  , CpsInp018+6, "diag"      },
 	{"Service"          , BIT_DIGITAL  , CpsInp018+2, "service"   },
 	{"Dip C"            , BIT_DIPSWITCH, &Cpi01E    , "dip"       },
};

STDINPUTINFO(wofsjb)


//----------------------------------------------------------------------------------------------
// Dip switches


#define CPS1_COINAGE_1(dipval) \
	{0     , 0xfe, 0   , 8   , "Coin A"                 }, \
	{dipval, 0x01, 0x07, 0x07, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x06, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x05, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x01, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x07, 0x02, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x07, 0x03, "1 Coin  4 Credits"      }, \
	{dipval, 0x01, 0x07, 0x04, "1 Coin  6 Credits"      }, \
							       \
	{0     , 0xfe, 0   , 8   , "Coin B"                 }, \
	{dipval, 0x01, 0x38, 0x38, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x30, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x28, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x08, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x38, 0x10, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x38, 0x18, "1 Coin  4 Credits"      }, \
	{dipval, 0x01, 0x38, 0x20, "1 Coin  6 Credits"      },

#define CPS1_COINAGE_2(dipval) \
	{0     , 0xfe, 0   , 8   , "Coinage"                }, \
	{dipval, 0x01, 0x07, 0x07, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x06, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x05, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x01, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x07, 0x02, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x07, 0x03, "1 Coin  4 Credits"      }, \
	{dipval, 0x01, 0x07, 0x04, "1 Coin  6 Credits"      },

#define CPS1_COINAGE_3(dipval) \
	{0     , 0xfe, 0   , 8   , "Coin A"                 }, \
	{dipval, 0x01, 0x07, 0x06, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x05, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x04, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x07, "2 Coins 1 Credit (1 to cont)"}, \
	{dipval, 0x01, 0x07, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x07, 0x01, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x07, 0x02, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x07, 0x03, "1 Coin  4 Credits"      }, \
							       \
	{0     , 0xfe, 0   , 8   , "Coin B"                 }, \
	{dipval, 0x01, 0x38, 0x30, "4 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x28, "3 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x20, "2 Coins 1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x38, "2 Coins 1 Credit (1 to cont)"}, \
	{dipval, 0x01, 0x38, 0x00, "1 Coin  1 Credit"       }, \
	{dipval, 0x01, 0x38, 0x08, "1 Coin  2 Credits"      }, \
	{dipval, 0x01, 0x38, 0x10, "1 Coin  3 Credits"      }, \
	{dipval, 0x01, 0x38, 0x18, "1 Coin  4 Credits"      },

#define CPS1_DIFFICULTY_1(dipval) \
	{0   , 0xfe, 0   , 8   , "Difficulty"               }, \
	{dipval, 0x01, 0x07, 0x00, "1 (Easiest)"            }, \
	{dipval, 0x01, 0x07, 0x01, "2"                      }, \
	{dipval, 0x01, 0x07, 0x02, "3"                      }, \
	{dipval, 0x01, 0x07, 0x03, "4 (Normal)"             }, \
	{dipval, 0x01, 0x07, 0x04, "5"                      }, \
	{dipval, 0x01, 0x07, 0x05, "6"                      }, \
	{dipval, 0x01, 0x07, 0x06, "7"                      }, \
	{dipval, 0x01, 0x07, 0x07, "8 (Hardest)"            },

#define CPS1_DIFFICULTY_2(dipval) \
	{0   , 0xfe, 0   , 8   , "Difficulty"               }, \
	{dipval, 0x01, 0x07, 0x03, "1 (Easiest)"            }, \
	{dipval, 0x01, 0x07, 0x02, "2"                      }, \
	{dipval, 0x01, 0x07, 0x01, "3"                      }, \
	{dipval, 0x01, 0x07, 0x00, "4 (Normal)"             }, \
	{dipval, 0x01, 0x07, 0x04, "5"                      }, \
	{dipval, 0x01, 0x07, 0x05, "6"                      }, \
	{dipval, 0x01, 0x07, 0x06, "7"                      }, \
	{dipval, 0x01, 0x07, 0x07, "8 (Hardest)"            },

static struct BurnDIPInfo NTFODIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x12, 0x01, 0x40, 0x00, "Off"                    },
	{0x12, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x13)

	{0   , 0xfe, 0   , 4   , "Level Up Timer"         },
	{0x13, 0x01, 0x18, 0x00, "More Slowly"            },
	{0x13, 0x01, 0x18, 0x08, "Slowly"                 },
	{0x13, 0x01, 0x18, 0x10, "Quickly"                },
	{0x13, 0x01, 0x18, 0x18, "More Quickly"           },

	{0   , 0xfe, 0   , 4   , "Bullet's Speed"         },
	{0x13, 0x01, 0x60, 0x00, "Very Slow"              },
	{0x13, 0x01, 0x60, 0x20, "Slow"                   },
	{0x13, 0x01, 0x60, 0x40, "Fast"                   },
	{0x13, 0x01, 0x60, 0x60, "Very Fast"              },

	{0   , 0xfe, 0   , 2   , "Initital Vitality"      },
	{0x13, 0x01, 0x80, 0x00, "3 bars"                 },
	{0x13, 0x01, 0x80, 0x80, "4 bars"                 },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Throttle Game Speed"    },
	{0x14, 0x01, 0x01, 0x01, "Off"                    },
	{0x14, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x04, 0x00, "Off"                    },
	{0x14, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x14, 0x01, 0x08, 0x00, "Off"                    },
	{0x14, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x14, 0x01, 0x20, 0x00, "Off"                    },
	{0x14, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x00, "Off"                    },
	{0x14, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(NTFO);

static struct BurnDIPInfo threewondersDIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x65, NULL                     },
	{0x15, 0xff, 0xff, 0x66, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x13)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x13, 0x01, 0x40, 0x00, "Off"                    },
	{0x13, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Action Lives"           },
	{0x14, 0x01, 0x03, 0x00, "1"                      },
	{0x14, 0x01, 0x03, 0x01, "2"                      },
	{0x14, 0x01, 0x03, 0x02, "3"                      },
	{0x14, 0x01, 0x03, 0x03, "5"                      },

	{0   , 0xfe, 0   , 4   , "Action Game Level"      },
	{0x14, 0x01, 0x0c, 0x00, "Easy"                   },
	{0x14, 0x01, 0x0c, 0x04, "Normal"                 },
	{0x14, 0x01, 0x0c, 0x08, "Hard"                   },
	{0x14, 0x01, 0x0c, 0x0c, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Shooting Lives"         },
	{0x14, 0x01, 0x30, 0x00, "1"                      },
	{0x14, 0x01, 0x30, 0x10, "2"                      },
	{0x14, 0x01, 0x30, 0x20, "3"                      },
	{0x14, 0x01, 0x30, 0x30, "5"                      },

	{0   , 0xfe, 0   , 4   , "Shooting Game Level"    },
	{0x14, 0x01, 0xc0, 0x00, "Easy"                   },
	{0x14, 0x01, 0xc0, 0x40, "Normal"                 },
	{0x14, 0x01, 0xc0, 0x80, "Hard"                   },
	{0x14, 0x01, 0xc0, 0xc0, "Hardest"                },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Puzzle Lives"           },
	{0x15, 0x01, 0x03, 0x00, "1"                      },
	{0x15, 0x01, 0x03, 0x01, "2"                      },
	{0x15, 0x01, 0x03, 0x02, "3"                      },
	{0x15, 0x01, 0x03, 0x03, "5"                      },

	{0   , 0xfe, 0   , 4   , "Puzzle Game Level"      },
	{0x15, 0x01, 0x0c, 0x00, "Easy"                   },
	{0x15, 0x01, 0x0c, 0x04, "Normal"                 },
	{0x15, 0x01, 0x0c, 0x08, "Hard"                   },
	{0x15, 0x01, 0x0c, 0x0c, "Hardest"                },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x15, 0x01, 0x10, 0x00, "Off"                    },
	{0x15, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x15, 0x01, 0x20, 0x00, "Off"                    },
	{0x15, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x15, 0x01, 0x80, 0x00, "Game"                   },
	{0x15, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(threewonders);

static struct BurnDIPInfo captcommDIPList[]=
{
	// Defaults
	{0x23, 0xff, 0xff, 0x00, NULL                     },
	{0x24, 0xff, 0xff, 0x0b, NULL                     },
	{0x25, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x23)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x23, 0x01, 0x40, 0x00, "Off"                    },
	{0x23, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x24)

	{0   , 0xfe, 0   , 4   , "Difficulty 2"           },
	{0x24, 0x01, 0x18, 0x00, "Easy"                   },
	{0x24, 0x01, 0x18, 0x08, "Normal"                 },
	{0x24, 0x01, 0x18, 0x10, "Hard"                   },
	{0x24, 0x01, 0x18, 0x18, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Play Mode"              },
	{0x24, 0x01, 0xc0, 0x80, "1 Player"               },
	{0x24, 0x01, 0xc0, 0x00, "2 Player"               },
	{0x24, 0x01, 0xc0, 0x40, "3 Player"               },
	{0x24, 0x01, 0xc0, 0xc0, "4 Player"               },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x25, 0x01, 0x03, 0x03, "1"                      },
	{0x25, 0x01, 0x03, 0x00, "2"                      },
	{0x25, 0x01, 0x03, 0x01, "3"                      },
	{0x25, 0x01, 0x03, 0x02, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x25, 0x01, 0x04, 0x00, "Off"                    },
	{0x25, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x25, 0x01, 0x08, 0x00, "Off"                    },
	{0x25, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x25, 0x01, 0x10, 0x00, "Off"                    },
	{0x25, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x25, 0x01, 0x20, 0x00, "Off"                    },
	{0x25, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x25, 0x01, 0x40, 0x00, "Off"                    },
	{0x25, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x25, 0x01, 0x80, 0x00, "Game"                   },
	{0x25, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(captcomm);

static struct BurnDIPInfo cawingDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Difficulty (Enemys strength)"},
	{0x16, 0x01, 0x07, 0x00, "1 (Easiest)"            },
	{0x16, 0x01, 0x07, 0x01, "2"                      },
	{0x16, 0x01, 0x07, 0x02, "3"                      },
	{0x16, 0x01, 0x07, 0x03, "4 (Normal)"             },
	{0x16, 0x01, 0x07, 0x04, "5"                      },
	{0x16, 0x01, 0x07, 0x05, "6"                      },
	{0x16, 0x01, 0x07, 0x06, "7"                      },
	{0x16, 0x01, 0x07, 0x07, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 4   , "Difficulty (Players strength)"},
	{0x16, 0x01, 0x18, 0x08, "Easy"                   },
	{0x16, 0x01, 0x18, 0x00, "Normal"                 },
	{0x16, 0x01, 0x18, 0x10, "Difficult"              },
	{0x16, 0x01, 0x18, 0x18, "Very Difficult"         },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(cawing);

static struct BurnDIPInfo cworld2jDIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x01, NULL                     },
	{0x11, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x0f)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Extended Test Mode"     },
	{0x0f, 0x01, 0x80, 0x00, "Off"                    },
	{0x0f, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x07, 0x01, "0"                      },
	{0x10, 0x01, 0x07, 0x02, "1"                      },
	{0x10, 0x01, 0x07, 0x03, "2"                      },
	{0x10, 0x01, 0x07, 0x04, "3"                      },
	{0x10, 0x01, 0x07, 0x05, "4"                      },

	{0   , 0xfe, 0   , 3   , "Extend"                 },
	{0x10, 0x01, 0x18, 0x18, "D"                      },
	{0x10, 0x01, 0x18, 0x08, "E"                      },
	{0x10, 0x01, 0x18, 0x00, "N"                      },

	{0   , 0xfe, 0   , 5   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0xe0, "1"                      },
	{0x10, 0x01, 0xe0, 0x60, "2"                      },
	{0x10, 0x01, 0xe0, 0x00, "3"                      },
	{0x10, 0x01, 0xe0, 0x40, "4"                      },
	{0x10, 0x01, 0xe0, 0x20, "5"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x00, "Off"                    },
	{0x11, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(cworld2j);

static struct BurnDIPInfo dinoDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1b, 0x01, 0x08, 0x00, "Off"                    },
	{0x1b, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(dino);

static struct BurnDIPInfo dynwarDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
        CPS1_COINAGE_3(0x14)

        {0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x80, 0x00, "Off"                    },
	{0x14, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_2(0x15)

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x01, 0x00, "Off"                    },
	{0x16, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Turbo Mode"             },
	{0x16, 0x01, 0x02, 0x00, "Off"                    },
	{0x16, 0x01, 0x02, 0x02, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x16, 0x01, 0x20, 0x20, "Off"                    },
	{0x16, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(dynwar);

static struct BurnDIPInfo ffightDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x0b, NULL                     },
	{0x16, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x14)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x14, 0x01, 0x40, 0x00, "Off"                    },
	{0x14, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x15)

	{0   , 0xfe, 0   , 4   , "Difficulty 2"           },
	{0x15, 0x01, 0x18, 0x00, "Easy"                   },
	{0x15, 0x01, 0x18, 0x08, "Normal"                 },
	{0x15, 0x01, 0x18, 0x10, "Hard"                   },
	{0x15, 0x01, 0x18, 0x18, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x60, 0x00, "100k"                   },
	{0x15, 0x01, 0x60, 0x20, "200k"                   },
	{0x15, 0x01, 0x60, 0x40, "100k and every 200k"    },
	{0x15, 0x01, 0x60, 0x60, "None"                   },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x03, 0x03, "1"                      },
	{0x16, 0x01, 0x03, 0x00, "2"                      },
	{0x16, 0x01, 0x03, 0x01, "3"                      },
	{0x16, 0x01, 0x03, 0x02, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x16, 0x01, 0x04, 0x00, "Off"                    },
	{0x16, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x08, 0x00, "Off"                    },
	{0x16, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x16, 0x01, 0x20, 0x00, "Off"                    },
	{0x16, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x00, "Off"                    },
	{0x16, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(ffight);

static struct BurnDIPInfo forgottnDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x03, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x12, 0x01, 0x40, 0x40, "Off"                    },
	{0x12, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x12, 0x01, 0x80, 0x00, "Off"                    },
	{0x12, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x13)

	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x40, 0x00, "Off"                    },
	{0x13, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(forgottn);

static struct BurnDIPInfo ghoulsDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x12, 0x01, 0xc0, 0x00, "Upright 1 Player"       },
	{0x12, 0x01, 0xc0, 0x40, "Upright 2 Players"      },
	{0x12, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x13)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x10, "10k, 30k and every 30k" },
	{0x13, 0x01, 0x30, 0x20, "20k, 50k and every 70k" },
	{0x13, 0x01, 0x30, 0x00, "30k, 60k and every 70k" },
	{0x13, 0x01, 0x30, 0x30, "40k, 70k and every 80k" },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x00, "3"                      },
	{0x14, 0x01, 0x03, 0x01, "4"                      },
	{0x14, 0x01, 0x03, 0x02, "5"                      },
	{0x14, 0x01, 0x03, 0x03, "6"                      },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(ghouls);

static struct BurnDIPInfo ghoulsuDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x12, 0x01, 0xc0, 0x00, "Upright 1 Player"       },
	{0x12, 0x01, 0xc0, 0x40, "Upright 2 Players"      },
	{0x12, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x13)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x10, "10k, 30k and every 30k" },
	{0x13, 0x01, 0x30, 0x20, "20k, 50k and every 70k" },
	{0x13, 0x01, 0x30, 0x00, "30k, 60k and every 70k" },
	{0x13, 0x01, 0x30, 0x30, "40k, 70k and every 80k" },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x03, "2"                      },
	{0x14, 0x01, 0x03, 0x00, "3"                      },
	{0x14, 0x01, 0x03, 0x01, "4"                      },
	{0x14, 0x01, 0x03, 0x02, "5"                      },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(ghoulsu);

static struct BurnDIPInfo daimakaiDIPList[]=
{
	// Defaults
	{0x12, 0xff, 0xff, 0x00, NULL                     },
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x12)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x12, 0x01, 0xc0, 0x00, "Upright 1 Player"       },
	{0x12, 0x01, 0xc0, 0x40, "Upright 2 Players"      },
	{0x12, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x13)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x10, "10k, 30k and every 30k" },
	{0x13, 0x01, 0x30, 0x20, "20k, 50k and every 70k" },
	{0x13, 0x01, 0x30, 0x00, "30k, 60k and every 70k" },
	{0x13, 0x01, 0x30, 0x30, "40k, 70k and every 80k" },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x00, "3"                      },
	{0x14, 0x01, 0x03, 0x01, "4"                      },
	{0x14, 0x01, 0x03, 0x02, "5"                      },
	{0x14, 0x01, 0x03, 0x03, "6"                      },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x14, 0x01, 0x80, 0x00, "Game"                   },
	{0x14, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(daimakai);

static struct BurnDIPInfo knightsDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Enemys Attack Frequency"},
	{0x1c, 0x01, 0x07, 0x00, "1 (Easiest)"            },
	{0x1c, 0x01, 0x07, 0x01, "2"                      },
	{0x1c, 0x01, 0x07, 0x02, "3"                      },
	{0x1c, 0x01, 0x07, 0x03, "4 (Normal)"             },
	{0x1c, 0x01, 0x07, 0x04, "5"                      },
	{0x1c, 0x01, 0x07, 0x05, "6"                      },
	{0x1c, 0x01, 0x07, 0x06, "7"                      },
	{0x1c, 0x01, 0x07, 0x07, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 8   , "Enemys Attack Power"    },
	{0x1c, 0x01, 0x38, 0x38, "1 (Easiest)"            },
	{0x1c, 0x01, 0x38, 0x30, "2"                      },
	{0x1c, 0x01, 0x38, 0x28, "3"                      },
	{0x1c, 0x01, 0x38, 0x00, "4 (Normal)"             },
	{0x1c, 0x01, 0x38, 0x08, "5"                      },
	{0x1c, 0x01, 0x38, 0x10, "6"                      },
	{0x1c, 0x01, 0x38, 0x18, "7"                      },
	{0x1c, 0x01, 0x38, 0x20, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1c, 0x01, 0x40, 0x40, "1"                      },
	{0x1c, 0x01, 0x40, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1c, 0x01, 0x80, 0x80, "2P"                     },
	{0x1c, 0x01, 0x80, 0x00, "3P"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x1d, 0x01, 0x03, 0x03, "1"                      },
	{0x1d, 0x01, 0x03, 0x00, "2"                      },
	{0x1d, 0x01, 0x03, 0x01, "3"                      },
	{0x1d, 0x01, 0x03, 0x02, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(knights);

static struct BurnDIPInfo kodDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1b, 0x01, 0x08, 0x08, "1"                      },
	{0x1b, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1b, 0x01, 0x10, 0x10, "2 Players"              },
	{0x1b, 0x01, 0x10, 0x00, "3 Players"              },

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1c, 0x01, 0x38, 0x08, "1"                      },
	{0x1c, 0x01, 0x38, 0x00, "2"                      },
	{0x1c, 0x01, 0x38, 0x10, "3"                      },
	{0x1c, 0x01, 0x38, 0x18, "4"                      },
	{0x1c, 0x01, 0x38, 0x20, "5"                      },
	{0x1c, 0x01, 0x38, 0x28, "6"                      },
	{0x1c, 0x01, 0x38, 0x30, "7"                      },
	{0x1c, 0x01, 0x38, 0x38, "8"                      },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x1c, 0x01, 0xc0, 0x40, "80k and every 400k"     },
	{0x1c, 0x01, 0xc0, 0x00, "160k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0x80, "200k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0xc0, "None"                   },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(kod);

static struct BurnDIPInfo kodjDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1b, 0x01, 0x08, 0x08, "1"                      },
	{0x1b, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1b, 0x01, 0x10, 0x10, "2 Players"              },
	{0x1b, 0x01, 0x10, 0x00, "3 Players"              },

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1c, 0x01, 0x38, 0x08, "1"                      },
	{0x1c, 0x01, 0x38, 0x00, "2"                      },
	{0x1c, 0x01, 0x38, 0x10, "3"                      },
	{0x1c, 0x01, 0x38, 0x18, "4"                      },
	{0x1c, 0x01, 0x38, 0x20, "5"                      },
	{0x1c, 0x01, 0x38, 0x28, "6"                      },
	{0x1c, 0x01, 0x38, 0x30, "7"                      },
	{0x1c, 0x01, 0x38, 0x38, "8"                      },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x1c, 0x01, 0xc0, 0x40, "80k and every 400k"     },
	{0x1c, 0x01, 0xc0, 0x80, "160k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0x00, "200k and every 450k"    },
	{0x1c, 0x01, 0xc0, 0xc0, "None"                   },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(kodj);

static struct BurnDIPInfo megamanDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x01, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	{0   , 0xfe, 0   , 19  , "Coinage"                },
	{0x15, 0x01, 0x1f, 0x00, "1 Coin  1 Credit"       },
	{0x15, 0x01, 0x1f, 0x01, "1 Coin  2 Credits"      },
	{0x15, 0x01, 0x1f, 0x02, "1 Coin  3 Credits"      },
	{0x15, 0x01, 0x1f, 0x03, "1 Coin  4 Credits"      },
	{0x15, 0x01, 0x1f, 0x04, "1 Coin  5 Credits"      },
	{0x15, 0x01, 0x1f, 0x05, "1 Coin  6 Credits"      },
	{0x15, 0x01, 0x1f, 0x06, "1 Coin  7 Credits"      },
	{0x15, 0x01, 0x1f, 0x07, "1 Coin  8 Credits"      },
	{0x15, 0x01, 0x1f, 0x08, "1 Coin  9 Credits"      },
	{0x15, 0x01, 0x1f, 0x09, "2 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0a, "3 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0b, "4 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0c, "5 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0d, "6 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0e, "7 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0f, "8 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x10, "9 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x11, "2 Coins Start 1 Coin Continue"},
	{0x15, 0x01, 0x1f, 0x12, "Free Play"              },

	{0   , 0xfe, 0   , 4   , "Coin Slots"             },
	{0x15, 0x01, 0x60, 0x40, "1, Common"              },
	{0x15, 0x01, 0x60, 0x00, "2, Common"              },
	{0x15, 0x01, 0x60, 0x20, "2, Individual"          },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x16, 0x01, 0x03, 0x00, "Easy"                   },
	{0x16, 0x01, 0x03, 0x01, "Normal"                 },
	{0x16, 0x01, 0x03, 0x02, "Hard"                   },
	{0x16, 0x01, 0x03, 0x03, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Time"                   },
	{0x16, 0x01, 0x0c, 0x00, "100"                    },
	{0x16, 0x01, 0x0c, 0x04, "90"                     },
	{0x16, 0x01, 0x0c, 0x08, "70"                     },
	{0x16, 0x01, 0x0c, 0x0c, "60"                     },

	{0   , 0xfe, 0   , 2   , "Voice"                  },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x17, 0x01, 0x01, 0x00, "Off"                    },
	{0x17, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x02, 0x02, "Off"                    },
	{0x17, 0x01, 0x02, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x04, 0x04, "Off"                    },
	{0x17, 0x01, 0x04, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(megaman);

static struct BurnDIPInfo rockmanjDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x01, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	{0   , 0xfe, 0   , 19  , "Coinage"                },
	{0x15, 0x01, 0x1f, 0x00, "1 Coin  1 Credit"       },
	{0x15, 0x01, 0x1f, 0x01, "1 Coin  2 Credits"      },
	{0x15, 0x01, 0x1f, 0x02, "1 Coin  3 Credits"      },
	{0x15, 0x01, 0x1f, 0x03, "1 Coin  4 Credits"      },
	{0x15, 0x01, 0x1f, 0x04, "1 Coin  5 Credits"      },
	{0x15, 0x01, 0x1f, 0x05, "1 Coin  6 Credits"      },
	{0x15, 0x01, 0x1f, 0x06, "1 Coin  7 Credits"      },
	{0x15, 0x01, 0x1f, 0x07, "1 Coin  8 Credits"      },
	{0x15, 0x01, 0x1f, 0x08, "1 Coin  9 Credits"      },
	{0x15, 0x01, 0x1f, 0x09, "2 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0a, "3 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0b, "4 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0c, "5 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0d, "6 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0e, "7 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x0f, "8 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x10, "9 Coins 1 Credit"       },
	{0x15, 0x01, 0x1f, 0x11, "2 Coins Start 1 Coin Continue"},
	{0x15, 0x01, 0x1f, 0x12, "Free Play"              },

	{0   , 0xfe, 0   , 4   , "Coin Slots"             },
	{0x15, 0x01, 0x60, 0x40, "1, Common"              },
	{0x15, 0x01, 0x60, 0x00, "2, Common"              },
	{0x15, 0x01, 0x60, 0x20, "2, Individual"          },

	// Dip B
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x16, 0x01, 0x03, 0x00, "Easy"                   },
	{0x16, 0x01, 0x03, 0x01, "Normal"                 },
	{0x16, 0x01, 0x03, 0x02, "Hard"                   },
	{0x16, 0x01, 0x03, 0x03, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Time"                   },
	{0x16, 0x01, 0x0c, 0x00, "100"                    },
	{0x16, 0x01, 0x0c, 0x04, "90"                     },
	{0x16, 0x01, 0x0c, 0x08, "70"                     },
	{0x16, 0x01, 0x0c, 0x0c, "60"                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x17, 0x01, 0x01, 0x00, "Off"                    },
	{0x17, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x02, 0x02, "Off"                    },
	{0x17, 0x01, 0x02, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x04, 0x04, "Off"                    },
	{0x17, 0x01, 0x04, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(rockmanj);

static struct BurnDIPInfo mercsDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1c, 0x01, 0x08, 0x08, "1"                      },
	{0x1c, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1c, 0x01, 0x10, 0x10, "2 Player"               },
	{0x1c, 0x01, 0x10, 0x00, "3 Player"               },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x1d, 0x01, 0x80, 0x00, "Off"                    },
	{0x1d, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(mercs);

static struct BurnDIPInfo mswordDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Players vitality consumption"},
	{0x16, 0x01, 0x07, 0x00, "1 (Easiest)"            },
	{0x16, 0x01, 0x07, 0x01, "2"                      },
	{0x16, 0x01, 0x07, 0x02, "3"                      },
	{0x16, 0x01, 0x07, 0x03, "4 (Normal)"             },
	{0x16, 0x01, 0x07, 0x04, "5"                      },
	{0x16, 0x01, 0x07, 0x05, "6"                      },
	{0x16, 0x01, 0x07, 0x06, "7"                      },
	{0x16, 0x01, 0x07, 0x07, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 8   , "Level 2"                },
	{0x16, 0x01, 0x38, 0x18, "1 (Easiest)"            },
	{0x16, 0x01, 0x38, 0x10, "2"                      },
	{0x16, 0x01, 0x38, 0x08, "3"                      },
	{0x16, 0x01, 0x38, 0x00, "4 (Normal)"             },
	{0x16, 0x01, 0x38, 0x20, "5"                      },
	{0x16, 0x01, 0x38, 0x28, "6"                      },
	{0x16, 0x01, 0x38, 0x30, "7"                      },
	{0x16, 0x01, 0x38, 0x38, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 2   , "Stage Select"           },
	{0x16, 0x01, 0x40, 0x00, "Off"                    },
	{0x16, 0x01, 0x40, 0x40, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Vitality"               },
	{0x17, 0x01, 0x03, 0x03, "1"                      },
	{0x17, 0x01, 0x03, 0x00, "2"                      },
	{0x17, 0x01, 0x03, 0x01, "3 (2 when continue)"    },
	{0x17, 0x01, 0x03, 0x02, "4 (3 when continue)"    },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(msword);


static struct BurnDIPInfo mtwinsDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x15)

	// Dip B
	CPS1_DIFFICULTY_1(0x16)

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x38, 0x28, "1"                      },
	{0x16, 0x01, 0x38, 0x20, "2"                      },
	{0x16, 0x01, 0x38, 0x30, "3"                      },
	{0x16, 0x01, 0x38, 0x38, "4"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(mtwins);

static struct BurnDIPInfo nemoDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x16)

	{0   , 0xfe, 0   , 3   , "Life Bar"               },
	{0x16, 0x01, 0x18, 0x18, "Minimum"                },
	{0x16, 0x01, 0x18, 0x00, "Medium"                 },
	{0x16, 0x01, 0x18, 0x10, "Maximum"                },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x17, 0x01, 0x03, 0x01, "1"                      },
	{0x17, 0x01, 0x03, 0x00, "2"                      },
	{0x17, 0x01, 0x03, 0x02, "3"                      },
	{0x17, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(nemo);

static struct BurnDIPInfo pang3DIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x08, 0x00, "Off"                    },
	{0x13, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(pang3);

static struct BurnDIPInfo pnickjDIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },
	{0x14, 0xff, 0xff, 0x03, NULL                     },
	{0x15, 0xff, 0xff, 0x20, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x13)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x13, 0x01, 0x08, 0x00, "1"                      },
	{0x13, 0x01, 0x08, 0x08, "2"                      },

	// Dip B
	CPS1_DIFFICULTY_1(0x14)

	{0   , 0xfe, 0   , 4   , "Vs. Play Mode"          },
	{0x14, 0x01, 0xc0, 0x00, "1 Game  Match"          },
	{0x14, 0x01, 0xc0, 0x40, "3 Games Match"          },
	{0x14, 0x01, 0xc0, 0x80, "5 Games Match"          },
	{0x14, 0x01, 0xc0, 0xc0, "7 Games Match"          },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x15, 0x01, 0x08, 0x00, "Off"                    },
	{0x15, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x15, 0x01, 0x10, 0x00, "Off"                    },
	{0x15, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x15, 0x01, 0x20, 0x00, "Off"                    },
	{0x15, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x15, 0x01, 0x80, 0x00, "Game"                   },
	{0x15, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(pnickj);

static struct BurnDIPInfo punishDIPList[]=
{
	// Defaults
	{0x13, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x08, 0x00, "Off"                    },
	{0x13, 0x01, 0x08, 0x08, "On"                     },
};

STDDIPINFO(punish);

static struct BurnDIPInfo qadDIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x0b, NULL                     },
	{0x11, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x0f)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 5   , "Difficulty"             },
	{0x10, 0x01, 0x07, 0x01, "Easiest"                },
	{0x10, 0x01, 0x07, 0x02, "Easy"                   },
	{0x10, 0x01, 0x07, 0x03, "Normal"                 },
	{0x10, 0x01, 0x07, 0x04, "Hard"                   },
	{0x10, 0x01, 0x07, 0x05, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Wisdom of Enemy"        },
	{0x10, 0x01, 0x18, 0x00, "Easy"                   },
	{0x10, 0x01, 0x18, 0x08, "Normal"                 },
	{0x10, 0x01, 0x18, 0x10, "Hard"                   },
	{0x10, 0x01, 0x18, 0x18, "Hardest"                },

	{0   , 0xfe, 0   , 5   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0x80, "1"                      },
	{0x10, 0x01, 0xe0, 0x60, "2"                      },
	{0x10, 0x01, 0xe0, 0x40, "3"                      },
	{0x10, 0x01, 0xe0, 0x20, "4"                      },
	{0x10, 0x01, 0xe0, 0x00, "5"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x20, "Off"                    },
	{0x11, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(qad);

static struct BurnDIPInfo qadjDIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x00, NULL                     },
	{0x11, 0xff, 0xff, 0x20, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x0f)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 5   , "Difficulty"             },
	{0x10, 0x01, 0x07, 0x00, "0"                      },
	{0x10, 0x01, 0x07, 0x01, "1"                      },
	{0x10, 0x01, 0x07, 0x02, "2"                      },
	{0x10, 0x01, 0x07, 0x03, "3"                      },
	{0x10, 0x01, 0x07, 0x04, "4"                      },

	{0   , 0xfe, 0   , 3   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0x40, "1"                      },
	{0x10, 0x01, 0xe0, 0x20, "2"                      },
	{0x10, 0x01, 0xe0, 0x00, "3"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(qadj);

static struct BurnDIPInfo qtono2DIPList[]=
{
	// Defaults
	{0x0f, 0xff, 0xff, 0x00, NULL                     },
	{0x10, 0xff, 0xff, 0x03, NULL                     },
	{0x11, 0xff, 0xff, 0x20, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x0f)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x0f, 0x01, 0x40, 0x00, "Off"                    },
	{0x0f, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x10)

	{0   , 0xfe, 0   , 5   , "Lives"                  },
	{0x10, 0x01, 0xe0, 0x80, "1"                      },
	{0x10, 0x01, 0xe0, 0x60, "2"                      },
	{0x10, 0x01, 0xe0, 0x00, "3"                      },
	{0x10, 0x01, 0xe0, 0x40, "4"                      },
	{0x10, 0x01, 0xe0, 0x20, "5"                      },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Infinite Lives"         },
	{0x11, 0x01, 0x02, 0x00, "Off"                    },
	{0x11, 0x01, 0x02, 0x02, "On"                     },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x04, 0x00, "Off"                    },
	{0x11, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x08, 0x00, "Off"                    },
	{0x11, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x11, 0x01, 0x80, 0x00, "Game"                   },
	{0x11, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(qtono2);

static struct BurnDIPInfo sf2DIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(sf2);

static struct BurnDIPInfo sf2jDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x03, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 2   , "2 Players Game"         },
	{0x1c, 0x01, 0x08, 0x00, "1 Credit/No Continue"   },
	{0x1c, 0x01, 0x08, 0x08, "2 Credits/Winner Continue"},

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(sf2j);

static struct BurnDIPInfo sf2m2DIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },
	{0x1c, 0xff, 0xff, 0x13, NULL                     },
	{0x1d, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x1b)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1b, 0x01, 0x40, 0x00, "Off"                    },
	{0x1b, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1c)

	{0   , 0xfe, 0   , 2   , "It needs to be High"    },
	{0x1c, 0x01, 0x10, 0x00, "Low"                    },
	{0x1c, 0x01, 0x10, 0x10, "High"                   },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1d, 0x01, 0x04, 0x00, "Off"                    },
	{0x1d, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x00, "Off"                    },
	{0x1d, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1d, 0x01, 0x10, 0x00, "Off"                    },
	{0x1d, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1d, 0x01, 0x80, 0x00, "Game"                   },
	{0x1d, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(sf2m2);

static struct BurnDIPInfo slammastDIPList[]=
{
	// Defaults
	{0x23, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x23, 0x01, 0x08, 0x08, "Off"                    },
	{0x23, 0x01, 0x08, 0x00, "On"                     },
};

STDDIPINFO(slammast);

static struct BurnDIPInfo striderDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x14)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x14, 0x01, 0xc0, 0x00, "Upright 1P"             },
	{0x14, 0x01, 0xc0, 0x40, "Upright 2P"             },
	{0x14, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x15)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x30, 0x00, "20k, 40k and every 60k" },
	{0x15, 0x01, 0x30, 0x10, "30k, 50k and every 70k" },
	{0x15, 0x01, 0x30, 0x20, "20k & 60k only"         },
	{0x15, 0x01, 0x30, 0x30, "30k & 60k only"         },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x03, 0x03, "2"                      },
	{0x16, 0x01, 0x03, 0x00, "3"                      },
	{0x16, 0x01, 0x03, 0x01, "4"                      },
	{0x16, 0x01, 0x03, 0x02, "5"                      },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x04, 0x00, "Off"                    },
	{0x16, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x16, 0x01, 0x08, 0x00, "Off"                    },
	{0x16, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x16, 0x01, 0x20, 0x20, "Off"                    },
	{0x16, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(strider);

static struct BurnDIPInfo strideruaDIPList[]=
{
	// Defaults
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x14)

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x14, 0x01, 0xc0, 0x00, "Upright 1P"             },
	{0x14, 0x01, 0xc0, 0x40, "Upright 2P"             },
	{0x14, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x08, 0x00, "Off"                    },
	{0x15, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x30, 0x00, "20k, 40k and every 60k" },
	{0x15, 0x01, 0x30, 0x10, "30k, 50k and every 70k" },
	{0x15, 0x01, 0x30, 0x20, "20k & 60k only"         },
	{0x15, 0x01, 0x30, 0x30, "30k & 60k only"         },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x03, 0x03, "2"                      },
	{0x16, 0x01, 0x03, 0x00, "3"                      },
	{0x16, 0x01, 0x03, 0x01, "4"                      },
	{0x16, 0x01, 0x03, 0x02, "5"                      },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x16, 0x01, 0x04, 0x00, "Off"                    },
	{0x16, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x16, 0x01, 0x08, 0x00, "Off"                    },
	{0x16, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x16, 0x01, 0x10, 0x00, "Off"                    },
	{0x16, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x16, 0x01, 0x20, 0x20, "Off"                    },
	{0x16, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x40, 0x40, "Off"                    },
	{0x16, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x16, 0x01, 0x80, 0x00, "Game"                   },
	{0x16, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(striderua);

static struct BurnDIPInfo unsquadDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x03, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
        CPS1_COINAGE_3(0x15)

	// Dip B
	CPS1_DIFFICULTY_1(0x16)

	{0   , 0xfe, 0   , 4   , "Damage"                 },
	{0x16, 0x01, 0x18, 0x08, "Small"                  },
	{0x16, 0x01, 0x18, 0x00, "Normal"                 },
	{0x16, 0x01, 0x18, 0x10, "Big"                    },
	{0x16, 0x01, 0x18, 0x18, "Biggest"                },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(unsquad);


static struct BurnDIPInfo varthDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x0b, NULL                     },
	{0x17, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_1(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x40, 0x00, "Off"                    },
	{0x15, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x16)

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x16, 0x01, 0x18, 0x00, "600k and every 1400k"   },
	{0x16, 0x01, 0x18, 0x08, "600k, 2000k and every 4500k"},
	{0x16, 0x01, 0x18, 0x10, "1200k, 3500k"           },
	{0x16, 0x01, 0x18, 0x18, "2000k only"             },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x17, 0x01, 0x03, 0x01, "1"                      },
	{0x17, 0x01, 0x03, 0x02, "2"                      },
	{0x17, 0x01, 0x03, 0x00, "3"                      },
	{0x17, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x17, 0x01, 0x04, 0x00, "Off"                    },
	{0x17, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x17, 0x01, 0x08, 0x00, "Off"                    },
	{0x17, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x17, 0x01, 0x20, 0x00, "Off"                    },
	{0x17, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(varth);

static struct BurnDIPInfo willowDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },
	{0x17, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
        CPS1_COINAGE_3(0x15)

        {0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x15, 0x01, 0xc0, 0x00, "Upright 1P"             },
	{0x15, 0x01, 0xc0, 0x40, "Upright 2P"             },
	{0x15, 0x01, 0xc0, 0xc0, "Cocktail"               },

	// Dip B
	CPS1_DIFFICULTY_2(0x16)

	{0   , 0xfe, 0   , 4   , "Nando Speed"            },
	{0x16, 0x01, 0x18, 0x08, "Slow"                   },
	{0x16, 0x01, 0x18, 0x00, "Normal"                 },
	{0x16, 0x01, 0x18, 0x10, "Fast"                   },
	{0x16, 0x01, 0x18, 0x18, "Very Fast"              },

	{0   , 0xfe, 0   , 2   , "Stage Magic Continue"   },
	{0x16, 0x01, 0x80, 0x00, "Off"                    },
	{0x16, 0x01, 0x80, 0x80, "On"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x17, 0x01, 0x03, 0x01, "1"                      },
	{0x17, 0x01, 0x03, 0x00, "2"                      },
	{0x17, 0x01, 0x03, 0x02, "3"                      },
	{0x17, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 4   , "Vitality"               },
	{0x17, 0x01, 0x0c, 0x0c, "2"                      },
	{0x17, 0x01, 0x0c, 0x00, "3"                      },
	{0x17, 0x01, 0x0c, 0x04, "4"                      },
	{0x17, 0x01, 0x0c, 0x08, "5"                      },

	{0   , 0xfe, 0   , 2   , "Screen"                 },
	{0x17, 0x01, 0x10, 0x00, "Off"                    },
	{0x17, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x17, 0x01, 0x20, 0x20, "Off"                    },
	{0x17, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x17, 0x01, 0x40, 0x40, "Off"                    },
	{0x17, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x17, 0x01, 0x80, 0x00, "Game"                   },
	{0x17, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(willow);

static struct BurnDIPInfo wofDIPList[]=
{
	// Defaults
	{0x1b, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1b, 0x01, 0x08, 0x08, "Off"                    },
	{0x1b, 0x01, 0x08, 0x00, "On"                     },
};

STDDIPINFO(wof);

static struct BurnDIPInfo dinohDIPList[]=
{
	// Defaults
	{0x1d, 0xff, 0xff, 0x00, NULL                     },
	{0x1e, 0xff, 0xff, 0x2b, NULL                     },
	{0x1f, 0xff, 0xff, 0x21, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x1d)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1d, 0x01, 0x08, 0x08, "1"                      },
	{0x1d, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1d, 0x01, 0x10, 0x10, "2P"                     },
	{0x1d, 0x01, 0x10, 0x00, "3P"                     },

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1e)

	{0   , 0xfe, 0   , 4   , "Difficulty 2"           },
	{0x1e, 0x01, 0x18, 0x00, "Easy"                   },
	{0x1e, 0x01, 0x18, 0x08, "Normal"                 },
	{0x1e, 0x01, 0x18, 0x10, "Hard"                   },
	{0x1e, 0x01, 0x18, 0x18, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x1e, 0x01, 0x60, 0x00, "300k and 700k"          },
	{0x1e, 0x01, 0x60, 0x20, "500k and 1000k"         },
	{0x1e, 0x01, 0x60, 0x40, "1000k"                  },
	{0x1e, 0x01, 0x60, 0x60, "None"                   },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x1f, 0x01, 0x03, 0x00, "1"                      },
	{0x1f, 0x01, 0x03, 0x01, "2"                      },
	{0x1f, 0x01, 0x03, 0x02, "3"                      },
	{0x1f, 0x01, 0x03, 0x03, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1f, 0x01, 0x04, 0x00, "Off"                    },
	{0x1f, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1f, 0x01, 0x08, 0x00, "Off"                    },
	{0x1f, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x1f, 0x01, 0x10, 0x00, "Off"                    },
	{0x1f, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1f, 0x01, 0x20, 0x00, "Off"                    },
	{0x1f, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1f, 0x01, 0x40, 0x00, "Off"                    },
	{0x1f, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1f, 0x01, 0x80, 0x00, "Game"                   },
	{0x1f, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(dinoh);

static struct BurnDIPInfo knightshDIPList[]=
{
	// Defaults
	{0x1d, 0xff, 0xff, 0x00, NULL                     },
	{0x1e, 0xff, 0xff, 0x03, NULL                     },
	{0x1f, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x1d)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Enemys Attack Frequency"},
	{0x1e, 0x01, 0x07, 0x00, "1 (Easiest)"            },
	{0x1e, 0x01, 0x07, 0x01, "2"                      },
	{0x1e, 0x01, 0x07, 0x02, "3"                      },
	{0x1e, 0x01, 0x07, 0x03, "4 (Normal)"             },
	{0x1e, 0x01, 0x07, 0x04, "5"                      },
	{0x1e, 0x01, 0x07, 0x05, "6"                      },
	{0x1e, 0x01, 0x07, 0x06, "7"                      },
	{0x1e, 0x01, 0x07, 0x07, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 8   , "Enemys Attack Power"    },
	{0x1e, 0x01, 0x38, 0x38, "1 (Easiest)"            },
	{0x1e, 0x01, 0x38, 0x30, "2"                      },
	{0x1e, 0x01, 0x38, 0x28, "3"                      },
	{0x1e, 0x01, 0x38, 0x00, "4 (Normal)"             },
	{0x1e, 0x01, 0x38, 0x08, "5"                      },
	{0x1e, 0x01, 0x38, 0x10, "6"                      },
	{0x1e, 0x01, 0x38, 0x18, "7"                      },
	{0x1e, 0x01, 0x38, 0x20, "8 (Hardest)"            },

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1e, 0x01, 0x40, 0x40, "1"                      },
	{0x1e, 0x01, 0x40, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1e, 0x01, 0x80, 0x80, "2P"                     },
	{0x1e, 0x01, 0x80, 0x00, "3P"                     },

	// Dip C
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x1f, 0x01, 0x03, 0x03, "1"                      },
	{0x1f, 0x01, 0x03, 0x00, "2"                      },
	{0x1f, 0x01, 0x03, 0x01, "3"                      },
	{0x1f, 0x01, 0x03, 0x02, "4"                      },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1f, 0x01, 0x04, 0x00, "Off"                    },
	{0x1f, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1f, 0x01, 0x08, 0x00, "Off"                    },
	{0x1f, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip"                   },
	{0x1f, 0x01, 0x10, 0x00, "Off"                    },
	{0x1f, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1f, 0x01, 0x20, 0x00, "Off"                    },
	{0x1f, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1f, 0x01, 0x40, 0x00, "Off"                    },
	{0x1f, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1f, 0x01, 0x80, 0x00, "Game"                   },
	{0x1f, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(knightsh);

static struct BurnDIPInfo kodhDIPList[]=
{
	// Defaults
	{0x1d, 0xff, 0xff, 0x00, NULL                     },
	{0x1e, 0xff, 0xff, 0x03, NULL                     },
	{0x1f, 0xff, 0xff, 0x60, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x1d)

	{0   , 0xfe, 0   , 2   , "Coin Slots"             },
	{0x1d, 0x01, 0x08, 0x08, "1"                      },
	{0x1d, 0x01, 0x08, 0x00, "3"                      },

	{0   , 0xfe, 0   , 2   , "Play Mode"              },
	{0x1d, 0x01, 0x10, 0x10, "2 Players"              },
	{0x1d, 0x01, 0x10, 0x00, "3 Players"              },

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x1d, 0x01, 0x40, 0x00, "Off"                    },
	{0x1d, 0x01, 0x40, 0x40, "On"                     },

	// Dip B
	CPS1_DIFFICULTY_1(0x1e)

	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1e, 0x01, 0x38, 0x08, "1"                      },
	{0x1e, 0x01, 0x38, 0x00, "2"                      },
	{0x1e, 0x01, 0x38, 0x10, "3"                      },
	{0x1e, 0x01, 0x38, 0x18, "4"                      },
	{0x1e, 0x01, 0x38, 0x20, "5"                      },
	{0x1e, 0x01, 0x38, 0x28, "6"                      },
	{0x1e, 0x01, 0x38, 0x30, "7"                      },
	{0x1e, 0x01, 0x38, 0x38, "8"                      },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x1e, 0x01, 0xc0, 0x40, "80k and every 400k"     },
	{0x1e, 0x01, 0xc0, 0x00, "160k and every 450k"    },
	{0x1e, 0x01, 0xc0, 0x80, "200k and every 450k"    },
	{0x1e, 0x01, 0xc0, 0xc0, "None"                   },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x1f, 0x01, 0x04, 0x00, "Off"                    },
	{0x1f, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1f, 0x01, 0x08, 0x00, "Off"                    },
	{0x1f, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1f, 0x01, 0x10, 0x00, "Off"                    },
	{0x1f, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x1f, 0x01, 0x20, 0x00, "Off"                    },
	{0x1f, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x1f, 0x01, 0x40, 0x00, "Off"                    },
	{0x1f, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Game Mode"              },
	{0x1f, 0x01, 0x80, 0x00, "Game"                   },
	{0x1f, 0x01, 0x80, 0x80, "Test"                   },
};

STDDIPINFO(kodh);

static struct BurnDIPInfo punisherhDIPList[]=
{
	// Defaults
	{0x15, 0xff, 0xff, 0x10, NULL                     },
	{0x16, 0xff, 0xff, 0x6B, NULL                     },

	// Dip A
	CPS1_COINAGE_2(0x15)

	{0   , 0xfe, 0   , 2   , "2C to Start, 1 to Cont" },
	{0x15, 0x01, 0x08, 0x00, "Off"                    },
	{0x15, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0x30, 0x00, "1"                      },
	{0x15, 0x01, 0x30, 0x10, "2"                      },
	{0x15, 0x01, 0x30, 0x20, "3"                      },
	{0x15, 0x01, 0x30, 0x30, "4"                      },

	{0   , 0xfe, 0   , 2   , "Sound"                  },
	{0x15, 0x01, 0x40, 0x00, "Q Sound"                },
	{0x15, 0x01, 0x40, 0x40, "Monaural"               },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x15, 0x01, 0x80, 0x00, "Off"                    },
	{0x15, 0x01, 0x80, 0x80, "On"                     },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x16, 0x01, 0x07, 0x00, "0 (Easiest)"            },
	{0x16, 0x01, 0x07, 0x01, "1"                      },
	{0x16, 0x01, 0x07, 0x02, "2"                      },
	{0x16, 0x01, 0x07, 0x03, "3 (Normal)"             },
	{0x16, 0x01, 0x07, 0x04, "4"                      },
	{0x16, 0x01, 0x07, 0x05, "5"                      },
	{0x16, 0x01, 0x07, 0x06, "6"                      },
	{0x16, 0x01, 0x07, 0x07, "7 (Hardest)"            },

	{0   , 0xfe, 0   , 4   , "Extend"                 },
	{0x16, 0x01, 0x18, 0x00, "800000"                 },
	{0x16, 0x01, 0x18, 0x08, "1800000"                },
	{0x16, 0x01, 0x18, 0x10, "2800000"                },
	{0x16, 0x01, 0x18, 0x18, "No Extend"              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x20, 0x00, "Off"                    },
	{0x16, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x16, 0x01, 0x40, 0x00, "Off"                    },
	{0x16, 0x01, 0x40, 0x40, "On"                     },
};

STDDIPINFO(punisherh);

static struct BurnDIPInfo wofhDIPList[]=
{
	// Defaults
	{0x1e, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1e, 0x01, 0x08, 0x08, "Off"                    },
	{0x1e, 0x01, 0x08, 0x00, "On"                     },
};

STDDIPINFO(wofh);

static struct BurnDIPInfo wof3jsDIPList[]=
{
	// Defaults
	{0x1d, 0xff, 0xff, 0x00, NULL                     },

	// Dip C
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x1d, 0x01, 0x08, 0x08, "Off"                    },
	{0x1d, 0x01, 0x08, 0x00, "On"                     },
};

STDDIPINFO(wof3js);

static struct BurnDIPInfo wofhfhDIPList[]=
{
	// Defaults
	{0x1d, 0xff, 0xff, 0x00, NULL                     },
	{0x1e, 0xff, 0xff, 0x13, NULL                     },
	{0x1f, 0xff, 0xff, 0x00, NULL                     },

	// Dip A
	{0   , 0xfe, 0   , 4   , "Coin A" },
	{0x1d, 0x01, 0x03, 0x00, "1 Coin  1 Credit"       },
	{0x1d, 0x01, 0x03, 0x01, "1 Coin  2 Credits"      },
	{0x1d, 0x01, 0x03, 0x02, "1 Coin  3 Credits"      },
	{0x1d, 0x01, 0x03, 0x03, "1 Coin  4 Credits"      },

	// Dip B
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x1e, 0x01, 0x07, 0x00, "0 (Easiest)"            },
	{0x1e, 0x01, 0x07, 0x01, "1"                      },
	{0x1e, 0x01, 0x07, 0x02, "2"                      },
	{0x1e, 0x01, 0x07, 0x03, "3 (Normal)"             },
	{0x1e, 0x01, 0x07, 0x04, "4"                      },
	{0x1e, 0x01, 0x07, 0x05, "5"                      },
	{0x1e, 0x01, 0x07, 0x06, "6"                      },
	{0x1e, 0x01, 0x07, 0x07, "7 (Hardest)"            },

	{0   , 0xfe, 0   , 8   , "Lives"                  },
	{0x1e, 0x01, 0x70, 0x00, "Start 1 Continue 1"     },
	{0x1e, 0x01, 0x70, 0x10, "Start 2 Continue 2"     },
	{0x1e, 0x01, 0x70, 0x20, "Start 3 Continue 3"     },
	{0x1e, 0x01, 0x70, 0x30, "Start 4 Continue 4"     },
	{0x1e, 0x01, 0x70, 0x40, "Start 1 Continue 2"     },
	{0x1e, 0x01, 0x70, 0x50, "Start 2 Continue 3"     },
	{0x1e, 0x01, 0x70, 0x60, "Start 3 Continue 4"     },
	{0x1e, 0x01, 0x70, 0x70, "Start 4 Continue 5"     },

	// Dip C
	{0   , 0xfe, 0   , 3   , "Coin Slots" },
	{0x1f, 0x01, 0x03, 0x00, "3 Players 3 Shooters"   },
	{0x1f, 0x01, 0x03, 0x01, "3 Players 1 Shooter"    },
	{0x1f, 0x01, 0x03, 0x02, "2 Players 1 Shooter"    },
};

STDDIPINFO(wofhfh);


//----------------------------------------------------------------------------------------------
// Drivers


#define A_BOARD_PLDS						\
	{ "buf1",          0x000117, 0xeb122de7, 0 | BRF_OPT },	\
	{ "ioa1",          0x000117, 0x59c7ee3b, 0 | BRF_OPT },	\
	{ "prg1",          0x000117, 0xf1129744, 0 | BRF_OPT },	\
	{ "rom1",          0x000117, 0x41dc73b9, 0 | BRF_OPT },	\
	{ "sou1",          0x000117, 0x84f4b2fe, 0 | BRF_OPT },

#define A_BOARD_QSOUND_PLDS					\
	{ "buf1",          0x000117, 0xeb122de7, 0 | BRF_OPT },	\
	{ "ioa1",          0x000117, 0x59c7ee3b, 0 | BRF_OPT },	\
	{ "prg2",          0x000117, 0x4386879a, 0 | BRF_OPT },	\
	{ "rom1",          0x000117, 0x41dc73b9, 0 | BRF_OPT },	\


// Forgotten Worlds (World)

static struct BurnRomInfo ForgottnRomDesc[] = {
	{ "lw_11.12f",  0x020000, 0x73e920b7, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "lw_15.12h",  0x020000, 0x50d7012d, 1 | BRF_ESS | BRF_PRG },
	{ "lw_10.13f",  0x020000, 0xbea45994, 1 | BRF_ESS | BRF_PRG },
	{ "lw_14.13h",  0x020000, 0x539b2339, 1 | BRF_ESS | BRF_PRG },
	{ "lw-07.10g",  0x080000, 0xfd252a26, 1 | BRF_ESS | BRF_PRG },

	{ "lw_2.2b",    0x020000, 0x4bd75fee, 2 | BRF_GRA },
	{ "lw_1.2a",    0x020000, 0x65f41485, 2 | BRF_GRA },
	{ "lw-08.9b",   0x080000, 0x25a8e43c, 2 | BRF_GRA },
	{ "lw-05.6d",   0x080000, 0xe4552fd7, 2 | BRF_GRA },
	{ "lw_30.8h",   0x020000, 0xb385954e, 2 | BRF_GRA },
	{ "lw_29.8f",   0x020000, 0x7bda1ac6, 2 | BRF_GRA },
	{ "lw_4.3b",    0x020000, 0x50cf757f, 2 | BRF_GRA },
	{ "lw_3.3a",    0x020000, 0xc03ef278, 2 | BRF_GRA },
	{ "lw_32.9h",   0x020000, 0x30967a15, 2 | BRF_GRA },
	{ "lw_31.9f",   0x020000, 0xc49d37fb, 2 | BRF_GRA },
	{ "lw-02.6b",   0x080000, 0x43e6c5c8, 2 | BRF_GRA },
	{ "lw_14.10b",  0x020000, 0x82862cce, 2 | BRF_GRA },
	{ "lw_13.10a",  0x020000, 0xb81c0e96, 2 | BRF_GRA },
	{ "lw-06.9d",   0x080000, 0x5b9edffc, 2 | BRF_GRA },
	{ "lw_26.10e",  0x020000, 0x57bcd032, 2 | BRF_GRA },
	{ "lw_25.10c",  0x020000, 0xbac91554, 2 | BRF_GRA },
	{ "lw_16.11b",  0x020000, 0x40b26554, 2 | BRF_GRA },
	{ "lw_15.11a",  0x020000, 0x1b7d2e07, 2 | BRF_GRA },
	{ "lw_28.11e",  0x020000, 0xa805ad30, 2 | BRF_GRA },
	{ "lw_27.11c",  0x020000, 0x103c1bd2, 2 | BRF_GRA },

	{ "lw_00.13c",  0x010000, 0x59df2a63, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "lw-03u.12e", 0x020000, 0x807d051f, 4 | BRF_SND },
	{ "lw-04u.13e", 0x020000, 0xe6cd098e, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "lw621.1a",   0x000117, 0x5eec6ce9, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.12b",   0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Forgottn) STD_ROM_FN(Forgottn)

static void forgottn_stars()
{
	CpsLoadStarsForgottnAlt(CpsStar, 15);
}

static int forgottnInit()
{
	pCpsInitCallback = forgottn_stars;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsForgottn = {
	"forgottn", NULL, NULL, "1988",
	"Forgotten Worlds (World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1_GENERIC,
	NULL,ForgottnRomInfo,ForgottnRomName,forgottnInputInfo, forgottnDIPInfo,
	forgottnInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Forgotten Worlds (US)

static struct BurnRomInfo ForgottnuRomDesc[] = {
	{ "lwu_11a.14f",   0x020000, 0xddf78831, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "lwu_15a.14g",   0x020000, 0xf7ce2097, 1 | BRF_ESS | BRF_PRG },
	{ "lwu_10a.13f",   0x020000, 0x8cb38c81, 1 | BRF_ESS | BRF_PRG },
	{ "lwu_14a.13g",   0x020000, 0xd70ef9fd, 1 | BRF_ESS | BRF_PRG },
	{ "lw-07.13e",     0x080000, 0xfd252a26, 1 | BRF_ESS | BRF_PRG },

	{ "lw-01.9d",      0x080000, 0x0318f298, 2 | BRF_GRA },
	{ "lw-08.9f",      0x080000, 0x25a8e43c, 2 | BRF_GRA },
	{ "lw-05.9e",      0x080000, 0xe4552fd7, 2 | BRF_GRA },
	{ "lw-12.9g",      0x080000, 0x8e6a832b, 2 | BRF_GRA },
	{ "lw-02.12d",     0x080000, 0x43e6c5c8, 2 | BRF_GRA },
	{ "lw-09.12f",     0x080000, 0x899cb4ad, 2 | BRF_GRA },
	{ "lw-06.12e",     0x080000, 0x5b9edffc, 2 | BRF_GRA },
	{ "lw-13.12g",     0x080000, 0x8e058ef5, 2 | BRF_GRA },

	{ "lwu_00.14a",    0x010000, 0x59df2a63, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "lw-03u.14c",    0x020000, 0x807d051f, 4 | BRF_SND },
	{ "lw-04u.13c",    0x020000, 0xe6cd098e, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Forgottnu) STD_ROM_FN(Forgottnu)

static void forgottnu_stars()
{
	CpsLoadStars(CpsStar, 9);
}

static int forgottnuInit()
{
	pCpsInitCallback = forgottnu_stars;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsForgottnu = {
	"forgottnu", "forgottn", NULL, "1988",
	"Forgotten Worlds (US)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_GENERIC,
	NULL,ForgottnuRomInfo,ForgottnuRomName,forgottnInputInfo, forgottnDIPInfo,
	forgottnuInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Lost Worlds (Japan)

static struct BurnRomInfo LostwrldRomDesc[] = {
	{ "lw_11c.14f", 0x020000, 0x67e42546, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "lw_15c.14g", 0x020000, 0x402e2a46, 1 | BRF_ESS | BRF_PRG },
	{ "lw_10c.13f", 0x020000, 0xc46479d7, 1 | BRF_ESS | BRF_PRG },
	{ "lw_14c.13g", 0x020000, 0x97670f4a, 1 | BRF_ESS | BRF_PRG },
	{ "lw-07.13e",  0x080000, 0xfd252a26, 1 | BRF_ESS | BRF_PRG },

	{ "lw-01.9d",   0x080000, 0x0318f298, 2 | BRF_GRA },
	{ "lw-08.9f",   0x080000, 0x25a8e43c, 2 | BRF_GRA },
	{ "lw-05.9e",   0x080000, 0xe4552fd7, 2 | BRF_GRA },
	{ "lw-12.9g",   0x080000, 0x8e6a832b, 2 | BRF_GRA },
	{ "lw-02.12d",  0x080000, 0x43e6c5c8, 2 | BRF_GRA },
	{ "lw-09.12f",  0x080000, 0x899cb4ad, 2 | BRF_GRA },
	{ "lw-06.12e",  0x080000, 0x5b9edffc, 2 | BRF_GRA },
	{ "lw-13.12g",  0x080000, 0x8e058ef5, 2 | BRF_GRA },

	{ "lw_00b.14a", 0x010000, 0x59df2a63, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "lw-03.14c",  0x020000, 0xce2159e7, 4 | BRF_SND },
	{ "lw-04.13c",  0x020000, 0x39305536, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Lostwrld) STD_ROM_FN(Lostwrld)

struct BurnDriver BurnDrvCpsLostwrld = {
	"lostwrld", "forgottn", NULL, "1988",
	"Lost Worlds (Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_GENERIC,
	NULL,LostwrldRomInfo,LostwrldRomName,forgottnInputInfo, forgottnDIPInfo,
	forgottnuInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Lost Worlds (Japan Old Ver.)

static struct BurnRomInfo LostwrldoRomDesc[] = {
	{ "lw_11.14f",  0x020000, 0x61e2cc56, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "lw_15.14g",  0x020000, 0x8a0c18d3, 1 | BRF_ESS | BRF_PRG },
	{ "lw_10.13f",  0x020000, 0x23bca4d5, 1 | BRF_ESS | BRF_PRG },
	{ "lw_14.13g",  0x020000, 0x3a023771, 1 | BRF_ESS | BRF_PRG },
	{ "lw-07.13e",  0x080000, 0xfd252a26, 1 | BRF_ESS | BRF_PRG },

	{ "lw-01.9d",   0x080000, 0x0318f298, 2 | BRF_GRA },
	{ "lw-08.9f",   0x080000, 0x25a8e43c, 2 | BRF_GRA },
	{ "lw-05.9e",   0x080000, 0xe4552fd7, 2 | BRF_GRA },
	{ "lw-12.9g",   0x080000, 0x8e6a832b, 2 | BRF_GRA },
	{ "lw-02.12d",  0x080000, 0x43e6c5c8, 2 | BRF_GRA },
	{ "lw-09.12f",  0x080000, 0x899cb4ad, 2 | BRF_GRA },
	{ "lw-06.12e",  0x080000, 0x5b9edffc, 2 | BRF_GRA },
	{ "lw-13.12g",  0x080000, 0x8e058ef5, 2 | BRF_GRA },

	{ "lw_00b.14a", 0x010000, 0x59df2a63, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "lw-03.14c",  0x020000, 0xce2159e7, 4 | BRF_SND },
	{ "lw-04.13c",  0x020000, 0x39305536, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Lostwrldo) STD_ROM_FN(Lostwrldo)

struct BurnDriver BurnDrvCpsLostwrldo = {
	"lostwrldo", "forgottn", NULL, "1988",
	"Lost Worlds (Japan Old Ver.)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_GENERIC,
	NULL,LostwrldoRomInfo,LostwrldoRomName,forgottnInputInfo, forgottnDIPInfo,
	forgottnuInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Ghouls'n Ghosts (World)

static struct BurnRomInfo GhoulsRomDesc[] = {
	{ "dme_29.10h", 0x020000, 0x166a58a2, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "dme_30.10j", 0x020000, 0x7ac8407a, 1 | BRF_ESS | BRF_PRG },
	{ "dme_27.9h",  0x020000, 0xf734b2be, 1 | BRF_ESS | BRF_PRG },
	{ "dme_28.9j",  0x020000, 0x03d3e714, 1 | BRF_ESS | BRF_PRG },
	{ "dm-17.7j",   0x080000, 0x3ea1b0f2, 1 | BRF_ESS | BRF_PRG },

	{ "dm-05.3a",   0x080000, 0x0ba9c0b0, 2 | BRF_GRA },
	{ "dm-07.3f",   0x080000, 0x5d760ab9, 2 | BRF_GRA },
	{ "dm-06.3c",   0x080000, 0x4ba90b59, 2 | BRF_GRA },
	{ "dm-08.3g",   0x080000, 0x4bdee9de, 2 | BRF_GRA },
	{ "dm_09.4a",   0x010000, 0xae24bb19, 2 | BRF_GRA },
	{ "dm_18.7a",   0x010000, 0xd34e271a, 2 | BRF_GRA },
	{ "dm_13.4e",   0x010000, 0x3f70dd37, 2 | BRF_GRA },
	{ "dm_22.7e",   0x010000, 0x7e69e2e6, 2 | BRF_GRA },
	{ "dm_11.4c",   0x010000, 0x37c9b6c6, 2 | BRF_GRA },
	{ "dm_20.7c",   0x010000, 0x2f1345b4, 2 | BRF_GRA },
	{ "dm_15.4g",   0x010000, 0x3c2a212a, 2 | BRF_GRA },
	{ "dm_24.7g",   0x010000, 0x889aac05, 2 | BRF_GRA },
	{ "dm_10.4b",   0x010000, 0xbcc0f28c, 2 | BRF_GRA },
	{ "dm_19.7b",   0x010000, 0x2a40166a, 2 | BRF_GRA },
	{ "dm_14.4f",   0x010000, 0x20f85c03, 2 | BRF_GRA },
	{ "dm_23.7f",   0x010000, 0x8426144b, 2 | BRF_GRA },
	{ "dm_12.4d",   0x010000, 0xda088d61, 2 | BRF_GRA },
	{ "dm_21.7d",   0x010000, 0x17e11df0, 2 | BRF_GRA },
	{ "dm_16.4h",   0x010000, 0xf187ba1c, 2 | BRF_GRA },
	{ "dm_25.7h",   0x010000, 0x29f79c78, 2 | BRF_GRA },

	{ "dm_26.10a",  0x010000, 0x3692f6e5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	A_BOARD_PLDS

	{ "dm620.2a",   0x000117, 0xf6e5f727, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.8i",    0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Ghouls) STD_ROM_FN(Ghouls)

static void ghouls_program_fix()
{
/*	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "ghouls"))
	{
		// Patch out self-test... it takes forever
		*((unsigned short*)(CpsRom + 0x61964)) = 0x4ef9;
		*((unsigned short*)(CpsRom + 0x61966)) = 0x0000;
		*((unsigned short*)(CpsRom + 0x61968)) = 0x0400;
	}
*/
	BurnByteswap(CpsRom + 0x080000, 0x080000);
}

static int ghoulsInit()
{
	pCpsInitCallback = ghouls_program_fix;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsGhouls = {
	"ghouls", NULL, NULL, "1988",
	"Ghouls'n Ghosts (World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,GhoulsRomInfo,GhoulsRomName,ghoulsInputInfo, ghoulsDIPInfo,
	ghoulsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Ghouls'n Ghosts (US)

static struct BurnRomInfo GhoulsuRomDesc[] = {
	{ "dmu_29.10h", 0x020000, 0x334d85b2, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "dmu_30.10j", 0x020000, 0xcee8ceb5, 1 | BRF_ESS | BRF_PRG },
	{ "dmu_27.9h",  0x020000, 0x4a524140, 1 | BRF_ESS | BRF_PRG },
	{ "dmu_28.9j",  0x020000, 0x94aae205, 1 | BRF_ESS | BRF_PRG },
	{ "dm-17.7j",   0x080000, 0x3ea1b0f2, 1 | BRF_ESS | BRF_PRG },

	{ "dm-05.3a",   0x080000, 0x0ba9c0b0, 2 | BRF_GRA },
	{ "dm-07.3f",   0x080000, 0x5d760ab9, 2 | BRF_GRA },
	{ "dm-06.3c",   0x080000, 0x4ba90b59, 2 | BRF_GRA },
	{ "dm-08.3g",   0x080000, 0x4bdee9de, 2 | BRF_GRA },
	{ "dm_09.4a",   0x010000, 0xae24bb19, 2 | BRF_GRA },
	{ "dm_18.7a",   0x010000, 0xd34e271a, 2 | BRF_GRA },
	{ "dm_13.4e",   0x010000, 0x3f70dd37, 2 | BRF_GRA },
	{ "dm_22.7e",   0x010000, 0x7e69e2e6, 2 | BRF_GRA },
	{ "dm_11.4c",   0x010000, 0x37c9b6c6, 2 | BRF_GRA },
	{ "dm_20.7c",   0x010000, 0x2f1345b4, 2 | BRF_GRA },
	{ "dm_15.4g",   0x010000, 0x3c2a212a, 2 | BRF_GRA },
	{ "dm_24.7g",   0x010000, 0x889aac05, 2 | BRF_GRA },
	{ "dm_10.4b",   0x010000, 0xbcc0f28c, 2 | BRF_GRA },
	{ "dm_19.7b",   0x010000, 0x2a40166a, 2 | BRF_GRA },
	{ "dm_14.4f",   0x010000, 0x20f85c03, 2 | BRF_GRA },
	{ "dm_23.7f",   0x010000, 0x8426144b, 2 | BRF_GRA },
	{ "dm_12.4d",   0x010000, 0xda088d61, 2 | BRF_GRA },
	{ "dm_21.7d",   0x010000, 0x17e11df0, 2 | BRF_GRA },
	{ "dm_16.4h",   0x010000, 0xf187ba1c, 2 | BRF_GRA },
	{ "dm_25.7h",   0x010000, 0x29f79c78, 2 | BRF_GRA },

	{ "dm_26.10a",  0x010000, 0x3692f6e5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	A_BOARD_PLDS

	{ "dm620.2a",   0x000117, 0xf6e5f727, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.8i",    0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Ghoulsu) STD_ROM_FN(Ghoulsu)

struct BurnDriver BurnDrvCpsGhoulsu = {
	"ghoulsu", "ghouls", NULL, "1988",
	"Ghouls'n Ghosts (US)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,GhoulsuRomInfo,GhoulsuRomName,ghoulsInputInfo, ghoulsuDIPInfo,
	ghoulsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Dai Makai-Mura (Japan)

static struct BurnRomInfo daimakaiRomDesc[] = {
	{ "dmj_38.12f", 0x020000, 0x82fd1798, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "dmj_39.12h", 0x020000, 0x35366ccc, 1 | BRF_ESS | BRF_PRG },
	{ "dmj_40.13f", 0x020000, 0xa17c170a, 1 | BRF_ESS | BRF_PRG },
	{ "dmj_41.13h", 0x020000, 0x6af0b391, 1 | BRF_ESS | BRF_PRG },
	{ "dm_33.10f",  0x020000, 0x384d60c4, 1 | BRF_ESS | BRF_PRG },
	{ "dm_34.10h",  0x020000, 0x19abe30f, 1 | BRF_ESS | BRF_PRG },
	{ "dm_35.11f",  0x020000, 0xc04b85c8, 1 | BRF_ESS | BRF_PRG },
	{ "dm_36.11h",  0x020000, 0x89be83de, 1 | BRF_ESS | BRF_PRG },

	{ "dm_02.4b",   0x020000, 0x8b98dc48, 2 | BRF_GRA },
	{ "dm_01.4a",   0x020000, 0x80896c33, 2 | BRF_GRA },
	{ "dm_10.9b",   0x020000, 0xc2e7d9ef, 2 | BRF_GRA },
	{ "dm_09.9a",   0x020000, 0xc9c4afa5, 2 | BRF_GRA },
	{ "dm_18.5e",   0x020000, 0x1aa0db99, 2 | BRF_GRA },
	{ "dm_17.5c",   0x020000, 0xdc6ed8ad, 2 | BRF_GRA },
	{ "dm_30.8h",   0x020000, 0xd9d3f8bd, 2 | BRF_GRA },
	{ "dm_29.8f",   0x020000, 0x49a48796, 2 | BRF_GRA },
	{ "dm_04.5b",   0x020000, 0xa4f4f8f0, 2 | BRF_GRA },
	{ "dm_03.5a",   0x020000, 0xb1033e62, 2 | BRF_GRA },
	{ "dm_12.10b",  0x020000, 0x10fdd76a, 2 | BRF_GRA },
	{ "dm_11.10a",  0x020000, 0x9040cb04, 2 | BRF_GRA },
	{ "dm_20.7e",   0x020000, 0x281d0b3e, 2 | BRF_GRA },
	{ "dm_19.7c",   0x020000, 0x2623b52f, 2 | BRF_GRA },
	{ "dm_32.9h",   0x020000, 0x99692344, 2 | BRF_GRA },
	{ "dm_31.9f",   0x020000, 0x54acb729, 2 | BRF_GRA },
	{ "dm_06.7b",   0x010000, 0xae24bb19, 2 | BRF_GRA },
	{ "dm_05.7a",   0x010000, 0xd34e271a, 2 | BRF_GRA },
	{ "dm_14.11b",  0x010000, 0x3f70dd37, 2 | BRF_GRA },
	{ "dm_13.11a",  0x010000, 0x7e69e2e6, 2 | BRF_GRA },
	{ "dm_22.8e",   0x010000, 0x37c9b6c6, 2 | BRF_GRA },
	{ "dm_21.8c",   0x010000, 0x2f1345b4, 2 | BRF_GRA },
	{ "dm_26.10e",  0x010000, 0x3c2a212a, 2 | BRF_GRA },
	{ "dm_25.10c",  0x010000, 0x889aac05, 2 | BRF_GRA },
	{ "dm_08.8b",   0x010000, 0xbcc0f28c, 2 | BRF_GRA },
	{ "dm_07.8a",   0x010000, 0x2a40166a, 2 | BRF_GRA },
	{ "dm_16.12b",  0x010000, 0x20f85c03, 2 | BRF_GRA },
	{ "dm_15.12a",  0x010000, 0x8426144b, 2 | BRF_GRA },
	{ "dm_24.9e",   0x010000, 0xda088d61, 2 | BRF_GRA },
	{ "dm_23.9c",   0x010000, 0x17e11df0, 2 | BRF_GRA },
	{ "dm_28.11e",  0x010000, 0xf187ba1c, 2 | BRF_GRA },
	{ "dm_27.11c",  0x010000, 0x29f79c78, 2 | BRF_GRA },

	{ "dm_37.13c",  0x010000, 0x3692f6e5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	A_BOARD_PLDS

	{ "dm22a.1a",   0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",   0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(daimakai) STD_ROM_FN(daimakai)

struct BurnDriver BurnDrvCpsDaimakai = {
	"daimakai", "ghouls", NULL, "1988",
	"Dai Makai-Mura (Japan)\0", NULL, "Capcom", "CPS1",
	L"\u5927\u9B54\u754C\u6751\0Dai Makai-Mura (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,daimakaiRomInfo,daimakaiRomName,ghoulsInputInfo, daimakaiDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Dai Makai-Mura (Japan Resale Ver.)

static struct BurnRomInfo daimakairRomDesc[] = {
	{ "damj_23.bin", 0x080000, 0xc3b248ec, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "damj_22.bin", 0x080000, 0x595ff2f3, 1 | BRF_ESS | BRF_PRG },

	{ "dam_01.bin",  0x080000, 0x0ba9c0b0, 2 | BRF_GRA },
	{ "dam_02.bin",  0x080000, 0x5d760ab9, 2 | BRF_GRA },
	{ "dam_03.bin",  0x080000, 0x4ba90b59, 2 | BRF_GRA },
	{ "dam_04.bin",  0x080000, 0x4bdee9de, 2 | BRF_GRA },
	{ "dam_05.bin",  0x080000, 0x7dc61b94, 2 | BRF_GRA },
	{ "dam_06.bin",  0x080000, 0xfde89758, 2 | BRF_GRA },
	{ "dam_07.bin",  0x080000, 0xec351d78, 2 | BRF_GRA },
	{ "dam_08.bin",  0x080000, 0xee2acc1e, 2 | BRF_GRA },

	{ "dam_09.bin",  0x020000, 0x0656ff53, 3 | BRF_ESS | BRF_PRG }, // z80 program

	A_BOARD_PLDS

	{ "dam63b.1a",   0x000117, 0x474b3c8a, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",    0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "bprg1.11d",   0x000117, 0x31793da7, 0 | BRF_OPT },
	{ "ioc1.ic7",    0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
	{ "c632.ic1",    0x000117, 0x0fbd9270, 0 | BRF_OPT },
};

STD_ROM_PICK(daimakair) STD_ROM_FN(daimakair)

struct BurnDriver BurnDrvCpsDaimakair = {
	"daimakair", "ghouls", NULL, "1988",
	"Dai Makai-Mura (Japan Resale Ver.)\0", NULL, "Capcom", "CPS1",
	L"\u5927\u9B54\u754C\u6751\0Dai Makai-Mura (Japan Resale Ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,daimakairRomInfo,daimakairRomName,ghoulsInputInfo, daimakaiDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Strider (US set 1)

static struct BurnRomInfo StriderRomDesc[] = {
	{ "strider.30", 0x020000, 0xda997474, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "strider.35", 0x020000, 0x5463aaa3, 1 | BRF_ESS | BRF_PRG },
	{ "strider.31", 0x020000, 0xd20786db, 1 | BRF_ESS | BRF_PRG },
	{ "strider.36", 0x020000, 0x21aa2863, 1 | BRF_ESS | BRF_PRG },
	{ "st-14.8h",   0x080000, 0x9b3cfc08, 1 | BRF_ESS | BRF_PRG },

	{ "st-2.8a",    0x080000, 0x4eee9aea, 2 | BRF_GRA },
	{ "st-11.10a",  0x080000, 0x2d7f21e4, 2 | BRF_GRA },
	{ "st-5.4a",    0x080000, 0x7705aa46, 2 | BRF_GRA },
	{ "st-9.6a",    0x080000, 0x5b18b722, 2 | BRF_GRA },
	{ "st-1.7a",    0x080000, 0x005f000b, 2 | BRF_GRA },
	{ "st-10.9a",   0x080000, 0xb9441519, 2 | BRF_GRA },
	{ "st-4.3a",    0x080000, 0xb7d04e8b, 2 | BRF_GRA },
	{ "st-8.5a",    0x080000, 0x6b4713b4, 2 | BRF_GRA },

	{ "strider.09", 0x010000, 0x2ed403bc, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "strider.18", 0x020000, 0x4386bc80, 4 | BRF_SND },
	{ "strider.19", 0x020000, 0x444536d7, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "st24m1.1a",  0x000117, 0xa80d357e, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.11e",   0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Strider) STD_ROM_FN(Strider)

static void strider_stars()
{
	CpsLoadStars(CpsStar, 5);
}

static int striderInit()
{
	pCpsInitCallback = strider_stars;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsStrider = {
	"strider", NULL, NULL, "1989",
	"Strider (US set 1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,StriderRomInfo,StriderRomName,striderInputInfo, striderDIPInfo,
	striderInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Strider (US set 2)

static struct BurnRomInfo StrideruaRomDesc[] = {
	{ "strid.30",   0x020000, 0x66aec273, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "strid.35",   0x020000, 0x50e0e865, 1 | BRF_ESS | BRF_PRG },
	{ "strid.31",   0x020000, 0xeae93bd1, 1 | BRF_ESS | BRF_PRG },
	{ "strid.36",   0x020000, 0xb904a31d, 1 | BRF_ESS | BRF_PRG },
	{ "st-14.8h",   0x080000, 0x9b3cfc08, 1 | BRF_ESS | BRF_PRG },

	{ "st-2.8a",    0x080000, 0x4eee9aea, 2 | BRF_GRA },
	{ "st-11.10a",  0x080000, 0x2d7f21e4, 2 | BRF_GRA },
	{ "st-5.4a",    0x080000, 0x7705aa46, 2 | BRF_GRA },
	{ "st-9.6a",    0x080000, 0x5b18b722, 2 | BRF_GRA },
	{ "st-1.7a",    0x080000, 0x005f000b, 2 | BRF_GRA },
	{ "st-10.9a",   0x080000, 0xb9441519, 2 | BRF_GRA },
	{ "st-4.3a",    0x080000, 0xb7d04e8b, 2 | BRF_GRA },
	{ "st-8.5a",    0x080000, 0x6b4713b4, 2 | BRF_GRA },

	{ "strid.09",   0x010000, 0x08d63519, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "strider.18", 0x020000, 0x4386bc80, 4 | BRF_SND },
	{ "strider.19", 0x020000, 0x444536d7, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "st24m1.1a",  0x000117, 0xa80d357e, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.11e",   0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Striderua) STD_ROM_FN(Striderua)

struct BurnDriver BurnDrvCpsStriderua = {
	"striderua", "strider", NULL, "1989",
	"Strider (US set 2)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,StrideruaRomInfo,StrideruaRomName,striderInputInfo, strideruaDIPInfo,
	striderInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Strider Hiryu (Japan)

static struct BurnRomInfo StriderjRomDesc[] = {
	{ "sth36.bin",  0x020000, 0x53c7b006, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sth42.bin",  0x020000, 0x4037f65f, 1 | BRF_ESS | BRF_PRG },
	{ "sth37.bin",  0x020000, 0x80e8877d, 1 | BRF_ESS | BRF_PRG },
	{ "sth43.bin",  0x020000, 0x6b3fa466, 1 | BRF_ESS | BRF_PRG },
	{ "sth34.bin",  0x020000, 0xbea770b5, 1 | BRF_ESS | BRF_PRG },
	{ "sth40.bin",  0x020000, 0x43b922dc, 1 | BRF_ESS | BRF_PRG },
	{ "sth35.bin",  0x020000, 0x5cc429da, 1 | BRF_ESS | BRF_PRG },
	{ "sth41.bin",  0x020000, 0x50af457f, 1 | BRF_ESS | BRF_PRG },

	{ "sth09.bin",  0x020000, 0x1ef6bfbd, 2 | BRF_GRA },
	{ "sth01.bin",  0x020000, 0x1e21b0c1, 2 | BRF_GRA },
	{ "sth13.bin",  0x020000, 0x063263ae, 2 | BRF_GRA },
	{ "sth05.bin",  0x020000, 0xec9f8714, 2 | BRF_GRA },
	{ "sth24.bin",  0x020000, 0x6356f4d2, 2 | BRF_GRA },
	{ "sth17.bin",  0x020000, 0xb4f73d86, 2 | BRF_GRA },
	{ "sth38.bin",  0x020000, 0xee5abfc2, 2 | BRF_GRA },
	{ "sth32.bin",  0x020000, 0x44a206a3, 2 | BRF_GRA },
	{ "sth10.bin",  0x020000, 0xdf3dd3bc, 2 | BRF_GRA },
	{ "sth02.bin",  0x020000, 0xc75f9ea0, 2 | BRF_GRA },
	{ "sth14.bin",  0x020000, 0x6c03e19d, 2 | BRF_GRA },
	{ "sth06.bin",  0x020000, 0xd84f5478, 2 | BRF_GRA },
	{ "sth25.bin",  0x020000, 0x921e506a, 2 | BRF_GRA },
	{ "sth18.bin",  0x020000, 0x5b318956, 2 | BRF_GRA },
	{ "sth39.bin",  0x020000, 0x9321d6aa, 2 | BRF_GRA },
	{ "sth33.bin",  0x020000, 0xb47ddfc7, 2 | BRF_GRA },
	{ "sth11.bin",  0x020000, 0x2484f241, 2 | BRF_GRA },
	{ "sth03.bin",  0x020000, 0xaaa07245, 2 | BRF_GRA },
	{ "sth15.bin",  0x020000, 0xe415d943, 2 | BRF_GRA },
	{ "sth07.bin",  0x020000, 0x97d072d2, 2 | BRF_GRA },
	{ "sth26.bin",  0x020000, 0x0ebfcb02, 2 | BRF_GRA },
	{ "sth19.bin",  0x020000, 0x257ce683, 2 | BRF_GRA },
	{ "sth28.bin",  0x020000, 0x98ac8cd1, 2 | BRF_GRA },
	{ "sth21.bin",  0x020000, 0x538d9423, 2 | BRF_GRA },
	{ "sth12.bin",  0x020000, 0xf670a477, 2 | BRF_GRA },
	{ "sth04.bin",  0x020000, 0x853d3e01, 2 | BRF_GRA },
	{ "sth16.bin",  0x020000, 0x4092019f, 2 | BRF_GRA },
	{ "sth08.bin",  0x020000, 0x2ce9b4c7, 2 | BRF_GRA },
	{ "sth27.bin",  0x020000, 0xf82c88d9, 2 | BRF_GRA },
	{ "sth20.bin",  0x020000, 0xeb584dd4, 2 | BRF_GRA },
	{ "sth29.bin",  0x020000, 0x34ae2997, 2 | BRF_GRA },
	{ "sth22.bin",  0x020000, 0x78dd9c48, 2 | BRF_GRA },

	{ "sth23.bin",  0x010000, 0x2ed403bc, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sth30.bin",  0x020000, 0x4386bc80, 4 | BRF_SND },
	{ "sth31.bin",  0x020000, 0x444536d7, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Striderj) STD_ROM_FN(Striderj)

static void striderj_stars()
{
	CpsLoadStarsByte(CpsStar, 8);
}

static int striderjInit()
{
	pCpsInitCallback = striderj_stars;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsStriderj = {
	"striderj", "strider", NULL, "1989",
	"Strider Hiryu (Japan)\0", NULL, "Capcom", "CPS1",
	L"\u30B9\u30C8\u30E9\u30A4\u30C0\u30FC\u98DB\u7ADC (Japan)\0Strider Hiryu (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,StriderjRomInfo,StriderjRomName,striderInputInfo, striderDIPInfo,
	striderjInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Strider Hiryu (Japan Resale Ver.)

static struct BurnRomInfo StriderjrRomDesc[] = {
	{ "sthj_23.bin", 0x080000, 0x046e7b12, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sthj_22.bin", 0x080000, 0x9b3cfc08, 1 | BRF_ESS | BRF_PRG },

	{ "sth_01.bin",  0x080000, 0x4eee9aea, 2 | BRF_GRA },
	{ "sth_02.bin",  0x080000, 0x2d7f21e4, 2 | BRF_GRA },
	{ "sth_03.bin",  0x080000, 0x7705aa46, 2 | BRF_GRA },
	{ "sth_04.bin",  0x080000, 0x5b18b722, 2 | BRF_GRA },
	{ "sth_05.bin",  0x080000, 0x005f000b, 2 | BRF_GRA },
	{ "sth_06.bin",  0x080000, 0xb9441519, 2 | BRF_GRA },
	{ "sth_07.bin",  0x080000, 0xb7d04e8b, 2 | BRF_GRA },
	{ "sth_08.bin",  0x080000, 0x6b4713b4, 2 | BRF_GRA },

	{ "sth_09.bin",  0x010000, 0x08d63519, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sth_18.bin",  0x020000, 0x4386bc80, 4 | BRF_SND },
	{ "sth_19.bin",  0x020000, 0x444536d7, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Striderjr) STD_ROM_FN(Striderjr)

static void striderjr_stars()
{
	CpsLoadStars(CpsStar, 2);
}

static int striderjrInit()
{
	pCpsInitCallback = striderjr_stars;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsStriderjr = {
	"striderjr", "strider", NULL, "1989",
	"Strider Hiryu (Japan Resale Ver.)\0", NULL, "Capcom", "CPS1",
	L"\u30B9\u30C8\u30E9\u30A4\u30C0\u30FC\u98DB\u7ADC (Japan Resale Ver.)\0Strider Hiryu (Japan Resale Ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,StriderjrRomInfo,StriderjrRomName,striderInputInfo, striderDIPInfo,
	striderjrInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Dynasty Wars (US set 1)

static struct BurnRomInfo DynwarRomDesc[] = {
	{ "30.11f",     0x020000, 0xf9ec6d68, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "35.11h",     0x020000, 0xe41fff2f, 1 | BRF_ESS | BRF_PRG },
	{ "31.12f",     0x020000, 0xe3de76ff, 1 | BRF_ESS | BRF_PRG },
	{ "36.12h",     0x020000, 0x7a13cfbf, 1 | BRF_ESS | BRF_PRG },
	{ "tkm-9.8h",   0x080000, 0x93654bcf, 1 | BRF_ESS | BRF_PRG },

	{ "tkm-5.7a",   0x080000, 0xf64bb6a0, 2 | BRF_GRA },
	{ "tkm-8.9a",   0x080000, 0x21fe6274, 2 | BRF_GRA },
	{ "tkm-6.3a",   0x080000, 0x0bf228cb, 2 | BRF_GRA },
	{ "tkm-7.5a",   0x080000, 0x1255dfb1, 2 | BRF_GRA },
	{ "tkm-1.8a",   0x080000, 0x44f7661e, 2 | BRF_GRA },
	{ "tkm-4.10a",  0x080000, 0xa54c515d, 2 | BRF_GRA },
	{ "tkm-2.4a",   0x080000, 0xca5c687c, 2 | BRF_GRA },
	{ "tkm-3.6a",   0x080000, 0xf9fe6591, 2 | BRF_GRA },

	{ "tke_17.12b", 0x010000, 0xb3b79d4f, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "tke_18.11c", 0x020000, 0xac6e307d, 4 | BRF_SND },
	{ "tke_19.12c", 0x020000, 0x068741db, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Dynwar) STD_ROM_FN(Dynwar)

struct BurnDriver BurnDrvCpsDynwar = {
	"dynwar", NULL, NULL, "1989",
	"Dynasty Wars (US set 1)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,DynwarRomInfo,DynwarRomName,dynwarInputInfo, dynwarDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Dynasty Wars (US set 2)

static struct BurnRomInfo DynwaruRomDesc[] = {
	{ "36",     0x020000, 0x895991d1, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "42",     0x020000, 0xc898d2e8, 1 | BRF_ESS | BRF_PRG },
	{ "37",     0x020000, 0xb228d58c, 1 | BRF_ESS | BRF_PRG },
	{ "43",     0x020000, 0x1a14375a, 1 | BRF_ESS | BRF_PRG },
	{ "34.bin", 0x020000, 0x8f663d00, 1 | BRF_ESS | BRF_PRG },
	{ "40.bin", 0x020000, 0x1586dbf3, 1 | BRF_ESS | BRF_PRG },
	{ "35.bin", 0x020000, 0x9db93d7a, 1 | BRF_ESS | BRF_PRG },
	{ "41.bin", 0x020000, 0x1aae69a4, 1 | BRF_ESS | BRF_PRG },

	{ "09.bin", 0x020000, 0xc3e83c69, 2 | BRF_GRA },
	{ "01.bin", 0x020000, 0x187b2886, 2 | BRF_GRA },
	{ "13.bin", 0x020000, 0x0273d87d, 2 | BRF_GRA },
	{ "05.bin", 0x020000, 0x339378b8, 2 | BRF_GRA },
	{ "24.bin", 0x020000, 0xc6909b6f, 2 | BRF_GRA },
	{ "17.bin", 0x020000, 0x2e2f8320, 2 | BRF_GRA },
	{ "38.bin", 0x020000, 0xcd7923ed, 2 | BRF_GRA },
	{ "32.bin", 0x020000, 0x21a0a453, 2 | BRF_GRA },
	{ "10.bin", 0x020000, 0xff28f8d0, 2 | BRF_GRA },
	{ "02.bin", 0x020000, 0xcc83c02f, 2 | BRF_GRA },
	{ "14",     0x020000, 0x58d9b32f, 2 | BRF_GRA },
	{ "06.bin", 0x020000, 0x6f9edd75, 2 | BRF_GRA },
	{ "25.bin", 0x020000, 0x152ea74a, 2 | BRF_GRA },
	{ "18.bin", 0x020000, 0x1833f932, 2 | BRF_GRA },
	{ "39.bin", 0x020000, 0xbc09b360, 2 | BRF_GRA },
	{ "33.bin", 0x020000, 0x89de1533, 2 | BRF_GRA },
	{ "11.bin", 0x020000, 0x29eaf490, 2 | BRF_GRA },
	{ "03.bin", 0x020000, 0x7bf51337, 2 | BRF_GRA },
	{ "15.bin", 0x020000, 0xd36cdb91, 2 | BRF_GRA },
	{ "07.bin", 0x020000, 0xe04af054, 2 | BRF_GRA },
	{ "26.bin", 0x020000, 0x07fc714b, 2 | BRF_GRA },
	{ "19.bin", 0x020000, 0x7114e5c6, 2 | BRF_GRA },
	{ "28.bin", 0x020000, 0xaf62bf07, 2 | BRF_GRA },
	{ "21.bin", 0x020000, 0x523f462a, 2 | BRF_GRA },
	{ "12.bin", 0x020000, 0x38652339, 2 | BRF_GRA },
	{ "04.bin", 0x020000, 0x4951bc0f, 2 | BRF_GRA },
	{ "16.bin", 0x020000, 0x381608ae, 2 | BRF_GRA },
	{ "08.bin", 0x020000, 0xb475d4e9, 2 | BRF_GRA },
	{ "27.bin", 0x020000, 0xa27e81fa, 2 | BRF_GRA },
	{ "20.bin", 0x020000, 0x002796dc, 2 | BRF_GRA },
	{ "29.bin", 0x020000, 0x6b41f82d, 2 | BRF_GRA },
	{ "22.bin", 0x020000, 0x52145369, 2 | BRF_GRA },

	{ "23.bin", 0x010000, 0xb3b79d4f, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "30",     0x020000, 0xac6e307d, 4 | BRF_SND },
	{ "31",     0x020000, 0x068741db, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Dynwaru) STD_ROM_FN(Dynwaru)

struct BurnDriver BurnDrvCpsDynwaru = {
	"dynwaru", "dynwar", NULL, "1989",
	"Dynasty Wars (US set 2)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,DynwaruRomInfo,DynwaruRomName,dynwarInputInfo, dynwarDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Tenchi wo Kurau (Japan)

static struct BurnRomInfo DynwarjRomDesc[] = {
	{ "36.bin", 0x020000, 0x1a516657, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "42.bin", 0x020000, 0x12a290a0, 1 | BRF_ESS | BRF_PRG },
	{ "37.bin", 0x020000, 0x932fc943, 1 | BRF_ESS | BRF_PRG },
	{ "43.bin", 0x020000, 0x872ad76d, 1 | BRF_ESS | BRF_PRG },
	{ "34.bin", 0x020000, 0x8f663d00, 1 | BRF_ESS | BRF_PRG },
	{ "40.bin", 0x020000, 0x1586dbf3, 1 | BRF_ESS | BRF_PRG },
	{ "35.bin", 0x020000, 0x9db93d7a, 1 | BRF_ESS | BRF_PRG },
	{ "41.bin", 0x020000, 0x1aae69a4, 1 | BRF_ESS | BRF_PRG },

	{ "09.bin", 0x020000, 0xc3e83c69, 2 | BRF_GRA },
	{ "01.bin", 0x020000, 0x187b2886, 2 | BRF_GRA },
	{ "13.bin", 0x020000, 0x0273d87d, 2 | BRF_GRA },
	{ "05.bin", 0x020000, 0x339378b8, 2 | BRF_GRA },
	{ "24.bin", 0x020000, 0xc6909b6f, 2 | BRF_GRA },
	{ "17.bin", 0x020000, 0x2e2f8320, 2 | BRF_GRA },
	{ "38.bin", 0x020000, 0xcd7923ed, 2 | BRF_GRA },
	{ "32.bin", 0x020000, 0x21a0a453, 2 | BRF_GRA },
	{ "10.bin", 0x020000, 0xff28f8d0, 2 | BRF_GRA },
	{ "02.bin", 0x020000, 0xcc83c02f, 2 | BRF_GRA },
	{ "14.bin", 0x020000, 0x18fb232c, 2 | BRF_GRA },
	{ "06.bin", 0x020000, 0x6f9edd75, 2 | BRF_GRA },
	{ "25.bin", 0x020000, 0x152ea74a, 2 | BRF_GRA },
	{ "18.bin", 0x020000, 0x1833f932, 2 | BRF_GRA },
	{ "39.bin", 0x020000, 0xbc09b360, 2 | BRF_GRA },
	{ "33.bin", 0x020000, 0x89de1533, 2 | BRF_GRA },
	{ "11.bin", 0x020000, 0x29eaf490, 2 | BRF_GRA },
	{ "03.bin", 0x020000, 0x7bf51337, 2 | BRF_GRA },
	{ "15.bin", 0x020000, 0xd36cdb91, 2 | BRF_GRA },
	{ "07.bin", 0x020000, 0xe04af054, 2 | BRF_GRA },
	{ "26.bin", 0x020000, 0x07fc714b, 2 | BRF_GRA },
	{ "19.bin", 0x020000, 0x7114e5c6, 2 | BRF_GRA },
	{ "28.bin", 0x020000, 0xaf62bf07, 2 | BRF_GRA },
	{ "21.bin", 0x020000, 0x523f462a, 2 | BRF_GRA },
	{ "12.bin", 0x020000, 0x38652339, 2 | BRF_GRA },
	{ "04.bin", 0x020000, 0x4951bc0f, 2 | BRF_GRA },
	{ "16.bin", 0x020000, 0x381608ae, 2 | BRF_GRA },
	{ "08.bin", 0x020000, 0xb475d4e9, 2 | BRF_GRA },
	{ "27.bin", 0x020000, 0xa27e81fa, 2 | BRF_GRA },
	{ "20.bin", 0x020000, 0x002796dc, 2 | BRF_GRA },
	{ "29.bin", 0x020000, 0x6b41f82d, 2 | BRF_GRA },
	{ "22.bin", 0x020000, 0x52145369, 2 | BRF_GRA },

	{ "23.bin", 0x010000, 0xb3b79d4f, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "30.bin", 0x020000, 0x7e5f6cb4, 4 | BRF_SND },
	{ "31.bin", 0x020000, 0x4a30c737, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Dynwarj) STD_ROM_FN(Dynwarj)

struct BurnDriver BurnDrvCpsDynwarj = {
	"dynwarj", "dynwar", NULL, "1989",
	"Tenchi wo Kurau (Japan)\0", NULL, "Capcom", "CPS1",
	L"\u8EE2\u5730\u3092\u55B0\u3089\u3046\0Tenchi wo Kurau\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,DynwarjRomInfo,DynwarjRomName,dynwarInputInfo, dynwarDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Willow (US)

static struct BurnRomInfo WillowRomDesc[] = {
	{ "wlu_30.rom",  0x020000, 0xd604dbb1, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "willow-u.35", 0x020000, 0x7a791e77, 1 | BRF_ESS | BRF_PRG },
	{ "wlu_31.rom",  0x020000, 0x0eb48a83, 1 | BRF_ESS | BRF_PRG },
	{ "wlu_36.rom",  0x020000, 0x36100209, 1 | BRF_ESS | BRF_PRG },
	{ "wl_32.rom",   0x080000, 0xdfd9f643, 1 | BRF_ESS | BRF_PRG },

	{ "wl_gfx5.rom", 0x080000, 0xafa74b73, 2 | BRF_GRA },
	{ "wl_gfx7.rom", 0x080000, 0x12a0dc0b, 2 | BRF_GRA },
	{ "wl_gfx1.rom", 0x080000, 0xc6f2abce, 2 | BRF_GRA },
	{ "wl_gfx3.rom", 0x080000, 0x4aa4c6d3, 2 | BRF_GRA },
	{ "wl_24.rom",   0x020000, 0x6f0adee5, 2 | BRF_GRA },
	{ "wl_14.rom",   0x020000, 0x9cf3027d, 2 | BRF_GRA },
	{ "wl_26.rom",   0x020000, 0xf09c8ecf, 2 | BRF_GRA },
	{ "wl_16.rom",   0x020000, 0xe35407aa, 2 | BRF_GRA },
	{ "wl_20.rom",   0x020000, 0x84992350, 2 | BRF_GRA },
	{ "wl_10.rom",   0x020000, 0xb87b5a36, 2 | BRF_GRA },
	{ "wl_22.rom",   0x020000, 0xfd3f89f0, 2 | BRF_GRA },
	{ "wl_12.rom",   0x020000, 0x7da49d69, 2 | BRF_GRA },

	{ "wl_09.rom",   0x010000, 0xf6b3d060, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "wl_18.rom",   0x020000, 0xbde23d4d, 4 | BRF_SND },
	{ "wl_19.rom",   0x020000, 0x683898f5, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "wl24b.1a",    0x000117, 0x7101cdf1, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.11e",    0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Willow) STD_ROM_FN(Willow)

struct BurnDriver BurnDrvCpsWillow = {
	"willow", NULL, NULL, "1989",
	"Willow (US)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,WillowRomInfo,WillowRomName,willowInputInfo, willowDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Willow (Japan, Japanese)

static struct BurnRomInfo WillowjRomDesc[] = {
	{ "wl36.bin",    0x020000, 0x2b0d7cbc, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "wl42.bin",    0x020000, 0x1ac39615, 1 | BRF_ESS | BRF_PRG },
	{ "wl37.bin",    0x020000, 0x30a717fa, 1 | BRF_ESS | BRF_PRG },
	{ "wl43.bin",    0x020000, 0xd0dddc9e, 1 | BRF_ESS | BRF_PRG },
	{ "wl_32.rom",   0x080000, 0xdfd9f643, 1 | BRF_ESS | BRF_PRG },

	{ "wl_gfx5.rom", 0x080000, 0xafa74b73, 2 | BRF_GRA },
	{ "wl_gfx7.rom", 0x080000, 0x12a0dc0b, 2 | BRF_GRA },
	{ "wl_gfx1.rom", 0x080000, 0xc6f2abce, 2 | BRF_GRA },
	{ "wl_gfx3.rom", 0x080000, 0x4aa4c6d3, 2 | BRF_GRA },
	{ "wl_24.rom",   0x020000, 0x6f0adee5, 2 | BRF_GRA },
	{ "wl_14.rom",   0x020000, 0x9cf3027d, 2 | BRF_GRA },
	{ "wl_26.rom",   0x020000, 0xf09c8ecf, 2 | BRF_GRA },
	{ "wl_16.rom",   0x020000, 0xe35407aa, 2 | BRF_GRA },
	{ "wl_20.rom",   0x020000, 0x84992350, 2 | BRF_GRA },
	{ "wl_10.rom",   0x020000, 0xb87b5a36, 2 | BRF_GRA },
	{ "wl_22.rom",   0x020000, 0xfd3f89f0, 2 | BRF_GRA },
	{ "wl_12.rom",   0x020000, 0x7da49d69, 2 | BRF_GRA },

	{ "wl23.bin",    0x010000, 0xf6b3d060, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "wl30.bin",    0x020000, 0xbde23d4d, 4 | BRF_SND },
	{ "wl32.bin",    0x020000, 0x683898f5, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "wl22b.1a",    0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",    0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Willowj) STD_ROM_FN(Willowj)

struct BurnDriver BurnDrvCpsWillowj = {
	"willowj", "willow", NULL, "1989",
	"Willow (Japan, Japanese)\0", "Japanese language", "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,WillowjRomInfo,WillowjRomName,willowInputInfo, willowDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Willow (Japan, English)

static struct BurnRomInfo WillowjeRomDesc[] = {
	{ "wlu_30.rom",  0x020000, 0xd604dbb1, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "wlu_35.rom",  0x020000, 0xdaee72fe, 1 | BRF_ESS | BRF_PRG },
	{ "wlu_31.rom",  0x020000, 0x0eb48a83, 1 | BRF_ESS | BRF_PRG },
	{ "wlu_36.rom",  0x020000, 0x36100209, 1 | BRF_ESS | BRF_PRG },
	{ "wl_32.rom",   0x080000, 0xdfd9f643, 1 | BRF_ESS | BRF_PRG },

	{ "wl_gfx5.rom", 0x080000, 0xafa74b73, 2 | BRF_GRA },
	{ "wl_gfx7.rom", 0x080000, 0x12a0dc0b, 2 | BRF_GRA },
	{ "wl_gfx1.rom", 0x080000, 0xc6f2abce, 2 | BRF_GRA },
	{ "wl_gfx3.rom", 0x080000, 0x4aa4c6d3, 2 | BRF_GRA },
	{ "wl_24.rom",   0x020000, 0x6f0adee5, 2 | BRF_GRA },
	{ "wl_14.rom",   0x020000, 0x9cf3027d, 2 | BRF_GRA },
	{ "wl_26.rom",   0x020000, 0xf09c8ecf, 2 | BRF_GRA },
	{ "wl_16.rom",   0x020000, 0xe35407aa, 2 | BRF_GRA },
	{ "wl_20.rom",   0x020000, 0x84992350, 2 | BRF_GRA },
	{ "wl_10.rom",   0x020000, 0xb87b5a36, 2 | BRF_GRA },
	{ "wl_22.rom",   0x020000, 0xfd3f89f0, 2 | BRF_GRA },
	{ "wl_12.rom",   0x020000, 0x7da49d69, 2 | BRF_GRA },

	{ "wl_09.rom",   0x010000, 0xf6b3d060, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "wl_18.rom",   0x020000, 0xbde23d4d, 4 | BRF_SND },
	{ "wl_19.rom",   0x020000, 0x683898f5, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "wl24b.1a",    0x000117, 0x7101cdf1, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.11e",    0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Willowje) STD_ROM_FN(Willowje)

struct BurnDriver BurnDrvCpsWillowje = {
	"willowje", "willow", NULL, "1989",
	"Willow (Japan, English)\0", "English language", "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,WillowjeRomInfo,WillowjeRomName,willowInputInfo, willowDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// U.N. Squadron (US)

static struct BurnRomInfo UnsquadRomDesc[] = {
	{ "aru_30.11f", 0x020000, 0x24d8f88d, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "aru_35.11h", 0x020000, 0x8b954b59, 1 | BRF_ESS | BRF_PRG },
	{ "aru_31.12f", 0x020000, 0x33e9694b, 1 | BRF_ESS | BRF_PRG },
	{ "aru_36.12h", 0x020000, 0x7cc8fb9e, 1 | BRF_ESS | BRF_PRG },
	{ "ar-32m.8h",  0x080000, 0xae1d7fb0, 1 | BRF_ESS | BRF_PRG },

	{ "ar-5m.7a",   0x080000, 0xbf4575d8, 2 | BRF_GRA },
	{ "ar-7m.9a",   0x080000, 0xa02945f4, 2 | BRF_GRA },
	{ "ar-1m.3a",   0x080000, 0x5965ca8d, 2 | BRF_GRA },
	{ "ar-3m.5a",   0x080000, 0xac6db17d, 2 | BRF_GRA },

	{ "ar_09.12b",  0x010000, 0xf3dd1367, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ar_18.11c",  0x020000, 0x584b43a9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ar24b.1a",   0x000117, 0x09a51271, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.11e",   0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Unsquad) STD_ROM_FN(Unsquad)

struct BurnDriver BurnDrvCpsUnsquad = {
	"unsquad", NULL, NULL, "1989",
	"U.N. Squadron (US)\0", NULL, "Daipro / Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,UnsquadRomInfo,UnsquadRomName,unsquadInputInfo, unsquadDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Area 88 (Japan)

static struct BurnRomInfo Area88RomDesc[] = {
	{ "ar_36.12f",   0x020000, 0x65030392, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ar_42.12h",   0x020000, 0xc48170de, 1 | BRF_ESS | BRF_PRG },
	{ "ar_37.13f",   0x020000, 0x33e9694b, 1 | BRF_ESS | BRF_PRG },
	{ "ar_43.13h",   0x020000, 0x7cc8fb9e, 1 | BRF_ESS | BRF_PRG },
	{ "ar_34.10f",   0x020000, 0xf6e80386, 1 | BRF_ESS | BRF_PRG },
	{ "ar_40.10h",   0x020000, 0xbe36c145, 1 | BRF_ESS | BRF_PRG },
	{ "ar_35.11f",   0x020000, 0x86d98ff3, 1 | BRF_ESS | BRF_PRG },
	{ "ar_41.11h",   0x020000, 0x758893d3, 1 | BRF_ESS | BRF_PRG },

	{ "ar_09.4b",    0x020000, 0xdb9376f8, 2 | BRF_GRA },
	{ "ar_01.4a",    0x020000, 0x392151b4, 2 | BRF_GRA },
	{ "ar_13.9b",    0x020000, 0x81436481, 2 | BRF_GRA },
	{ "ar_05.9a",    0x020000, 0xe246ed9f, 2 | BRF_GRA },
	{ "ar_24.5e",    0x020000, 0x9cd6e2a3, 2 | BRF_GRA },
	{ "ar_17.5c",    0x020000, 0x0b8e0df4, 2 | BRF_GRA },
	{ "ar_38.8h",    0x020000, 0x8b9e75b9, 2 | BRF_GRA },
	{ "ar_32.8f",    0x020000, 0xdb6acdcf, 2 | BRF_GRA },
	{ "ar_10.5b",    0x020000, 0x4219b622, 2 | BRF_GRA },
	{ "ar_02.5a",    0x020000, 0xbac5dec5, 2 | BRF_GRA },
	{ "ar_14.10b",   0x020000, 0xe6bae179, 2 | BRF_GRA },
	{ "ar_06.10a",   0x020000, 0xc8f04223, 2 | BRF_GRA },
	{ "ar_25.7e",    0x020000, 0x15ccf981, 2 | BRF_GRA },
	{ "ar_18.7c",    0x020000, 0x9336db6a, 2 | BRF_GRA },
	{ "ar_39.9h",    0x020000, 0x9b8e1363, 2 | BRF_GRA },
	{ "ar_33.9f",    0x020000, 0x3968f4b5, 2 | BRF_GRA },

	{ "ar_23.13c",   0x010000, 0xf3dd1367, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ar_30.12e",   0x020000, 0x584b43a9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ar22b.1a",    0x000117, 0xf1db9030, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.12c",    0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Area88) STD_ROM_FN(Area88)

struct BurnDriver BurnDrvCpsArea88 = {
	"area88", "unsquad", NULL, "1989",
	"Area 88 (Japan)\0", NULL, "Daipro / Capcom", "CPS1",
	L"\u30A8\u30EA\u30A2\uFF18\uFF18\0Area 88\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Area88RomInfo,Area88RomName,unsquadInputInfo, unsquadDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Final Fight (World)

static struct BurnRomInfo FfightRomDesc[] = {
	{ "ff_36.11f",  0x020000, 0xf9a5ce83, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ff_42.11h",  0x020000, 0x65f11215, 1 | BRF_ESS | BRF_PRG },
	{ "ff_37.12f",  0x020000, 0xe1033784, 1 | BRF_ESS | BRF_PRG },
	{ "ffe_43.12h", 0x020000, 0x995e968a, 1 | BRF_ESS | BRF_PRG },
	{ "ff-32m.8h",  0x080000, 0xc747696e, 1 | BRF_ESS | BRF_PRG },

	{ "ff-5m.7a",   0x080000, 0x9c284108, 2 | BRF_GRA },
	{ "ff-7m.9a",   0x080000, 0xa7584dfb, 2 | BRF_GRA },
	{ "ff-1m.3a",   0x080000, 0x0b605e44, 2 | BRF_GRA },
	{ "ff-3m.5a",   0x080000, 0x52291cd2, 2 | BRF_GRA },

	{ "ff_09.12b",  0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ff_18.11c",  0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ff_19.12c",  0x020000, 0x1ef137f9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "s224b.1a",   0x000117, 0x4e85b158, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",   0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Ffight) STD_ROM_FN(Ffight)

struct BurnDriver BurnDrvCpsFfight = {
	"ffight", NULL, NULL, "1989",
	"Final Fight (World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,FfightRomInfo,FfightRomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Final Fight (US)

static struct BurnRomInfo FfightuRomDesc[] = {
	{ "ff_36.11f",  0x020000, 0xf9a5ce83, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ff_42.11h",  0x020000, 0x65f11215, 1 | BRF_ESS | BRF_PRG },
	{ "ff_37.12f",  0x020000, 0xe1033784, 1 | BRF_ESS | BRF_PRG },
	{ "ff43.rom",   0x020000, 0x4ca65947, 1 | BRF_ESS | BRF_PRG },
	{ "ff-32m.8h",  0x080000, 0xc747696e, 1 | BRF_ESS | BRF_PRG },

	{ "ff-5m.7a",   0x080000, 0x9c284108, 2 | BRF_GRA },
	{ "ff-7m.9a",   0x080000, 0xa7584dfb, 2 | BRF_GRA },
	{ "ff-1m.3a",   0x080000, 0x0b605e44, 2 | BRF_GRA },
	{ "ff-3m.5a",   0x080000, 0x52291cd2, 2 | BRF_GRA },

	{ "ff_09.12b",  0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ff_18.11c",  0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ff_19.12c",  0x020000, 0x1ef137f9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "s224b.1a",   0x000117, 0x4e85b158, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",   0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Ffightu) STD_ROM_FN(Ffightu)

struct BurnDriver BurnDrvCpsFfightu = {
	"ffightu", "ffight", NULL, "1989",
	"Final Fight (US)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,FfightuRomInfo,FfightuRomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Final Fight (US 900112)

static struct BurnRomInfo FfightuaRomDesc[] = {
	{ "ffu_36.11f", 0x020000, 0xe2a48af9, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ffu_42.11h", 0x020000, 0xf4bb480e, 1 | BRF_ESS | BRF_PRG },
	{ "ffu_37.12f", 0x020000, 0xc371c667, 1 | BRF_ESS | BRF_PRG },
	{ "ffu_43.12h", 0x020000, 0x2f5771f9, 1 | BRF_ESS | BRF_PRG },
	{ "ff-32m.8h",  0x080000, 0xc747696e, 1 | BRF_ESS | BRF_PRG },

	{ "ff-5m.7a",   0x080000, 0x9c284108, 2 | BRF_GRA },
	{ "ff-7m.9a",   0x080000, 0xa7584dfb, 2 | BRF_GRA },
	{ "ff-1m.3a",   0x080000, 0x0b605e44, 2 | BRF_GRA },
	{ "ff-3m.5a",   0x080000, 0x52291cd2, 2 | BRF_GRA },

	{ "ff_09.12b",  0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ff_18.11c",  0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ff_19.12c",  0x020000, 0x1ef137f9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "s224b.1a",   0x000117, 0x4e85b158, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",   0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Ffightua) STD_ROM_FN(Ffightua)

struct BurnDriver BurnDrvCpsFfightua = {
	"ffightua", "ffight", NULL, "1989",
	"Final Fight (US 900112)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,FfightuaRomInfo,FfightuaRomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Final Fight (US 900613)

static struct BurnRomInfo FfightubRomDesc[] = {
	{ "ffu30",      0x020000, 0xed988977, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ffu35",      0x020000, 0x07bf1c21, 1 | BRF_ESS | BRF_PRG },
	{ "ffu31",      0x020000, 0xdba5a476, 1 | BRF_ESS | BRF_PRG },
	{ "ffu36",      0x020000, 0x4d89f542, 1 | BRF_ESS | BRF_PRG },
	{ "ff-32m.8h",  0x080000, 0xc747696e, 1 | BRF_ESS | BRF_PRG },

	{ "ff-5m.7a",   0x080000, 0x9c284108, 2 | BRF_GRA },
	{ "ff-7m.9a",   0x080000, 0xa7584dfb, 2 | BRF_GRA },
	{ "ff-1m.3a",   0x080000, 0x0b605e44, 2 | BRF_GRA },
	{ "ff-3m.5a",   0x080000, 0x52291cd2, 2 | BRF_GRA },

	{ "ff_09.12b",  0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ff_18.11c",  0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ff_19.12c",  0x020000, 0x1ef137f9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "s224b.1a",   0x000117, 0x4e85b158, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",   0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Ffightub) STD_ROM_FN(Ffightub)

struct BurnDriver BurnDrvCpsFfightub = {
	"ffightub", "ffight",NULL,"1989",
	"Final Fight (US 900613)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,FfightubRomInfo,FfightubRomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Final Fight (Japan)

static struct BurnRomInfo FfightjRomDesc[] = {
	{ "ff36.bin",     0x020000, 0xf9a5ce83, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ff42.bin",     0x020000, 0x65f11215, 1 | BRF_ESS | BRF_PRG },
	{ "ff37.bin",     0x020000, 0xe1033784, 1 | BRF_ESS | BRF_PRG },
	{ "ff43.bin",     0x020000, 0xb6dee1c3, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_34.10f",   0x020000, 0x0c8dc3fc, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_40.10h",   0x020000, 0x8075bab9, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_35.11f",   0x020000, 0x4a934121, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_41.11h",   0x020000, 0x2af68154, 1 | BRF_ESS | BRF_PRG },

	{ "ffj_09.4b",    0x020000, 0x5b116d0d, 2 | BRF_GRA },
	{ "ffj_01.4a",    0x020000, 0x815b1797, 2 | BRF_GRA },
	{ "ffj_13.9b",    0x020000, 0x8721a7da, 2 | BRF_GRA },
	{ "ffj_05.9a",    0x020000, 0xd0fcd4b5, 2 | BRF_GRA },
	{ "ffj_24.5e",    0x020000, 0xa1ab607a, 2 | BRF_GRA },
	{ "ffj_17.5c",    0x020000, 0x2dc18cf4, 2 | BRF_GRA },
	{ "ffj_38.8h",    0x020000, 0x6535a57f, 2 | BRF_GRA },
	{ "ffj_32.8f",    0x020000, 0xc8bc4a57, 2 | BRF_GRA },
	{ "ffj_10.5b",    0x020000, 0x624a924a, 2 | BRF_GRA },
	{ "ffj_02.5a",    0x020000, 0x5d91f694, 2 | BRF_GRA },
	{ "ffj_14.10b",   0x020000, 0x0a2e9101, 2 | BRF_GRA },
	{ "ffj_06.10a",   0x020000, 0x1c18f042, 2 | BRF_GRA },
	{ "ffj_25.7e",    0x020000, 0x6e8181ea, 2 | BRF_GRA },
	{ "ffj_18.7c",    0x020000, 0xb19ede59, 2 | BRF_GRA },
	{ "ffj_39.9h",    0x020000, 0x9416b477, 2 | BRF_GRA },
	{ "ffj_33.9f",    0x020000, 0x7369fa07, 2 | BRF_GRA },

	{ "ff_23.13c",    0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ffj_30.12e",   0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ffj_31.13e",   0x020000, 0x1ef137f9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "s222b.1a",     0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",     0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Ffightj) STD_ROM_FN(Ffightj)

struct BurnDriver BurnDrvCpsFfightj = {
	"ffightj", "ffight", NULL, "1989",
	"Final Fight (Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,FfightjRomInfo,FfightjRomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Final Fight (Japan 900305)

static struct BurnRomInfo Ffightj1RomDesc[] = {
	{ "ff30-36.rom",  0x020000, 0x088ed1c9, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ff35-42.rom",  0x020000, 0xc4c491e6, 1 | BRF_ESS | BRF_PRG },
	{ "ff31-37.rom",  0x020000, 0x708557ff, 1 | BRF_ESS | BRF_PRG },
	{ "ff36-43.rom",  0x020000, 0xc004004a, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_34.10f",   0x020000, 0x0c8dc3fc, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_40.10h",   0x020000, 0x8075bab9, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_35.11f",   0x020000, 0x4a934121, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_41.11h",   0x020000, 0x2af68154, 1 | BRF_ESS | BRF_PRG },

	{ "ffj_09.4b",    0x020000, 0x5b116d0d, 2 | BRF_GRA },
	{ "ffj_01.4a",    0x020000, 0x815b1797, 2 | BRF_GRA },
	{ "ffj_13.9b",    0x020000, 0x8721a7da, 2 | BRF_GRA },
	{ "ffj_05.9a",    0x020000, 0xd0fcd4b5, 2 | BRF_GRA },
	{ "ffj_24.5e",    0x020000, 0xa1ab607a, 2 | BRF_GRA },
	{ "ffj_17.5c",    0x020000, 0x2dc18cf4, 2 | BRF_GRA },
	{ "ffj_38.8h",    0x020000, 0x6535a57f, 2 | BRF_GRA },
	{ "ffj_32.8f",    0x020000, 0xc8bc4a57, 2 | BRF_GRA },
	{ "ffj_10.5b",    0x020000, 0x624a924a, 2 | BRF_GRA },
	{ "ffj_02.5a",    0x020000, 0x5d91f694, 2 | BRF_GRA },
	{ "ffj_14.10b",   0x020000, 0x0a2e9101, 2 | BRF_GRA },
	{ "ffj_06.10a",   0x020000, 0x1c18f042, 2 | BRF_GRA },
	{ "ffj_25.7e",    0x020000, 0x6e8181ea, 2 | BRF_GRA },
	{ "ffj_18.7c",    0x020000, 0xb19ede59, 2 | BRF_GRA },
	{ "ffj_39.9h",    0x020000, 0x9416b477, 2 | BRF_GRA },
	{ "ffj_33.9f",    0x020000, 0x7369fa07, 2 | BRF_GRA },

	{ "ff_23.13c",    0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ffj_30.12e",   0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ffj_31.13e",   0x020000, 0x1ef137f9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "s222b.1a",     0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",     0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Ffightj1) STD_ROM_FN(Ffightj1)

struct BurnDriver BurnDrvCpsFfightj1 = {
	"ffightj1", "ffight", NULL, "1989",
	"Final Fight (Japan 900305)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Ffightj1RomInfo,Ffightj1RomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Final Fight (Japan 900112)

static struct BurnRomInfo Ffightj2RomDesc[] = {
	{ "ffj_36.12f",   0x020000, 0xe2a48af9, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ffj_42.12h",   0x020000, 0xf4bb480e, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_37.13f",   0x020000, 0xc371c667, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_43.13h",   0x020000, 0x6f81f194, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_34.10f",   0x020000, 0x0c8dc3fc, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_40.10h",   0x020000, 0x8075bab9, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_35.11f",   0x020000, 0x4a934121, 1 | BRF_ESS | BRF_PRG },
	{ "ffj_41.11h",   0x020000, 0x2af68154, 1 | BRF_ESS | BRF_PRG },

	{ "ffj_09.4b",    0x020000, 0x5b116d0d, 2 | BRF_GRA },
	{ "ffj_01.4a",    0x020000, 0x815b1797, 2 | BRF_GRA },
	{ "ffj_13.9b",    0x020000, 0x8721a7da, 2 | BRF_GRA },
	{ "ffj_05.9a",    0x020000, 0xd0fcd4b5, 2 | BRF_GRA },
	{ "ffj_24.5e",    0x020000, 0xa1ab607a, 2 | BRF_GRA },
	{ "ffj_17.5c",    0x020000, 0x2dc18cf4, 2 | BRF_GRA },
	{ "ffj_38.8h",    0x020000, 0x6535a57f, 2 | BRF_GRA },
	{ "ffj_32.8f",    0x020000, 0xc8bc4a57, 2 | BRF_GRA },
	{ "ffj_10.5b",    0x020000, 0x624a924a, 2 | BRF_GRA },
	{ "ffj_02.5a",    0x020000, 0x5d91f694, 2 | BRF_GRA },
	{ "ffj_14.10b",   0x020000, 0x0a2e9101, 2 | BRF_GRA },
	{ "ffj_06.10a",   0x020000, 0x1c18f042, 2 | BRF_GRA },
	{ "ffj_25.7e",    0x020000, 0x6e8181ea, 2 | BRF_GRA },
	{ "ffj_18.7c",    0x020000, 0xb19ede59, 2 | BRF_GRA },
	{ "ffj_39.9h",    0x020000, 0x9416b477, 2 | BRF_GRA },
	{ "ffj_33.9f",    0x020000, 0x7369fa07, 2 | BRF_GRA },

	{ "ff_23.13c",    0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ffj_30.12e",   0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ffj_31.13e",   0x020000, 0x1ef137f9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "s222b.1a",     0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "lwio.12c",     0x000117, 0xad52b90c, 0 | BRF_OPT },
};

STD_ROM_PICK(Ffightj2) STD_ROM_FN(Ffightj2)

struct BurnDriver BurnDrvCpsFfightj2 = {
	"ffightj2", "ffight", NULL, "1989",
	"Final Fight (Japan 900112)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Ffightj2RomInfo,Ffightj2RomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Smart / Final Fight (Japan, hack)

static struct BurnRomInfo FfightjhRomDesc[] = {
	{ "ff.23.bin",  0x080000, 0xae3dda7f, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ff.22.bin",  0x080000, 0xb2d5a3aa, 1 | BRF_ESS | BRF_PRG },

	{ "ff.01.bin",  0x080000, 0x969d18e2, 2 | BRF_GRA },
	{ "ff.02.bin",  0x080000, 0x02b59f99, 2 | BRF_GRA },
	{ "ff.03.bin",  0x080000, 0x01d507ae, 2 | BRF_GRA },
	{ "ff.04.bin",  0x080000, 0xf7c4ceb0, 2 | BRF_GRA },

	{ "ff.09.bin",  0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ff.18.bin",  0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "ff.19.bin",  0x020000, 0x1ef137f9, 4 | BRF_SND },
};

STD_ROM_PICK(Ffightjh) STD_ROM_FN(Ffightjh)

struct BurnDriver BurnDrvCpsFfightjh = {
	"ffightjh", "ffight", NULL, "1989",
	"Street Smart / Final Fight (Japan, hack)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,FfightjhRomInfo,FfightjhRomName,ffightInputInfo, ffightDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// On CPS-1-based hardware (bootleg clone)

// Final Crash (World, bootleg)

static struct BurnRomInfo fcrashRomDesc[] = {
	{ "9.bin",  0x020000, 0xc6854c91, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "5.bin",  0x020000, 0x77f7c2b3, 1 | BRF_ESS | BRF_PRG },
	{ "8.bin",  0x020000, 0x1895b3df, 1 | BRF_ESS | BRF_PRG },
	{ "4.bin",  0x020000, 0xbbd411ee, 1 | BRF_ESS | BRF_PRG },
	{ "7.bin",  0x020000, 0x5b23ebf2, 1 | BRF_ESS | BRF_PRG },
	{ "3.bin",  0x020000, 0xaba2aebe, 1 | BRF_ESS | BRF_PRG },
	{ "6.bin",  0x020000, 0xd4bf37f6, 1 | BRF_ESS | BRF_PRG },
	{ "2.bin",  0x020000, 0x07ac8f43, 1 | BRF_ESS | BRF_PRG },

	{ "18.bin", 0x020000, 0xf1eee6d9, 2 | BRF_GRA },
	{ "20.bin", 0x020000, 0x675f4537, 2 | BRF_GRA },
	{ "22.bin", 0x020000, 0xdb8a32ac, 2 | BRF_GRA },
	{ "24.bin", 0x020000, 0xf4113e57, 2 | BRF_GRA },
	{ "10.bin", 0x020000, 0xd478853e, 2 | BRF_GRA },
	{ "12.bin", 0x020000, 0x25055642, 2 | BRF_GRA },
	{ "14.bin", 0x020000, 0xb77d0328, 2 | BRF_GRA },
	{ "16.bin", 0x020000, 0xea111a79, 2 | BRF_GRA },
	{ "19.bin", 0x020000, 0xb3aa1f48, 2 | BRF_GRA },
	{ "21.bin", 0x020000, 0x04d175c9, 2 | BRF_GRA },
	{ "23.bin", 0x020000, 0xe592ba4f, 2 | BRF_GRA },
	{ "25.bin", 0x020000, 0xb89a740f, 2 | BRF_GRA },
	{ "11.bin", 0x020000, 0xd4457a60, 2 | BRF_GRA },
	{ "13.bin", 0x020000, 0x3b26a37d, 2 | BRF_GRA },
	{ "15.bin", 0x020000, 0x6d837e09, 2 | BRF_GRA },
	{ "17.bin", 0x020000, 0xc59a4d6c, 2 | BRF_GRA },

	{ "1.bin",  0x020000, 0x5b276c14, 3 | BRF_ESS | BRF_PRG }, // Audio CPU + Sample Data
};

STD_ROM_PICK(fcrash) STD_ROM_FN(fcrash)

/*
static void fcrash_load()
{
	unsigned char *tmp = (unsigned char*)malloc(0x020000);
	for (int i = 0; i < 16; i++) {
		BurnLoadRom(tmp, 8 + i, 1);

		for (int j = 0; j < 0x020000; j++) {
			CpsGfx[(i & ~3) * 0x020000 + (j * 4) + (i & 3)] = tmp[j];
		}
	}
	free (tmp);

	unsigned int *mem32 = (unsigned int*)CpsGfx;
	for (int i = 0; i < 0x200000 / 4; i++)
	{
		mem32[i] = BITSWAP32(~mem32[i], 31, 23, 15, 7,
						30, 22, 14, 6,
						29, 21, 13, 5,
						28, 20, 12, 4,
						27, 19, 11, 3,
						26, 18, 10, 2,
						25, 17,  9, 1,
						24, 16,  8, 0);
	}
}
*/

static int fcrashInit()
{
//	pCpsInitCallback = fcrash_load;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsfcrash = {
	"fcrash", "ffight", NULL, "1989",
	"Final Crash (World, bootleg)\0", "No Sound", "[Capcom] (Playmark bootleg)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,fcrashRomInfo,fcrashRomName,ffightInputInfo, ffightDIPInfo,
	fcrashInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// 1941 - Counter Attack (World)

static struct BurnRomInfo NTFORomDesc[] = {
	{ "41e_30.rom",  0x020000, 0x9deb1e75, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "41e_35.rom",  0x020000, 0xd63942b3, 1 | BRF_ESS | BRF_PRG },
	{ "41e_31.rom",  0x020000, 0xdf201112, 1 | BRF_ESS | BRF_PRG },
	{ "41e_36.rom",  0x020000, 0x816a818f, 1 | BRF_ESS | BRF_PRG },
	{ "41_32.rom",   0x080000, 0x4e9648ca, 1 | BRF_ESS | BRF_PRG },

	{ "41_gfx5.rom", 0x080000, 0x01d1cb11, 2 | BRF_GRA },
	{ "41_gfx7.rom", 0x080000, 0xaeaa3509, 2 | BRF_GRA },
	{ "41_gfx1.rom", 0x080000, 0xff77985a, 2 | BRF_GRA },
	{ "41_gfx3.rom", 0x080000, 0x983be58f, 2 | BRF_GRA },

	{ "41_09.rom",   0x010000, 0x0f9d8527, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "41_18.rom",   0x020000, 0xd1f15aeb, 4 | BRF_SND },
	{ "41_19.rom",   0x020000, 0x15aec3a6, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(NTFO) STD_ROM_FN(NTFO)

struct BurnDriver BurnDrvCps1941 = {
	"1941", NULL, NULL, "1990",
	"1941 - Counter Attack (World)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1,
	NULL,NTFORomInfo,NTFORomName,NTFOInputInfo, NTFODIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// 1941 - Counter Attack (Japan)

static struct BurnRomInfo NTFOJRomDesc[] = {
	{ "4136.bin",    0x020000, 0x7fbd42ab, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "4142.bin",    0x020000, 0xc7781f89, 1 | BRF_ESS | BRF_PRG },
	{ "4137.bin",    0x020000, 0xc6464b0b, 1 | BRF_ESS | BRF_PRG },
	{ "4143.bin",    0x020000, 0x440fc0b5, 1 | BRF_ESS | BRF_PRG },
	{ "41_32.rom",   0x080000, 0x4e9648ca, 1 | BRF_ESS | BRF_PRG },

	{ "41_gfx5.rom", 0x080000, 0x01d1cb11, 2 | BRF_GRA },
	{ "41_gfx7.rom", 0x080000, 0xaeaa3509, 2 | BRF_GRA },
	{ "41_gfx1.rom", 0x080000, 0xff77985a, 2 | BRF_GRA },
	{ "41_gfx3.rom", 0x080000, 0x983be58f, 2 | BRF_GRA },

	{ "41_09.rom",   0x010000, 0x0f9d8527, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "41_18.rom",   0x020000, 0xd1f15aeb, 4 | BRF_SND },
	{ "41_19.rom",   0x020000, 0x15aec3a6, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(NTFOJ) STD_ROM_FN(NTFOJ)

struct BurnDriver BurnDrvCps1941j = {
	"1941j", "1941", NULL, "1990",
	"1941 - Counter Attack (Japan)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1,
	NULL,NTFOJRomInfo,NTFOJRomName,NTFOInputInfo, NTFODIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Mercs (World 900302)

static struct BurnRomInfo MercsRomDesc[] = {
	{ "so2_30e.11f",  0x020000, 0xe17f9bf7, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "so2_35e.11h",  0x020000, 0x78e63575, 1 | BRF_ESS | BRF_PRG },
	{ "so2_31e.12f",  0x020000, 0x51204d36, 1 | BRF_ESS | BRF_PRG },
	{ "so2_36e.12h",  0x020000, 0x9cfba8b4, 1 | BRF_ESS | BRF_PRG },
	{ "so2-32m.8h",   0x080000, 0x2eb5cf0c, 1 | BRF_ESS | BRF_PRG },

	{ "so2-6m.8a",    0x080000, 0xaa6102af, 2 |  BRF_GRA },
	{ "so2-8m.10a",   0x080000, 0x839e6869, 2 |  BRF_GRA },
	{ "so2-2m.4a",    0x080000, 0x597c2875, 2 |  BRF_GRA },
	{ "so2-4m.6a",    0x080000, 0x912a9ca0, 2 |  BRF_GRA },
	{ "so2_24.7d",    0x020000, 0x3f254efe, 2 |  BRF_GRA },
	{ "so2_14.7c",    0x020000, 0xf5a8905e, 2 |  BRF_GRA },
	{ "so2_26.9d",    0x020000, 0xf3aa5a4a, 2 |  BRF_GRA },
	{ "so2_16.9c",    0x020000, 0xb43cd1a8, 2 |  BRF_GRA },
	{ "so2_20.3d",    0x020000, 0x8ca751a3, 2 |  BRF_GRA },
	{ "so2_10.3c",    0x020000, 0xe9f569fd, 2 |  BRF_GRA },
	{ "so2_22.5d",    0x020000, 0xfce9a377, 2 |  BRF_GRA },
	{ "so2_12.5c",    0x020000, 0xb7df8a06, 2 |  BRF_GRA },

	{ "so2_09.12b",   0x010000, 0xd09d7c7a, 3 |  BRF_ESS | BRF_PRG },

	{ "so2_18.11c",   0x020000, 0xbbea1643, 4 |  BRF_SND },
	{ "so2_19.12c",   0x020000, 0xac58aa71, 4 |  BRF_SND },

	A_BOARD_PLDS

	{ "o224b.1a",     0x000117, 0xc211c8cd, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",     0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "c628",         0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercs) STD_ROM_FN(Mercs)

struct BurnDriver BurnDrvCpsMercs = {
	"mercs", NULL, NULL, "1990",
	"Mercs (World 900302)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1,
	NULL,MercsRomInfo,MercsRomName,mercsInputInfo, mercsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Mercs (US 900302)

static struct BurnRomInfo MercsuRomDesc[] = {
	{ "so2_30.11f",   0x020000, 0xe17f9bf7, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "s02_35.11h",   0x020000, 0x4477df61, 1 | BRF_ESS | BRF_PRG },
	{ "so2_31.12f",   0x020000, 0x51204d36, 1 | BRF_ESS | BRF_PRG },
	{ "so2_36.12h",   0x020000, 0x9cfba8b4, 1 | BRF_ESS | BRF_PRG },
	{ "so2-32m.8h",   0x080000, 0x2eb5cf0c, 1 | BRF_ESS | BRF_PRG },

	{ "so2-6m.8a",    0x080000, 0xaa6102af, 2 |  BRF_GRA },
	{ "so2-8m.10a",   0x080000, 0x839e6869, 2 |  BRF_GRA },
	{ "so2-2m.4a",    0x080000, 0x597c2875, 2 |  BRF_GRA },
	{ "so2-4m.6a",    0x080000, 0x912a9ca0, 2 |  BRF_GRA },
	{ "so2_24.7d",    0x020000, 0x3f254efe, 2 |  BRF_GRA },
	{ "so2_14.7c",    0x020000, 0xf5a8905e, 2 |  BRF_GRA },
	{ "so2_26.9d",    0x020000, 0xf3aa5a4a, 2 |  BRF_GRA },
	{ "so2_16.9c",    0x020000, 0xb43cd1a8, 2 |  BRF_GRA },
	{ "so2_20.3d",    0x020000, 0x8ca751a3, 2 |  BRF_GRA },
	{ "so2_10.3c",    0x020000, 0xe9f569fd, 2 |  BRF_GRA },
	{ "so2_22.5d",    0x020000, 0xfce9a377, 2 |  BRF_GRA },
	{ "so2_12.5c",    0x020000, 0xb7df8a06, 2 |  BRF_GRA },

	{ "so2_09.12b",   0x010000, 0xd09d7c7a, 3 |  BRF_ESS | BRF_PRG },

	{ "so2_18.11c",   0x020000, 0xbbea1643, 4 |  BRF_SND },
	{ "so2_19.12c",   0x020000, 0xac58aa71, 4 |  BRF_SND },

	A_BOARD_PLDS

	{ "o224b.1a",     0x000117, 0xc211c8cd, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",     0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "c628",         0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercsu) STD_ROM_FN(Mercsu)

struct BurnDriver BurnDrvCpsMercsu = {
	"mercsu", "mercs", NULL, "1990",
	"Mercs (US 900302)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1,
	NULL,MercsuRomInfo,MercsuRomName,mercsInputInfo, mercsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Mercs (US 900608)

static struct BurnRomInfo MercsuaRomDesc[] = {
	{ "so2_30a.11f",  0x020000, 0xe4e725d7, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "so2_35a.11h",  0x020000, 0xe7843445, 1 | BRF_ESS | BRF_PRG },
	{ "so2_31a.12f",  0x020000, 0xc0b91dea, 1 | BRF_ESS | BRF_PRG },
	{ "so2_36a.12h",  0x020000, 0x591edf6c, 1 | BRF_ESS | BRF_PRG },
	{ "so2-32m.8h",   0x080000, 0x2eb5cf0c, 1 | BRF_ESS | BRF_PRG },

	{ "so2-6m.8a",    0x080000, 0xaa6102af, 2 |  BRF_GRA },
	{ "so2-8m.10a",   0x080000, 0x839e6869, 2 |  BRF_GRA },
	{ "so2-2m.4a",    0x080000, 0x597c2875, 2 |  BRF_GRA },
	{ "so2-4m.6a",    0x080000, 0x912a9ca0, 2 |  BRF_GRA },
	{ "so2_24.7d",    0x020000, 0x3f254efe, 2 |  BRF_GRA },
	{ "so2_14.7c",    0x020000, 0xf5a8905e, 2 |  BRF_GRA },
	{ "so2_26.9d",    0x020000, 0xf3aa5a4a, 2 |  BRF_GRA },
	{ "so2_16.9c",    0x020000, 0xb43cd1a8, 2 |  BRF_GRA },
	{ "so2_20.3d",    0x020000, 0x8ca751a3, 2 |  BRF_GRA },
	{ "so2_10.3c",    0x020000, 0xe9f569fd, 2 |  BRF_GRA },
	{ "so2_22.5d",    0x020000, 0xfce9a377, 2 |  BRF_GRA },
	{ "so2_12.5c",    0x020000, 0xb7df8a06, 2 |  BRF_GRA },

	{ "so2_09.12b",   0x010000, 0xd09d7c7a, 3 |  BRF_ESS | BRF_PRG },

	{ "so2_18.11c",   0x020000, 0xbbea1643, 4 |  BRF_SND },
	{ "so2_19.12c",   0x020000, 0xac58aa71, 4 |  BRF_SND },

	A_BOARD_PLDS

	{ "o224b.1a",     0x000117, 0xc211c8cd, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",     0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "c628",         0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercsua) STD_ROM_FN(Mercsua)

struct BurnDriver BurnDrvCpsMercsua = {
	"mercsua", "mercs", NULL, "1990",
	"Mercs (US 900608)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1,
	NULL,MercsuaRomInfo,MercsuaRomName,mercsInputInfo, mercsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Senjo no Ookami II (Japan 900302)

static struct BurnRomInfo MercsjRomDesc[] = {
	{ "so2_36.bin",  0x020000, 0xe17f9bf7, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "so2_42.bin",  0x020000, 0x2c3884c6, 1 | BRF_ESS | BRF_PRG },
	{ "so2_37.bin",  0x020000, 0x51204d36, 1 | BRF_ESS | BRF_PRG },
	{ "so2_43.bin",  0x020000, 0x9cfba8b4, 1 | BRF_ESS | BRF_PRG },
	{ "so2_34.bin",  0x020000, 0xb8dae95f, 1 | BRF_ESS | BRF_PRG },
	{ "so2_40.bin",  0x020000, 0xde37771c, 1 | BRF_ESS | BRF_PRG },
	{ "so2_35.bin",  0x020000, 0x7d24394d, 1 | BRF_ESS | BRF_PRG },
	{ "so2_41.bin",  0x020000, 0x914f85e0, 1 | BRF_ESS | BRF_PRG },

	{ "so2_09.bin",  0x020000, 0x690c261d, 2 |  BRF_GRA },
	{ "so2_01.bin",  0x020000, 0x31fd2715, 2 |  BRF_GRA },
	{ "so2_13.bin",  0x020000, 0xb5e48282, 2 |  BRF_GRA },
	{ "so2_05.bin",  0x020000, 0x54bed82c, 2 |  BRF_GRA },
	{ "so2_24.bin",  0x020000, 0x78b6f0cb, 2 |  BRF_GRA },
	{ "so2_17.bin",  0x020000, 0xe78bb308, 2 |  BRF_GRA },
	{ "so2_38.bin",  0x020000, 0x0010a9a2, 2 |  BRF_GRA },
	{ "so2_32.bin",  0x020000, 0x75dffc9a, 2 |  BRF_GRA },
	{ "so2_10.bin",  0x020000, 0x2f871714, 2 |  BRF_GRA },
	{ "so2_02.bin",  0x020000, 0xb4b2a0b7, 2 |  BRF_GRA },
	{ "so2_14.bin",  0x020000, 0x737a744b, 2 |  BRF_GRA },
	{ "so2_06.bin",  0x020000, 0x9d756f51, 2 |  BRF_GRA },
	{ "so2_25.bin",  0x020000, 0x6d0e05d6, 2 |  BRF_GRA },
	{ "so2_18.bin",  0x020000, 0x96f61f4e, 2 |  BRF_GRA },
	{ "so2_39.bin",  0x020000, 0xd52ba336, 2 |  BRF_GRA },
	{ "so2_33.bin",  0x020000, 0x39b90d25, 2 |  BRF_GRA },
	{ "so2_11.bin",  0x020000, 0x3f254efe, 2 |  BRF_GRA },
	{ "so2_03.bin",  0x020000, 0xf5a8905e, 2 |  BRF_GRA },
	{ "so2_15.bin",  0x020000, 0xf3aa5a4a, 2 |  BRF_GRA },
	{ "so2_07.bin",  0x020000, 0xb43cd1a8, 2 |  BRF_GRA },
	{ "so2_26.bin",  0x020000, 0x8ca751a3, 2 |  BRF_GRA },
	{ "so2_19.bin",  0x020000, 0xe9f569fd, 2 |  BRF_GRA },
	{ "so2_28.bin",  0x020000, 0xfce9a377, 2 |  BRF_GRA },
	{ "so2_21.bin",  0x020000, 0xb7df8a06, 2 |  BRF_GRA },

	{ "so2_23.bin",  0x010000, 0xd09d7c7a, 3 |  BRF_ESS | BRF_PRG },

	{ "so2_30.bin",  0x020000, 0xbbea1643, 4 |  BRF_SND },
	{ "so2_31.bin",  0x020000, 0xac58aa71, 4 |  BRF_SND },

	A_BOARD_PLDS

	{ "o222b.1a",    0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12c",    0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "c628",        0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// c-board PLDs
};

STD_ROM_PICK(Mercsj) STD_ROM_FN(Mercsj)

struct BurnDriver BurnDrvCpsMercsj = {
	"mercsj", "mercs", NULL, "1990",
	"Senjou no Ookami II (Japan 900302)\0", NULL, "Capcom", "CPS1",
	L"\u6226\u5834\u306E\u72FC II (Ookami 2 Japan 900302)\0Senjou no Ookami II\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_CAPCOM_CPS1,
	NULL,MercsjRomInfo,MercsjRomName,mercsInputInfo, mercsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Mega Twins (World 900619)

static struct BurnRomInfo MtwinsRomDesc[] = {
	{ "che_30.11f",  0x020000, 0x9a2a2db1, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "che_35.11h",  0x020000, 0xa7f96b02, 1 | BRF_ESS | BRF_PRG },
	{ "che_31.12f",  0x020000, 0xbbff8a99, 1 | BRF_ESS | BRF_PRG },
	{ "che_36.12h",  0x020000, 0x0fa00c39, 1 | BRF_ESS | BRF_PRG },
	{ "ck-32m.8h",   0x080000, 0x9b70bd41, 1 | BRF_ESS | BRF_PRG },

	{ "ck-5m.7a",    0x080000, 0x4ec75f15, 2 | BRF_GRA },
	{ "ck-7m.9a",    0x080000, 0xd85d00d6, 2 | BRF_GRA },
	{ "ck-1m.3a",    0x080000, 0xf33ca9d4, 2 | BRF_GRA },
	{ "ck-3m.5a",    0x080000, 0x0ba2047f, 2 | BRF_GRA },

	{ "ch_09.12b",   0x010000, 0x4d4255b7, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ch_18.12b",   0x020000, 0xf909e8de, 4 | BRF_SND },
	{ "ch_19.12c",   0x020000, 0xfc158cf7, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Mtwins) STD_ROM_FN(Mtwins)

struct BurnDriver BurnDrvCpsMtwins = {
	"mtwins", NULL, NULL, "1990",
	"Mega Twins (World 900619)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,MtwinsRomInfo,MtwinsRomName,mtwinsInputInfo, mtwinsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Chiki Chiki Boys (Japan 900619)

static struct BurnRomInfo ChikijRomDesc[] = {
	{ "chj_36a.bin",  0x020000, 0xec1328d8, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "chj_42a.bin",  0x020000, 0x4ae13503, 1 | BRF_ESS | BRF_PRG },
	{ "chj_37a.bin",  0x020000, 0x46d2cf7b, 1 | BRF_ESS | BRF_PRG },
	{ "chj_43a.bin",  0x020000, 0x8d387fe8, 1 | BRF_ESS | BRF_PRG },
	{ "ch_34.bin",    0x020000, 0x609ed2f9, 1 | BRF_ESS | BRF_PRG },
	{ "ch_40.bin",    0x020000, 0xbe0d8301, 1 | BRF_ESS | BRF_PRG },
	{ "ch_35.bin",    0x020000, 0xb810867f, 1 | BRF_ESS | BRF_PRG },
	{ "ch_41.bin",    0x020000, 0x8ad96155, 1 | BRF_ESS | BRF_PRG },

	{ "ch_09.bin",    0x020000, 0x567ab3ca, 2 | BRF_GRA },
	{ "ch_01.bin",    0x020000, 0x7f3b7b56, 2 | BRF_GRA },
	{ "ch_13.bin",    0x020000, 0x12a7a8ba, 2 | BRF_GRA },
	{ "ch_05.bin",    0x020000, 0x6c1afb9a, 2 | BRF_GRA },
	{ "ch_24.bin",    0x020000, 0x9cb6e6bc, 2 | BRF_GRA },
	{ "ch_17.bin",    0x020000, 0xfe490846, 2 | BRF_GRA },
	{ "ch_38.bin",    0x020000, 0x6e5c8cb6, 2 | BRF_GRA },
	{ "ch_32.bin",    0x020000, 0x317d27b0, 2 | BRF_GRA },
	{ "ch_10.bin",    0x020000, 0xe8251a9b, 2 | BRF_GRA },
	{ "ch_02.bin",    0x020000, 0x7c8c88fb, 2 | BRF_GRA },
	{ "ch_14.bin",    0x020000, 0x4012ec4b, 2 | BRF_GRA },
	{ "ch_06.bin",    0x020000, 0x81884b2b, 2 | BRF_GRA },
	{ "ch_25.bin",    0x020000, 0x1dfcbac5, 2 | BRF_GRA },
	{ "ch_18.bin",    0x020000, 0x516a34d1, 2 | BRF_GRA },
	{ "ch_39.bin",    0x020000, 0x872fb2a4, 2 | BRF_GRA },
	{ "ch_33.bin",    0x020000, 0x30dc5ded, 2 | BRF_GRA },

	{ "ch_23.bin",    0x010000, 0x4d4255b7, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ch_30.bin",    0x020000, 0xf909e8de, 4 | BRF_SND },
	{ "ch_31.bin",    0x020000, 0xfc158cf7, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Chikij) STD_ROM_FN(Chikij)

struct BurnDriver BurnDrvCpsChikij = {
	"chikij", "mtwins", NULL, "1990",
	"Chiki Chiki Boys (Japan 900619)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,ChikijRomInfo,ChikijRomName,mtwinsInputInfo, mtwinsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Magic Sword - Heroic Fantasy (World 900725)

static struct BurnRomInfo MswordRomDesc[] = {
	{ "mse_30.11f",  0x020000, 0x03fc8dbc, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mse_35.11h",  0x020000, 0xd5bf66cd, 1 | BRF_ESS | BRF_PRG },
	{ "mse_31.12f",  0x020000, 0x30332bcf, 1 | BRF_ESS | BRF_PRG },
	{ "mse_36.12h",  0x020000, 0x8f7d6ce9, 1 | BRF_ESS | BRF_PRG },
	{ "ms-32m.8h",   0x080000, 0x2475ddfc, 1 | BRF_ESS | BRF_PRG },

	{ "ms-5m.7a",    0x080000, 0xc00fe7e2, 2 | BRF_GRA },
	{ "ms-7m.9a",    0x080000, 0x4ccacac5, 2 | BRF_GRA },
	{ "ms-1m.3a",    0x080000, 0x0d2bbe00, 2 | BRF_GRA },
	{ "ms-3m.5a",    0x080000, 0x3a1a5bf4, 2 | BRF_GRA },

	{ "ms_9.12b",    0x010000, 0x57b29519, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ms_18.11c",   0x020000, 0xfb64e90d, 4 | BRF_SND },
	{ "ms_19.12c",   0x020000, 0x74f892b9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ms24b.1a",    0x000117, 0x636dbe6d, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Msword) STD_ROM_FN(Msword)

struct BurnDriver BurnDrvCpsMsword = {
	"msword", NULL, NULL, "1990",
	"Magic Sword - Heroic Fantasy (World 900725)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,MswordRomInfo,MswordRomName,mswordInputInfo, mswordDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Magic Sword - Heroic Fantasy (World 900623)

static struct BurnRomInfo Mswordr1RomDesc[] = {
	{ "ms_30.11f",   0x020000, 0x21c1f078, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "ms_35.11h",   0x020000, 0xa540a73a, 1 | BRF_ESS | BRF_PRG },
	{ "ms_31.12f",   0x020000, 0xd7e762b5, 1 | BRF_ESS | BRF_PRG },
	{ "ms_36.12h",   0x020000, 0x66f2dcdb, 1 | BRF_ESS | BRF_PRG },
	{ "ms-32m.8h",   0x080000, 0x2475ddfc, 1 | BRF_ESS | BRF_PRG },

	{ "ms-5m.7a",    0x080000, 0xc00fe7e2, 2 | BRF_GRA },
	{ "ms-7m.9a",    0x080000, 0x4ccacac5, 2 | BRF_GRA },
	{ "ms-1m.3a",    0x080000, 0x0d2bbe00, 2 | BRF_GRA },
	{ "ms-3m.5a",    0x080000, 0x3a1a5bf4, 2 | BRF_GRA },

	{ "ms_9.12b",    0x010000, 0x57b29519, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ms_18.11c",   0x020000, 0xfb64e90d, 4 | BRF_SND },
	{ "ms_19.12c",   0x020000, 0x74f892b9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ms24b.1a",    0x000117, 0x636dbe6d, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Mswordr1) STD_ROM_FN(Mswordr1)

struct BurnDriver BurnDrvCpsMswordr1 = {
	"mswordr1", "msword", NULL, "1990",
	"Magic Sword - Heroic Fantasy (World 900623)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Mswordr1RomInfo,Mswordr1RomName,mswordInputInfo, mswordDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Magic Sword - Heroic Fantasy (US 900725)

static struct BurnRomInfo MsworduRomDesc[] = {
	{ "msu_30.11f",  0x020000, 0xd963c816, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "msu_35.11h",  0x020000, 0x72f179b3, 1 | BRF_ESS | BRF_PRG },
	{ "msu_31.12f",  0x020000, 0x20cd7904, 1 | BRF_ESS | BRF_PRG },
	{ "msu_36.12h",  0x020000, 0xbf88c080, 1 | BRF_ESS | BRF_PRG },
	{ "ms-32m.8h",   0x080000, 0x2475ddfc, 1 | BRF_ESS | BRF_PRG },

	{ "ms-5m.7a",    0x080000, 0xc00fe7e2, 2 | BRF_GRA },
	{ "ms-7m.9a",    0x080000, 0x4ccacac5, 2 | BRF_GRA },
	{ "ms-1m.3a",    0x080000, 0x0d2bbe00, 2 | BRF_GRA },
	{ "ms-3m.5a",    0x080000, 0x3a1a5bf4, 2 | BRF_GRA },

	{ "ms_9.12b",    0x010000, 0x57b29519, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ms_18.11c",   0x020000, 0xfb64e90d, 4 | BRF_SND },
	{ "ms_19.12c",   0x020000, 0x74f892b9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ms24b.1a",    0x000117, 0x636dbe6d, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Mswordu) STD_ROM_FN(Mswordu)

struct BurnDriver BurnDrvCpsMswordu = {
	"mswordu", "msword", NULL, "1990",
	"Magic Sword - Heroic Fantasy (US 900725)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,MsworduRomInfo,MsworduRomName,mswordInputInfo, mswordDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Magic Sword (Japan 900623)

static struct BurnRomInfo MswordjRomDesc[] = {
	{ "msj_36.12f",  0x020000, 0x04f0ef50, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "msj_42.12h",  0x020000, 0x9fcbb9cd, 1 | BRF_ESS | BRF_PRG },
	{ "msj_37.13f",  0x020000, 0x6c060d70, 1 | BRF_ESS | BRF_PRG },
	{ "msj_43.13h",  0x020000, 0xaec77787, 1 | BRF_ESS | BRF_PRG },
	{ "ms_34.10f",   0x020000, 0x0e59a62d, 1 | BRF_ESS | BRF_PRG },
	{ "ms_40.10h",   0x020000, 0xbabade3a, 1 | BRF_ESS | BRF_PRG },
	{ "ms_35.11f",   0x020000, 0x03da99d1, 1 | BRF_ESS | BRF_PRG },
	{ "ms_41.11h",   0x020000, 0xfadf99ea, 1 | BRF_ESS | BRF_PRG },

	{ "ms_09.4b",    0x020000, 0x4adee6f6, 2 | BRF_GRA },
	{ "ms_01.4a",    0x020000, 0xf7ab1b88, 2 | BRF_GRA },
	{ "ms_13.9b",    0x020000, 0xe01adc4b, 2 | BRF_GRA },
	{ "ms_05.9a",    0x020000, 0xf62c2369, 2 | BRF_GRA },
	{ "ms_24.5e",    0x020000, 0xbe64a3a1, 2 | BRF_GRA },
	{ "ms_17.5c",    0x020000, 0x0bc1665f, 2 | BRF_GRA },
	{ "ms_38.8h",    0x020000, 0x904a2ed5, 2 | BRF_GRA },
	{ "ms_32.8f",    0x020000, 0x3d89c530, 2 | BRF_GRA },
	{ "ms_10.5b",    0x020000, 0xf02c0718, 2 | BRF_GRA },
	{ "ms_02.5a",    0x020000, 0xd071a405, 2 | BRF_GRA },
	{ "ms_14.10b",   0x020000, 0xdfb2e4df, 2 | BRF_GRA },
	{ "ms_06.10a",   0x020000, 0xd3ce2a91, 2 | BRF_GRA },
	{ "ms_25.7e",    0x020000, 0x0f199d56, 2 | BRF_GRA },
	{ "ms_18.7c",    0x020000, 0x1ba76df2, 2 | BRF_GRA },
	{ "ms_39.9h",    0x020000, 0x01efce86, 2 | BRF_GRA },
	{ "ms_33.9f",    0x020000, 0xce25defc, 2 | BRF_GRA },

	{ "ms_23.13b",   0x010000, 0x57b29519, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ms_30.12c",   0x020000, 0xfb64e90d, 4 | BRF_SND },
	{ "ms_31.13c",   0x020000, 0x74f892b9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ms22b.1a",    0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Mswordj) STD_ROM_FN(Mswordj)

struct BurnDriver BurnDrvCpsMswordj = {
	"mswordj", "msword", NULL, "1990",
	"Magic Sword (Japan 900623)\0", NULL, "Capcom", "CPS1",
	L"Magic Sword - \u30DE\u30B8\u30C3\u30AF\uFF65\u30BD\u30FC\u30C9 (23.06.1990 Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,MswordjRomInfo,MswordjRomName,mswordInputInfo, mswordDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Carrier Air Wing (World 901012)

static struct BurnRomInfo CawingRomDesc[] = {
	{ "cae_30a.11f", 0x020000, 0x91fceacd, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cae_35a.11h", 0x020000, 0x3ef03083, 1 | BRF_ESS | BRF_PRG },
	{ "cae_31a.12f", 0x020000, 0xe5b75caf, 1 | BRF_ESS | BRF_PRG },
	{ "cae_36a.12h", 0x020000, 0xc73fd713, 1 | BRF_ESS | BRF_PRG },
	{ "ca-32m.8h",   0x080000, 0x0c4837d4, 1 | BRF_ESS | BRF_PRG },

	{ "ca-5m.7a",    0x080000, 0x66d4cc37, 2 | BRF_GRA },
	{ "ca-7m.9a",    0x080000, 0xb6f896f2, 2 | BRF_GRA },
	{ "ca-1m.3a",    0x080000, 0x4d0620fd, 2 | BRF_GRA },
	{ "ca-3m.5a",    0x080000, 0x0b0341c3, 2 | BRF_GRA },

	{ "ca_9.12b",    0x010000, 0x96fe7485, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ca_18.11c",   0x020000, 0x4a613a2c, 4 | BRF_SND },
	{ "ca_19.12c",   0x020000, 0x74584493, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ca24b.1a",    0x000117, 0x76ec0b1c, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Cawing) STD_ROM_FN(Cawing)

struct BurnDriver BurnDrvCpsCawing = {
	"cawing", NULL, NULL, "1990",
	"Carrier Air Wing (World 901012)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,CawingRomInfo,CawingRomName,cawingInputInfo, cawingDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Carrier Air Wing (World 901009)

static struct BurnRomInfo cawingr1RomDesc[] = {
	{ "cae_30.11f",  0x020000, 0x23305cd5, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cae_35.11h",  0x020000, 0x69419113, 1 | BRF_ESS | BRF_PRG },
	{ "cae_31.12f",  0x020000, 0x9008dfb3, 1 | BRF_ESS | BRF_PRG },
	{ "cae_36.12h",  0x020000, 0x4dbf6f8e, 1 | BRF_ESS | BRF_PRG },
	{ "ca-32m.8h",   0x080000, 0x0c4837d4, 1 | BRF_ESS | BRF_PRG },

	{ "ca-5m.7a",    0x080000, 0x66d4cc37, 2 | BRF_GRA },
	{ "ca-7m.9a",    0x080000, 0xb6f896f2, 2 | BRF_GRA },
	{ "ca-1m.3a",    0x080000, 0x4d0620fd, 2 | BRF_GRA },
	{ "ca-3m.5a",    0x080000, 0x0b0341c3, 2 | BRF_GRA },

	{ "ca_9.12b",    0x010000, 0x96fe7485, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ca_18.11c",   0x020000, 0x4a613a2c, 4 | BRF_SND },
	{ "ca_19.12c",   0x020000, 0x74584493, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ca24b.1a",    0x000117, 0x76ec0b1c, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(cawingr1) STD_ROM_FN(cawingr1)

struct BurnDriver BurnDrvCpscawingr1 = {
	"cawingr1", "cawing", NULL, "1990",
	"Carrier Air Wing (World 901009)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,cawingr1RomInfo,cawingr1RomName,cawingInputInfo, cawingDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Carrier Air Wing (US 901012)

static struct BurnRomInfo CawinguRomDesc[] = {
	{ "cae_30a.11f", 0x020000, 0x91fceacd, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cau_35a.11h", 0x020000, 0xf090d9b2, 1 | BRF_ESS | BRF_PRG },
	{ "cae_31a.12f", 0x020000, 0xe5b75caf, 1 | BRF_ESS | BRF_PRG },
	{ "cae_36a.12h", 0x020000, 0xc73fd713, 1 | BRF_ESS | BRF_PRG },
	{ "ca-32m.8h",   0x080000, 0x0c4837d4, 1 | BRF_ESS | BRF_PRG },

	{ "ca-5m.7a",    0x080000, 0x66d4cc37, 2 | BRF_GRA },
	{ "ca-7m.9a",    0x080000, 0xb6f896f2, 2 | BRF_GRA },
	{ "ca-1m.3a",    0x080000, 0x4d0620fd, 2 | BRF_GRA },
	{ "ca-3m.5a",    0x080000, 0x0b0341c3, 2 | BRF_GRA },

	{ "ca_9.12b",    0x010000, 0x96fe7485, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ca_18.11c",   0x020000, 0x4a613a2c, 4 | BRF_SND },
	{ "ca_19.12c",   0x020000, 0x74584493, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ca24b.1a",    0x000117, 0x76ec0b1c, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Cawingu) STD_ROM_FN(Cawingu)

struct BurnDriver BurnDrvCpsCawingu = {
	"cawingu", "cawing", NULL, "1990",
	"Carrier Air Wing (US 901012)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,CawinguRomInfo,CawinguRomName,cawingInputInfo, cawingDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// U.S. Navy (Japan 901012)

static struct BurnRomInfo CawingjRomDesc[] = {
	{ "caj_36a.12f", 0x020000, 0x91fceacd, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "caj_42a.12h", 0x020000, 0x039f8362, 1 | BRF_ESS | BRF_PRG },
	{ "caj_37a.13f", 0x020000, 0xe5b75caf, 1 | BRF_ESS | BRF_PRG },
	{ "caj_43a.13h", 0x020000, 0xc73fd713, 1 | BRF_ESS | BRF_PRG },
	{ "caj_34.10f",  0x020000, 0x51ea57f4, 1 | BRF_ESS | BRF_PRG },
	{ "caj_40.10h",  0x020000, 0x2ab71ae1, 1 | BRF_ESS | BRF_PRG },
	{ "caj_35.11f",  0x020000, 0x01d71973, 1 | BRF_ESS | BRF_PRG },
	{ "caj_41.11h",  0x020000, 0x3a43b538, 1 | BRF_ESS | BRF_PRG },

	{ "caj_09.4b",   0x020000, 0x41b0f9a6, 2 | BRF_GRA },
	{ "caj_01.4a",   0x020000, 0x1002d0b8, 2 | BRF_GRA },
	{ "caj_13.9b",   0x020000, 0x6f3948b2, 2 | BRF_GRA },
	{ "caj_05.9a",   0x020000, 0x207373d7, 2 | BRF_GRA },
	{ "caj_24.5e",   0x020000, 0xe356aad7, 2 | BRF_GRA },
	{ "caj_17.5c",   0x020000, 0x540f2fd8, 2 | BRF_GRA },
	{ "caj_38.8h",   0x020000, 0x2464d4ab, 2 | BRF_GRA },
	{ "caj_32.8f",   0x020000, 0x9b5836b3, 2 | BRF_GRA },
	{ "caj_10.5b",   0x020000, 0xbf8a5f52, 2 | BRF_GRA },
	{ "caj_02.5a",   0x020000, 0x125b018d, 2 | BRF_GRA },
	{ "caj_14.10b",  0x020000, 0x8458e7d7, 2 | BRF_GRA },
	{ "caj_06.10a",  0x020000, 0xcf80e164, 2 | BRF_GRA },
	{ "caj_25.7e",   0x020000, 0xcdd0204d, 2 | BRF_GRA },
	{ "caj_18.7c",   0x020000, 0x29c1d4b1, 2 | BRF_GRA },
	{ "caj_39.9h",   0x020000, 0xeea23b67, 2 | BRF_GRA },
	{ "caj_33.9f",   0x020000, 0xdde3891f, 2 | BRF_GRA },

	{ "caj_23.13b",  0x010000, 0x96fe7485, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "caj_30.12c",  0x020000, 0x4a613a2c, 4 | BRF_SND },
	{ "caj_31.13c",  0x020000, 0x74584493, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "ca22b.1a",    0x000117, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Cawingj) STD_ROM_FN(Cawingj)

struct BurnDriver BurnDrvCpsCawingj = {
	"cawingj", "cawing", NULL, "1990",
	"U.S. Navy (Japan 901012)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,CawingjRomInfo,CawingjRomName,cawingInputInfo, cawingDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Nemo (World 901130)

static struct BurnRomInfo NemoRomDesc[] = {
	{ "nme_30a.11f", 0x020000, 0xd2c03e56, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "nme_35a.11h", 0x020000, 0x5fd31661, 1 | BRF_ESS | BRF_PRG },
	{ "nme_31a.12f", 0x020000, 0xb2bd4f6f, 1 | BRF_ESS | BRF_PRG },
	{ "nme_36a.12h", 0x020000, 0xee9450e3, 1 | BRF_ESS | BRF_PRG },
	{ "nm-32m.8h",   0x080000, 0xd6d1add3, 1 | BRF_ESS | BRF_PRG },

	{ "nm-5m.7a",    0x080000, 0x487b8747, 2 | BRF_GRA },
	{ "nm-7m.9a",    0x080000, 0x203dc8c6, 2 | BRF_GRA },
	{ "nm-1m.3a",    0x080000, 0x9e878024, 2 | BRF_GRA },
	{ "nm-3m.5a",    0x080000, 0xbb01e6b6, 2 | BRF_GRA },

	{ "nm_09.12b",   0x010000, 0x0f4b0581, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "nm_18.11c",   0x020000, 0xbab333d4, 4 | BRF_SND },
	{ "nm_19.12c",   0x020000, 0x2650a0a8, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "nm24b.1a",    0x000117, 0x7b25bac6, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Nemo) STD_ROM_FN(Nemo)

struct BurnDriver BurnDrvCpsNemo = {
	"nemo", NULL, NULL, "1990",
	"Nemo (World 901130)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,NemoRomInfo,NemoRomName,nemoInputInfo, nemoDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Nemo (Japan 901120)

static struct BurnRomInfo NemojRomDesc[] = {
	{ "nmj_36a.12f",  0x020000, 0xdaeceabb, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "nmj_42a.12h",  0x020000, 0x55024740, 1 | BRF_ESS | BRF_PRG },
	{ "nmj_37a.13f",  0x020000, 0x619068b6, 1 | BRF_ESS | BRF_PRG },
	{ "nmj_43a.13h",  0x020000, 0xa948a53b, 1 | BRF_ESS | BRF_PRG },
	{ "nm_34.10f",    0x020000, 0x5737feed, 1 | BRF_ESS | BRF_PRG },
	{ "nm_40.10h",    0x020000, 0x8a4099f3, 1 | BRF_ESS | BRF_PRG },
	{ "nm_35.11f",    0x020000, 0xbd11a7f8, 1 | BRF_ESS | BRF_PRG },
	{ "nm_41.11h",    0x020000, 0x6309603d, 1 | BRF_ESS | BRF_PRG },

	{ "nm_09.4b",     0x020000, 0x9d60d286, 2 | BRF_GRA },
	{ "nm_01.4a",     0x020000, 0x8a83f7c4, 2 | BRF_GRA },
	{ "nm_13.9b",     0x020000, 0xa4909fe0, 2 | BRF_GRA },
	{ "nm_05.9a",     0x020000, 0x16db1e61, 2 | BRF_GRA },
	{ "nm_24.5e",     0x020000, 0x3312c648, 2 | BRF_GRA },
	{ "nm_17.5c",     0x020000, 0xccfc50e2, 2 | BRF_GRA },
	{ "nm_38.8h",     0x020000, 0xae98a997, 2 | BRF_GRA },
	{ "nm_32.8f",     0x020000, 0xb3704dde, 2 | BRF_GRA },
	{ "nm_10.5b",     0x020000, 0x33c1388c, 2 | BRF_GRA },
	{ "nm_02.5a",     0x020000, 0x84c69469, 2 | BRF_GRA },
	{ "nm_14.10b",    0x020000, 0x66612270, 2 | BRF_GRA },
	{ "nm_06.10a",    0x020000, 0x8b9bcf95, 2 | BRF_GRA },
	{ "nm_25.7e",     0x020000, 0xacfc84d2, 2 | BRF_GRA },
	{ "nm_18.7c",     0x020000, 0x4347deed, 2 | BRF_GRA },
	{ "nm_39.9h",     0x020000, 0x6a274ecd, 2 | BRF_GRA },
	{ "nm_33.9f",     0x020000, 0xc469dc74, 2 | BRF_GRA },

	{ "nm_23.13b",    0x010000, 0x8d3c5a42, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "nm_30.12c",    0x020000, 0xbab333d4, 4 | BRF_SND },
	{ "nm_31.13c",    0x020000, 0x2650a0a8, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "nm22b.1a",     0x000117, 0x00000000, 0 | BRF_OPT  | BRF_NODUMP },	// b-board PLDs
	{ "iob1.12e",     0x000117, 0x3abc0700, 0 | BRF_OPT },
};

STD_ROM_PICK(Nemoj) STD_ROM_FN(Nemoj)

struct BurnDriver BurnDrvCpsNemoj = {
	"nemoj", "nemo", NULL, "1990",
	"Nemo (Japan 901120)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,NemojRomInfo,NemojRomName,nemoInputInfo, nemoDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (World 910522)

static struct BurnRomInfo Sf2RomDesc[] = {
	{ "sf2e.30g",    0x020000, 0xfe39ee33, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2e.37g",    0x020000, 0xfb92cd74, 1 | BRF_ESS | BRF_PRG },
	{ "sf2e.31g",    0x020000, 0x69a0a301, 1 | BRF_ESS | BRF_PRG },
	{ "sf2e.38g",    0x020000, 0x5e22db70, 1 | BRF_ESS | BRF_PRG },
	{ "sf2e.28g",    0x020000, 0x8bf9f1e5, 1 | BRF_ESS | BRF_PRG },
	{ "sf2e.35g",    0x020000, 0x626ef934, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2) STD_ROM_FN(Sf2)

struct BurnDriver BurnDrvCpsSf2 = {
	"sf2", NULL, NULL, "1991",
	"Street Fighter II - The World Warrior (World 910522)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2RomInfo,Sf2RomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (World 910214)

static struct BurnRomInfo Sf2ebRomDesc[] = {
	{ "sf2_30a.bin",  0x020000, 0x57bd7051, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2e_37b.rom", 0x020000, 0x62691cdd, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_31a.bin",  0x020000, 0xa673143d, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_38a.bin",  0x020000, 0x4c2ccef7, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_28a.bin",  0x020000, 0x4009955e, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_35a.bin",  0x020000, 0x8c1f3994, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin",  0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin",  0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",   0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",   0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",   0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",   0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",   0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",   0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",   0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",   0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",   0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",   0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",   0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",   0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",   0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2eb) STD_ROM_FN(Sf2eb)

struct BurnDriver BurnDrvCpsSf2eb = {
	"sf2eb", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (World 910214)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2ebRomInfo,Sf2ebRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (World 910214, TAB Austria bootleg)

static struct BurnRomInfo Sf2ebblRomDesc[] = {
	{ "12.bin",         0x040000, 0xa258b4d5, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "09.bin",         0x040000, 0x59ccd474, 1 | BRF_ESS | BRF_PRG },
	{ "11.bin",         0x040000, 0x82097d63, 1 | BRF_ESS | BRF_PRG },
	{ "10.bin",         0x040000, 0x0c83844d, 1 | BRF_ESS | BRF_PRG },

	{ "1b_yf082.bin",   0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "1d_yf028.bin",   0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "1a_yf087.bin",   0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "1c_yf088.bin",   0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "1f_yf085.bin",   0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "1h_yf115.bin",   0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "1e_yf111.bin",   0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "1g_yf002.bin",   0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "1j_yf117.bin",   0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "1l_ye040.bin",   0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "1i_yf038.bin",   0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "1k_ye039.bin",   0x080000, 0x0627c831, 2 | BRF_GRA },
	{ "05.bin",         0x020000, 0xa505621e, 2 | BRF_GRA },
	{ "06.bin",         0x020000, 0x23775344, 2 | BRF_GRA },
	{ "07.bin",         0x020000, 0xde6271fb, 2 | BRF_GRA },
	{ "08.bin",         0x020000, 0x81c9550f, 2 | BRF_GRA },

	{ "03.bin",         0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "02.bin",         0x020000, 0x7f162009, 4 | BRF_SND },
	{ "01.bin",         0x020000, 0xbeade53f, 4 | BRF_SND },

	{ "04.bin",         0x010000, 0x13ea1c44, 0 | BRF_OPT }, // bootleg priority?
};

STD_ROM_PICK(Sf2ebbl) STD_ROM_FN(Sf2ebbl)

/* These map over the MASK roms on this bootleg to get rid of the CAPCOM logo (wasteful, but correct) */
static void sf2ebbl_load()
{
	memcpy(CpsGfx + 0x400000, CpsGfx + 0x600000, 0x080000);
}

static int Sf2ebblInit()
{
	pCpsInitCallback = sf2ebbl_load;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2ebbl = {
	"sf2ebbl", "sf2", NULL, "1992",
	"Street Fighter II - The World Warrior (World 910214, TAB Austria bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2ebblRomInfo,Sf2ebblRomName,Sf2yycInputInfo, sf2DIPInfo,
	Sf2ebblInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (Quicken Pt-I, bootleg)

static struct BurnRomInfo Sf2qp1RomDesc[] = {
	{ "stfii-qkn-cps-17.33",  0x080000, 0x3a9458ee, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "stfii-qkn-cps-17.34",  0x080000, 0x4ed215d8, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",   0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",   0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",   0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",   0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",   0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",   0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",   0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",   0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",   0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",   0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",   0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",   0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",   0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2qp1) STD_ROM_FN(Sf2qp1)

static void sf2qp1_load()
{
	unsigned char *pTemp = (unsigned char*)malloc(0x40000);
	if (pTemp != NULL) {
		memcpy(pTemp, CpsRom+0x40000, 0x40000);
		memcpy(CpsRom+0x40000, CpsRom+0xc0000, 0x40000);
		memcpy(CpsRom+0xc0000, pTemp, 0x40000);
		free(pTemp);
	}
}

static int Sf2qp1Init()
{
	pCpsInitCallback = sf2qp1_load;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2qp1 = {
	"sf2qp1", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (Quicken Pt-I, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2qp1RomInfo,Sf2qp1RomName,sf2InputInfo, sf2DIPInfo,
	Sf2qp1Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (US 910206)

static struct BurnRomInfo Sf2uaRomDesc[] = {
	{ "sf2u.30a",    0x020000, 0x08beb861, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2u.37a",    0x020000, 0xb7638d69, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.31a",    0x020000, 0x0d5394e0, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.38a",    0x020000, 0x42d6a79e, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.28a",    0x020000, 0x387a175c, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.35a",    0x020000, 0xa1a5adcc, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ua) STD_ROM_FN(Sf2ua)

struct BurnDriver BurnDrvCpsSf2ua = {
	"sf2ua", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (US 910206)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2uaRomInfo,Sf2uaRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (US 910214)

static struct BurnRomInfo Sf2ubRomDesc[] = {
	{ "sf2_30a.bin", 0x020000, 0x57bd7051, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2u.37b",    0x020000, 0x4a54d479, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_31a.bin", 0x020000, 0xa673143d, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_38a.bin", 0x020000, 0x4c2ccef7, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_28a.bin", 0x020000, 0x4009955e, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_35a.bin", 0x020000, 0x8c1f3994, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ub) STD_ROM_FN(Sf2ub)

struct BurnDriver BurnDrvCpsSf2ub = {
	"sf2ub", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (US 910214)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2ubRomInfo,Sf2ubRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (US 910318)

static struct BurnRomInfo Sf2udRomDesc[] = {
	{ "sf2u.30d",    0x020000, 0x4bb2657c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2u.37d",    0x020000, 0xb33b42f2, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.31d",    0x020000, 0xd57b67d7, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.38d",    0x020000, 0x9c8916ef, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.28d",    0x020000, 0x175819d1, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.35d",    0x020000, 0x82060da4, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ud) STD_ROM_FN(Sf2ud)

struct BurnDriver BurnDrvCpsSf2ud = {
	"sf2ud", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (US 910318)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2udRomInfo,Sf2udRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (US 910228)

static struct BurnRomInfo Sf2ueRomDesc[] = {
	{ "sf2u.30e",      0x020000, 0xf37cd088, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2u.37e",      0x020000, 0x6c61a513, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.31e",      0x020000, 0x7c4771b4, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.38e",      0x020000, 0xa4bd0cd9, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.28e",      0x020000, 0xe3b95625, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.35e",      0x020000, 0x3648769a, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin",   0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin",   0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",    0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",    0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",    0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",    0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",    0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",    0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",    0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",    0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",    0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",    0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",    0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",    0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",    0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",    0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",    0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ue) STD_ROM_FN(Sf2ue)

struct BurnDriver BurnDrvCpsSf2ue = {
	"sf2ue", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (US 910228)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2ueRomInfo,Sf2ueRomName,Sf2ueInputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (US 910411)

static struct BurnRomInfo Sf2ufRomDesc[] = {
	{ "sf2u.30f",    0x020000, 0xfe39ee33, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2u.37f",    0x020000, 0x169e7388, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.31f",    0x020000, 0x69a0a301, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.38f",    0x020000, 0x1510e4e2, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.28f",    0x020000, 0xacd8175b, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.35f",    0x020000, 0xc0a80bd1, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2uf) STD_ROM_FN(Sf2uf)

struct BurnDriver BurnDrvCpsSf2uf = {
	"sf2uf", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (US 910411)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2ufRomInfo,Sf2ufRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (US 910522)

static struct BurnRomInfo Sf2uiRomDesc[] = {
	{ "sf2u.30f",    0x020000, 0xfe39ee33, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2u.37i",    0x020000, 0x9df707dd, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.31f",    0x020000, 0x69a0a301, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.38i",    0x020000, 0x4cb46daf, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.28i",    0x020000, 0x1580be4c, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.35i",    0x020000, 0x1468d185, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ui) STD_ROM_FN(Sf2ui)

struct BurnDriver BurnDrvCpsSf2ui = {
	"sf2ui", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (US 910522)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2uiRomInfo,Sf2uiRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (US 911101)

static struct BurnRomInfo Sf2ukRomDesc[] = {
	{ "sf2u.30k",    0x020000, 0x8f66076c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2u.37k",    0x020000, 0x4e1f6a83, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.31k",    0x020000, 0xf9f89f60, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.38k",    0x020000, 0x6ce0a85a, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.28k",    0x020000, 0x8e958f31, 1 | BRF_ESS | BRF_PRG },
	{ "sf2u.35k",    0x020000, 0xfce76fad, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2uk) STD_ROM_FN(Sf2uk)

struct BurnDriver BurnDrvCpsSf2uk = {
	"sf2uk", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (US 911101)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2ukRomInfo,Sf2ukRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (Japan 911210)

static struct BurnRomInfo Sf2jRomDesc[] = {
	{ "sf2j30.bin",  0x020000, 0x79022b31, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2j37.bin",  0x020000, 0x516776ec, 1 | BRF_ESS | BRF_PRG },
	{ "sf2j31.bin",  0x020000, 0xfe15cb39, 1 | BRF_ESS | BRF_PRG },
	{ "sf2j38.bin",  0x020000, 0x38614d70, 1 | BRF_ESS | BRF_PRG },
	{ "sf2j28.bin",  0x020000, 0xd283187a, 1 | BRF_ESS | BRF_PRG },
	{ "sf2j35.bin",  0x020000, 0xd28158e4, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin", 0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin", 0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",  0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",  0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",  0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",  0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",  0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",  0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",  0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",  0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",  0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",  0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",  0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",  0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",  0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2j) STD_ROM_FN(Sf2j)

struct BurnDriver BurnDrvCpsSf2j = {
	"sf2j", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (Japan 911210)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2jRomInfo,Sf2jRomName,sf2InputInfo, sf2jDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (Japan 910214)

static struct BurnRomInfo Sf2jaRomDesc[] = {
	{ "sf2_30a.bin",  0x020000, 0x57bd7051, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2j_37a.bin", 0x020000, 0x1e1f6844, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_31a.bin",  0x020000, 0xa673143d, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_38a.bin",  0x020000, 0x4c2ccef7, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_28a.bin",  0x020000, 0x4009955e, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_35a.bin",  0x020000, 0x8c1f3994, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin",  0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin",  0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",   0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",   0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",   0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",   0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",   0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",   0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",   0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",   0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",   0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",   0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",   0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",   0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",   0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ja) STD_ROM_FN(Sf2ja)

struct BurnDriver BurnDrvCpsSf2ja = {
	"sf2ja", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (Japan 910214)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2jaRomInfo,Sf2jaRomName,sf2InputInfo, sf2jDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (Japan 910306)

static struct BurnRomInfo Sf2jcRomDesc[] = {
	{ "sf2_30c.bin",  0x020000, 0x8add35ec, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sf2j_37c.bin", 0x020000, 0x0d74a256, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_31c.bin",  0x020000, 0xc4fff4a9, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_38c.bin",  0x020000, 0x8210fc0e, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_28c.bin",  0x020000, 0x6eddd5e8, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_35c.bin",  0x020000, 0x6bcb404c, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin",  0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin",  0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",   0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",   0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",   0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",   0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",   0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",   0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",   0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",   0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",   0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",   0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",   0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",   0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",   0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2jc) STD_ROM_FN(Sf2jc)

struct BurnDriver BurnDrvCpsSf2jc = {
	"sf2jc", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (Japan 910306)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2jcRomInfo,Sf2jcRomName,sf2InputInfo, sf2jDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Three Wonders (World 910520)

static struct BurnRomInfo Wonder3wRomDesc[] = {
	{ "rte_30a.11f", 0x020000, 0xef5b8b33, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "rte_35a.11h", 0x020000, 0x7d705529, 1 | BRF_ESS | BRF_PRG },
	{ "rte_31a.12f", 0x020000, 0x32835e5e, 1 | BRF_ESS | BRF_PRG },
	{ "rte_36a.12h", 0x020000, 0x7637975f, 1 | BRF_ESS | BRF_PRG },
	{ "rt_28a.9f",   0x020000, 0x054137c8, 1 | BRF_ESS | BRF_PRG },
	{ "rt_33a.9h",   0x020000, 0x7264cb1b, 1 | BRF_ESS | BRF_PRG },
	{ "rte_29a.10f", 0x020000, 0xcddaa919, 1 | BRF_ESS | BRF_PRG },
	{ "rte_34a.10h", 0x020000, 0xed52e7e5, 1 | BRF_ESS | BRF_PRG },

	{ "rt-5m.7a",    0x080000, 0x86aef804, 2 | BRF_GRA },
	{ "rt-7m.9a",    0x080000, 0x4f057110, 2 | BRF_GRA },
	{ "rt-1m.3a",    0x080000, 0x902489d0, 2 | BRF_GRA },
	{ "rt-3m.5a",    0x080000, 0xe35ce720, 2 | BRF_GRA },
	{ "rt-6m.8a",    0x080000, 0x13cb0e7c, 2 | BRF_GRA },
	{ "rt-8m.10a",   0x080000, 0x1f055014, 2 | BRF_GRA },
	{ "rt-2m.4a",    0x080000, 0xe9a034f4, 2 | BRF_GRA },
	{ "rt-4m.6a",    0x080000, 0xdf0eea8b, 2 | BRF_GRA },

	{ "rt_09.12b",   0x010000, 0xabfca165, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "rt_18.11c",   0x020000, 0x26b211ab, 4 | BRF_SND },
	{ "rt_19.12c",   0x020000, 0xdbe64ad0, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "rt24b.1a",    0x000117, 0x54b85159, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "ioc1.ic1",    0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wonder3w) STD_ROM_FN(Wonder3w)

struct BurnDriver BurnDrvCps3wonders = {
	"3wonders", NULL, NULL, "1991",
	"Three Wonders (World 910520)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,Wonder3wRomInfo,Wonder3wRomName,wonders3InputInfo, threewondersDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Three Wonders (US 910520)

static struct BurnRomInfo Wonder3uRomDesc[] = {
	{ "3wonders.30", 0x020000, 0x0b156fd8, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "3wonders.35", 0x020000, 0x57350bf4, 1 | BRF_ESS | BRF_PRG },
	{ "3wonders.31", 0x020000, 0x0e723fcc, 1 | BRF_ESS | BRF_PRG },
	{ "3wonders.36", 0x020000, 0x523a45dc, 1 | BRF_ESS | BRF_PRG },
	{ "rt_28a.9f",   0x020000, 0x054137c8, 1 | BRF_ESS | BRF_PRG },
	{ "rt_33a.9h",   0x020000, 0x7264cb1b, 1 | BRF_ESS | BRF_PRG },
	{ "3wonders.29", 0x020000, 0x37ba3e20, 1 | BRF_ESS | BRF_PRG },
	{ "3wonders.34", 0x020000, 0xf99f46c0, 1 | BRF_ESS | BRF_PRG },

	{ "rt-5m.7a",    0x080000, 0x86aef804, 2 | BRF_GRA },
	{ "rt-7m.9a",    0x080000, 0x4f057110, 2 | BRF_GRA },
	{ "rt-1m.3a",    0x080000, 0x902489d0, 2 | BRF_GRA },
	{ "rt-3m.5a",    0x080000, 0xe35ce720, 2 | BRF_GRA },
	{ "rt-6m.8a",    0x080000, 0x13cb0e7c, 2 | BRF_GRA },
	{ "rt-8m.10a",   0x080000, 0x1f055014, 2 | BRF_GRA },
	{ "rt-2m.4a",    0x080000, 0xe9a034f4, 2 | BRF_GRA },
	{ "rt-4m.6a",    0x080000, 0xdf0eea8b, 2 | BRF_GRA },

	{ "rt_09.12b",   0x010000, 0xabfca165, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "rt_18.11c",   0x020000, 0x26b211ab, 4 | BRF_SND },
	{ "rt_19.12c",   0x020000, 0xdbe64ad0, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "rt24b.1a",    0x000117, 0x54b85159, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "ioc1.ic1",    0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wonder3u) STD_ROM_FN(Wonder3u)

struct BurnDriver BurnDrvCps3wondersu = {
	"3wondersu", "3wonders", NULL, "1991",
	"Three Wonders (US 910520)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Wonder3uRomInfo,Wonder3uRomName,wonders3InputInfo, threewondersDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Wonder 3 (Japan 910520)

static struct BurnRomInfo Wonder3RomDesc[] = {
	{ "rtj_36.12f",  0x020000, 0xe3741247, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "rtj_42.12h",  0x020000, 0xb4baa117, 1 | BRF_ESS | BRF_PRG },
	{ "rtj_37.13f",  0x020000, 0xa1f677b0, 1 | BRF_ESS | BRF_PRG },
	{ "rtj_43.13h",  0x020000, 0x85337a47, 1 | BRF_ESS | BRF_PRG },
	{ "rt_34.10f",   0x020000, 0x054137c8, 1 | BRF_ESS | BRF_PRG },
	{ "rt_40.10h",   0x020000, 0x7264cb1b, 1 | BRF_ESS | BRF_PRG },
	{ "rtj_35.11f",  0x020000, 0xe72f9ea3, 1 | BRF_ESS | BRF_PRG },
	{ "rtj_41.11h",  0x020000, 0xa11ee998, 1 | BRF_ESS | BRF_PRG },

 	{ "rt_09.4b",    0x020000, 0x2c40e480, 2 | BRF_GRA },
	{ "rt_01.4a",    0x020000, 0x3e11f8cd, 2 | BRF_GRA },
	{ "rt_13.9b",    0x020000, 0x51009117, 2 | BRF_GRA },
	{ "rt_05.9a",    0x020000, 0x283fd470, 2 | BRF_GRA },
	{ "rt_24.5e",    0x020000, 0xee4484ce, 2 | BRF_GRA },
	{ "rt_17.5c",    0x020000, 0xe5dcddeb, 2 | BRF_GRA },
	{ "rt_38.8h",    0x020000, 0xb2940c2d, 2 | BRF_GRA },
	{ "rt_32.8f",    0x020000, 0x08e2b758, 2 | BRF_GRA },
 	{ "rt_10.5b",    0x020000, 0xe3f3ff94, 2 | BRF_GRA },
	{ "rt_02.5a",    0x020000, 0xfcffd73c, 2 | BRF_GRA },
	{ "rt_14.10b",   0x020000, 0x5c546d9a, 2 | BRF_GRA },
	{ "rt_06.10a",   0x020000, 0xd9650bc4, 2 | BRF_GRA },
	{ "rt_25.7e",    0x020000, 0x11b28831, 2 | BRF_GRA },
	{ "rt_18.7c",    0x020000, 0xce1afb7c, 2 | BRF_GRA },
	{ "rt_39.9h",    0x020000, 0xea7ac9ee, 2 | BRF_GRA },
	{ "rt_33.9f",    0x020000, 0xd6a99384, 2 | BRF_GRA },
 	{ "rt_11.7b",    0x020000, 0x04f3c298, 2 | BRF_GRA },
	{ "rt_03.7a",    0x020000, 0x98087e08, 2 | BRF_GRA },
	{ "rt_15.11b",   0x020000, 0xb6aba565, 2 | BRF_GRA },
	{ "rt_07.11a",   0x020000, 0xc62defa1, 2 | BRF_GRA },
	{ "rt_26.8e",    0x020000, 0x532f542e, 2 | BRF_GRA },
	{ "rt_19.8c",    0x020000, 0x1f0f72bd, 2 | BRF_GRA },
	{ "rt_28.10e",   0x020000, 0x6064e499, 2 | BRF_GRA },
	{ "rt_21.10c",   0x020000, 0x20012ddc, 2 | BRF_GRA },
 	{ "rt_12.8b",    0x020000, 0xe54664cc, 2 | BRF_GRA },
	{ "rt_04.8a",    0x020000, 0x4d7b9a1a, 2 | BRF_GRA },
	{ "rt_16.12b",   0x020000, 0x37c96cfc, 2 | BRF_GRA },
	{ "rt_08.12a",   0x020000, 0x75f4975b, 2 | BRF_GRA },
	{ "rt_27.9e",    0x020000, 0xec6edc0f, 2 | BRF_GRA },
	{ "rt_20.9c",    0x020000, 0x4fe52659, 2 | BRF_GRA },
	{ "rt_29.11e",   0x020000, 0x8fa77f9f, 2 | BRF_GRA },
	{ "rt_22.11c",   0x020000, 0x228a0d4a, 2 | BRF_GRA },

	{ "rt_23.13b",   0x010000, 0xabfca165, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "rt_30.12c",   0x020000, 0x26b211ab, 4 | BRF_SND },
	{ "rt_31.13c",   0x020000, 0xdbe64ad0, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "rt22b.1a",    0x000117, 0x89560d6a, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12e",    0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "ioc1.ic1",    0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wonder3) STD_ROM_FN(Wonder3)

struct BurnDriver BurnDrvCpsWonder3 = {
	"wonder3", "3wonders", NULL, "1991",
	"Wonder 3 (Japan 910520)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,Wonder3RomInfo,Wonder3RomName,wonders3InputInfo, threewondersDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Three Wonders (hack?)

static struct BurnRomInfo Wonder3hRomDesc[] = {
	{ "22.bin",        0x020000, 0xe6071884, 1 | BRF_ESS | BRF_PRG },
	{ "26.bin",        0x020000, 0xa28447b7, 1 | BRF_ESS | BRF_PRG },
	{ "23.bin",        0x020000, 0xfd3d6509, 1 | BRF_ESS | BRF_PRG },
	{ "27.bin",        0x020000, 0x999cba3d, 1 | BRF_ESS | BRF_PRG },
	{ "rt_28a.9f",     0x020000, 0x054137c8, 1 | BRF_ESS | BRF_PRG },
	{ "rt_33a.9h",     0x020000, 0x7264cb1b, 1 | BRF_ESS | BRF_PRG },
	{ "rte_29a.10f",   0x020000, 0xcddaa919, 1 | BRF_ESS | BRF_PRG },
	{ "rte_34a.10h",   0x020000, 0xed52e7e5, 1 | BRF_ESS | BRF_PRG },

	{ "16.bin",        0x040000, 0xc997bca2, 2 | BRF_GRA },
	{ "6.bin",         0x040000, 0x3eea321a, 2 | BRF_GRA },
	{ "18.bin",        0x040000, 0x98acdfd4, 2 | BRF_GRA },
	{ "8.bin",         0x040000, 0xdc9ca6f9, 2 | BRF_GRA },
	{ "12.bin",        0x040000, 0x0d8a6007, 2 | BRF_GRA },
	{ "2.bin",         0x040000, 0xd75563b9, 2 | BRF_GRA },
	{ "14.bin",        0x040000, 0x84369a28, 2 | BRF_GRA },
	{ "4.bin",         0x040000, 0xd4831578, 2 | BRF_GRA },
	{ "17.bin",        0x040000, 0x040edff5, 2 | BRF_GRA },
	{ "7.bin",         0x040000, 0xc7c0468c, 2 | BRF_GRA },
	{ "19.bin",        0x040000, 0x9fef114f, 2 | BRF_GRA },
	{ "9.bin",         0x040000, 0x48cbfba5, 2 | BRF_GRA },
	{ "13.bin",        0x040000, 0x8fc3d7d1, 2 | BRF_GRA },
	{ "3.bin",         0x040000, 0xc65e9a86, 2 | BRF_GRA },
	{ "15.bin",        0x040000, 0xf239341a, 2 | BRF_GRA },
	{ "5.bin",         0x040000, 0x947ac944, 2 | BRF_GRA },

	{ "rt_9.12b",      0x010000, 0xabfca165, 3 | BRF_ESS | BRF_PRG },

	{ "rt_18.11c",     0x020000, 0x26b211ab, 4 | BRF_SND },
	{ "rt_19.12c",     0x020000, 0xdbe64ad0, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "rt24b.1a",      0x000117, 0x54b85159, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.11e",      0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "ioc1.ic1",      0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wonder3h)
STD_ROM_FN(Wonder3h)

struct BurnDriver BurnDrvCps3wondersh = {
	"3wondersh", "3wonders", NULL, "1991",
	"Three Wonders (hack?)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Wonder3hRomInfo, Wonder3hRomName, wonders3InputInfo, threewondersDIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// The King of Dragons (World 910711)

static struct BurnRomInfo KodRomDesc[] = {
	{ "kod30.rom", 0x020000, 0xc7414fd4, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "kod37.rom", 0x020000, 0xa5bf40d2, 1 | BRF_ESS | BRF_PRG },
	{ "kod31.rom", 0x020000, 0x1fffc7bd, 1 | BRF_ESS | BRF_PRG },
	{ "kod38.rom", 0x020000, 0x89e57a82, 1 | BRF_ESS | BRF_PRG },
	{ "kod28.rom", 0x020000, 0x9367bcd9, 1 | BRF_ESS | BRF_PRG },
	{ "kod35.rom", 0x020000, 0x4ca6a48a, 1 | BRF_ESS | BRF_PRG },
	{ "kod29.rom", 0x020000, 0x6a0ba878, 1 | BRF_ESS | BRF_PRG },
	{ "kod36.rom", 0x020000, 0xb509b39d, 1 | BRF_ESS | BRF_PRG },

	{ "kod02.rom", 0x080000, 0xe45b8701, 2 | BRF_GRA },
	{ "kod04.rom", 0x080000, 0xa7750322, 2 | BRF_GRA },
	{ "kod01.rom", 0x080000, 0x5f74bf78, 2 | BRF_GRA },
	{ "kod03.rom", 0x080000, 0x5e5303bf, 2 | BRF_GRA },
	{ "kod11.rom", 0x080000, 0x113358f3, 2 | BRF_GRA },
	{ "kod13.rom", 0x080000, 0x38853c44, 2 | BRF_GRA },
	{ "kod10.rom", 0x080000, 0x9ef36604, 2 | BRF_GRA },
	{ "kod12.rom", 0x080000, 0x402b9b4f, 2 | BRF_GRA },

	{ "kod09.rom", 0x010000, 0xf5514510, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "kod18.rom", 0x020000, 0x69ecb2c8, 4 | BRF_SND },
	{ "kod19.rom", 0x020000, 0x02d851c1, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Kod) STD_ROM_FN(Kod)

struct BurnDriver BurnDrvCpsKod = {
	"kod", NULL, NULL, "1991",
	"The King of Dragons (World 910711)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,3,HARDWARE_CAPCOM_CPS1,
	NULL,KodRomInfo,KodRomName,kodInputInfo, kodDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The King of Dragons (US 910910)

static struct BurnRomInfo KoduRomDesc[] = {
	{ "kdu-30b.bin", 0x020000, 0x825817f9, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "kdu-37b.bin", 0x020000, 0xd2422dfb, 1 | BRF_ESS | BRF_PRG },
	{ "kdu-31b.bin", 0x020000, 0x9af36039, 1 | BRF_ESS | BRF_PRG },
	{ "kdu-38b.bin", 0x020000, 0xbe8405a1, 1 | BRF_ESS | BRF_PRG },
	{ "kod28.rom",   0x020000, 0x9367bcd9, 1 | BRF_ESS | BRF_PRG },
	{ "kod35.rom",   0x020000, 0x4ca6a48a, 1 | BRF_ESS | BRF_PRG },
	{ "kd-29.bin",   0x020000, 0x0360fa72, 1 | BRF_ESS | BRF_PRG },
	{ "kd-36a.bin",  0x020000, 0x95a3cef8, 1 | BRF_ESS | BRF_PRG },

	{ "kod02.rom",   0x080000, 0xe45b8701, 2 | BRF_GRA },
	{ "kod04.rom",   0x080000, 0xa7750322, 2 | BRF_GRA },
	{ "kod01.rom",   0x080000, 0x5f74bf78, 2 | BRF_GRA },
	{ "kod03.rom",   0x080000, 0x5e5303bf, 2 | BRF_GRA },
	{ "kod11.rom",   0x080000, 0x113358f3, 2 | BRF_GRA },
	{ "kod13.rom",   0x080000, 0x38853c44, 2 | BRF_GRA },
	{ "kod10.rom",   0x080000, 0x9ef36604, 2 | BRF_GRA },
	{ "kod12.rom",   0x080000, 0x402b9b4f, 2 | BRF_GRA },

	{ "kd09.bin",    0x010000, 0xbac6ec26, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "kd18.bin",    0x020000, 0x4c63181d, 4 | BRF_SND },
	{ "kd19.bin",    0x020000, 0x92941b80, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Kodu) STD_ROM_FN(Kodu)

struct BurnDriver BurnDrvCpsKodu = {
	"kodu", "kod", NULL, "1991",
	"The King of Dragons (US 910910)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1,
	NULL,KoduRomInfo,KoduRomName,kodInputInfo, kodDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The King of Dragons (Japan 910805)

static struct BurnRomInfo KodjRomDesc[] = {
	{ "kd30.bin",  0x020000, 0xebc788ad, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "kd37.bin",  0x020000, 0xe55c3529, 1 | BRF_ESS | BRF_PRG },
	{ "kd31.bin",  0x020000, 0xc710d722, 1 | BRF_ESS | BRF_PRG },
	{ "kd38.bin",  0x020000, 0x57d6ed3a, 1 | BRF_ESS | BRF_PRG },
	{ "kd33.bin",  0x080000, 0x9bd7ad4b, 1 | BRF_ESS | BRF_PRG },

	{ "kod02.rom", 0x080000, 0xe45b8701, 2 | BRF_GRA },
	{ "kod04.rom", 0x080000, 0xa7750322, 2 | BRF_GRA },
	{ "kod01.rom", 0x080000, 0x5f74bf78, 2 | BRF_GRA },
	{ "kod03.rom", 0x080000, 0x5e5303bf, 2 | BRF_GRA },
	{ "kod11.rom", 0x080000, 0x113358f3, 2 | BRF_GRA },
	{ "kod13.rom", 0x080000, 0x38853c44, 2 | BRF_GRA },
	{ "kod10.rom", 0x080000, 0x9ef36604, 2 | BRF_GRA },
	{ "kod12.rom", 0x080000, 0x402b9b4f, 2 | BRF_GRA },

	{ "kd09.bin",  0x010000, 0xbac6ec26, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "kd18.bin",  0x020000, 0x4c63181d, 4 | BRF_SND },
	{ "kd19.bin",  0x020000, 0x92941b80, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Kodj) STD_ROM_FN(Kodj)

struct BurnDriver BurnDrvCpsKodj = {
	"kodj", "kod", NULL, "1991",
	"The King of Dragons (Japan 910805)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1,
	NULL,KodjRomInfo,KodjRomName,kodInputInfo, kodjDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The King of Dragons (bootleg)

static struct BurnRomInfo KodbRomDesc[] = {
	{ "3.ic172",   0x080000, 0x036dd74c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "4.ic171",   0x080000, 0x3e4b7295, 1 | BRF_ESS | BRF_PRG },

	{ "cp.ic90",   0x080000, 0xe3b8589e, 2 | BRF_GRA },
	{ "dp.ic89",   0x080000, 0x3eec9580, 2 | BRF_GRA },
	{ "ap.ic88",   0x080000, 0xfdf5f163, 2 | BRF_GRA },
	{ "bp.ic87",   0x080000, 0x4e1c52b7, 2 | BRF_GRA },
	{ "ci.ic91",   0x080000, 0x22228bc5, 2 | BRF_GRA },
	{ "di.ic92",   0x080000, 0xab031763, 2 | BRF_GRA },
	{ "ai.ic93",   0x080000, 0xcffbf4be, 2 | BRF_GRA },
	{ "bi.ic94",   0x080000, 0x4a1b43fe, 2 | BRF_GRA },

	{ "1.ic28",    0x010000, 0x01cae60c, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "2.ic19",    0x040000, 0xa2db1575, 4 | BRF_SND },
};

STD_ROM_PICK(Kodb) STD_ROM_FN(Kodb)

static void bootleg_progload()
{
	// First two program roms are 512kb and
	// should be interleaved
	BurnLoadRom(CpsRom + 0x000001, 0, 2);
	BurnLoadRom(CpsRom + 0x000000, 1, 2);
}

static void kodb_patch()
{
	bootleg_progload();

	// Patch protection? check
	CpsRom[0x412] = 0x6;
	CpsRom[0x414] = 0x64;
	CpsRom[0x415] = 0x71;
	CpsRom[0xC44] = 0x1C;
	CpsRom[0xC45] = 0x10;
	CpsRom[0xC46] = 0x88;
	CpsRom[0xC47] = 0xEF;
	CpsRom[0xC48] = 0x1C;
	CpsRom[0xC49] = 0x12;
	CpsRom[0x34FE] = 0x7C;
	CpsRom[0x34FF] = 0x22;
	CpsRom[0x3500] = 0x90;
	CpsRom[0x3502] = 0x0;
	CpsRom[0x3503] = 0x0;
	CpsRom[0x36AA] = 0xFA;
	CpsRom[0x36AB] = 0x45;
	CpsRom[0x36AC] = 0x2E;
	CpsRom[0x36AE] = 0x1A;
	CpsRom[0x36AF] = 0x30;
	CpsRom[0x4999] = 0x49;
	CpsRom[0x499A] = 0x90;
	CpsRom[0x499C] = 0x0;
	CpsRom[0x499D] = 0x0;
	CpsRom[0x5E64] = 0x0;
	CpsRom[0x5E65] = 0x61;
	CpsRom[0x5E66] = 0x3E;
	CpsRom[0x5E68] = 0x6D;
	CpsRom[0x5E69] = 0x8;
	CpsRom[0x5E92] = 0x0;
	CpsRom[0x5E93] = 0x0;
	CpsRom[0x5EA0] = 0x0;
	CpsRom[0x5EA1] = 0x40;
	CpsRom[0x610A] = 0xC2;
	CpsRom[0x610B] = 0x38;
	CpsRom[0x610C] = 0xC3;
	CpsRom[0x610D] = 0x38;
	CpsRom[0x610E] = 0xE9;
	CpsRom[0x610F] = 0x38;
	CpsRom[0x6168] = 0xC2;
	CpsRom[0x6169] = 0x38;
	CpsRom[0x616A] = 0xC3;
	CpsRom[0x616B] = 0x38;
	CpsRom[0x616C] = 0xC0;
	CpsRom[0x616D] = 0x38;
	CpsRom[0x616E] = 0xC4;
	CpsRom[0x616F] = 0x38;
	CpsRom[0x6188] = 0xC2;
	CpsRom[0x6189] = 0x38;
	CpsRom[0x618A] = 0xC3;
	CpsRom[0x618B] = 0x38;
	CpsRom[0x618C] = 0xC0;
	CpsRom[0x618D] = 0x38;
	CpsRom[0x618E] = 0xC4;
	CpsRom[0x618F] = 0x38;
	CpsRom[0x61A0] = 0xC2;
	CpsRom[0x61A1] = 0x38;
	CpsRom[0x61A2] = 0xC3;
	CpsRom[0x61A3] = 0x38;
	CpsRom[0x61A4] = 0xC0;
	CpsRom[0x61A5] = 0x38;
	CpsRom[0x61A6] = 0xC4;
	CpsRom[0x61A7] = 0x38;
	CpsRom[0x61B8] = 0xC2;
	CpsRom[0x61B9] = 0x38;
	CpsRom[0x61BA] = 0xC3;
	CpsRom[0x61BB] = 0x38;
	CpsRom[0x61BC] = 0xC0;
	CpsRom[0x61BD] = 0x38;
	CpsRom[0x61BE] = 0xC4;
	CpsRom[0x61BF] = 0x38;
	CpsRom[0x6232] = 0xC0;
	CpsRom[0x6233] = 0x38;
	CpsRom[0x6234] = 0xC3;
	CpsRom[0x6235] = 0x38;
	CpsRom[0x6236] = 0xC1;
	CpsRom[0x6237] = 0x38;
	CpsRom[0x6238] = 0xC4;
	CpsRom[0x6239] = 0x38;
	CpsRom[0x623A] = 0x6C;
	CpsRom[0x623B] = 0x2;
	CpsRom[0x623C] = 0xFF;
	CpsRom[0x623E] = 0xFE;
	CpsRom[0x623F] = 0xFF;
	CpsRom[0x6284] = 0xC0;
	CpsRom[0x6285] = 0x38;
	CpsRom[0x6286] = 0xC3;
	CpsRom[0x6287] = 0x38;
	CpsRom[0x6288] = 0xC1;
	CpsRom[0x6289] = 0x38;
	CpsRom[0x628A] = 0xC4;
	CpsRom[0x628B] = 0x38;
	CpsRom[0x62CE] = 0xC0;
	CpsRom[0x62CF] = 0x38;
	CpsRom[0x62D0] = 0xC3;
	CpsRom[0x62D1] = 0x38;
	CpsRom[0x62D2] = 0xC1;
	CpsRom[0x62D3] = 0x38;
	CpsRom[0x62D4] = 0xC4;
	CpsRom[0x62D5] = 0x38;
	CpsRom[0x6318] = 0xC0;
	CpsRom[0x6319] = 0x38;
	CpsRom[0x631A] = 0xC3;
	CpsRom[0x631B] = 0x38;
	CpsRom[0x631C] = 0xC1;
	CpsRom[0x631D] = 0x38;
	CpsRom[0x631E] = 0xC4;
	CpsRom[0x631F] = 0x38;
	CpsRom[0x63B0] = 0xC0;
	CpsRom[0x63B1] = 0x38;
	CpsRom[0x63B2] = 0xC3;
	CpsRom[0x63B3] = 0x38;
	CpsRom[0x63B4] = 0xC1;
	CpsRom[0x63B5] = 0x38;
	CpsRom[0x63B6] = 0xD9;
	CpsRom[0x63B7] = 0x38;
	CpsRom[0x640A] = 0xC0;
	CpsRom[0x640B] = 0x38;
	CpsRom[0x640C] = 0xC3;
	CpsRom[0x640D] = 0x38;
	CpsRom[0x640E] = 0xC1;
	CpsRom[0x640F] = 0x38;
	CpsRom[0x6410] = 0xD9;
	CpsRom[0x6411] = 0x38;
	CpsRom[0x645C] = 0xC0;
	CpsRom[0x645D] = 0x38;
	CpsRom[0x645E] = 0xC3;
	CpsRom[0x645F] = 0x38;
	CpsRom[0x6460] = 0xC1;
	CpsRom[0x6461] = 0x38;
	CpsRom[0x6462] = 0xD9;
	CpsRom[0x6463] = 0x38;
	CpsRom[0x64AE] = 0xC0;
	CpsRom[0x64AF] = 0x38;
	CpsRom[0x64B0] = 0xC3;
	CpsRom[0x64B1] = 0x38;
	CpsRom[0x64B2] = 0xC1;
	CpsRom[0x64B3] = 0x38;
	CpsRom[0x64B4] = 0xD9;
	CpsRom[0x64B5] = 0x38;
	CpsRom[0x657E] = 0xC0;
	CpsRom[0x657F] = 0x38;
	CpsRom[0x6580] = 0xC3;
	CpsRom[0x6581] = 0x38;
	CpsRom[0x6582] = 0xC1;
	CpsRom[0x6583] = 0x38;
	CpsRom[0x6584] = 0xC4;
	CpsRom[0x6585] = 0x38;
	CpsRom[0x65D4] = 0xC0;
	CpsRom[0x65D5] = 0x38;
	CpsRom[0x65D6] = 0xC3;
	CpsRom[0x65D7] = 0x38;
	CpsRom[0x65D8] = 0xC1;
	CpsRom[0x65D9] = 0x38;
	CpsRom[0x65DA] = 0xC4;
	CpsRom[0x65DB] = 0x38;
	CpsRom[0x6622] = 0xC0;
	CpsRom[0x6623] = 0x38;
	CpsRom[0x6624] = 0xC3;
	CpsRom[0x6625] = 0x38;
	CpsRom[0x6626] = 0xC1;
	CpsRom[0x6627] = 0x38;
	CpsRom[0x6628] = 0xC4;
	CpsRom[0x6629] = 0x38;
	CpsRom[0x6670] = 0xC0;
	CpsRom[0x6671] = 0x38;
	CpsRom[0x6672] = 0xC3;
	CpsRom[0x6673] = 0x38;
	CpsRom[0x6674] = 0xC1;
	CpsRom[0x6675] = 0x38;
	CpsRom[0x6676] = 0xC4;
	CpsRom[0x6677] = 0x38;
	CpsRom[0x6708] = 0xC1;
	CpsRom[0x6709] = 0x38;
	CpsRom[0x670A] = 0xC2;
	CpsRom[0x670B] = 0x38;
	CpsRom[0x670C] = 0xC3;
	CpsRom[0x670D] = 0x38;
	CpsRom[0x6870] = 0xC2;
	CpsRom[0x6871] = 0x38;
	CpsRom[0x6872] = 0xC3;
	CpsRom[0x6873] = 0x38;
	CpsRom[0x6874] = 0xFC;
	CpsRom[0x6875] = 0x38;
	CpsRom[0x9D96] = 0x69;
	CpsRom[0x9D97] = 0x42;
	CpsRom[0x9D98] = 0x4;
	CpsRom[0x9D9A] = 0x69;
	CpsRom[0x9D9B] = 0x42;
	CpsRom[0xE1A6] = 0x0;
	CpsRom[0xE1A7] = 0x7A;
	CpsRom[0xE1A8] = 0x2D;
	CpsRom[0xE1A9] = 0x1A;
	CpsRom[0xE1AA] = 0x49;
	CpsRom[0xE1AB] = 0x8E;
	CpsRom[0x69958] = 0x91;
	CpsRom[0x69959] = 0x42;
	CpsRom[0x6995A] = 0xE9;
	CpsRom[0x6995B] = 0x43;
	CpsRom[0x6995C] = 0x80;
	CpsRom[0x6995D] = 0x0;
	CpsRom[0x7A12E] = 0x80;
	CpsRom[0x7A130] = 0x6C;
	CpsRom[0x7A131] = 0x1;
	CpsRom[0x7A134] = 0x80;
	CpsRom[0x7A136] = 0x6A;
	CpsRom[0x7A137] = 0x1;
}

static int kodbInit()
{
	Kodb = 1;
	pCpsInitCallback = kodb_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsKodb = {
	"kodb", "kod", NULL, "1991",
	"The King of Dragons (bootleg 910731)\0", NULL, "[Capcom] (Playmark bootleg)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1,
	NULL,KodbRomInfo,KodbRomName,kodInputInfo, kodDIPInfo,
	kodbInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Captain Commando (World 911014)

static struct BurnRomInfo CaptcommRomDesc[] = {
	{ "cce_23d.rom", 0x080000, 0x19c58ece, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cc_22d.rom",  0x080000, 0xa91949b7, 1 | BRF_ESS | BRF_PRG },
	{ "cc_24d.rom",  0x020000, 0x680e543f, 1 | BRF_ESS | BRF_PRG },
	{ "cc_28d.rom",  0x020000, 0x8820039f, 1 | BRF_ESS | BRF_PRG },

	{ "gfx_01.rom",  0x080000, 0x7261d8ba, 2 | BRF_GRA },
	{ "gfx_03.rom",  0x080000, 0x6a60f949, 2 | BRF_GRA },
	{ "gfx_02.rom",  0x080000, 0x00637302, 2 | BRF_GRA },
	{ "gfx_04.rom",  0x080000, 0xcc87cf61, 2 | BRF_GRA },
	{ "gfx_05.rom",  0x080000, 0x28718bed, 2 | BRF_GRA },
	{ "gfx_07.rom",  0x080000, 0xd4acc53a, 2 | BRF_GRA },
	{ "gfx_06.rom",  0x080000, 0x0c69f151, 2 | BRF_GRA },
	{ "gfx_08.rom",  0x080000, 0x1f9ebb97, 2 | BRF_GRA },

	{ "cc_09.rom",   0x010000, 0x698e8b58, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "cc_18.rom",   0x020000, 0x6de2c2db, 4 | BRF_SND },
	{ "cc_19.rom",   0x020000, 0xb99091ae, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "cc63b.1a",    0x000117, 0xcae8f0f9, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",    0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "ccprg.11d",   0x000117, 0xe1c225c4, 0 | BRF_OPT },
	{ "ioc1.ic7",    0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
	{ "c632.ic1",    0x000117, 0x0fbd9270, 0 | BRF_OPT },
};

STD_ROM_PICK(Captcomm) STD_ROM_FN(Captcomm)

struct BurnDriver BurnDrvCpsCaptcomm = {
	"captcomm", NULL, NULL, "1991",
	"Captain Commando (World 911014)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,4,HARDWARE_CAPCOM_CPS1,
	NULL,CaptcommRomInfo,CaptcommRomName,captcommInputInfo, captcommDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Captain Commando (US 910928)

static struct BurnRomInfo CaptcommuRomDesc[] = {
	{ "23b",        0x080000, 0x03da44fd, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "22c",        0x080000, 0x9b82a052, 1 | BRF_ESS | BRF_PRG },
	{ "24b",        0x020000, 0x84ff99b2, 1 | BRF_ESS | BRF_PRG },
	{ "28b",        0x020000, 0xfbcec223, 1 | BRF_ESS | BRF_PRG },

	{ "gfx_01.rom", 0x080000, 0x7261d8ba, 2 | BRF_GRA },
	{ "gfx_03.rom", 0x080000, 0x6a60f949, 2 | BRF_GRA },
	{ "gfx_02.rom", 0x080000, 0x00637302, 2 | BRF_GRA },
	{ "gfx_04.rom", 0x080000, 0xcc87cf61, 2 | BRF_GRA },
	{ "gfx_05.rom", 0x080000, 0x28718bed, 2 | BRF_GRA },
	{ "gfx_07.rom", 0x080000, 0xd4acc53a, 2 | BRF_GRA },
	{ "gfx_06.rom", 0x080000, 0x0c69f151, 2 | BRF_GRA },
	{ "gfx_08.rom", 0x080000, 0x1f9ebb97, 2 | BRF_GRA },

	{ "cc_09.rom",  0x010000, 0x698e8b58, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "cc_18.rom",  0x020000, 0x6de2c2db, 4 | BRF_SND },
	{ "cc_19.rom",  0x020000, 0xb99091ae, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "cc63b.1a",   0x000117, 0xcae8f0f9, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",   0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "ccprg.11d",  0x000117, 0xe1c225c4, 0 | BRF_OPT },
	{ "ioc1.ic7",   0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
	{ "c632.ic1",   0x000117, 0x0fbd9270, 0 | BRF_OPT },
};

STD_ROM_PICK(Captcommu) STD_ROM_FN(Captcommu)

struct BurnDriver BurnDrvCpsCaptcommu = {
	"captcommu", "captcomm", NULL, "1991",
	"Captain Commando (US 910928)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,4,HARDWARE_CAPCOM_CPS1,
	NULL,CaptcommuRomInfo,CaptcommuRomName,captcommInputInfo, captcommDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Captain Commando (Japan 911202)

static struct BurnRomInfo CaptcommjRomDesc[] = {
	{ "cc23.bin",   0x080000, 0x5b482b62, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cc22.bin",   0x080000, 0x0fd34195, 1 | BRF_ESS | BRF_PRG },
	{ "cc24.bin",   0x020000, 0x3a794f25, 1 | BRF_ESS | BRF_PRG },
	{ "cc28.bin",   0x020000, 0xfc3c2906, 1 | BRF_ESS | BRF_PRG },

	{ "gfx_01.rom", 0x080000, 0x7261d8ba, 2 | BRF_GRA },
	{ "gfx_03.rom", 0x080000, 0x6a60f949, 2 | BRF_GRA },
	{ "gfx_02.rom", 0x080000, 0x00637302, 2 | BRF_GRA },
	{ "gfx_04.rom", 0x080000, 0xcc87cf61, 2 | BRF_GRA },
	{ "gfx_05.rom", 0x080000, 0x28718bed, 2 | BRF_GRA },
	{ "gfx_07.rom", 0x080000, 0xd4acc53a, 2 | BRF_GRA },
	{ "gfx_06.rom", 0x080000, 0x0c69f151, 2 | BRF_GRA },
	{ "gfx_08.rom", 0x080000, 0x1f9ebb97, 2 | BRF_GRA },

	{ "cc_09.rom",  0x010000, 0x698e8b58, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "cc_18.rom",  0x020000, 0x6de2c2db, 4 | BRF_SND },
	{ "cc_19.rom",  0x020000, 0xb99091ae, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "cc63b.1a",   0x000117, 0xcae8f0f9, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",   0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "ccprg.11d",  0x000117, 0xe1c225c4, 0 | BRF_OPT },
	{ "ioc1.ic7",   0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
	{ "c632.ic1",   0x000117, 0x0fbd9270, 0 | BRF_OPT },
};

STD_ROM_PICK(Captcommj) STD_ROM_FN(Captcommj)

struct BurnDriver BurnDrvCpsCaptcommj = {
	"captcommj", "captcomm", NULL, "1991",
	"Captain Commando (Japan 911202)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,4,HARDWARE_CAPCOM_CPS1,
	NULL,CaptcommjRomInfo,CaptcommjRomName,captcommInputInfo, captcommDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Captain Commando (bootleg)

static struct BurnRomInfo CaptcommbRomDesc[] = {
	{ "25.bin",      0x080000, 0xcb71ed7a, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "27.bin",      0x080000, 0x47cb2e87, 1 | BRF_ESS | BRF_PRG },
	{ "24.bin",      0x040000, 0x79794279, 1 | BRF_ESS | BRF_PRG },
	{ "26.bin",      0x040000, 0xb01077ba, 1 | BRF_ESS | BRF_PRG },

	{ "c91e-01.bin", 0x100000, 0xf863071c, 2 | BRF_GRA },
	{ "c91e-02.bin", 0x100000, 0x4b03c308, 2 | BRF_GRA },
	{ "c91e-03.bin", 0x100000, 0x3383ea96, 2 | BRF_GRA },
	{ "c91e-04.bin", 0x100000, 0xb8e1f4cf, 2 | BRF_GRA },

	{ "1.bin",       0x010000, 0x698e8b58, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "c91e-05.bin", 0x040000, 0x096115fb, 4 | BRF_SND },
};

STD_ROM_PICK(Captcommb) STD_ROM_FN(Captcommb)

static void bootleg_progload2()
{
	// First four program roms are 512kb and
	// should be interleaved
	BurnLoadRom(CpsRom + 0x000001, 0, 2);
	BurnLoadRom(CpsRom + 0x000000, 1, 2);
	BurnLoadRom(CpsRom + 0x100001, 2, 2);
	BurnLoadRom(CpsRom + 0x100000, 3, 2);
}

static void captcommb_patch()
{
	bootleg_progload2();

	CpsRom[0x4B8A] = 0x91;
	CpsRom[0x4B8D] = 0x0;
	CpsRom[0x4B8E] = 0x2D;
	CpsRom[0x4B8F] = 0x8;
	CpsRom[0x4F22] = 0x6D;
	CpsRom[0x4F23] = 0x8;
	CpsRom[0x4F24] = 0x6;
	CpsRom[0x4F26] = 0x1B;
	CpsRom[0x4F27] = 0x28;
}

static int captcommbInit()
{
	pCpsInitCallback = captcommb_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsCaptcommb = {
	"captcommb", "captcomm", NULL, "1991",
	"Captain Commando (bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,4,HARDWARE_CAPCOM_CPS1,
	NULL,CaptcommbRomInfo,CaptcommbRomName,captcommInputInfo, captcommDIPInfo,
	captcommbInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Knights of the Round (World 911127)

static struct BurnRomInfo KnightsRomDesc[] = {
	{ "kr-23e.8f",  0x080000, 0x1b3997eb, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "kr-22.7f",   0x080000, 0xd0b671a9, 1 | BRF_ESS | BRF_PRG },

	{ "kr-5m.3a",   0x080000, 0x9e36c1a4, 2 | BRF_GRA },
	{ "kr-7m.4a",   0x080000, 0xc5832cae, 2 | BRF_GRA },
	{ "kr-1m.5a",   0x080000, 0xf095be2d, 2 | BRF_GRA },
	{ "kr-3m.6a",   0x080000, 0x179dfd96, 2 | BRF_GRA },
	{ "kr-6m.7a",   0x080000, 0x1f4298d2, 2 | BRF_GRA },
	{ "kr-8m.8a",   0x080000, 0x37fa8751, 2 | BRF_GRA },
	{ "kr-2m.9a",   0x080000, 0x0200bc3d, 2 | BRF_GRA },
	{ "kr-4m.10a",  0x080000, 0x0bb2b4e7, 2 | BRF_GRA },

	{ "kr-09.11a",  0x010000, 0x5e44d9ee, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "kr-18.11c",  0x020000, 0xda69d15f, 4 | BRF_SND },
	{ "kr-19.12c",  0x020000, 0xbfc654e9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "kr63b.1a",   0x000117, 0xfd5b6522, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",   0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "bprg1.11d",  0x000117, 0x31793da7, 0 | BRF_OPT },
	{ "ioc1.ic7",   0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
	{ "c632.ic1",   0x000117, 0x0fbd9270, 0 | BRF_OPT },
};

STD_ROM_PICK(Knights) STD_ROM_FN(Knights)

struct BurnDriver BurnDrvCpsKnights = {
	"knights", NULL, NULL, "1991",
	"Knights of the Round (World 911127)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,3,HARDWARE_CAPCOM_CPS1,
	NULL,KnightsRomInfo,KnightsRomName,knightsInputInfo, knightsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Knights of the Round (US 911127)

static struct BurnRomInfo KnightsuRomDesc[] = {
	{ "kru23.rom",   0x080000, 0x252bc2ba, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "kr_22.rom",   0x080000, 0xd0b671a9, 1 | BRF_ESS | BRF_PRG },

	{ "kr_gfx1.rom", 0x080000, 0x9e36c1a4, 2 | BRF_GRA },
	{ "kr_gfx3.rom", 0x080000, 0xc5832cae, 2 | BRF_GRA },
	{ "kr_gfx2.rom", 0x080000, 0xf095be2d, 2 | BRF_GRA },
	{ "kr_gfx4.rom", 0x080000, 0x179dfd96, 2 | BRF_GRA },
	{ "kr_gfx5.rom", 0x080000, 0x1f4298d2, 2 | BRF_GRA },
	{ "kr_gfx7.rom", 0x080000, 0x37fa8751, 2 | BRF_GRA },
	{ "kr_gfx6.rom", 0x080000, 0x0200bc3d, 2 | BRF_GRA },
	{ "kr_gfx8.rom", 0x080000, 0x0bb2b4e7, 2 | BRF_GRA },

	{ "kr_09.rom",   0x010000, 0x5e44d9ee, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "kr_18.rom",   0x020000, 0xda69d15f, 4 | BRF_SND },
	{ "kr_19.rom",   0x020000, 0xbfc654e9, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "kr63b.1a",    0x000117, 0xfd5b6522, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",    0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "bprg1.11d",   0x000117, 0x31793da7, 0 | BRF_OPT },
	{ "ioc1.ic7",    0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
	{ "c632.ic1",    0x000117, 0x0fbd9270, 0 | BRF_OPT },
};

STD_ROM_PICK(Knightsu) STD_ROM_FN(Knightsu)

struct BurnDriver BurnDrvCpsKnightsu = {
	"knightsu", "knights", NULL, "1991",
	"Knights of the Round (US 911127)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1,
	NULL,KnightsuRomInfo,KnightsuRomName,knightsInputInfo, knightsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Knights of the Round (Japan 911127)

static struct BurnRomInfo KnightsjRomDesc[] = {
	{ "krj30.bin",   0x020000, 0xad3d1a8e, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "krj37.bin",   0x020000, 0xe694a491, 1 | BRF_ESS | BRF_PRG },
	{ "krj31.bin",   0x020000, 0x85596094, 1 | BRF_ESS | BRF_PRG },
	{ "krj38.bin",   0x020000, 0x9198bf8f, 1 | BRF_ESS | BRF_PRG },
	{ "kr_22.rom",   0x080000, 0xd0b671a9, 1 | BRF_ESS | BRF_PRG },

	{ "kr_gfx1.rom", 0x080000, 0x9e36c1a4, 2 | BRF_GRA },
	{ "kr_gfx3.rom", 0x080000, 0xc5832cae, 2 | BRF_GRA },
	{ "kr_gfx2.rom", 0x080000, 0xf095be2d, 2 | BRF_GRA },
	{ "kr_gfx4.rom", 0x080000, 0x179dfd96, 2 | BRF_GRA },
	{ "kr_gfx5.rom", 0x080000, 0x1f4298d2, 2 | BRF_GRA },
	{ "kr_gfx7.rom", 0x080000, 0x37fa8751, 2 | BRF_GRA },
	{ "kr_gfx6.rom", 0x080000, 0x0200bc3d, 2 | BRF_GRA },
	{ "kr_gfx8.rom", 0x080000, 0x0bb2b4e7, 2 | BRF_GRA },

	{ "kr_09.rom",   0x010000, 0x5e44d9ee, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "kr_18.rom",   0x020000, 0xda69d15f, 4 | BRF_SND },
	{ "kr_19.rom",   0x020000, 0xbfc654e9, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Knightsj) STD_ROM_FN(Knightsj)

struct BurnDriver BurnDrvCpsKnightsj = {
	"knightsj", "knights", NULL, "1991",
	"Knights of the Round (Japan 911127)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1,
	NULL,KnightsjRomInfo,KnightsjRomName,knightsInputInfo, knightsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Knights of the Round (bootleg set 1)

static struct BurnRomInfo KnightsbRomDesc[] = {
	{ "5.ic172",       0x40000, 0x7fd91118, 1 | BRF_ESS | BRF_PRG },
	{ "3.ic173",       0x40000, 0xc9c6e720, 1 | BRF_ESS | BRF_PRG },
	{ "4.ic176",       0x40000, 0xaf352703, 1 | BRF_ESS | BRF_PRG },
	{ "2.ic175",       0x40000, 0x1eb91343, 1 | BRF_ESS | BRF_PRG },

	{ "kr_gfx1.rom",   0x80000, 0x9e36c1a4, 2 | BRF_GRA },
	{ "kr_gfx3.rom",   0x80000, 0xc5832cae, 2 | BRF_GRA },
	{ "kr_gfx2.rom",   0x80000, 0xf095be2d, 2 | BRF_GRA },
	{ "kr_gfx4.rom",   0x80000, 0x179dfd96, 2 | BRF_GRA },
	{ "kr_gfx5.rom",   0x80000, 0x1f4298d2, 2 | BRF_GRA },
	{ "kr_gfx7.rom",   0x80000, 0x37fa8751, 2 | BRF_GRA },
	{ "kr_gfx6.rom",   0x80000, 0x0200bc3d, 2 | BRF_GRA },
	{ "kr_gfx8.rom",   0x80000, 0x0bb2b4e7, 2 | BRF_GRA },

	{ "1.ic26",        0x40000, 0xbd6f9cc1, 3 | BRF_ESS | BRF_PRG },
};

STD_ROM_PICK(Knightsb) STD_ROM_FN(Knightsb)

struct BurnDriver BurnDrvCpsKnightsb = {
	"knightsb", "knights", NULL, "1991",
	"Knights of the Round (bootleg set 1)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1,
	NULL,KnightsbRomInfo,KnightsbRomName,knightsInputInfo, knightsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (World 920313)

static struct BurnRomInfo Sf2ceRomDesc[] = {
	{"sf2ce.23",    0x080000, 0x3f846b74, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2ce.22",    0x080000, 0x99f1cca4, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ce) STD_ROM_FN(Sf2ce)

struct BurnDriver BurnDrvCpsSf2ce = {
	"sf2ce", NULL, NULL, "1992",
	"Street Fighter II' - Champion Edition (World 920313)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2ceRomInfo, Sf2ceRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (US 920313)

static struct BurnRomInfo Sf2ceuaRomDesc[] = {
	{"s92u-23a",    0x080000, 0xac44415b, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2ce.22",    0x080000, 0x99f1cca4, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ceua) STD_ROM_FN(Sf2ceua)

struct BurnDriver BurnDrvCpsSf2ceua = {
	"sf2ceua", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (US 920313)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2ceuaRomInfo, Sf2ceuaRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (US 920513)

static struct BurnRomInfo Sf2ceubRomDesc[] = {
	{"s92-23b",    0x080000, 0x996a3015, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"s92-22b",    0x080000, 0x2bbe15ed, 1 | BRF_ESS | BRF_PRG },
	{"s92-21b",    0x080000, 0xb383cb1c, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin", 0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin", 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin", 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin", 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin", 0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin", 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin", 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin", 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin", 0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin", 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin", 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin", 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin", 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin", 0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin", 0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ceub) STD_ROM_FN(Sf2ceub)

struct BurnDriver BurnDrvCpsSf2ceub={
	"sf2ceub", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (US 920513)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2ceubRomInfo, Sf2ceubRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (US 920803)

static struct BurnRomInfo Sf2ceucRomDesc[] = {
	{"s92-23c",    0x080000, 0x0a8b6aa2, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"s92-22c",    0x080000, 0x5fd8630b, 1 | BRF_ESS | BRF_PRG },
	{"s92-21b",    0x080000, 0xb383cb1c, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin", 0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin", 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin", 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin", 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin", 0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin", 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin", 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin", 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin", 0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin", 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin", 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin", 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin", 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin", 0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin", 0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ceuc) STD_ROM_FN(Sf2ceuc)

struct BurnDriver BurnDrvCpsSf2ceuc={
	"sf2ceuc", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (US 920803)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2ceucRomInfo, Sf2ceucRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Japan 920513)

static struct BurnRomInfo Sf2cejRomDesc[] = {
	{"s92j_23b.bin", 0x080000, 0x140876c5, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"s92j_22b.bin", 0x080000, 0x2fbb3bfe, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin",  0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",   0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",   0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",   0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",   0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",   0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",   0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",   0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",   0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",   0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",   0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",   0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",   0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",   0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2cej) STD_ROM_FN(Sf2cej)

struct BurnDriver BurnDrvCpsSf2cej={
	"sf2cej", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Japan 920513)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2cejRomInfo, Sf2cejRomName, sf2InputInfo, sf2jDIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Rainbow set 1, bootleg)

static struct BurnRomInfo Sf2rbRomDesc[] = {
	{"sf2d__23.rom", 0x080000, 0x450532b0, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2d__22.rom", 0x080000, 0xfe9d9cf5, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin",  0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",   0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",   0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",   0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",   0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",   0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",   0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",   0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",   0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",   0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",   0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",   0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",   0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",   0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2rb) STD_ROM_FN(Sf2rb)

static void sf2rb_patch()
{
	// byteswap first two roms
	BurnByteswap(CpsRom, 0x100000);

	*((unsigned short*)(CpsRom + 0xe5464)) = 0x6012;
}

static int sf2rbInit()
{
	pCpsInitCallback = sf2rb_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2rb={
	"sf2rb", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Rainbow set 1, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2rbRomInfo, Sf2rbRomName, sf2InputInfo, sf2DIPInfo,
	sf2rbInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Rainbow set 2, bootleg)

static struct BurnRomInfo Sf2rb2RomDesc[] = {
	{"27.bin",      0x020000, 0x40296ecd, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"31.bin",      0x020000, 0x87954a41, 1 | BRF_ESS | BRF_PRG },
	{"26.bin",      0x020000, 0xa6974195, 1 | BRF_ESS | BRF_PRG },
	{"30.bin",      0x020000, 0x8141fe32, 1 | BRF_ESS | BRF_PRG },
	{"25.bin",      0x020000, 0x9ef8f772, 1 | BRF_ESS | BRF_PRG },
	{"29.bin",      0x020000, 0x7d9c479c, 1 | BRF_ESS | BRF_PRG },
	{"24.bin",      0x020000, 0x93579684, 1 | BRF_ESS | BRF_PRG },
	{"28.bin",      0x020000, 0xff728865, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2rb2) STD_ROM_FN(Sf2rb2)

static void sf2rb2_patch()
{
	*((unsigned short*)(CpsRom + 0xe5332)) = 0x6014;
}

static int sf2rb2Init()
{
	pCpsInitCallback = sf2rb2_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2rb2={
	"sf2rb2", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Rainbow set 2, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2rb2RomInfo, Sf2rb2RomName, sf2InputInfo, sf2DIPInfo,
	sf2rb2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Rainbow set 3, bootleg)

static struct BurnRomInfo Sf2rb3RomDesc[] = {
	{"sf2_ce_rb.23",0x080000, 0x202f9e50, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2_ce_rb.22",0x080000, 0x145e5219, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2rb3) STD_ROM_FN(Sf2rb3)

struct BurnDriver BurnDrvCpsSf2rb3={
	"sf2rb3", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Rainbow set 3, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2rb3RomInfo, Sf2rb3RomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Red Wave, bootleg)

static struct BurnRomInfo Sf2redRomDesc[] = {
	{"sf2red.23",  0x080000, 0x40276abb, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2red.22",  0x080000, 0x18daf387, 1 | BRF_ESS | BRF_PRG },
	{"sf2red.21",  0x080000, 0x52c486bb, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin", 0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin", 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin", 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin", 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin", 0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin", 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin", 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin", 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin", 0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin", 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin", 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin", 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin", 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin", 0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin", 0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2red) STD_ROM_FN(Sf2red)

struct BurnDriver BurnDrvCpsSf2red={
	"sf2red", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Red Wave, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2redRomInfo, Sf2redRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II! - Champion Edition (V004, bootleg)

static struct BurnRomInfo Sf2v004RomDesc[] = {
	{"sf2v004.23", 0x080000, 0x52d19f2c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2v004.22", 0x080000, 0x4b26fde7, 1 | BRF_ESS | BRF_PRG },
	{"sf2red.21",  0x080000, 0x52c486bb, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin", 0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin", 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin", 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin", 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin", 0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin", 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin", 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin", 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin", 0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin", 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin", 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin", 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin", 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin", 0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin", 0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2v004) STD_ROM_FN(Sf2v004)

struct BurnDriver BurnDrvCpsSf2v004={
	"sf2v004", "sf2ce", NULL, "1992",
	"Street Fighter II! - Champion Edition (V004, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2v004RomInfo, Sf2v004RomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Accelerator!, bootleg)

static struct BurnRomInfo Sf2accRomDesc[] = {
	{"sf2ca_23-c.bin", 0x080000, 0xe7c8c5a6, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2ca_22-c.bin", 0x080000, 0x99f1cca4, 1 | BRF_ESS | BRF_PRG },
	{"sf2ca_21-c.bin", 0x040000, 0xcf7fcc8c, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",   0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",   0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",   0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",   0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",   0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",   0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",   0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",   0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",   0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",   0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",   0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",   0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",   0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2acc) STD_ROM_FN(Sf2acc)

struct BurnDriver BurnDrvCpsSf2acc={
	"sf2acc", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Accelerator!, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2accRomInfo, Sf2accRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Accelerator Pt.II, bootleg)

static struct BurnRomInfo Sf2accp2RomDesc[] = {
	{"sf2ca-23.bin", 0x080000, 0x36c3ba2f, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2ca-22.bin", 0x080000, 0x0550453d, 1 | BRF_ESS | BRF_PRG },
	{"sf2ca-21.bin", 0x040000, 0x4c1c43ba, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",   0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",   0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",   0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",   0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",   0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",   0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",   0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",   0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",   0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",   0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",   0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",   0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",   0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2accp2) STD_ROM_FN(Sf2accp2)

static void Sf2accp2_patch()
{
	*((unsigned short*)(CpsRom + 0x11756)) = 0x4e71;
}

static int Sf2accp2Init()
{
	pCpsInitCallback = Sf2accp2_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2accp2={
	"sf2accp2", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Accelerator Pt.II, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2accp2RomInfo, Sf2accp2RomName, sf2InputInfo, sf2DIPInfo,
	Sf2accp2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Double K.O. Turbo II, bootleg)

static struct BurnRomInfo Sf2dkot2RomDesc[] = {
	{"turboii.23",   0x080000, 0x9bbfe420, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"turboii.22",   0x080000, 0x3e57ba19, 1 | BRF_ESS | BRF_PRG },
	{"turboii.21",   0x080000, 0xed4186bd, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",   0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",   0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",   0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",   0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",   0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",   0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",   0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",   0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",   0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",   0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",   0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",   0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",   0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2dkot2) STD_ROM_FN(Sf2dkot2)

static int Sf2dkot2Init()
{
	int nRet;

	nRet = CpsInit();

	SekOpen(0);
	SekMapMemory(CpsRom + 0x000000,	0x280000, 0x2fffff, SM_ROM);
	SekMapMemory(CpsRom + 0x080000,	0x200000, 0x27ffff, SM_ROM);
	SekClose();

	return nRet;
}

struct BurnDriver BurnDrvCpsSf2dkot2={
	"sf2dkot2", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Double K.O. Turbo II, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2dkot2RomInfo, Sf2dkot2RomName, sf2InputInfo, sf2DIPInfo,
	Sf2dkot2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (M1, bootleg)

static struct BurnRomInfo Sf2m1RomDesc[] = {
	{"222e", 	0x080000, 0x1e20d0a3, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"196e", 	0x080000, 0x88cc38a3, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m1) STD_ROM_FN(Sf2m1)

static void sf2m1_fix()
{
	bootleg_progload();

#if 1
	// hack the program rom to make it run
	unsigned char patch_data[0x10C] = {
		0x3C, 0x03, 0xED, 0x33, 0x26, 0x00, 0x80, 0x00,	0x00, 0x01, 0xED, 0x33, 0x2E, 0x00, 0x80, 0x00,
		0x08, 0x01, 0xED, 0x33, 0x2A, 0x00, 0x80, 0x00,	0x04, 0x01, 0xED, 0x33, 0x28, 0x00, 0x80, 0x00,
		0x02, 0x01, 0xED, 0x33, 0x2C, 0x00, 0x80, 0x00,	0x06, 0x01, 0xED, 0x33, 0x64, 0x00, 0x80, 0x00,
		0x14, 0x01, 0xED, 0x33, 0x5C, 0x00, 0x80, 0x00,	0x0C, 0x01, 0xED, 0x33, 0x60, 0x00, 0x80, 0x00,
		0x10, 0x01, 0xED, 0x33, 0x62, 0x00, 0x80, 0x00,	0x12, 0x01, 0xED, 0x33, 0x5E, 0x00, 0x80, 0x00,
		0x0E, 0x01, 0xED, 0x33, 0x66, 0x00, 0x80, 0x00,	0x16, 0x01, 0xED, 0x33, 0x70, 0x00, 0x80, 0x00,
		0x20, 0x01, 0x2D, 0x10, 0xCE, 0x02, 0x2D, 0x12,	0xDB, 0x02, 0x00, 0xB3, 0x40, 0x1B, 0x89, 0x00,
		0x48, 0xE1, 0x6D, 0x80, 0x48, 0x00, 0xC0, 0x33,	0x80, 0x00, 0x22, 0x01, 0xED, 0x33, 0x4E, 0x00,
		0x80, 0x00, 0x66, 0x01, 0xED, 0x13, 0xD7, 0x02,	0x80, 0x00, 0x30, 0x00, 0xED, 0x33, 0x50, 0x00,
		0x80, 0x00, 0x68, 0x01, 0xED, 0x33, 0x52, 0x00,	0x80, 0x00, 0x6A, 0x01, 0xED, 0x33, 0x54, 0x00,
		0x80, 0x00, 0x6C, 0x01, 0xED, 0x33, 0x56, 0x00,	0x80, 0x00, 0x6E, 0x01, 0x00, 0x61, 0x78, 0x0E,
		0xED, 0x33, 0x58, 0x00, 0x80, 0x00, 0x70, 0x01,	0xED, 0x33, 0x30, 0x00, 0x80, 0x00, 0x0A, 0x01,
		0xB8, 0x4E, 0x68, 0x14, 0xB8, 0x4E, 0x02, 0x63,	0x00, 0x61, 0x32, 0x11, 0x00, 0x61, 0x40, 0x12,
		0x00, 0x61, 0xBC, 0x0F, 0x2D, 0x52, 0x1C, 0x00,	0xED, 0x50, 0x0E, 0x82, 0x2D, 0x42, 0xDE, 0x02,
		0xED, 0x41, 0x00, 0x80, 0x3C, 0x30, 0x0F, 0x00,	0x28, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x66,
		0x28, 0x53, 0x01, 0x00, 0x06, 0x66, 0x7C, 0x11,	0x04, 0x00, 0x00, 0x00, 0xE8, 0x41, 0x20, 0x00,
		0xC8, 0x51, 0xE6, 0xFF, 0x3C, 0x30, 0x50, 0x00, 0xC8, 0x51, 0xFE, 0xFF
	};

	memcpy (CpsRom + 0x5d8, patch_data, 0x10C);

	*((unsigned short*)(CpsRom + 0x15A4)) = 0xFFC0; // 0x0000 - BG Alignment - Wofh kludge
//	*((unsigned short*)(CpsRom + 0x6322)) = 0x0181; // 0x0006 - SFX		 - Cawingb kludge
#endif

#if 0
	int i;
	unsigned char data0[0x100] = {
		0x3C, 0x03, 0xED, 0x33, 0x26, 0x00, 0x80, 0x00, 0x00, 0x01, 0xED, 0x33, 0x2E, 0x00, 0x80, 0x00,
		0x08, 0x01, 0xED, 0x33, 0x2A, 0x00, 0x80, 0x00, 0x04, 0x01, 0xED, 0x33, 0x28, 0x00, 0x80, 0x00,
		0x02, 0x01, 0xED, 0x33, 0x2C, 0x00, 0x80, 0x00, 0x06, 0x01, 0xED, 0x33, 0x64, 0x00, 0x80, 0x00,
		0x14, 0x01, 0xED, 0x33, 0x5C, 0x00, 0x80, 0x00, 0x0C, 0x01, 0xED, 0x33, 0x60, 0x00, 0x80, 0x00,
		0x10, 0x01, 0xED, 0x33, 0x62, 0x00, 0x80, 0x00, 0x12, 0x01, 0xED, 0x33, 0x5E, 0x00, 0x80, 0x00,
		0x0E, 0x01, 0xED, 0x33, 0x66, 0x00, 0x80, 0x00, 0x16, 0x01, 0xED, 0x33, 0x70, 0x00, 0x80, 0x00,
		0x20, 0x01, 0x2D, 0x10, 0xCE, 0x02, 0x2D, 0x12, 0xDB, 0x02, 0x00, 0xB3, 0x40, 0x1B, 0x89, 0x00,
		0x48, 0xE1, 0x6D, 0x80, 0x48, 0x00, 0xC0, 0x33, 0x80, 0x00, 0x22, 0x01, 0xED, 0x33, 0x4E, 0x00,
		0x80, 0x00, 0x66, 0x01, 0xED, 0x13, 0xD7, 0x02, 0x80, 0x00, 0x30, 0x00, 0xED, 0x33, 0x50, 0x00,
		0x80, 0x00, 0x68, 0x01, 0xED, 0x33, 0x52, 0x00, 0x80, 0x00, 0x6A, 0x01, 0xED, 0x33, 0x54, 0x00,
		0x80, 0x00, 0x6C, 0x01, 0xED, 0x33, 0x56, 0x00, 0x80, 0x00, 0x6E, 0x01, 0x00, 0x61, 0x78, 0x0E,
		0xED, 0x33, 0x58, 0x00, 0x80, 0x00, 0x70, 0x01, 0xED, 0x33, 0x30, 0x00, 0x80, 0x00, 0x0A, 0x01,
		0xB8, 0x4E, 0x68, 0x14, 0xB8, 0x4E, 0x02, 0x63, 0x00, 0x61, 0x32, 0x11, 0x00, 0x61, 0x40, 0x12,
		0x00, 0x61, 0xBC, 0x0F, 0x2D, 0x52, 0x1C, 0x00, 0xED, 0x50, 0x0E, 0x82, 0x2D, 0x42, 0xDE, 0x02,
		0xED, 0x41, 0x00, 0x80, 0x3C, 0x30, 0x0F, 0x00, 0x28, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x66,
		0x28, 0x53, 0x01, 0x00, 0x06, 0x66, 0x7C, 0x11, 0x04, 0x00, 0x00, 0x00, 0xE8, 0x41, 0x20, 0x00
	};

	unsigned char data1[8] = {
		0xC8, 0x51, 0xE6, 0xFF, 0x3C, 0x30, 0x50, 0x00
	};

	for (i = 0; i < 0x100; i++)
		CpsRom[i+0x5D8] = data0[i];

	for (i = 0; i < 0x008; i++)
		CpsRom[i+0x6D8] = data1[i];

	*((unsigned short*)(CpsRom + 0x06E2)) = 0xFFFE; // Black scr
	*((unsigned short*)(CpsRom + 0x15A4)) = 0xFFC0; // Alignment
//	*((unsigned short*)(CpsRom + 0x6322)) = 0x0181; // SFX
#endif
}

/*
static void __fastcall Sf2m1ScrollWrite(unsigned int a, unsigned short d)
{
	if (a == 0x980000) {
		CpsReg[0x0e] = d & 0xff;
		CpsReg[0x0f] = d >> 8;
		return;
	}

	if (a == 0x980002) {
		d -= 0x40;
		CpsReg[0x0c] = d & 0xff;
		CpsReg[0x0d] = d >> 8;
		return;
	}

	if (a == 0x980004) {
		CpsReg[0x12] = d & 0xff;
		CpsReg[0x13] = d >> 8;
		return;
	}

	if (a == 0x980006) {
		d -= 0x40;
		CpsReg[0x10] = d & 0xff;
		CpsReg[0x11] = d >> 8;
		return;
	}

	if (a == 0x980008) {
		CpsReg[0x16] = d & 0xff;
		CpsReg[0x17] = d >> 8;
		return;
	}

	if (a == 0x98000a) {
		d -= 0x40;
		CpsReg[0x14] = d & 0xff;
		CpsReg[0x15] = d >> 8;
		return;
	}

	if (a == 0x980016) {
		CpsReg[0x06] = d & 0xff;
		CpsReg[0x07] = d >> 8;
		return;
	}

	if (a == 0x980166) {
		CpsReg[0x66] = d & 0xff;
		CpsReg[0x67] = d >> 8;
		return;
	}
}
*/

static int sf2m1Init()
{
	int nRet = 0;

	Port6SoundWrite = 1;
	pCpsInitCallback = sf2m1_fix;
//	pCpsInitCallback = bootleg_progload;

	nRet = CpsInit();
/*
	SekOpen(0);
	SekMapHandler(1, 0x980000, 0x980167, SM_WRITE);
	SekSetWriteWordHandler(1, Sf2m1ScrollWrite);
	SekClose();
*/
	return nRet;
}

static int sf2m1Exit()
{
	Port6SoundWrite = 0;

	return CpsExit();
}

struct BurnDriver BurnDrvCpsSf2m1={
	"sf2m1", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M1, bootleg)\0", "Imperfect Graphic", "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2m1RomInfo, Sf2m1RomName, Sf2m1InputInfo, sf2DIPInfo,
	sf2m1Init, sf2m1Exit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (M2, bootleg)

static struct BurnRomInfo Sf2m2RomDesc[] = {
	{"ch222esp",    0x080000, 0x9e6d058a, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"ch196esp",    0x080000, 0xed2ff437, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m2) STD_ROM_FN(Sf2m2)

static int sf2m2Init()
{
	pCpsInitCallback = bootleg_progload;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2m2={
	"sf2m2", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M2, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2m2RomInfo, Sf2m2RomName, Sf2yycInputInfo, sf2m2DIPInfo,
	sf2m2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (M3, bootleg)

static struct BurnRomInfo Sf2m3RomDesc[] = {
	{"u222chp",     0x080000, 0xdb567b66, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"u196chp",     0x080000, 0x95ea597e, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m3) STD_ROM_FN(Sf2m3)

static unsigned char __fastcall Sf2m3ReadByte(unsigned int a)
{
	unsigned char d = 0xff;

	switch (a) {
		case 0x800010: {
			d = (unsigned char)~Inp010;
			return d;
		}

		case 0x800011: {
			d = (unsigned char)~Inp011;
			return d;
		}

		case 0x800029: {
			d = (unsigned char)~Inp029;
			return d;
		}

		case 0x800186: {
			d = (unsigned char)~Inp186;
			return d;
		}

		case 0x80002a: {
			d = (unsigned char)~Cpi01A;
			return d;
		}

		case 0x80002c: {
			d = (unsigned char)~Cpi01C;
			return d;
		}

		case 0x80002e: {
			d = (unsigned char)~Cpi01E;
			return d;
		}
	}

//	bprintf(PRINT_NORMAL, _T("Read byte %x\n"), a);

	return 0;
}

void __fastcall Sf2m3WriteByte(unsigned int a, unsigned char d)
{
	switch (a) {
		case 0x800191: {
			PsndSyncZ80((long long)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndCode = d;
			return;
		}
	}

//	bprintf(PRINT_NORMAL, _T("Write byte %x, %x\n"), a, d);
}

static void __fastcall Sf2m3WriteWord(unsigned int a, unsigned short d)
{
	switch (a) {
		case 0x800100: {
			CpsReg[0x00] = d & 0xff;
			CpsReg[0x01] = d >> 8;
			return;
		}

		case 0x800102: {
			CpsReg[0x02] = d & 0xff;
			CpsReg[0x03] = d >> 8;
			return;
		}

		case 0x800104: {
			CpsReg[0x04] = d & 0xff;
			CpsReg[0x05] = d >> 8;
			return;
		}

		case 0x800106: {
			CpsReg[0x06] = d & 0xff;
			CpsReg[0x07] = d >> 8;
			return;
		}

		case 0x80010a: {
			CpsReg[0x0a] = d & 0xff;
			CpsReg[0x0b] = d >> 8;
			return;
		}

		case 0x800122: {
			CpsReg[0x22] = d & 0xff;
			CpsReg[0x23] = d >> 8;
			return;
		}

		case 0x80014a: {
			CpsReg[0x4a] = d & 0xff;
			CpsReg[0x4b] = d >> 8;
			return;
		}

		case 0x80014c: {
			CpsReg[0x4c] = d & 0xff;
			CpsReg[0x4d] = d >> 8;
			return;
		}

		case 0x80014e: {
			CpsReg[0x4e] = d & 0xff;
			CpsReg[0x4f] = d >> 8;
			return;
		}

		case 0x800150: {
			CpsReg[0x50] = d & 0xff;
			CpsReg[0x51] = d >> 8;
			return;
		}

		case 0x800152: {
			CpsReg[0x52] = d & 0xff;
			CpsReg[0x53] = d >> 8;
			return;
		}

		case 0x8001a8: {
			CpsReg[0x08] = d & 0xff;
			CpsReg[0x09] = d >> 8;
			return;
		}

		case 0x8001ac: {
			CpsReg[0x0c] = d & 0xff;
			CpsReg[0x0d] = d >> 8;
			return;
		}

		case 0x8001ae: {
			CpsReg[0x0e] = d & 0xff;
			CpsReg[0x0f] = d >> 8;
			return;
		}

		case 0x8001b0: {
			CpsReg[0x10] = d & 0xff;
			CpsReg[0x11] = d >> 8;
			return;
		}

		case 0x8001b2: {
			CpsReg[0x12] = d & 0xff;
			CpsReg[0x13] = d >> 8;
			return;
		}

		case 0x8001b4: {
			CpsReg[0x14] = d & 0xff;
			CpsReg[0x15] = d >> 8;
			return;
		}

		case 0x8001b6: {
			CpsReg[0x16] = d & 0xff;
			CpsReg[0x17] = d >> 8;
			return;
		}

		case 0x8001c0: {
			CpsReg[0x20] = d & 0xff;
			CpsReg[0x21] = d >> 8;
			return;
		}

		case 0x8001c4: {
			CpsReg[0xc4] = d & 0xff;
			CpsReg[0xc5] = d >> 8;
			return;
		}
	}

//	bprintf(PRINT_NORMAL, _T("Write word %x, %x\n"), a, d);
}

static int sf2m3Init()
{
	int nRet = 0;

	pCpsInitCallback = bootleg_progload;

	nRet = CpsInit();

	SekOpen(0);
	SekMapHandler(1, 0x800000, 0x800200, SM_RAM);
	SekSetReadByteHandler(1, Sf2m3ReadByte);
	SekSetWriteByteHandler(1, Sf2m3WriteByte);
	SekSetWriteWordHandler(1, Sf2m3WriteWord);
	SekClose();

	return nRet;
}

struct BurnDriver BurnDrvCpsSf2m3={
	"sf2m3", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M3, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2m3RomInfo, Sf2m3RomName, Sf2m3InputInfo, sf2DIPInfo,
	sf2m3Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (M4, bootleg)

static struct BurnRomInfo Sf2m4RomDesc[] = {
	{"u222ne",      0x080000, 0x7133489e, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"u196ne",      0x080000, 0xb07a4f90, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m4) STD_ROM_FN(Sf2m4)

struct BurnDriver BurnDrvCpsSf2m4={
	"sf2m4", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M4, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2m4RomInfo, Sf2m4RomName, Sf2yycInputInfo, sf2jDIPInfo,
	sf2m2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (M5, bootleg)

static struct BurnRomInfo Sf2m5RomDesc[] = {
	{"u222",        0x080000, 0x03991fba, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"u196",        0x080000, 0x39f15a1e, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m5) STD_ROM_FN(Sf2m5)

struct BurnDriver BurnDrvCpsSf2m5={
	"sf2m5", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M5, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2m5RomInfo, Sf2m5RomName, Sf2yycInputInfo, sf2DIPInfo,
	sf2m2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (M6, bootleg)

static struct BurnRomInfo Sf2m6RomDesc[] = {
	{"u222-6b",     0x080000, 0x0a3692be, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"u196-6b",     0x080000, 0x80454da7, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m6) STD_ROM_FN(Sf2m6)

struct BurnDriver BurnDrvCpsSf2m6={
	"sf2m6", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M6, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2m6RomInfo, Sf2m6RomName, Sf2yycInputInfo, sf2DIPInfo,
	sf2m2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (M7, bootleg)

static struct BurnRomInfo Sf2m7RomDesc[] = {
	{"u222-2i",     0x040000, 0x1ca7adbd, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"u196-2i",     0x040000, 0xf758408c, 1 | BRF_ESS | BRF_PRG },
	{"u222-2s",     0x040000, 0x720cea3e, 1 | BRF_ESS | BRF_PRG },
	{"u196-2s",     0x040000, 0x9932832c, 1 | BRF_ESS | BRF_PRG },
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m7) STD_ROM_FN(Sf2m7)

struct BurnDriver BurnDrvCpsSf2m7={
	"sf2m7", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M7, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2m7RomInfo, Sf2m7RomName, Sf2yycInputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (YYC, bootleg)

static struct BurnRomInfo Sf2yycRomDesc[] = {
	{"b12.rom",     0x080000, 0x8f742fd5, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"b14.rom",     0x080000, 0x8831ec7f, 1 | BRF_ESS | BRF_PRG },
	{"b11.rom",     0x020000, 0x94a46525, 1 | BRF_ESS | BRF_PRG },
	{"b13.rom",     0x020000, 0x8fb3dd47, 1 | BRF_ESS | BRF_PRG },
	{"b11.rom",     0x020000, 0x94a46525, 1 | BRF_ESS | BRF_PRG },
	{"b13.rom",     0x020000, 0x8fb3dd47, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2yyc) STD_ROM_FN(Sf2yyc)

static void sf2yyc_patch()
{
	bootleg_progload();

	// Patch invalid opcodes (protection?)
	*((unsigned short*)(CpsRom + 0xe55be)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55ca)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55cc)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55ce)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55d0)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55d2)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55d4)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55d6)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55d8)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55da)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55de)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55e2)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55e4)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55e6)) = 0x4e71;
	*((unsigned short*)(CpsRom + 0xe55e8)) = 0x4e71;
}

static int sf2yycInit()
{
	pCpsInitCallback = sf2yyc_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2yyc={
	"sf2yyc", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (YYC, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2yycRomInfo, Sf2yycRomName, Sf2yycInputInfo, sf2DIPInfo,
	sf2yycInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Xiang Long, Chinese bootleg)

static struct BurnRomInfo KouryuRomDesc[] = {
	{"u222.rom",    0x080000, 0x9236a79a, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"u196.rom",    0x080000, 0xb23a869d, 1 | BRF_ESS | BRF_PRG },
	{"u221.rom",    0x020000, 0x64e6e091, 1 | BRF_ESS | BRF_PRG },
	{"u195.rom",    0x020000, 0xc95e4443, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Kouryu) STD_ROM_FN(Kouryu)

struct BurnDriver BurnDrvCpsKouryu={
	"sf2koryu", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Xiang Long, Chinese bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, KouryuRomInfo, KouryuRomName, Sf2yycInputInfo, sf2DIPInfo,
	sf2m2Init, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Champion Edition (Magic Delta Turbo, bootleg)

static struct BurnRomInfo Sf2mdtRomDesc[] = {
	{"3.ic172", 0x080000, 0x5301b41f, 1 | BRF_ESS | BRF_PRG }, // 0
	{"1.ic171", 0x080000, 0xc1c803f6, 1 | BRF_ESS | BRF_PRG }, // 1
	{"4.ic176", 0x020000, 0x1073b7b6, 1 | BRF_ESS | BRF_PRG }, // 2
	{"2.ic175", 0x020000, 0x924c6ce2, 1 | BRF_ESS | BRF_PRG }, // 3

	{"7.ic90",  0x080000, 0x896eaf48, 2 | BRF_GRA }, // 4
	{"13.ic89", 0x080000, 0x305dd72a, 2 | BRF_GRA },
	{"10.ic88", 0x080000, 0xef3f5be8, 2 | BRF_GRA },
	{"16.ic87", 0x080000, 0xe57f6db9, 2 | BRF_GRA },
	{"6.ic91",  0x080000, 0x054cd5c4, 2 | BRF_GRA }, // 8
	{"12.ic92", 0x080000, 0x87e069e8, 2 | BRF_GRA },
	{"9.ic93",  0x080000, 0x818ca33d, 2 | BRF_GRA },
	{"15.ic94", 0x080000, 0x5dfb44d1, 2 | BRF_GRA },
	{"8.ic86",  0x080000, 0x34bbb3fa, 2 | BRF_GRA }, // 12
	{"14.ic85", 0x080000, 0x7d9f1a67, 2 | BRF_GRA },
	{"11.ic84", 0x080000, 0xcea6d1d6, 2 | BRF_GRA },
	{"17.ic83", 0x080000, 0x91a9a05d, 2 | BRF_GRA },

	{"5.ic26",  0x020000, 0x17d5ba8a, 3 | BRF_ESS | BRF_PRG }, // 16
};

STD_ROM_PICK(Sf2mdt) STD_ROM_FN(Sf2mdt)

struct BurnDriver BurnDrvCpsSf2mdt = {
	"sf2mdt", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Magic Delta Turbo, bootleg)\0","No Sound","bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2mdtRomInfo,Sf2mdtRomName,sf2InputInfo, sf2DIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Varth - Operation Thunderstorm (World 920714)

static struct BurnRomInfo VarthRomDesc[] = {
	{ "vae_30b.11f", 0x020000, 0xadb8d391, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "vae_35b.11h", 0x020000, 0x44e5548f, 1 | BRF_ESS | BRF_PRG },
	{ "vae_31b.12f", 0x020000, 0x1749a71c, 1 | BRF_ESS | BRF_PRG },
	{ "vae_36b.12h", 0x020000, 0x5f2e2450, 1 | BRF_ESS | BRF_PRG },
	{ "vae_28b.9f",  0x020000, 0xe524ca50, 1 | BRF_ESS | BRF_PRG },
	{ "vae_33b.9h",  0x020000, 0xc0bbf8c9, 1 | BRF_ESS | BRF_PRG },
	{ "vae_29b.10f", 0x020000, 0x6640996a, 1 | BRF_ESS | BRF_PRG },
	{ "vae_34b.10h", 0x020000, 0xfa59be8a, 1 | BRF_ESS | BRF_PRG },

	{ "va-5m.7a",    0x080000, 0xb1fb726e, 2 | BRF_GRA },
	{ "va-7m.9a",    0x080000, 0x4c6588cd, 2 | BRF_GRA },
	{ "va-1m.3a",    0x080000, 0x0b1ace37, 2 | BRF_GRA },
	{ "va-3m.5a",    0x080000, 0x44dfe706, 2 | BRF_GRA },

	{ "va_09.12b",   0x010000, 0x7a99446e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "va_18.11c",   0x020000, 0xde30510e, 4 | BRF_SND },
	{ "va_19.12c",   0x020000, 0x0610a4ac, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Varth) STD_ROM_FN(Varth)

struct BurnDriver BurnDrvCpsVarth = {
	"varth", NULL, NULL, "1992",
	"Varth - Operation Thunderstorm (World 920714)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1,
	NULL,VarthRomInfo,VarthRomName,varthInputInfo, varthDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Varth - Operation Thunderstorm (World 920612)

static struct BurnRomInfo varthr1RomDesc[] = {
	{ "vae_30a.11f", 0x020000, 0x7fcd0091, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "vae_35a.11h", 0x020000, 0x35cf9509, 1 | BRF_ESS | BRF_PRG },
	{ "vae_31a.12f", 0x020000, 0x15e5ee81, 1 | BRF_ESS | BRF_PRG },
	{ "vae_36a.12h", 0x020000, 0x153a201e, 1 | BRF_ESS | BRF_PRG },
	{ "vae_28a.9f",  0x020000, 0x7a0e0d25, 1 | BRF_ESS | BRF_PRG },
	{ "vae_33a.9h",  0x020000, 0xf2365922, 1 | BRF_ESS | BRF_PRG },
	{ "vae_29a.10f", 0x020000, 0x5e2cd2c3, 1 | BRF_ESS | BRF_PRG },
	{ "vae_34a.10h", 0x020000, 0x3d9bdf83, 1 | BRF_ESS | BRF_PRG },

	{ "va-5m.7a",    0x080000, 0xb1fb726e, 2 | BRF_GRA },
	{ "va-7m.9a",    0x080000, 0x4c6588cd, 2 | BRF_GRA },
	{ "va-1m.3a",    0x080000, 0x0b1ace37, 2 | BRF_GRA },
	{ "va-3m.5a",    0x080000, 0x44dfe706, 2 | BRF_GRA },

	{ "va_09.12b",   0x010000, 0x7a99446e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "va_18.11c",   0x020000, 0xde30510e, 4 | BRF_SND },
	{ "va_19.12c",   0x020000, 0x0610a4ac, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(varthr1) STD_ROM_FN(varthr1)

struct BurnDriver BurnDrvCpsvarthr1 = {
	"varthr1", "varth", NULL, "1992",
	"Varth - Operation Thunderstorm (World 920612)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1,
	NULL,varthr1RomInfo,varthr1RomName,varthInputInfo, varthDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Varth - Operation Thunderstorm (US 920612)

static struct BurnRomInfo VarthuRomDesc[] = {
	{ "vau23a.bin",  0x080000, 0xfbe68726, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "vau22a.bin",  0x080000, 0x0ed71bbd, 1 | BRF_ESS | BRF_PRG },

	{ "va-5m.bin",   0x080000, 0xb1fb726e, 2 | BRF_GRA },
	{ "va-7m.bin",   0x080000, 0x4c6588cd, 2 | BRF_GRA },
	{ "va-1m.bin",   0x080000, 0x0b1ace37, 2 | BRF_GRA },
	{ "va-3m.bin",   0x080000, 0x44dfe706, 2 | BRF_GRA },

	{ "va_09.bin",   0x010000, 0x7a99446e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "va_18.bin",   0x020000, 0xde30510e, 4 | BRF_SND },
	{ "va_19.bin",   0x020000, 0x0610a4ac, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Varthu) STD_ROM_FN(Varthu)

struct BurnDriver BurnDrvCpsVarthu = {
	"varthu", "varth", NULL, "1992",
	"Varth - Operation Thunderstorm (US 920612)\0", NULL, "Capcom (Romstar license)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_CAPCOM_CPS1,
	NULL,VarthuRomInfo,VarthuRomName,varthInputInfo, varthDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Varth - Operation Thunderstorm (Japan 920714)

static struct BurnRomInfo VarthjRomDesc[] = {
	{ "vaj_36b.12f",  0x020000, 0x1d798d6a, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "vaj_42b.12h",  0x020000, 0x0f720233, 1 | BRF_ESS | BRF_PRG },
	{ "vaj_37b.13f",  0x020000, 0x24414b17, 1 | BRF_ESS | BRF_PRG },
	{ "vaj_43b.13h",  0x020000, 0x34b4b06c, 1 | BRF_ESS | BRF_PRG },
	{ "vaj_34b.10f",  0x020000, 0x87c79aed, 1 | BRF_ESS | BRF_PRG },
	{ "vaj_40b.10h",  0x020000, 0x210b4bd0, 1 | BRF_ESS | BRF_PRG },
	{ "vaj_35b.11f",  0x020000, 0x6b0da69f, 1 | BRF_ESS | BRF_PRG },
	{ "vaj_41b.11h",  0x020000, 0x6542c8a4, 1 | BRF_ESS | BRF_PRG },

	{ "va_09.4b",     0x020000, 0x183dfaa8, 2 | BRF_GRA },
	{ "va_01.4a",     0x020000, 0xc41312b5, 2 | BRF_GRA },
	{ "va_13.9b",     0x020000, 0x45537e69, 2 | BRF_GRA },
	{ "va_05.9a",     0x020000, 0x7065d4e9, 2 | BRF_GRA },
	{ "va_24.5e",     0x020000, 0x57191ccf, 2 | BRF_GRA },
	{ "va_17.5c",     0x020000, 0x054f5a5b, 2 | BRF_GRA },
	{ "va_38.8h",     0x020000, 0xe117a17e, 2 | BRF_GRA },
	{ "va_32.8f",     0x020000, 0x3b4f40b2, 2 | BRF_GRA },
	{ "va_10.5b",     0x020000, 0xd62750cd, 2 | BRF_GRA },
	{ "va_02.5a",     0x020000, 0x11590325, 2 | BRF_GRA },
	{ "va_14.10b",    0x020000, 0xdc2f4783, 2 | BRF_GRA },
	{ "va_06.10a",    0x020000, 0x06e833ac, 2 | BRF_GRA },
	{ "va_25.7e",     0x020000, 0x51d90690, 2 | BRF_GRA },
	{ "va_18.7c",     0x020000, 0xa17817c0, 2 | BRF_GRA },
	{ "va_39.9h",     0x020000, 0xb0b12f51, 2 | BRF_GRA },
	{ "va_33.9f",     0x020000, 0x4b003af7, 2 | BRF_GRA },

	{ "va_23.13c",    0x010000, 0x7a99446e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "va_30.12e",    0x020000, 0xde30510e, 4 | BRF_SND },
	{ "va_31.13e",    0x020000, 0x0610a4ac, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "va22b.1a",     0x000117, 0xbd7cd574, 0 | BRF_OPT },	// b-board PLDs
	{ "lwio.12c",     0x000117, 0xad52b90c, 0 | BRF_OPT },

	{ "ioc1.ic1",     0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Varthj) STD_ROM_FN(Varthj)

struct BurnDriver BurnDrvCpsVarthj = {
	"varthj", "varth", NULL, "1992",
	"Varth - Operation Thunderstorm (Japan 920714)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2 ,HARDWARE_CAPCOM_CPS1,
	NULL,VarthjRomInfo,VarthjRomName,varthInputInfo, varthDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,224,384,3,4
};


// Capcom World 2 (Japan 920611)

static struct BurnRomInfo Cworld2jRomDesc[] = {
	{ "q536.bin", 0x020000, 0x38a08099, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "q542.bin", 0x020000, 0x4d29b3a4, 1 | BRF_ESS | BRF_PRG },
	{ "q537.bin", 0x020000, 0xeb547ebc, 1 | BRF_ESS | BRF_PRG },
	{ "q543.bin", 0x020000, 0x3ef65ea8, 1 | BRF_ESS | BRF_PRG },
	{ "q534.bin", 0x020000, 0x7fcc1317, 1 | BRF_ESS | BRF_PRG },
	{ "q540.bin", 0x020000, 0x7f14b7b4, 1 | BRF_ESS | BRF_PRG },
	{ "q535.bin", 0x020000, 0xabacee26, 1 | BRF_ESS | BRF_PRG },
	{ "q541.bin", 0x020000, 0xd3654067, 1 | BRF_ESS | BRF_PRG },

	{ "q509.bin", 0x020000, 0x48496d80, 2 | BRF_GRA },
	{ "q501.bin", 0x020000, 0xc5453f56, 2 | BRF_GRA },
	{ "q513.bin", 0x020000, 0xc741ac52, 2 | BRF_GRA },
	{ "q505.bin", 0x020000, 0x143e068f, 2 | BRF_GRA },
	{ "q524.bin", 0x020000, 0xb419d139, 2 | BRF_GRA },
	{ "q517.bin", 0x020000, 0xbd3b4d11, 2 | BRF_GRA },
	{ "q538.bin", 0x020000, 0x9c24670c, 2 | BRF_GRA },
	{ "q532.bin", 0x020000, 0x3ef9c7c2, 2 | BRF_GRA },
	{ "q510.bin", 0x020000, 0x119e5e93, 2 | BRF_GRA },
	{ "q502.bin", 0x020000, 0xa2cadcbe, 2 | BRF_GRA },
	{ "q514.bin", 0x020000, 0xa8755f82, 2 | BRF_GRA },
	{ "q506.bin", 0x020000, 0xc92a91fc, 2 | BRF_GRA },
	{ "q525.bin", 0x020000, 0x979237cb, 2 | BRF_GRA },
	{ "q518.bin", 0x020000, 0xc57da03c, 2 | BRF_GRA },
	{ "q539.bin", 0x020000, 0xa5839b25, 2 | BRF_GRA },
	{ "q533.bin", 0x020000, 0x04d03930, 2 | BRF_GRA },

	{ "q523.bin", 0x010000, 0xe14dc524, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "q530.bin", 0x020000, 0xd10c1b68, 4 | BRF_SND },
	{ "q531.bin", 0x020000, 0x7d17e496, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Cworld2j) STD_ROM_FN(Cworld2j)

struct BurnDriver BurnDrvCpsCworld2j = {
	"cworld2j", NULL, NULL, "1992",
	"Capcom World 2 (Japan 920611)\0", NULL, "Capcom", "CPS1",
	L"Capcom World 2 (\u30AF\u30A4\u30BA\uFF15 \u3042\u3069\u3079\u3093\u3061\u3083\u30FC\u304F\u3044\u305A \u304B\u3077\u3053\u3093\u308F\u30FC\u308B\u3069\uFF12 Japan 920611)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,Cworld2jRomInfo,Cworld2jRomName,cworld2jInputInfo, cworld2jDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Warriors of Fate (World 921002)

static struct BurnRomInfo WofRomDesc[] = {
	{ "tk2e_23b.rom", 0x080000, 0x11fb2ed1, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "tk2e_22b.rom", 0x080000, 0x479b3f24, 1 | BRF_ESS | BRF_PRG },

	{ "tk2_gfx1.rom", 0x080000, 0x0d9cb9bf, 2 | BRF_GRA },
	{ "tk2_gfx3.rom", 0x080000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom", 0x080000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom", 0x080000, 0xe349551c, 2 | BRF_GRA },
	{ "tk2_gfx5.rom", 0x080000, 0x291f0f0b, 2 | BRF_GRA },
	{ "tk2_gfx7.rom", 0x080000, 0x3edeb949, 2 | BRF_GRA },
	{ "tk2_gfx6.rom", 0x080000, 0x1abd14d6, 2 | BRF_GRA },
	{ "tk2_gfx8.rom", 0x080000, 0xb27948e3, 2 | BRF_GRA },

	{ "tk2_qa.rom",   0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "tk2_q1.rom",   0x080000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",   0x080000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",   0x080000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",   0x080000, 0x36642e88, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS

	{ "tk263b.1a",    0x000117, 0xc4b0349b, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",     0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "bprg1.11d",    0x000117, 0x31793da7, 0 | BRF_OPT },

	{ "ioc1.ic7",     0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wof) STD_ROM_FN(Wof)

static int wofInit()
{
	pCpsInitCallback = wof_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWof = {
	"wof", NULL, NULL, "1992",
	"Warriors of Fate (World 921002)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofRomInfo,WofRomName,wofInputInfo, wofDIPInfo,
	wofInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II (Asia 921005)

static struct BurnRomInfo WofaRomDesc[] = {
	{ "tk2a_23b.rom", 0x080000, 0x2e024628, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "tk2a_22b.rom", 0x080000, 0x900ad4cd, 1 | BRF_ESS | BRF_PRG },

	{ "tk2_gfx1.rom", 0x080000, 0x0d9cb9bf, 2 | BRF_GRA },
	{ "tk2_gfx3.rom", 0x080000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom", 0x080000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom", 0x080000, 0xe349551c, 2 | BRF_GRA },
	{ "tk2_gfx5.rom", 0x080000, 0x291f0f0b, 2 | BRF_GRA },
	{ "tk2_gfx7.rom", 0x080000, 0x3edeb949, 2 | BRF_GRA },
	{ "tk2_gfx6.rom", 0x080000, 0x1abd14d6, 2 | BRF_GRA },
	{ "tk2_gfx8.rom", 0x080000, 0xb27948e3, 2 | BRF_GRA },

	{ "tk2_qa.rom",   0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "tk2_q1.rom",   0x080000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",   0x080000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",   0x080000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",   0x080000, 0x36642e88, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS

	{ "tk263b.1a",    0x000117, 0xc4b0349b, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",     0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "bprg1.11d",    0x000117, 0x31793da7, 0 | BRF_OPT },

	{ "ioc1.ic7",     0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wofa) STD_ROM_FN(Wofa)

struct BurnDriver BurnDrvCpsWofa = {
	"wofa", "wof", NULL, "1992",
	"Sangokushi II (Asia 921005)\0", NULL, "Capcom", "CPS1 / QSound",
	L"\u4E09\u56FD\u5FD7II (Asia 921005)\0Sangokushi II\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofaRomInfo,WofaRomName,wofInputInfo, wofDIPInfo,
	wofInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Warriors of Fate (US 921031)

static struct BurnRomInfo WofuRomDesc[] = {
	{ "tk2u.23c",     0x080000, 0x29b89c12, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "tk2u.22c",     0x080000, 0xf5af4774, 1 | BRF_ESS | BRF_PRG },

	{ "tk2_gfx1.rom", 0x080000, 0x0d9cb9bf, 2 | BRF_GRA },
	{ "tk2_gfx3.rom", 0x080000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom", 0x080000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom", 0x080000, 0xe349551c, 2 | BRF_GRA },
	{ "tk2_gfx5.rom", 0x080000, 0x291f0f0b, 2 | BRF_GRA },
	{ "tk2_gfx7.rom", 0x080000, 0x3edeb949, 2 | BRF_GRA },
	{ "tk2_gfx6.rom", 0x080000, 0x1abd14d6, 2 | BRF_GRA },
	{ "tk2_gfx8.rom", 0x080000, 0xb27948e3, 2 | BRF_GRA },

	{ "tk2_qa.rom",   0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "tk2_q1.rom",   0x080000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",   0x080000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",   0x080000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",   0x080000, 0x36642e88, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS

	{ "tk263b.1a",    0x000117, 0xc4b0349b, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",     0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "bprg1.11d",    0x000117, 0x31793da7, 0 | BRF_OPT },

	{ "ioc1.ic7",     0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wofu) STD_ROM_FN(Wofu)

struct BurnDriver BurnDrvCpsWofu = {
	"wofu", "wof", NULL, "1992",
	"Warriors of Fate (US 921031)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofuRomInfo,WofuRomName,wofInputInfo, wofDIPInfo,
	wofInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Tenchi wo Kurau II - Sekiheki no Tatakai (Japan 921031)

static struct BurnRomInfo WofjRomDesc[] = {
	{ "tk2j23c.bin",  0x080000, 0x9b215a68, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "tk2j22c.bin",  0x080000, 0xb74b09ac, 1 | BRF_ESS | BRF_PRG },

	{ "tk2_gfx1.rom", 0x080000, 0x0d9cb9bf, 2 | BRF_GRA },
	{ "tk2_gfx3.rom", 0x080000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom", 0x080000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom", 0x080000, 0xe349551c, 2 | BRF_GRA },
	{ "tk205.bin",    0x080000, 0xe4a44d53, 2 | BRF_GRA },
	{ "tk206.bin",    0x080000, 0x58066ba8, 2 | BRF_GRA },
	{ "tk207.bin",    0x080000, 0xd706568e, 2 | BRF_GRA },
	{ "tk208.bin",    0x080000, 0xd4a19a02, 2 | BRF_GRA },

	{ "tk2_qa.rom",   0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "tk2_q1.rom",   0x080000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",   0x080000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",   0x080000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",   0x080000, 0x36642e88, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS

	{ "tk263b.1a",    0x000117, 0xc4b0349b, 0 | BRF_OPT },	// b-board PLDs
	{ "iob1.12d",     0x000117, 0x3abc0700, 0 | BRF_OPT },
	{ "bprg1.11d",    0x000117, 0x31793da7, 0 | BRF_OPT },

	{ "ioc1.ic7",     0x000117, 0x0d182081, 0 | BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Wofj) STD_ROM_FN(Wofj)

struct BurnDriver BurnDrvCpsWofj = {
	"wofj", "wof", NULL, "1992",
	"Tenchi wo Kurau II - Sekiheki no Tatakai (Japan 921031)\0", NULL, "Capcom", "CPS1 / QSound",
	L"\u8EE2\u5730\u3092\u55B0\u3089\u3046II - \u8D64\u58C1\u306E\u6226\u3044 (Japan 921031)\0Tenchi wo Kurau II - Sekiheki no Tatakai\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofjRomInfo,WofjRomName,wofInputInfo, wofDIPInfo,
	wofInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Hyper Fighting (World 921209)

static struct BurnRomInfo Sf2hfRomDesc[] = {
	{"s92e_23a.bin", 0x080000, 0x2dd72514, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2_22.bin",   0x080000, 0xaea6e035, 1 | BRF_ESS | BRF_PRG },
	{"sf2_21.bin",   0x080000, 0xfd200288, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",   0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",   0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",   0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",   0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",   0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",   0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",   0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",   0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s2t_10.bin",   0x080000, 0x3c042686, 2 | BRF_GRA },
	{"s2t_11.bin",   0x080000, 0x8b7e7183, 2 | BRF_GRA },
	{"s2t_12.bin",   0x080000, 0x293c888c, 2 | BRF_GRA },
	{"s2t_13.bin",   0x080000, 0x842b35a4, 2 | BRF_GRA },

	{"s92_09.bin",   0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2hf) STD_ROM_FN(Sf2hf)

struct BurnDriver BurnDrvCpsSf2hf={
	"sf2hf", "sf2ce", NULL, "1992",
	"Street Fighter II' - Hyper Fighting (World 921209)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2hfRomInfo, Sf2hfRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' - Hyper Fighting (US 921209)

static struct BurnRomInfo Sf2tRomDesc[] = {
	{"sf2.23" ,    0x080000, 0x89a1fc38, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"sf2_22.bin", 0x080000, 0xaea6e035, 1 | BRF_ESS | BRF_PRG },
	{"sf2_21.bin", 0x080000, 0xfd200288, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin", 0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin", 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin", 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin", 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin", 0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin", 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin", 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin", 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s2t_10.bin", 0x080000, 0x3c042686, 2 | BRF_GRA },
	{"s2t_11.bin", 0x080000, 0x8b7e7183, 2 | BRF_GRA },
	{"s2t_12.bin", 0x080000, 0x293c888c, 2 | BRF_GRA },
	{"s2t_13.bin", 0x080000, 0x842b35a4, 2 | BRF_GRA },

	{"s92_09.bin", 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin", 0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin", 0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2t) STD_ROM_FN(Sf2t)

struct BurnDriver BurnDrvCpsSf2t={
	"sf2t", "sf2hf", NULL, "1992",
	"Street Fighter II' - Hyper Fighting (US 921209)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2tRomInfo, Sf2tRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Street Fighter II' Turbo - Hyper Fighting (Japan 921209)

static struct BurnRomInfo Sf2tjRomDesc[] = {
	{"s2tj_23.bin", 0x080000, 0xea73b4dc, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{"s2t_22.bin",  0x080000, 0xaea6e035, 1 | BRF_ESS | BRF_PRG },
	{"s2t_21.bin",  0x080000, 0xfd200288, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s2t_10.bin",  0x080000, 0x3c042686, 2 | BRF_GRA },
	{"s2t_11.bin",  0x080000, 0x8b7e7183, 2 | BRF_GRA },
	{"s2t_12.bin",  0x080000, 0x293c888c, 2 | BRF_GRA },
	{"s2t_13.bin",  0x080000, 0x842b35a4, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2tj) STD_ROM_FN(Sf2tj)

struct BurnDriver BurnDrvCpsSf2tj={
	"sf2tj", "sf2hf", NULL, "1992",
	"Street Fighter II' Turbo - Hyper Fighting (Japan 921209)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2tjRomInfo, Sf2tjRomName, sf2InputInfo, sf2jDIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 4, 3
};


// Cadillacs and Dinosaurs (World 930201)

static struct BurnRomInfo DinoRomDesc[] = {
	{ "cde_23a.rom",  0x080000, 0x8f4e585e, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cde_22a.rom",  0x080000, 0x9278aa12, 1 | BRF_ESS | BRF_PRG },
	{ "cde_21a.rom",  0x080000, 0x66d23de2, 1 | BRF_ESS | BRF_PRG },

	{ "cd_gfx01.rom", 0x080000, 0x8da4f917, 2 | BRF_GRA },
	{ "cd_gfx03.rom", 0x080000, 0x6c40f603, 2 | BRF_GRA },
	{ "cd_gfx02.rom", 0x080000, 0x09c8fc2d, 2 | BRF_GRA },
	{ "cd_gfx04.rom", 0x080000, 0x637ff38f, 2 | BRF_GRA },
	{ "cd_gfx05.rom", 0x080000, 0x470befee, 2 | BRF_GRA },
	{ "cd_gfx07.rom", 0x080000, 0x22bfb7a3, 2 | BRF_GRA },
	{ "cd_gfx06.rom", 0x080000, 0xe7599ac4, 2 | BRF_GRA },
	{ "cd_gfx08.rom", 0x080000, 0x211b4b15, 2 | BRF_GRA },

	{ "cd_q.rom",     0x020000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "cd_q1.rom",    0x080000, 0x60927775, 5 | BRF_SND },
	{ "cd_q2.rom",    0x080000, 0x770f4c47, 5 | BRF_SND },
	{ "cd_q3.rom",    0x080000, 0x2f273ffc, 5 | BRF_SND },
	{ "cd_q4.rom",    0x080000, 0x2c67821d, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Dino) STD_ROM_FN(Dino)

static int dinoInit()
{
	pCpsInitCallback = dino_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsDino = {
	"dino", NULL, NULL, "1993",
	"Cadillacs and Dinosaurs (World 930201)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinoRomInfo,DinoRomName,dinoInputInfo, dinoDIPInfo,
	dinoInit, CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs and Dinosaurs (US 930201)

static struct BurnRomInfo DinouRomDesc[] = {
	{ "cdu.23a",      0x080000, 0x7c2543cd, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cdu.22a",      0x080000, 0xfab740a9, 1 | BRF_ESS | BRF_PRG },
	{ "cde_21a.rom",  0x080000, 0x66d23de2, 1 | BRF_ESS | BRF_PRG },

	{ "cd_gfx01.rom", 0x080000, 0x8da4f917, 2 | BRF_GRA },
	{ "cd_gfx03.rom", 0x080000, 0x6c40f603, 2 | BRF_GRA },
	{ "cd_gfx02.rom", 0x080000, 0x09c8fc2d, 2 | BRF_GRA },
	{ "cd_gfx04.rom", 0x080000, 0x637ff38f, 2 | BRF_GRA },
	{ "cd_gfx05.rom", 0x080000, 0x470befee, 2 | BRF_GRA },
	{ "cd_gfx07.rom", 0x080000, 0x22bfb7a3, 2 | BRF_GRA },
	{ "cd_gfx06.rom", 0x080000, 0xe7599ac4, 2 | BRF_GRA },
	{ "cd_gfx08.rom", 0x080000, 0x211b4b15, 2 | BRF_GRA },

	{ "cd_q.rom",     0x020000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "cd_q1.rom",    0x080000, 0x60927775, 5 | BRF_SND },
	{ "cd_q2.rom",    0x080000, 0x770f4c47, 5 | BRF_SND },
	{ "cd_q3.rom",    0x080000, 0x2f273ffc, 5 | BRF_SND },
	{ "cd_q4.rom",    0x080000, 0x2c67821d, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Dinou) STD_ROM_FN(Dinou)

struct BurnDriver BurnDrvCpsDinou = {
	"dinou", "dino", NULL, "1993",
	"Cadillacs and Dinosaurs (US 930201)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinouRomInfo,DinouRomName,dinoInputInfo, dinoDIPInfo,
	dinoInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs Kyouryuu-Shinseiki (Japan 930201)

static struct BurnRomInfo DinojRomDesc[] = {
	{ "cdj-23a.8f",   0x080000, 0x5f3ece96, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "cdj-22a.7f",   0x080000, 0xa0d8de29, 1 | BRF_ESS | BRF_PRG },
	{ "cde_21a.rom",  0x080000, 0x66d23de2, 1 | BRF_ESS | BRF_PRG },

	{ "cd_gfx01.rom", 0x080000, 0x8da4f917, 2 | BRF_GRA },
	{ "cd_gfx03.rom", 0x080000, 0x6c40f603, 2 | BRF_GRA },
	{ "cd_gfx02.rom", 0x080000, 0x09c8fc2d, 2 | BRF_GRA },
	{ "cd_gfx04.rom", 0x080000, 0x637ff38f, 2 | BRF_GRA },
	{ "cd_gfx05.rom", 0x080000, 0x470befee, 2 | BRF_GRA },
	{ "cd_gfx07.rom", 0x080000, 0x22bfb7a3, 2 | BRF_GRA },
	{ "cd_gfx06.rom", 0x080000, 0xe7599ac4, 2 | BRF_GRA },
	{ "cd_gfx08.rom", 0x080000, 0x211b4b15, 2 | BRF_GRA },

	{ "cd_q.rom",     0x020000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "cd_q1.rom",    0x080000, 0x60927775, 5 | BRF_SND },
	{ "cd_q2.rom",    0x080000, 0x770f4c47, 5 | BRF_SND },
	{ "cd_q3.rom",    0x080000, 0x2f273ffc, 5 | BRF_SND },
	{ "cd_q4.rom",    0x080000, 0x2c67821d, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Dinoj) STD_ROM_FN(Dinoj)

struct BurnDriver BurnDrvCpsDinoj = {
	"dinoj", "dino", NULL, "1993",
	"Cadillacs Kyouryuu-Shinseiki (Japan 930201)\0", NULL, "Capcom", "CPS1, QSound",
	L"Cadillacs \u6050\u7ADC\u65B0\u4E16\u7D00 (Japan 930201)\0Cadillacs Kyouryuu-Shinseiki\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinojRomInfo,DinojRomName,dinoInputInfo, dinoDIPInfo,
	dinoInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs and Dinosaurs (bootleg with PIC16c57, set 1)

static struct BurnRomInfo DinopicRomDesc[] = {
	{ "5.bin",     0x80000, 0x96dfcbf1, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "3.bin",     0x80000, 0x13dfeb08, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "7.bin",     0x80000, 0x6133f349, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "2.bin",     0x80000, 0x0e4058ba, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "4.bin",     0x80000, 0xf3c2c98d, 2 | BRF_GRA },			 // 4
	{ "8.bin",     0x80000, 0xd574befc, 2 | BRF_GRA },
	{ "9.bin",     0x80000, 0x55ef0adc, 2 | BRF_GRA },
	{ "6.bin",     0x80000, 0xcc0805fc, 2 | BRF_GRA },
	{ "13.bin",    0x80000, 0x1371f714, 2 | BRF_GRA },			 // 8
	{ "12.bin",    0x80000, 0xb284c4a7, 2 | BRF_GRA },
	{ "11.bin",    0x80000, 0xb7ad3394, 2 | BRF_GRA },
	{ "10.bin",    0x80000, 0x88847705, 2 | BRF_GRA },

	{ "cd_q.rom",  0x20000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // 12
	{ "pic16c57-rp",0x2d4c, 0x5a6d393c, 0 | BRF_PRG | BRF_OPT }, // 12

	{ "cd_q1.rom", 0x80000, 0x60927775, 5 | BRF_SND },			 // 13
	{ "cd_q2.rom", 0x80000, 0x770f4c47, 5 | BRF_SND },			 // 14
	{ "cd_q3.rom", 0x80000, 0x2f273ffc, 5 | BRF_SND },			 // 15
	{ "cd_q4.rom", 0x80000, 0x2c67821d, 5 | BRF_SND },			 // 16
	{ "1.bin",     0x80000, 0x7d921309, 5 | BRF_SND },			 // 13
};

STD_ROM_PICK(Dinopic) STD_ROM_FN(Dinopic)

static void dinohb_patch()
{
	// Fix draw scroll
	*((unsigned short*)(CpsRom + 0x6C2)) = 0xFFC0;

	// Fix gfx
	*((unsigned short*)(CpsRom + 0x472)) = 0x33FC;
	*((unsigned short*)(CpsRom + 0x474)) = 0x9000;
	*((unsigned short*)(CpsRom + 0x476)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x478)) = 0x0100;
	*((unsigned short*)(CpsRom + 0x47A)) = 0x33FC;
	*((unsigned short*)(CpsRom + 0x47C)) = 0x9080;
	*((unsigned short*)(CpsRom + 0x47E)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x480)) = 0x0102;
	*((unsigned short*)(CpsRom + 0x482)) = 0x33FC;
	*((unsigned short*)(CpsRom + 0x484)) = 0x90C0;
	*((unsigned short*)(CpsRom + 0x486)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x488)) = 0x0104;
	*((unsigned short*)(CpsRom + 0x48A)) = 0x33FC;
	*((unsigned short*)(CpsRom + 0x48C)) = 0x9100;
	*((unsigned short*)(CpsRom + 0x48E)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x490)) = 0x0106;
	*((unsigned short*)(CpsRom + 0x6CC)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x6CE)) = 0x010C;
	*((unsigned short*)(CpsRom + 0x6DE)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x6E0)) = 0x0110;
	*((unsigned short*)(CpsRom + 0x6F0)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x6F2)) = 0x0114;
	*((unsigned short*)(CpsRom + 0x704)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x706)) = 0x010E;
	*((unsigned short*)(CpsRom + 0x718)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x71A)) = 0x0112;
	*((unsigned short*)(CpsRom + 0x72C)) = 0x0080;
	*((unsigned short*)(CpsRom + 0x72E)) = 0x0116;

	// Fix screen transitions
	*((unsigned short*)(CpsRom + 0xB28)) = 0x7000;
	*((unsigned short*)(CpsRom + 0xB2A)) = 0x7200;
	*((unsigned short*)(CpsRom + 0xB2C)) = 0x343C;
	*((unsigned short*)(CpsRom + 0xB32)) = 0x20C1;

	// Fix sound
	*((unsigned short*)(CpsRom + 0x666)) = 0x00F1;
	*((unsigned short*)(CpsRom + 0x668)) = 0x8002;
	*((unsigned short*)(CpsRom + 0xAAA6C)) = 0x00D8;
}

/*
static void __fastcall DinopicScrollWrite(unsigned int a, unsigned short d)
{
	if (a == 0x980000) {
		CpsLayer1YOffs = d;
		return;
	}

	if (a == 0x980002) {
		CpsLayer1XOffs = d - 0x40;
		return;
	}

	if (a == 0x980004) {
		CpsLayer2YOffs = d;
		return;
	}

	if (a == 0x980006) {
		CpsLayer2XOffs = d - 0x40;
		return;
	}

	if (a == 0x980008) {
		CpsLayer3YOffs = d;
		return;
	}

	if (a == 0x98000a) {
		CpsLayer3XOffs = d - 0x40;
		return;
	}
}
*/

static void dinopic_decode()
{
	bootleg_progload2();
	dinohb_patch();
	dino_decode();
}

static int dinopicInit()
{
	int nRet = 0;

//	Cps1pic = 1;
//	Dinopic = 1;

	pCpsInitCallback = dinopic_decode;
	nRet = CpsInit();
/*
	BootlegSpriteRam = (unsigned char*)malloc(0x2000);

	SekOpen(0);
	SekMapMemory(BootlegSpriteRam, 0x990000, 0x991FFF, SM_RAM);
	SekMapHandler(1, 0x980000, 0x98000b, SM_WRITE);
	SekSetWriteWordHandler(1, DinopicScrollWrite);
	SekClose();
*/
	return nRet;
}

static int dinopicExit()
{
//	Cps1pic = 0;
//	Dinopic = 0;

	return CpsExit();
}

struct BurnDriver BurnDrvCpsDinopic = {
	"dinopic", "dino", NULL, "1993",
	"Cadillacs and Dinosaurs (bootleg with PIC16c57, set 1)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinopicRomInfo,DinopicRomName,dinoInputInfo, dinoDIPInfo,
	dinopicInit,dinopicExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs and Dinosaurs (bootleg with PIC16c57, set 2)

static struct BurnRomInfo Dinopic2RomDesc[] = {
	{ "27c4000-m12481.bin",       0x80000, 0x96dfcbf1, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "27c4000-m12374r-2.bin",    0x80000, 0x13dfeb08, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "27c4000-m12374r-3.bin",    0x80000, 0x6133f349, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "27c4000-m12374r-1.bin",    0x80000, 0x0e4058ba, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "27c4000-m12481-4.bin",     0x80000, 0xf3c2c98d, 2 | BRF_GRA },			 // 4
	{ "27c4000-m12481-3.bin",     0x80000, 0x417a4816, 2 | BRF_GRA },
	{ "27c4000-m12481-2.bin",     0x80000, 0x55ef0adc, 2 | BRF_GRA },
	{ "27c4000-m12481-1.bin",     0x80000, 0xcc0805fc, 2 | BRF_GRA },
	{ "27c4000-m12481-8.bin",     0x80000, 0x1371f714, 2 | BRF_GRA },			 // 8
	{ "27c4000-m12481-7.bin",     0x80000, 0xb284c4a7, 2 | BRF_GRA },
	{ "27c4000-m12481-6.bin",     0x80000, 0xb7ad3394, 2 | BRF_GRA },
	{ "27c4000-m12481-5.bin",     0x80000, 0x88847705, 2 | BRF_GRA },

	{ "cd_q.rom",                 0x20000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // 12
	{ "pic16c57-xt.hex",          0x26cc,  0xa6a5eac4, 0 | BRF_PRG | BRF_OPT }, // 12

	{ "cd_q1.rom",                0x80000, 0x60927775, 5 | BRF_SND },			 // 13
	{ "cd_q2.rom",                0x80000, 0x770f4c47, 5 | BRF_SND },			 // 14
	{ "cd_q3.rom",                0x80000, 0x2f273ffc, 5 | BRF_SND },			 // 15
	{ "cd_q4.rom",                0x80000, 0x2c67821d, 5 | BRF_SND },			 // 16
	{ "27c4000-m12623.bin",       0x80000, 0x7d921309, 5 | BRF_SND },			 // 13

	{ "cat93c46p.bin",            0x080,   0xd49fa351, 0 | BRF_OPT },			 // 14
	{ "gal20v8a-1.bin",           0x157,   0xcd99ca47, 0 | BRF_OPT },			 // 15
	{ "gal20v8a-2.bin",           0x157,   0x60d016b9, 0 | BRF_OPT },			 // 16
	{ "gal20v8a-3.bin",           0x157,   0x049b7f4f, 0 | BRF_OPT },			 // 17
	{ "palce16v8h-1.bin",         0x117,   0x48253c66, 0 | BRF_OPT },			 // 18
	{ "palce16v8h-2.bin",         0x117,   0x9ae375ba, 0 | BRF_OPT },			 // 19
};

STD_ROM_PICK(Dinopic2) STD_ROM_FN(Dinopic2)

struct BurnDriver BurnDrvCpsDinopic2 = {
	"dinopic2", "dino", NULL, "1993",
	"Cadillacs and Dinosaurs (bootleg with PIC16c57, set 2)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,Dinopic2RomInfo,Dinopic2RomName,dinoInputInfo, dinoDIPInfo,
	dinopicInit,dinopicExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The Punisher (World 930422)

static struct BurnRomInfo PunisherRomDesc[] = {
	{ "pse_26.rom",  0x020000, 0x389a99d2, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "pse_30.rom",  0x020000, 0x68fb06ac, 1 | BRF_ESS | BRF_PRG },
	{ "pse_27.rom",  0x020000, 0x3eb181c3, 1 | BRF_ESS | BRF_PRG },
	{ "pse_31.rom",  0x020000, 0x37108e7b, 1 | BRF_ESS | BRF_PRG },
	{ "pse_24.rom",  0x020000, 0x0f434414, 1 | BRF_ESS | BRF_PRG },
	{ "pse_28.rom",  0x020000, 0xb732345d, 1 | BRF_ESS | BRF_PRG },
	{ "pse_25.rom",  0x020000, 0xb77102e2, 1 | BRF_ESS | BRF_PRG },
	{ "pse_29.rom",  0x020000, 0xec037bce, 1 | BRF_ESS | BRF_PRG },
	{ "ps_21.rom",   0x080000, 0x8affa5a9, 1 | BRF_ESS | BRF_PRG },

	{ "ps_gfx1.rom", 0x080000, 0x77b7ccab, 2 | BRF_GRA },
	{ "ps_gfx3.rom", 0x080000, 0x0122720b, 2 | BRF_GRA },
	{ "ps_gfx2.rom", 0x080000, 0x64fa58d4, 2 | BRF_GRA },
	{ "ps_gfx4.rom", 0x080000, 0x60da42c8, 2 | BRF_GRA },
	{ "ps_gfx5.rom", 0x080000, 0xc54ea839, 2 | BRF_GRA },
	{ "ps_gfx7.rom", 0x080000, 0x04c5acbd, 2 | BRF_GRA },
	{ "ps_gfx6.rom", 0x080000, 0xa544f4cc, 2 | BRF_GRA },
	{ "ps_gfx8.rom", 0x080000, 0x8f02f436, 2 | BRF_GRA },

	{ "ps_q.rom",    0x020000, 0x49ff4446, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ps_q1.rom",   0x080000, 0x31fd8726, 5 | BRF_SND },
	{ "ps_q2.rom",   0x080000, 0x980a9eef, 5 | BRF_SND },
	{ "ps_q3.rom",   0x080000, 0x0dd44491, 5 | BRF_SND },
	{ "ps_q4.rom",   0x080000, 0xbed42f03, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Punisher) STD_ROM_FN(Punisher)

static int PunisherInit()
{
	pCpsInitCallback = punisher_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsPunisher = {
	"punisher", NULL, NULL, "1993",
	"The Punisher (World 930422)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,PunisherRomInfo,PunisherRomName,punisherInputInfo, punishDIPInfo,
	PunisherInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The Punisher (US 930422)

static struct BurnRomInfo PunisheruRomDesc[] = {
	{ "psu26.rom",   0x020000, 0x9236d121, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "psu30.rom",   0x020000, 0x8320e501, 1 | BRF_ESS | BRF_PRG },
	{ "psu27.rom",   0x020000, 0x61c960a1, 1 | BRF_ESS | BRF_PRG },
	{ "psu31.rom",   0x020000, 0x78d4c298, 1 | BRF_ESS | BRF_PRG },
	{ "psu24.rom",   0x020000, 0x1cfecad7, 1 | BRF_ESS | BRF_PRG },
	{ "psu28.rom",   0x020000, 0xbdf921c1, 1 | BRF_ESS | BRF_PRG },
	{ "psu25.rom",   0x020000, 0xc51acc94, 1 | BRF_ESS | BRF_PRG },
	{ "psu29.rom",   0x020000, 0x52dce1ca, 1 | BRF_ESS | BRF_PRG },
	{ "ps_21.rom",   0x080000, 0x8affa5a9, 1 | BRF_ESS | BRF_PRG },

	{ "ps_gfx1.rom", 0x080000, 0x77b7ccab, 2 | BRF_GRA },
	{ "ps_gfx3.rom", 0x080000, 0x0122720b, 2 | BRF_GRA },
	{ "ps_gfx2.rom", 0x080000, 0x64fa58d4, 2 | BRF_GRA },
	{ "ps_gfx4.rom", 0x080000, 0x60da42c8, 2 | BRF_GRA },
	{ "ps_gfx5.rom", 0x080000, 0xc54ea839, 2 | BRF_GRA },
	{ "ps_gfx7.rom", 0x080000, 0x04c5acbd, 2 | BRF_GRA },
	{ "ps_gfx6.rom", 0x080000, 0xa544f4cc, 2 | BRF_GRA },
	{ "ps_gfx8.rom", 0x080000, 0x8f02f436, 2 | BRF_GRA },

	{ "ps_q.rom",    0x020000, 0x49ff4446, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ps_q1.rom",   0x080000, 0x31fd8726, 5 | BRF_SND },
	{ "ps_q2.rom",   0x080000, 0x980a9eef, 5 | BRF_SND },
	{ "ps_q3.rom",   0x080000, 0x0dd44491, 5 | BRF_SND },
	{ "ps_q4.rom",   0x080000, 0xbed42f03, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Punisheru) STD_ROM_FN(Punisheru)

struct BurnDriver BurnDrvCpsPunisheru = {
	"punisheru", "punisher", NULL, "1993",
	"The Punisher (US 930422)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,PunisheruRomInfo,PunisheruRomName,punisherInputInfo, punishDIPInfo,
	PunisherInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The Punisher (Japan 930422)

static struct BurnRomInfo PunisherjRomDesc[] = {
	{ "psj23.bin",   0x080000, 0x6b2fda52, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "psj22.bin",   0x080000, 0xe01036bc, 1 | BRF_ESS | BRF_PRG },
	{ "ps_21.rom",   0x080000, 0x8affa5a9, 1 | BRF_ESS | BRF_PRG },

	{ "ps_gfx1.rom", 0x080000, 0x77b7ccab, 2 | BRF_GRA },
	{ "ps_gfx3.rom", 0x080000, 0x0122720b, 2 | BRF_GRA },
	{ "ps_gfx2.rom", 0x080000, 0x64fa58d4, 2 | BRF_GRA },
	{ "ps_gfx4.rom", 0x080000, 0x60da42c8, 2 | BRF_GRA },
	{ "ps_gfx5.rom", 0x080000, 0xc54ea839, 2 | BRF_GRA },
	{ "ps_gfx7.rom", 0x080000, 0x04c5acbd, 2 | BRF_GRA },
	{ "ps_gfx6.rom", 0x080000, 0xa544f4cc, 2 | BRF_GRA },
	{ "ps_gfx8.rom", 0x080000, 0x8f02f436, 2 | BRF_GRA },

	{ "ps_q.rom",    0x020000, 0x49ff4446, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ps_q1.rom",   0x080000, 0x31fd8726, 5 | BRF_SND },
	{ "ps_q2.rom",   0x080000, 0x980a9eef, 5 | BRF_SND },
	{ "ps_q3.rom",   0x080000, 0x0dd44491, 5 | BRF_SND },
	{ "ps_q4.rom",   0x080000, 0xbed42f03, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Punisherj) STD_ROM_FN(Punisherj)

struct BurnDriver BurnDrvCpsPunisherj = {
	"punisherj", "punisher", NULL, "1993",
	"The Punisher (Japan 930422)\0", NULL, "Capcom", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,PunisherjRomInfo,PunisherjRomName,punisherInputInfo, punishDIPInfo,
	PunisherInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The Punisher (bootleg with PIC16c57, set 1)

static struct BurnRomInfo PunipicRomDesc[] = {
	{ "cpu5.bin",     0x80000,  0xc3151563, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "cpu3.bin",     0x80000,  0x8c2593ac, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "cpu4.bin",     0x80000,  0x665a5485, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "cpu2.bin",     0x80000,  0xd7b13f39, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "gfx9.bin",     0x80000,  0x9b9a887a, 2 | BRF_GRA },			 // 4
	{ "gfx8.bin",     0x80000,  0x2b94287a, 2 | BRF_GRA },
	{ "gfx7.bin",     0x80000,  0xe9bd74f5, 2 | BRF_GRA },
	{ "gfx6.bin",     0x80000,  0xa5e1c8a4, 2 | BRF_GRA },
	{ "gfx13.bin",    0x80000,  0x6d75a193, 2 | BRF_GRA },			 // 8
	{ "gfx12.bin",    0x80000,  0xa3c205c1, 2 | BRF_GRA },
	{ "gfx11.bin",    0x80000,  0x22f2ec92, 2 | BRF_GRA },
	{ "gfx10.bin",    0x80000,  0x763974c9, 2 | BRF_GRA },

	{ "ps_q.rom",    0x020000, 0x49ff4446, 3 | BRF_ESS | BRF_PRG }, // z80 program
	{ "pic16c57",    0x4000,   0x00000000, 0 | BRF_PRG | BRF_OPT }, // 12

	{ "ps_q1.rom",   0x080000, 0x31fd8726, 5 | BRF_SND },
	{ "ps_q2.rom",   0x080000, 0x980a9eef, 5 | BRF_SND },
	{ "ps_q3.rom",   0x080000, 0x0dd44491, 5 | BRF_SND },
	{ "ps_q4.rom",   0x080000, 0xbed42f03, 5 | BRF_SND },
	{ "sound.bin",   0x80000,  0xaeec9dc6, 5 | BRF_SND }, // 13
};

STD_ROM_PICK(Punipic) STD_ROM_FN(Punipic)

static void punipic_decode()
{
	bootleg_progload2();

	CpsRom[0xAB3] = 0x33;
	CpsRom[0xAB4] = 0x30;
	CpsRom[0xAB6] = 0x80;
	CpsRom[0xAB7] = 0x0;
	CpsRom[0xAB8] = 0x56;
	CpsRom[0xAB9] = 0x1;
	CpsRom[0xABA] = 0xED;
	CpsRom[0xABB] = 0x33;
	CpsRom[0xABC] = 0x32;
	CpsRom[0xABD] = 0xDB;
	CpsRom[0xABE] = 0x80;
	CpsRom[0xABF] = 0x0;
	CpsRom[0xAC0] = 0x48;
	CpsRom[0xAC1] = 0x1;
	CpsRom[0xAC2] = 0xED;
	CpsRom[0xAC3] = 0x33;
	CpsRom[0xAC4] = 0x34;
	CpsRom[0xAC5] = 0xDB;
	CpsRom[0xAC6] = 0x80;
	CpsRom[0xAC7] = 0x0;
	CpsRom[0xAC8] = 0x4A;
	CpsRom[0xAC9] = 0x1;
	CpsRom[0xACA] = 0xED;
	CpsRom[0xACB] = 0x33;
	CpsRom[0xACC] = 0x2E;
	CpsRom[0xACD] = 0xDB;
	CpsRom[0xACE] = 0x80;
	CpsRom[0xAD0] = 0x52;
	CpsRom[0xAD1] = 0x1;
	CpsRom[0xAD2] = 0xED;
	CpsRom[0xAD3] = 0x33;
	CpsRom[0xAD4] = 0x88;
	CpsRom[0xAD5] = 0xDB;
	CpsRom[0xAD6] = 0x80;
	CpsRom[0xAD7] = 0x0;
	CpsRom[0xAD8] = 0x0;
	CpsRom[0xAD9] = 0x1;
	CpsRom[0xADA] = 0xED;
	CpsRom[0xADB] = 0x33;
	CpsRom[0xADC] = 0x8A;
	CpsRom[0xADD] = 0xDB;
	CpsRom[0xADE] = 0x80;
	CpsRom[0xADF] = 0x0;
	CpsRom[0xAE0] = 0x2;
	CpsRom[0xAE1] = 0x1;
	CpsRom[0xAE2] = 0xED;
	CpsRom[0xAE3] = 0x33;
	CpsRom[0xAE4] = 0x8C;
	CpsRom[0xAE5] = 0xDB;
	CpsRom[0xAE6] = 0x80;
	CpsRom[0xAE7] = 0x0;
	CpsRom[0xAE8] = 0x4;
	CpsRom[0xAE9] = 0x1;
	CpsRom[0xAEA] = 0xED;
	CpsRom[0xAEB] = 0x33;
	CpsRom[0xAEC] = 0x8E;
	CpsRom[0xAED] = 0xDB;
	CpsRom[0xAEE] = 0x80;
	CpsRom[0xAEF] = 0x0;
	CpsRom[0xAF0] = 0x6;
	CpsRom[0xAF1] = 0x1;
	CpsRom[0xAF2] = 0xED;
	CpsRom[0xAF3] = 0x33;
	CpsRom[0xAF4] = 0x90;
	CpsRom[0xAF5] = 0xDB;
	CpsRom[0xAF6] = 0x80;
	CpsRom[0xAF7] = 0x0;
	CpsRom[0xAF8] = 0x8;
	CpsRom[0xAF9] = 0x1;
	CpsRom[0xAFA] = 0xED;
	CpsRom[0xAFB] = 0x50;
	CpsRom[0xAFC] = 0x49;
	CpsRom[0xAFD] = 0xDB;
	CpsRom[0xAFE] = 0xFC;
	CpsRom[0xAFF] = 0x33;
	CpsRom[0xB00] = 0x3F;
	CpsRom[0xB02] = 0x80;
	CpsRom[0xB03] = 0x0;
	CpsRom[0xB04] = 0x4C;
	CpsRom[0xB05] = 0x1;
	CpsRom[0xB06] = 0xFC;
	CpsRom[0xB07] = 0x33;
	CpsRom[0xB08] = 0x40;
	CpsRom[0xB09] = 0x91;
	CpsRom[0xB0A] = 0x80;
	CpsRom[0xB0C] = 0xA;
	CpsRom[0xB0D] = 0x1;
	CpsRom[0xB98] = 0xC0;
	CpsRom[0xBA4] = 0x40;
	CpsRom[0xBA5] = 0xD2;
	CpsRom[0xBC8] = 0x40;
	CpsRom[0xBC9] = 0xD2;
	CpsRom[0xBA8] = 0x80;
	CpsRom[0xBAA] = 0xC;
	CpsRom[0xBAB] = 0x1;
	CpsRom[0xBBA] = 0x80;
	CpsRom[0xBBC] = 0x10;
	CpsRom[0xBBD] = 0x1;
	CpsRom[0xBCC] = 0x80;
	CpsRom[0xBCE] = 0x14;
	CpsRom[0xBCF] = 0x1;
	CpsRom[0xBDA] = 0x80;
	CpsRom[0xBDC] = 0xE;
	CpsRom[0xBDD] = 0x1;
	CpsRom[0xBEE] = 0x80;
	CpsRom[0xBF0] = 0x12;
	CpsRom[0xBF1] = 0x1;
	CpsRom[0xC02] = 0x80;
	CpsRom[0xC04] = 0x16;
	CpsRom[0xC05] = 0x1;
	CpsRom[0x4DBC] = 0x0;
	CpsRom[0x4DBE] = 0x58;
	CpsRom[0x4DBF] = 0x0;
	CpsRom[0x4DC0] = 0xF9;
	CpsRom[0x4DC1] = 0x41;
	CpsRom[0x4DC2] = 0xF1;
	CpsRom[0x4DC3] = 0x0;
	CpsRom[0x4DC4] = 0x0;
	CpsRom[0x4DC5] = 0x80;
	CpsRom[0x4DC6] = 0x28;
	CpsRom[0x4DC7] = 0x4A;
	CpsRom[0x4DC8] = 0x1F;
	CpsRom[0x4DC9] = 0x0;
	CpsRom[0x4DCA] = 0x0;
	CpsRom[0x4DCB] = 0x6A;
	CpsRom[0x4DCC] = 0x4A;
	CpsRom[0x4DCD] = 0x0;
	CpsRom[0x4DCE] = 0xED;
	CpsRom[0x4DCF] = 0x49;
	CpsRom[0x4DD0] = 0x96;
	CpsRom[0x4DD1] = 0xE2;
	CpsRom[0x4DD2] = 0xF4;
	CpsRom[0x4DD3] = 0x49;
	CpsRom[0x4DD4] = 0x0;
	CpsRom[0x4DD5] = 0x0;
	CpsRom[0x4DD6] = 0x40;
	CpsRom[0x4DD7] = 0x6;
	CpsRom[0x4DD8] = 0x10;
	CpsRom[0x4DDA] = 0x40;
	CpsRom[0x4DDB] = 0x2;
	CpsRom[0x4DDC] = 0xFF;
	CpsRom[0x4DDD] = 0xF;
	CpsRom[0x4DDE] = 0x40;
	CpsRom[0x4DDF] = 0x3B;
	CpsRom[0x4DE0] = 0xE8;
	CpsRom[0x4DE1] = 0xDA;
	CpsRom[0x4DE2] = 0x5C;
	CpsRom[0x4DE3] = 0x11;
	CpsRom[0x4DE4] = 0x1;
	CpsRom[0x4DE6] = 0x5C;
	CpsRom[0x4DE7] = 0x11;
	CpsRom[0x4DE8] = 0x3;
	CpsRom[0x4DE9] = 0x0;
	CpsRom[0x4DEA] = 0x5C;
	CpsRom[0x4DEB] = 0x11;
	CpsRom[0x4DEC] = 0x7;
	CpsRom[0x4DED] = 0x0;
	CpsRom[0x4DEE] = 0x5C;
	CpsRom[0x4DEF] = 0x11;
	CpsRom[0x4DF0] = 0x9;
	CpsRom[0x4DF1] = 0x0;
	CpsRom[0x4DF2] = 0x1C;
	CpsRom[0x4DF3] = 0x30;
	CpsRom[0x4DF4] = 0x40;
	CpsRom[0x4DF5] = 0x11;
	CpsRom[0x4DF6] = 0xD;
	CpsRom[0x4DF7] = 0x0;
	CpsRom[0x4DF8] = 0x5C;
	CpsRom[0x4DF9] = 0x11;
	CpsRom[0x4DFA] = 0xF;
	CpsRom[0x4DFB] = 0x0;
	CpsRom[0x4DFC] = 0x5C;
	CpsRom[0x4DFD] = 0x11;
	CpsRom[0x4DFE] = 0x11;
	CpsRom[0x4E00] = 0x5C;
	CpsRom[0x4E01] = 0x11;
	CpsRom[0x4E02] = 0x17;
	CpsRom[0x4E04] = 0x5C;
	CpsRom[0x4E05] = 0x11;
	CpsRom[0x4E06] = 0x19;
	CpsRom[0x4E07] = 0x0;
	CpsRom[0x4E08] = 0x5C;
	CpsRom[0x4E09] = 0x11;
	CpsRom[0x4E0A] = 0x13;
	CpsRom[0x4E0B] = 0x0;
	CpsRom[0x4E0C] = 0x5C;
	CpsRom[0x4E0D] = 0x11;
	CpsRom[0x4E0E] = 0x15;
	CpsRom[0x4E0F] = 0x0;
	CpsRom[0x4E10] = 0x7C;
	CpsRom[0x4E11] = 0x11;
	CpsRom[0x4E12] = 0x0;
	CpsRom[0x4E14] = 0x1F;
	CpsRom[0x4E16] = 0x75;
	CpsRom[0x4E17] = 0x4E;
	CpsRom[0x14CA0] = 0x90;
	CpsRom[0x14CA3] = 0x0;
	CpsRom[0x14CB0] = 0x90;
	CpsRom[0x14CB3] = 0x40;
	CpsRom[0x14F68] = 0xFC;
	CpsRom[0x14F69] = 0x28;
	CpsRom[0x14F6A] = 0x0;
	CpsRom[0x14F6D] = 0x1;

	punisher_decode();
}

static int PunipicInit()
{
//	Cps1pic = 1;
	pCpsInitCallback = punipic_decode;

	return CpsInit();
}

static int PunipicExit()
{
//	Cps1pic = 0;

	return CpsExit();
}

struct BurnDriver BurnDrvCpsPunipic = {
	"punipic", "punisher", NULL, "1993",
	"The Punisher (bootleg with PIC16c57, set 1)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,PunipicRomInfo,PunipicRomName,punisherInputInfo, punishDIPInfo,
	PunipicInit,PunipicExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The Punisher (bootleg with PIC16c57, set 2)

static struct BurnRomInfo Punipic2RomDesc[] = {
	{ "prg4.bin",    0x80000,  0xc3151563, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "prg3.bin",    0x80000,  0x8c2593ac, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "prg2.bin",    0x80000,  0x665a5485, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "prg1.bin",    0x80000,  0xd7b13f39, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "pu11256.bin", 0x200000, 0x6581faea, 2 | BRF_GRA },	 // 4
	{ "pu13478.bin", 0x200000, 0x61613de4, 2 | BRF_GRA },

	{ "ps_q.rom",    0x020000, 0x49ff4446, 3 | BRF_ESS | BRF_PRG }, // z80 program
	{ "pic16c57",    0x4000,   0x00000000, 0 | BRF_PRG | BRF_OPT }, // 12

	{ "ps_q1.rom",   0x080000, 0x31fd8726, 5 | BRF_SND },
	{ "ps_q2.rom",   0x080000, 0x980a9eef, 5 | BRF_SND },
	{ "ps_q3.rom",   0x080000, 0x0dd44491, 5 | BRF_SND },
	{ "ps_q4.rom",   0x080000, 0xbed42f03, 5 | BRF_SND },
	{ "sound.bin",   0x80000,  0xaeec9dc6, 5 | BRF_SND }, // 13

	{ "93c46.bin",   0x80,     0x36ab4e7d, 0 | BRF_OPT }, // 14
};

STD_ROM_PICK(Punipic2) STD_ROM_FN(Punipic2)

struct BurnDriver BurnDrvCpsPunipic2 = {
	"punipic2", "punisher", NULL, "1993",
	"The Punisher (bootleg with PIC16c57, set 2)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,Punipic2RomInfo,Punipic2RomName,punisherInputInfo, punishDIPInfo,
	PunipicInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The Punisher (bootleg with PIC16c57, set 3)

static struct BurnRomInfo Punipic3RomDesc[] = {
	{ "psb5b.rom",   0x80000,  0x58f42c05, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "psb3b.rom",   0x80000,  0x90113db4, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "psb4a.rom",   0x80000,  0x665a5485, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "psb2a.rom",   0x80000,  0xd7b13f39, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "psb-a.rom",   0x200000, 0x57f0f5e3, 2 | BRF_GRA },	 // 4
	{ "psb-b.rom",   0x200000, 0xd9eb867e, 2 | BRF_GRA },

	{ "ps_q.rom",    0x020000, 0x49ff4446, 3 | BRF_ESS | BRF_PRG }, // z80 program
//	{ "pic16c57",    0x4000,   0x00000000, 0 | BRF_PRG | BRF_OPT }, // 6

	{ "ps_q1.rom",   0x080000, 0x31fd8726, 5 | BRF_SND },
	{ "ps_q2.rom",   0x080000, 0x980a9eef, 5 | BRF_SND },
	{ "ps_q3.rom",   0x080000, 0x0dd44491, 5 | BRF_SND },
	{ "ps_q4.rom",   0x080000, 0xbed42f03, 5 | BRF_SND },
//	{ "sound.bin",   0x80000,  0xaeec9dc6, 5 | BRF_SND }, // 7
};

STD_ROM_PICK(Punipic3) STD_ROM_FN(Punipic3)

struct BurnDriver BurnDrvCpsPunipic3 = {
	"punipic3", "punisher", NULL, "1993",
	"The Punisher (bootleg with PIC16c57, set 3)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,Punipic3RomInfo,Punipic3RomName,punisherInputInfo, punishDIPInfo,
	PunipicInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Saturday Night Slam Masters (World 930713)

static struct BurnRomInfo SlammastRomDesc[] = {
	{ "mbe_23e.rom",  0x080000, 0x5394057a, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbe_24b.rom",  0x020000, 0x95d5e729, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_28b.rom",  0x020000, 0xb1c7cbcb, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_25b.rom",  0x020000, 0xa50d3fd4, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_29b.rom",  0x020000, 0x08e32e56, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_21a.rom",  0x080000, 0xd5007b05, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_20a.rom",  0x080000, 0xaeb557b0, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_qa.rom",    0x020000, 0xe21a03c4, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Slammast) STD_ROM_FN(Slammast)

static void slampro_decode()
{
	memcpy (CpsEncZRom, CpsZRom, 0x40000);

	slammast_decode();
}

static int SlammastInit()
{
	pCpsInitCallback = slampro_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSlammast = {
	"slammast", NULL, NULL, "1993",
	"Saturday Night Slam Masters (World 930713)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,4,HARDWARE_CAPCOM_CPS1,
	NULL,SlammastRomInfo,SlammastRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Saturday Night Slam Masters (US 930713)

static struct BurnRomInfo SlammastuRomDesc[] = {
	{ "mbu-23e.rom",  0x080000, 0x224f0062, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbe_24b.rom",  0x020000, 0x95d5e729, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_28b.rom",  0x020000, 0xb1c7cbcb, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_25b.rom",  0x020000, 0xa50d3fd4, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_29b.rom",  0x020000, 0x08e32e56, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_21a.rom",  0x080000, 0xd5007b05, 1 | BRF_ESS | BRF_PRG },
	{ "mbu-20a.rom",  0x080000, 0xfc848af5, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_qa.rom",    0x020000, 0xe21a03c4, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Slammastu) STD_ROM_FN(Slammastu)

struct BurnDriver BurnDrvCpsSlammastu = {
	"slammastu", "slammast", NULL, "1993",
	"Saturday Night Slam Masters (US 930713)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,4,HARDWARE_CAPCOM_CPS1,
	NULL,SlammastuRomInfo,SlammastuRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Muscle Bomber - The Body Explosion (Japan 930713)

static struct BurnRomInfo MbomberjRomDesc[] = {
	{ "mbj23e",      0x080000, 0x0d06036a, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbe_24b.rom", 0x020000, 0x95d5e729, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_28b.rom", 0x020000, 0xb1c7cbcb, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_25b.rom", 0x020000, 0xa50d3fd4, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_29b.rom", 0x020000, 0x08e32e56, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_21a.rom", 0x080000, 0xd5007b05, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_20a.rom", 0x080000, 0xaeb557b0, 1 | BRF_ESS | BRF_PRG },

	{ "mbj_01.bin",  0x080000, 0xa53b1c81, 2 | BRF_GRA },
	{ "mbj_03.bin",  0x080000, 0x23fe10f6, 2 | BRF_GRA },
	{ "mbj_02.bin",  0x080000, 0xcb866c2f, 2 | BRF_GRA },
	{ "mbj_04.bin",  0x080000, 0xc9143e75, 2 | BRF_GRA },
	{ "mb_05.bin",   0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",   0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",   0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",   0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",   0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",   0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",   0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",   0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_qa.rom",   0x020000, 0xe21a03c4, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",   0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",   0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",   0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",   0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",   0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",   0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",   0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",   0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Mbomberj) STD_ROM_FN(Mbomberj)

struct BurnDriver BurnDrvCpsMbomberj = {
	"mbomberj", "slammast", NULL, "1993",
	"Muscle Bomber - The Body Explosion (Japan 930713)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,4,HARDWARE_CAPCOM_CPS1,
	NULL,MbomberjRomInfo,MbomberjRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Muscle Bomber Duo - Ultimate Team Battle (World 931206)

static struct BurnRomInfo MbombrdRomDesc[] = {
	{ "mbd_26.bin",   0x020000, 0x72b7451c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbde_30.rom",  0x020000, 0xa036dc16, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_27.bin",   0x020000, 0x4086f534, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_31.bin",   0x020000, 0x085f47f0, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_24.bin",   0x020000, 0xc20895a5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_28.bin",   0x020000, 0x2618d5e1, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_25.bin",   0x020000, 0x9bdb6b11, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_29.bin",   0x020000, 0x3f52d5e5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_21.bin",   0x080000, 0x690c026a, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_20.bin",   0x080000, 0xb8b2139b, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_q.bin",     0x020000, 0xd6fa76d1, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Mbombrd) STD_ROM_FN(Mbombrd)

struct BurnDriver BurnDrvCpsMbombrd = {
	"mbombrd", NULL, NULL, "1993",
	"Muscle Bomber Duo - Ultimate Team Battle (World 931206)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,4,HARDWARE_CAPCOM_CPS1,
	NULL,MbombrdRomInfo,MbombrdRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Muscle Bomber Duo - Heat Up Warriors (Japan 931206)

static struct BurnRomInfo MbombrdjRomDesc[] = {
	{ "mbd_26.bin",   0x020000, 0x72b7451c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbdj_30.bin",  0x020000, 0xbeff31cf, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_27.bin",   0x020000, 0x4086f534, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_31.bin",   0x020000, 0x085f47f0, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_24.bin",   0x020000, 0xc20895a5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_28.bin",   0x020000, 0x2618d5e1, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_25.bin",   0x020000, 0x9bdb6b11, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_29.bin",   0x020000, 0x3f52d5e5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_21.bin",   0x080000, 0x690c026a, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_20.bin",   0x080000, 0xb8b2139b, 1 | BRF_ESS | BRF_PRG },

	{ "mbj_01.bin",   0x080000, 0xa53b1c81, 2 | BRF_GRA },
	{ "mbj_02.bin",   0x080000, 0xcb866c2f, 2 | BRF_GRA },
	{ "mbj_03.bin",   0x080000, 0x23fe10f6, 2 | BRF_GRA },
	{ "mbj_04.bin",   0x080000, 0xc9143e75, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_q.bin",     0x020000, 0xd6fa76d1, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Mbombrdj) STD_ROM_FN(Mbombrdj)

struct BurnDriver BurnDrvCpsMbombrdj = {
	"mbombrdj", "mbombrd", NULL, "1993",
	"Muscle Bomber Duo - Heat Up Warriors (Japan 931206)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,4,HARDWARE_CAPCOM_CPS1,
	NULL,MbombrdjRomInfo,MbombrdjRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Pnickies (Japan 940608)

static struct BurnRomInfo PnickjRomDesc[] = {
	{ "pnij36.bin", 0x020000, 0x2d4ffb2b, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "pnij42.bin", 0x020000, 0xc085dfaf, 1 | BRF_ESS | BRF_PRG },

	{ "pnij09.bin", 0x020000, 0x48177b0a, 2 | BRF_GRA },
	{ "pnij01.bin", 0x020000, 0x01a0f311, 2 | BRF_GRA },
	{ "pnij13.bin", 0x020000, 0x406451b0, 2 | BRF_GRA },
	{ "pnij05.bin", 0x020000, 0x8c515dc0, 2 | BRF_GRA },
	{ "pnij26.bin", 0x020000, 0xe2af981e, 2 | BRF_GRA },
	{ "pnij18.bin", 0x020000, 0xf17a0e56, 2 | BRF_GRA },
	{ "pnij38.bin", 0x020000, 0xeb75bd8c, 2 | BRF_GRA },
	{ "pnij32.bin", 0x020000, 0x84560bef, 2 | BRF_GRA },
	{ "pnij10.bin", 0x020000, 0xc2acc171, 2 | BRF_GRA },
	{ "pnij02.bin", 0x020000, 0x0e21fc33, 2 | BRF_GRA },
	{ "pnij14.bin", 0x020000, 0x7fe59b19, 2 | BRF_GRA },
	{ "pnij06.bin", 0x020000, 0x79f4bfe3, 2 | BRF_GRA },
	{ "pnij27.bin", 0x020000, 0x83d5cb0e, 2 | BRF_GRA },
	{ "pnij19.bin", 0x020000, 0xaf08b230, 2 | BRF_GRA },
	{ "pnij39.bin", 0x020000, 0x70fbe579, 2 | BRF_GRA },
	{ "pnij33.bin", 0x020000, 0x3ed2c680, 2 | BRF_GRA },

	{ "pnij17.bin", 0x010000, 0xe86f787a, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "pnij24.bin", 0x020000, 0x5092257d, 4 | BRF_SND },
	{ "pnij25.bin", 0x020000, 0x22109aaa, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Pnickj) STD_ROM_FN(Pnickj)

struct BurnDriver BurnDrvCpsPnickj = {
	"pnickj", NULL, NULL, "1994",
	"Pnickies (Japan 940608)\0", NULL, "Compile (Capcom license)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,PnickjRomInfo,PnickjRomName,pnickjInputInfo, pnickjDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Quiz & Dragons (US 920701)

static struct BurnRomInfo QadRomDesc[] = {
	{ "qdu_36a.12f", 0x020000, 0xde9c24a0, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "qdu_42a.12h", 0x020000, 0xcfe36f0c, 1 | BRF_ESS | BRF_PRG },
	{ "qdu_37a.13f", 0x020000, 0x10d22320, 1 | BRF_ESS | BRF_PRG },
	{ "qdu_43a.13h", 0x020000, 0x15e6beb9, 1 | BRF_ESS | BRF_PRG },

	{ "qdu_09.4b",   0x020000, 0x8c3f9f44, 2 | BRF_GRA },
	{ "qdu_01.4a",   0x020000, 0xf688cf8f, 2 | BRF_GRA },
	{ "qdu_13.9b",   0x020000, 0xafbd551b, 2 | BRF_GRA },
	{ "qdu_05.9a",   0x020000, 0xc3db0910, 2 | BRF_GRA },
	{ "qdu_24.5e",   0x020000, 0x2f1bd0ec, 2 | BRF_GRA },
	{ "qdu_17.5c",   0x020000, 0xa812f9e2, 2 | BRF_GRA },
	{ "qdu_38.8h",   0x020000, 0xccdddd1f, 2 | BRF_GRA },
	{ "qdu_32.8f",   0x020000, 0xa8d295d3, 2 | BRF_GRA },

	{ "qdu_23.13b",  0x010000, 0xcfb5264b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "qdu_30.12c",  0x020000, 0xf190da84, 4 | BRF_SND },
	{ "qdu_31.13c",  0x020000, 0xb7583f73, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "qd22b.1a",    0x000117, 0x783c53ab, BRF_OPT },	// b-board PLDs
	{ "iob1.12e",    0x000117, 0x3abc0700, BRF_OPT },
	{ "ioc1.ic1",    0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
};

STD_ROM_PICK(Qad) STD_ROM_FN(Qad)

struct BurnDriver BurnDrvCpsQad = {
	"qad", NULL, NULL, "1992",
	"Quiz & Dragons (US 920701)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,QadRomInfo,QadRomName,qadInputInfo, qadDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Quiz & Dragons (Japan 940921)

static struct BurnRomInfo QadjRomDesc[] = {
	{ "qad23a.bin", 0x080000, 0x4d3553de, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "qad22a.bin", 0x080000, 0x3191ddd0, 1 | BRF_ESS | BRF_PRG },

	{ "qad01.bin",  0x080000, 0x9d853b57, 2 | BRF_GRA },
	{ "qad02.bin",  0x080000, 0xb35976c4, 2 | BRF_GRA },
	{ "qad03.bin",  0x080000, 0xcea4ca8c, 2 | BRF_GRA },
	{ "qad04.bin",  0x080000, 0x41b74d1b, 2 | BRF_GRA },

	{ "qad09.bin",  0x010000, 0x733161cc, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "qad18.bin",  0x020000, 0x2bfe6f6a, 4 | BRF_SND },
	{ "qad19.bin",  0x020000, 0x13d3236b, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Qadj) STD_ROM_FN(Qadj)

struct BurnDriver BurnDrvCpsQadj = {
	"qadj", "qad", NULL, "1992",
	"Quiz & Dragons (Japan 940921)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,QadjRomInfo,QadjRomName,qadInputInfo, qadjDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Quiz Tonosama no Yabou 2 Zenkoku-ban (Japan 950123)

static struct BurnRomInfo Qtono2RomDesc[] = {
	{ "tn2j-30.11e", 0x020000, 0x9226eb5e, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "tn2j-37.11f", 0x020000, 0xd1d30da1, 1 | BRF_ESS | BRF_PRG },
	{ "tn2j-31.12e", 0x020000, 0x015e6a8a, 1 | BRF_ESS | BRF_PRG },
	{ "tn2j-38.12f", 0x020000, 0x1f139bcc, 1 | BRF_ESS | BRF_PRG },
	{ "tn2j-28.9e",  0x020000, 0x86d27f71, 1 | BRF_ESS | BRF_PRG },
	{ "tn2j-35.9f",  0x020000, 0x7a1ab87d, 1 | BRF_ESS | BRF_PRG },
	{ "tn2j-29.10e", 0x020000, 0x9c384e99, 1 | BRF_ESS | BRF_PRG },
	{ "tn2j-36.10f", 0x020000, 0x4c4b2a0a, 1 | BRF_ESS | BRF_PRG },

	{ "tn2-02m.4a",  0x080000, 0xf2016a34, 2 | BRF_GRA },
	{ "tn2-04m.6a",  0x080000, 0x094e0fb1, 2 | BRF_GRA },
	{ "tn2-01m.3a",  0x080000, 0xcb950cf9, 2 | BRF_GRA },
	{ "tn2-03m.5a",  0x080000, 0x18a5bf59, 2 | BRF_GRA },
	{ "tn2-11m.4c",  0x080000, 0xd0edd30b, 2 | BRF_GRA },
	{ "tn2-13m.6c",  0x080000, 0x426621c3, 2 | BRF_GRA },
	{ "tn2-10m.3c",  0x080000, 0xa34ece70, 2 | BRF_GRA },
	{ "tn2-12m.5c",  0x080000, 0xe04ff2f4, 2 | BRF_GRA },

	{ "tn2j-09.12a", 0x008000, 0x6d8edcef, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "tn2j-18.11c", 0x020000, 0xa40bf9a7, 4 | BRF_SND },
	{ "tn2j-19.12c", 0x020000, 0x5b3b931e, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Qtono2) STD_ROM_FN(Qtono2)

struct BurnDriver BurnDrvCpsQtono2 = {
	"qtono2", NULL, NULL, "1995",
	"Quiz Tonosama no Yabou 2 Zenkoku-ban (Japan 950123)\0", NULL, "Capcom", "CPS1",
	L"\u30AF\u30A4\u30BA\u6BBF\u69D8\u306E\u91CE\u671B\uFF12 (\u3068\u306E\u3055\u307E\u306E\u3084\u307C\u3046 2 Japan 950123)\0Quiz Tonosama no Yabou 2 Zenkoku-ban\0", NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,Qtono2RomInfo,Qtono2RomName,qadInputInfo, qtono2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Pang! 3 (Euro 950601)

static struct BurnRomInfo Pang3RomDesc[] = {
	{ "pa3e-17a.11l", 0x080000, 0xa213fa80, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "pa3e-16a.10l", 0x080000, 0x7169ea67, 1 | BRF_ESS | BRF_PRG },

	{ "pa3-01m.2c",   0x200000, 0x068a152c, 2 | BRF_GRA },
	{ "pa3-07m.2f",   0x200000, 0x3a4a619d, 2 | BRF_GRA },

	{ "pa3-11.11f",   0x008000, 0x90a08c46, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "pa3-05.10d",   0x020000, 0x73a10d5d, 4 | BRF_SND },
	{ "pa3-06.11d",   0x020000, 0xaffa4f82, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Pang3) STD_ROM_FN(Pang3)

static void pang3_decrypt()
{
	unsigned short *rom = (unsigned short *)CpsRom;
	int A,src,dst;

	for (A = 0x80000; A < 0x100000; A += 2)
	{
		// only the low 8 bits of each word are encrypted
		src = swapWord(rom[A/2]);
		dst = src & 0xff00;
		if ( src & 0x01) dst ^= 0x04;
		if ( src & 0x02) dst ^= 0x21;
		if ( src & 0x04) dst ^= 0x01;
		if (~src & 0x08) dst ^= 0x50;
		if ( src & 0x10) dst ^= 0x40;
		if ( src & 0x20) dst ^= 0x06;
		if ( src & 0x40) dst ^= 0x08;
		if (~src & 0x80) dst ^= 0x88;
		rom[A/2] = swapWord(dst);
	}
}

static int pang3Init()
{
	pCpsInitCallback = pang3_decrypt;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsPang3 = {
	"pang3", NULL, NULL, "1995",
	"Pang! 3 (Euro 950601)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1_GENERIC,
	NULL,Pang3RomInfo,Pang3RomName,pang3InputInfo, NULL,
	pang3Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Pang! 3 (Euro 950511, not encrypted)

static struct BurnRomInfo Pang3nRomDesc[] = {
	{ "pa3w-17.11l", 0x080000, 0x12138234, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "pa3w-16.10l", 0x080000, 0xd1ba585c, 1 | BRF_ESS | BRF_PRG },

	{ "pa3-01m.2c",  0x200000, 0x068a152c, 2 | BRF_GRA },
	{ "pa3-07m.2f",  0x200000, 0x3a4a619d, 2 | BRF_GRA },

	{ "pa3-11.11f",  0x008000, 0x90a08c46, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "pa3-05.10d",  0x020000, 0x73a10d5d, 4 | BRF_SND },
	{ "pa3-06.11d",  0x020000, 0xaffa4f82, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Pang3n) STD_ROM_FN(Pang3n)

struct BurnDriver BurnDrvCpsPang3n = {
	"pang3n", "pang3", NULL, "1995",
	"Pang! 3 (Euro 950511, not encrypted)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_GENERIC,
	NULL,Pang3nRomInfo,Pang3nRomName,pang3nInputInfo, pang3DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Pang! 3 (Japan 950511)

static struct BurnRomInfo Pang3jRomDesc[] = {
	{ "pa3j-17.11l", 0x080000, 0x21f6e51f, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "pa3j-16.10l", 0x080000, 0xca1d7897, 1 | BRF_ESS | BRF_PRG },

	{ "pa3-01m.2c",  0x200000, 0x068a152c, 2 | BRF_GRA },
	{ "pa3-07m.2f",  0x200000, 0x3a4a619d, 2 | BRF_GRA },

	{ "pa3-11.11f",  0x008000, 0x90a08c46, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "pa3-05.10d",  0x020000, 0x73a10d5d, 4 | BRF_SND },
	{ "pa3-06.11d",  0x020000, 0xaffa4f82, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Pang3j) STD_ROM_FN(Pang3j)

struct BurnDriver BurnDrvCpsPang3j = {
	"pang3j", "pang3", NULL, "1995",
	"Pang! 3 (Japan 950511)\0", NULL, "Mitchell", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_GENERIC,
	NULL,Pang3jRomInfo,Pang3jRomName,pang3nInputInfo, pang3DIPInfo,
	pang3Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Mega Man - The Power Battle (Asia 951006)

static struct BurnRomInfo megamanRomDesc[] = {
	{ "rcma_23b.rom", 0x080000, 0x61e4a397, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "rcma_22b.rom", 0x080000, 0x708268c4, 1 | BRF_ESS | BRF_PRG },
	{ "rcma_21a.rom", 0x080000, 0x4376ea95, 1 | BRF_ESS | BRF_PRG },

	{ "rcm_01.rom",   0x080000, 0x6ecdf13f, 2 | BRF_GRA },
	{ "rcm_02.rom",   0x080000, 0x944d4f0f, 2 | BRF_GRA },
	{ "rcm_03.rom",   0x080000, 0x36f3073c, 2 | BRF_GRA },
	{ "rcm_04.rom",   0x080000, 0x54e622ff, 2 | BRF_GRA },
	{ "rcm_05.rom",   0x080000, 0x5dd131fd, 2 | BRF_GRA },
	{ "rcm_06.rom",   0x080000, 0xf0faf813, 2 | BRF_GRA },
	{ "rcm_07.rom",   0x080000, 0x826de013, 2 | BRF_GRA },
	{ "rcm_08.rom",   0x080000, 0xfbff64cf, 2 | BRF_GRA },
	{ "rcm_10.rom",   0x080000, 0x4dc8ada9, 2 | BRF_GRA },
	{ "rcm_11.rom",   0x080000, 0xf2b9ee06, 2 | BRF_GRA },
	{ "rcm_12.rom",   0x080000, 0xfed5f203, 2 | BRF_GRA },
	{ "rcm_13.rom",   0x080000, 0x5069d4a9, 2 | BRF_GRA },
	{ "rcm_14.rom",   0x080000, 0x303be3bd, 2 | BRF_GRA },
	{ "rcm_15.rom",   0x080000, 0x4f2d372f, 2 | BRF_GRA },
	{ "rcm_16.rom",   0x080000, 0x93d97fde, 2 | BRF_GRA },
	{ "rcm_17.rom",   0x080000, 0x92371042, 2 | BRF_GRA },

	{ "rcm_09.rom",   0x020000, 0x9632d6ef, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "rcm_18.rom",   0x020000, 0x80f1f8aa, 4 | BRF_SND },
	{ "rcm_19.rom",   0x020000, 0xf257dbe1, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "rcm63b.1a",    0x000117, 0x84acd494, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",     0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",    0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",     0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",     0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(megaman) STD_ROM_FN(megaman)

struct BurnDriver BurnDrvCpsMegaman = {
	"megaman", NULL, NULL, "1995",
	"Mega Man - The Power Battle (Asia 951006)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPS1,
	NULL,megamanRomInfo,megamanRomName,megamanInputInfo, megamanDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Rockman - The Power Battle (Japan 950922)

static struct BurnRomInfo RockmanjRomDesc[] = {
	{ "rcm23a.bin", 0x080000, 0xefd96cb2, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "rcm22a.bin", 0x080000, 0x8729a689, 1 | BRF_ESS | BRF_PRG },
	{ "rcm21a.bin", 0x080000, 0x517ccde2, 1 | BRF_ESS | BRF_PRG },

	{ "rcm_01.rom", 0x080000, 0x6ecdf13f, 2 | BRF_GRA },
	{ "rcm_02.rom", 0x080000, 0x944d4f0f, 2 | BRF_GRA },
	{ "rcm_03.rom", 0x080000, 0x36f3073c, 2 | BRF_GRA },
	{ "rcm_04.rom", 0x080000, 0x54e622ff, 2 | BRF_GRA },
	{ "rcm_05.rom", 0x080000, 0x5dd131fd, 2 | BRF_GRA },
	{ "rcm_06.rom", 0x080000, 0xf0faf813, 2 | BRF_GRA },
	{ "rcm_07.rom", 0x080000, 0x826de013, 2 | BRF_GRA },
	{ "rcm_08.rom", 0x080000, 0xfbff64cf, 2 | BRF_GRA },
	{ "rcm_10.rom", 0x080000, 0x4dc8ada9, 2 | BRF_GRA },
	{ "rcm_11.rom", 0x080000, 0xf2b9ee06, 2 | BRF_GRA },
	{ "rcm_12.rom", 0x080000, 0xfed5f203, 2 | BRF_GRA },
	{ "rcm_13.rom", 0x080000, 0x5069d4a9, 2 | BRF_GRA },
	{ "rcm_14.rom", 0x080000, 0x303be3bd, 2 | BRF_GRA },
	{ "rcm_15.rom", 0x080000, 0x4f2d372f, 2 | BRF_GRA },
	{ "rcm_16.rom", 0x080000, 0x93d97fde, 2 | BRF_GRA },
	{ "rcm_17.rom", 0x080000, 0x92371042, 2 | BRF_GRA },

	{ "rcm_09.rom", 0x020000, 0x9632d6ef, 3 | BRF_ESS | BRF_PRG },	// z80 program

	{ "rcm_18.rom", 0x020000, 0x80f1f8aa, 4 | BRF_SND },
	{ "rcm_19.rom", 0x020000, 0xf257dbe1, 4 | BRF_SND },

	A_BOARD_PLDS

	{ "rcm63b.1a",  0x000117, 0x84acd494, BRF_OPT },	// b-board PLDs
	{ "iob1.12d",   0x000117, 0x3abc0700, BRF_OPT },
	{ "bprg1.11d",  0x000117, 0x31793da7, BRF_OPT },
	{ "ioc1.ic7",   0x000117, 0x0d182081, BRF_OPT },	// c-board PLDs
	{ "c632.ic1",   0x000117, 0x0fbd9270, BRF_OPT },
};

STD_ROM_PICK(Rockmanj) STD_ROM_FN(Rockmanj)

struct BurnDriver BurnDrvCpsRockmanj = {
	"rockmanj", "megaman", NULL, "1995",
	"Rockman - The Power Battle (Japan 950922)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1,
	NULL,RockmanjRomInfo,RockmanjRomName,megamanInputInfo, rockmanjDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


//------------------------------------------------------------------------------------------
// hacks && bootlegs
//


// Carrier Air Wing (bootleg 901012)

static struct BurnRomInfo CawingbRomDesc[] = {
	{ "2.16d",       0x80000, 0x8125d3f0,  1 | BRF_ESS | BRF_PRG }, //  1
	{ "1.19d",       0x80000, 0xb19b10ce,  1 | BRF_ESS | BRF_PRG }, //  0

	{ "ca-5m.7a",    0x080000, 0x66d4cc37, 2 | BRF_GRA },
	{ "ca-7m.9a",    0x080000, 0xb6f896f2, 2 | BRF_GRA },
	{ "ca-1m.3a",    0x080000, 0x4d0620fd, 2 | BRF_GRA },
	{ "ca-3m.5a",    0x080000, 0x0b0341c3, 2 | BRF_GRA },

	{ "ca_9.12b",    0x010000, 0x96fe7485, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "ca_18.11c",   0x020000, 0x4a613a2c, 4 | BRF_SND },
	{ "ca_19.12c",   0x020000, 0x74584493, 4 | BRF_SND },
};

STD_ROM_PICK(Cawingb) STD_ROM_FN(Cawingb)

struct BurnDriver BurnDrvCpsCawingb = {
	"cawingb", "cawing", NULL, "1990",
	"Carrier Air Wing (bootleg 901012)\0", "Missing GFX", "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,CawingbRomInfo,CawingbRomName,cawingbInputInfo, cawingDIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Dai Makai-Mura (bootleg)

static struct BurnRomInfo daimakbRomDesc[] = {
	{ "dmjb2.bin",  0x080000, 0x7d5f9f84, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "dmjb1.bin",  0x080000, 0x9b945cc4, 1 | BRF_ESS | BRF_PRG },

	{ "dm_02.4b",   0x020000, 0x8b98dc48, 2 | BRF_GRA },
	{ "dm_01.4a",   0x020000, 0x80896c33, 2 | BRF_GRA },
	{ "dm_10.9b",   0x020000, 0xc2e7d9ef, 2 | BRF_GRA },
	{ "dm_09.9a",   0x020000, 0xc9c4afa5, 2 | BRF_GRA },
	{ "dm_18.5e",   0x020000, 0x1aa0db99, 2 | BRF_GRA },
	{ "dm_17.5c",   0x020000, 0xdc6ed8ad, 2 | BRF_GRA },
	{ "dm_30.8h",   0x020000, 0xd9d3f8bd, 2 | BRF_GRA },
	{ "dm_29.8f",   0x020000, 0x49a48796, 2 | BRF_GRA },
	{ "dm_04.5b",   0x020000, 0xa4f4f8f0, 2 | BRF_GRA },
	{ "dm_03.5a",   0x020000, 0xb1033e62, 2 | BRF_GRA },
	{ "dm_12.10b",  0x020000, 0x10fdd76a, 2 | BRF_GRA },
	{ "dm_11.10a",  0x020000, 0x9040cb04, 2 | BRF_GRA },
	{ "dm_20.7e",   0x020000, 0x281d0b3e, 2 | BRF_GRA },
	{ "dm_19.7c",   0x020000, 0x2623b52f, 2 | BRF_GRA },
	{ "dm_32.9h",   0x020000, 0x99692344, 2 | BRF_GRA },
	{ "dm_31.9f",   0x020000, 0x54acb729, 2 | BRF_GRA },
	{ "dm_06.7b",   0x010000, 0xae24bb19, 2 | BRF_GRA },
	{ "dm_05.7a",   0x010000, 0xd34e271a, 2 | BRF_GRA },
	{ "dm_14.11b",  0x010000, 0x3f70dd37, 2 | BRF_GRA },
	{ "dm_13.11a",  0x010000, 0x7e69e2e6, 2 | BRF_GRA },
	{ "dm_22.8e",   0x010000, 0x37c9b6c6, 2 | BRF_GRA },
	{ "dm_21.8c",   0x010000, 0x2f1345b4, 2 | BRF_GRA },
	{ "dm_26.10e",  0x010000, 0x3c2a212a, 2 | BRF_GRA },
	{ "dm_25.10c",  0x010000, 0x889aac05, 2 | BRF_GRA },
	{ "dm_08.8b",   0x010000, 0xbcc0f28c, 2 | BRF_GRA },
	{ "dm_07.8a",   0x010000, 0x2a40166a, 2 | BRF_GRA },
	{ "dm_16.12b",  0x010000, 0x20f85c03, 2 | BRF_GRA },
	{ "dm_15.12a",  0x010000, 0x8426144b, 2 | BRF_GRA },
	{ "dm_24.9e",   0x010000, 0xda088d61, 2 | BRF_GRA },
	{ "dm_23.9c",   0x010000, 0x17e11df0, 2 | BRF_GRA },
	{ "dm_28.11e",  0x010000, 0xf187ba1c, 2 | BRF_GRA },
	{ "dm_27.11c",  0x010000, 0x29f79c78, 2 | BRF_GRA },

	{ "dm_37.13c",  0x010000, 0x3692f6e5, 3 | BRF_ESS | BRF_PRG }, // z80 program
};

STD_ROM_PICK(daimakb); STD_ROM_FN(daimakb);

static int DaimakbInit()
{
	Port6SoundWrite = 1;

	return sf2m2Init();
}

struct BurnDriver BurnDrvCpsDaimakb = {
	"daimakb", "ghouls", NULL, "1988",
	"Dai Makai-Mura (Japan, bootleg)\0", NULL, "bootleg", "CPS1",
	L"\u5927\u9B54\u754C\u6751 (Japan, bootleg)\0Dai Makai-Mura (Japan, bootleg)\0", NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,daimakbRomInfo,daimakbRomName,ghoulsInputInfo, daimakaiDIPInfo,
	DaimakbInit,sf2m1Exit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The King of Dragons (hack)

static struct BurnRomInfo KodhRomDesc[] = {
	{ "23.096",      0x080000, 0xdaf89cfb, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "22.096",      0x080000, 0xc83e19d8, 1 | BRF_ESS | BRF_PRG },

	{ "1.096",       0x080000, 0x09261881, 2 | BRF_GRA },
	{ "2.096",       0x080000, 0xbc121ff2, 2 | BRF_GRA },
	{ "3.096",       0x080000, 0xf463ae22, 2 | BRF_GRA },
	{ "4.096",       0x080000, 0x01308733, 2 | BRF_GRA },
	{ "kod11.rom",   0x080000, 0x113358f3, 2 | BRF_GRA },
	{ "kod13.rom",   0x080000, 0x38853c44, 2 | BRF_GRA },
	{ "7.096",       0x080000, 0xc7ab4704, 2 | BRF_GRA },
	{ "kod12.rom",   0x080000, 0x402b9b4f, 2 | BRF_GRA },

	{ "kd09.bin",    0x010000, 0xbac6ec26, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "kd18.bin",    0x020000, 0x4c63181d, 4 | BRF_SND },
	{ "kd19.bin",    0x020000, 0x92941b80, 4 | BRF_SND },
};

STD_ROM_PICK(Kodh) STD_ROM_FN(Kodh)

struct BurnDriver BurnDrvCpsKodh = {
	"kodh", "kod", NULL, "1991",
	"The King of Dragons (hack)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1,
	NULL,KodhRomInfo,KodhRomName,kodhInputInfo, kodhDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs and Dinosaurs (hack set 1)

static struct BurnRomInfo DinohRomDesc[] = {
	{ "cda_23h.rom",   0x80000, 0x8e2a9cf0, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "cda_22h.rom",   0x80000, 0xf72cd219, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "cda_21h.rom",   0x80000, 0xbc275b76, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "cda_20h.rom",   0x80000, 0x8987c975, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "cd_gfx01.rom",  0x80000, 0x8da4f917, 2 | BRF_GRA },			 // 4
	{ "cd_gfx03.rom",  0x80000, 0x6c40f603, 2 | BRF_GRA },
	{ "cd_gfx02.rom",  0x80000, 0x09c8fc2d, 2 | BRF_GRA },
	{ "cd_gfx04.rom",  0x80000, 0x637ff38f, 2 | BRF_GRA },
	{ "cd_gfx05.rom",  0x80000, 0x470befee, 2 | BRF_GRA },			 // 8
	{ "cd_gfx07.rom",  0x80000, 0x22bfb7a3, 2 | BRF_GRA },
	{ "cd_gfx06.rom",  0x80000, 0xe7599ac4, 2 | BRF_GRA },
	{ "cd_gfx08.rom",  0x80000, 0x211b4b15, 2 | BRF_GRA },

	{ "cd_q.rom",      0x20000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // 12

	{ "cd_q1.rom",     0x80000, 0x60927775, 5 | BRF_SND },			 // 13
	{ "cd_q2.rom",     0x80000, 0x770f4c47, 5 | BRF_SND },			 // 14
	{ "cd_q3.rom",     0x80000, 0x2f273ffc, 5 | BRF_SND },			 // 15
	{ "cd_q4.rom",     0x80000, 0x2c67821d, 5 | BRF_SND },			 // 16
};

STD_ROM_PICK(Dinoh) STD_ROM_FN(Dinoh)

static void dinoh_decode()
{
	// Patch Q-Sound Test
	*((unsigned short*)(CpsRom + 0xaacf4)) = 0x4e71;

	dino_decode();
}

static int dinohInit()
{
	Cps1QsHack = 1;

	pCpsInitCallback = dinoh_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsDinoh = {
	"dinoh", "dino", NULL, "1993",
	"Cadillacs and Dinosaurs (hack set 1)\0", NULL, "bootleg", "CPS1 / QSound",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinohRomInfo,DinohRomName,dinohInputInfo, dinohDIPInfo,
	dinohInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs and Dinosaurs (hack set 2)

static struct BurnRomInfo DinohaRomDesc[] = {
	{ "kl2-r1.800",   0x100000, 0x4C70DCA7, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "kl2-l2.800",   0x100000, 0xC6AE7338, 1 | BRF_ESS | BRF_PRG }, // 1

	{ "cd-a.160",     0x200000, 0x7e4F9FB3, 2 | BRF_GRA },			 // 2
	{ "cd-b.160",     0x200000, 0x89532d85, 2 | BRF_GRA },			 // 3

	{ "cd_q.rom",     0x020000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // 4

	{ "cd_q1.rom",    0x080000, 0x60927775, 5 | BRF_SND },			 // 5
	{ "cd_q2.rom",    0x080000, 0x770f4c47, 5 | BRF_SND },			 // 6
	{ "cd_q3.rom",    0x080000, 0x2f273ffc, 5 | BRF_SND },			 // 7
	{ "cd_q4.rom",    0x080000, 0x2c67821d, 5 | BRF_SND },			 // 8
};

STD_ROM_PICK(Dinoha) STD_ROM_FN(Dinoha)

static void dinoha_decode()
{
	// Invert program roms
	unsigned char *pTemp = (unsigned char*)malloc(0x80000);
	if (pTemp != NULL) {
		memcpy(pTemp, CpsRom, 0x80000);
		memcpy(CpsRom, CpsRom+0x80000, 0x80000);
		memcpy(CpsRom+0x80000, pTemp, 0x80000);
		memcpy(pTemp, CpsRom+0x100000, 0x80000);
		memcpy(CpsRom+0x100000, CpsRom+0x180000, 0x80000);
		memcpy(CpsRom+0x180000, pTemp, 0x80000);
		free(pTemp);
	}

	dinoh_decode();
}

static int dinohaInit()
{
	Cps1QsHack = 1;

	pCpsInitCallback = dinoha_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsDinoha = {
	"dinoha", "dino", NULL, "1997",
	"Cadillacs and Dinosaurs (hack set 2)\0", NULL, "bootleg", "CPS1 / QSound",
	L"97 \u6050\u9F99 II \u4EE3 (hack set 2)\0Cadillacs and Dinosaurs (hack set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinohaRomInfo,DinohaRomName,dinohInputInfo, dinohDIPInfo,
	dinohaInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs and Dinosaurs (hack set 3)

static struct BurnRomInfo DinohbRomDesc[] = {
	{ "cd-d.800",     0x100000, 0x2a7b2915, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "cd-e.800",     0x100000, 0xe8370226, 1 | BRF_ESS | BRF_PRG }, // 1

	{ "cd-a.160",     0x200000, 0x7e4F9FB3, 2 | BRF_GRA },			 // 2
	{ "cd-b.160",     0x200000, 0x89532d85, 2 | BRF_GRA },			 // 3

	{ "cd_q.rom",     0x020000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // 4

	{ "cd_q1.rom",    0x080000, 0x60927775, 5 | BRF_SND },			 // 5
	{ "cd_q2.rom",    0x080000, 0x770f4c47, 5 | BRF_SND },			 // 6
	{ "cd_q3.rom",    0x080000, 0x2f273ffc, 5 | BRF_SND },			 // 7
	{ "cd_q4.rom",    0x080000, 0x2c67821d, 5 | BRF_SND },			 // 8
};

STD_ROM_PICK(Dinohb) STD_ROM_FN(Dinohb)

static void dinohb_decode()
{
	// Fix Change Char
	*((unsigned short*)(CpsRom + 0x1900DA)) = 0x0018;
	*((unsigned short*)(CpsRom + 0x1900F8)) = 0x0018;

	dinohb_patch();
	dino_decode();
}

static int dinohbInit()
{
	int nRet = 0;

//	Cps1pic = 1;
//	Dinopic = 1;

	pCpsInitCallback = dinohb_decode;
	nRet = CpsInit();
/*
	BootlegSpriteRam = (unsigned char*)malloc(0x2000);

	SekOpen(0);
	SekMapMemory(BootlegSpriteRam, 0x990000, 0x991FFF, SM_RAM);
	SekMapHandler(1, 0x980000, 0x98000b, SM_WRITE);
	SekSetWriteWordHandler(1, DinopicScrollWrite);
	SekClose();
*/
	return nRet;
}

struct BurnDriver BurnDrvCpsDinohb = {
	"dinohb", "dino", NULL, "1997",
	"Cadillacs and Dinosaurs (hack set 3)\0", NULL, "bootleg", "CPS1 / QSound",
	L"97 \u6050\u9F99 II \u4EE3 (hack set 3)\0Cadillacs and Dinosaurs (hack set 3)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinohbRomInfo,DinohbRomName,dinohbInputInfo, dinohDIPInfo,
	dinohbInit,dinopicExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Cadillacs and Dinosaurs: Dinosaur Hunter (hack set 4)

static struct BurnRomInfo DinohcRomDesc[] = {
	{ "u23",    0x80000, 0x8d5ddc5d, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "u22",    0x80000, 0xf72cd219, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "u21",    0x80000, 0xbc275b76, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "u20",    0x80000, 0x8987c975, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "u1",     0x80000, 0xa01a9fb5, 2 | BRF_GRA },		  // 4
	{ "u2",     0x80000, 0xbdf02c17, 2 | BRF_GRA },
	{ "u3",     0x80000, 0x058beefa, 2 | BRF_GRA },
	{ "u4",     0x80000, 0x5028a9f1, 2 | BRF_GRA },
	{ "u5",     0x80000, 0xd77f89ea, 2 | BRF_GRA },		  // 8
	{ "u6",     0x80000, 0xbfbcb034, 2 | BRF_GRA },
	{ "u7",     0x80000, 0xa2544d4e, 2 | BRF_GRA },
	{ "u8",     0x80000, 0x8869bbb1, 2 | BRF_GRA },

	{ "cd_q.rom",      0x20000, 0x605fdb0b, 3 | BRF_ESS | BRF_PRG }, // 12
	{ "u9",     0x10000, 0x2eb16a83, 0 | BRF_PRG | BRF_OPT }, // 12

	{ "cd_q1.rom",     0x80000, 0x60927775, 5 | BRF_SND },	  // 13
	{ "cd_q2.rom",     0x80000, 0x770f4c47, 5 | BRF_SND },	  // 14
	{ "cd_q3.rom",     0x80000, 0x2f273ffc, 5 | BRF_SND },	  // 15
	{ "cd_q4.rom",     0x80000, 0x2c67821d, 5 | BRF_SND },	  // 16
	{ "u18",    0x20000, 0x8d2899ba, 0 | BRF_SND | BRF_OPT }, // 13
	{ "u19",    0x20000, 0xb34a4b42, 0 | BRF_SND | BRF_OPT }, // 14
};

STD_ROM_PICK(Dinohc) STD_ROM_FN(Dinohc)

static void dinohc_decode()
{
	CpsRom[0x760] = 0x80;
	CpsRom[0x762] = 0x77;
	CpsRom[0x763] = 0x1;
	CpsRom[0x792] = 0x80;
	CpsRom[0x794] = 0x77;
	CpsRom[0x795] = 0x1;
	CpsRom[0x7B6] = 0x80;
	CpsRom[0x7B8] = 0x77;
	CpsRom[0x7B9] = 0x1;

	dinoh_decode();
}

static int dinohcInit()
{
	Cps1QsHack = 1;

	pCpsInitCallback = dinohc_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsDinohc = {
	"dinohc", "dino", NULL, "1993",
	"Cadillacs and Dinosaurs: Dinosaur Hunter (hack set 4)\0", NULL, "bootleg", "CPS1 / QSound",
	L"\u6050\u9F99\u730E\u4EBA (hack set 4)\0Cadillacs and Dinosaurs (hack set 4)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,DinohcRomInfo,DinohcRomName,dinohInputInfo, dinohDIPInfo,
	dinohcInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Knights of the Round (hack)

static struct BurnRomInfo KnightshRomDesc[] = {
	{ "krh_23.rom",    0x80000, 0xfa2ff63d, 1 | BRF_ESS | BRF_PRG },
	{ "krh_22.rom",    0x80000, 0x1438d070, 1 | BRF_ESS | BRF_PRG },

	{ "kr_gfx1.rom",   0x80000, 0x9e36c1a4, 2 | BRF_GRA },
	{ "kr_gfx3.rom",   0x80000, 0xc5832cae, 2 | BRF_GRA },
	{ "kr_gfx2.rom",   0x80000, 0xf095be2d, 2 | BRF_GRA },
	{ "kr_gfx4.rom",   0x80000, 0x179dfd96, 2 | BRF_GRA },
	{ "kr_gfx5.rom",   0x80000, 0x1f4298d2, 2 | BRF_GRA },
	{ "kr_gfx7.rom",   0x80000, 0x37fa8751, 2 | BRF_GRA },
	{ "kr_gfx6.rom",   0x80000, 0x0200bc3d, 2 | BRF_GRA },
	{ "kr_gfx8.rom",   0x80000, 0x0bb2b4e7, 2 | BRF_GRA },

	{ "kr_09.rom",     0x10000, 0x5e44d9ee, 3 | BRF_ESS | BRF_PRG },

	{ "kr_18.rom",     0x20000, 0xda69d15f, 4 | BRF_SND },
	{ "kr_19.rom",     0x20000, 0xbfc654e9, 4 | BRF_SND },
};

STD_ROM_PICK(Knightsh) STD_ROM_FN(Knightsh)

struct BurnDriver BurnDrvCpsKnightsh = {
	"knightsh", "knights", NULL, "1991",
	"Knights of the Round (hack)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1,
	NULL,KnightshRomInfo,KnightshRomName,knightshInputInfo, knightshDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Knights of the Round (bootleg set 2)

static struct BurnRomInfo Knightsb2RomDesc[] = {
	{ "040-z.02",      0x80000, 0x95d00a7e, 1 | BRF_ESS | BRF_PRG },
	{ "040-r.02",      0x80000, 0x5a9d0b64, 1 | BRF_ESS | BRF_PRG },

	{ "kr_gfx1.rom",   0x80000, 0x9e36c1a4, 2 | BRF_GRA },
	{ "kr_gfx3.rom",   0x80000, 0xc5832cae, 2 | BRF_GRA },
	{ "kr_gfx2.rom",   0x80000, 0xf095be2d, 2 | BRF_GRA },
	{ "kr_gfx4.rom",   0x80000, 0x179dfd96, 2 | BRF_GRA },
	{ "kr_gfx5.rom",   0x80000, 0x1f4298d2, 2 | BRF_GRA },
	{ "kr_gfx7.rom",   0x80000, 0x37fa8751, 2 | BRF_GRA },
	{ "kr_gfx6.rom",   0x80000, 0x0200bc3d, 2 | BRF_GRA },
	{ "kr_gfx8.rom",   0x80000, 0x0bb2b4e7, 2 | BRF_GRA },

	{ "kr_09.rom",     0x10000, 0x5e44d9ee, 3 | BRF_ESS | BRF_PRG },

	{ "kr_18.rom",     0x20000, 0xda69d15f, 4 | BRF_SND },
	{ "kr_19.rom",     0x20000, 0xbfc654e9, 4 | BRF_SND },
};

STD_ROM_PICK(Knightsb2) STD_ROM_FN(Knightsb2)

struct BurnDriver BurnDrvCpsKnightsb2 = {
	"knightsb2", "knights", NULL, "1991",
	"Knights of the Round (bootleg set 2)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1,
	NULL,Knightsb2RomInfo,Knightsb2RomName,knightsInputInfo, knightsDIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Knights of the Round (Japan, bootleg)

static struct BurnRomInfo KnightsjbRomDesc[] = {
	{ "23.096",    0x80000, 0x7733b8a6, 1 | BRF_ESS | BRF_PRG },
	{ "22.096",    0x80000, 0xd0b671a9, 1 | BRF_ESS | BRF_PRG },

	{ "1.096",     0x80000, 0x91541596, 2 | BRF_GRA },
	{ "2.096",     0x80000, 0x5d332114, 2 | BRF_GRA },
	{ "3.096",     0x80000, 0x60488eba, 2 | BRF_GRA },
	{ "4.096",     0x80000, 0x394bdd11, 2 | BRF_GRA },
	{ "5.096",     0x80000, 0xb52af98b, 2 | BRF_GRA },
	{ "6.096",     0x80000, 0xe44e5eac, 2 | BRF_GRA },
	{ "7.096",     0x80000, 0x0200bc3d, 2 | BRF_GRA },
	{ "8.096",     0x80000, 0x0bb2b4e7, 2 | BRF_GRA },

	{ "9.512",     0x10000, 0x5e44d9ee, 3 | BRF_ESS | BRF_PRG },

	{ "18.010",    0x20000, 0x9b9be3ab, 4 | BRF_SND },
	{ "19.010",    0x20000, 0xbfc654e9, 4 | BRF_SND },
};

STD_ROM_PICK(Knightsjb) STD_ROM_FN(Knightsjb)

struct BurnDriver BurnDrvCpsKnightsjb = {
	"knightsjb", "knights", NULL, "1991",
	"Knights of the Round (Japan, bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1,
	NULL,KnightsjbRomInfo,KnightsjbRomName,knightsInputInfo, knightsDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// The Punisher (hack)

static struct BurnRomInfo PunisherhRomDesc[] = {
	{ "23.096",      0x080000, 0xbfa45d23, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "22.096",      0x080000, 0x092578a4, 1 | BRF_ESS | BRF_PRG },
	{ "21.096",      0x080000, 0xd21ccddb, 1 | BRF_ESS | BRF_PRG },
	{ "20.096",      0x080000, 0xf9f334ce, 1 | BRF_ESS | BRF_PRG },

	{ "1.096",       0x080000, 0xad468e07, 2 | BRF_GRA },
	{ "2.096",       0x080000, 0xb9fdb6b5, 2 | BRF_GRA },
	{ "3.096",       0x080000, 0xbe0b1a78, 2 | BRF_GRA },
	{ "4.096",       0x080000, 0xbba67a43, 2 | BRF_GRA },
	{ "ps_gfx5.rom", 0x080000, 0xc54ea839, 2 | BRF_GRA },
	{ "ps_gfx7.rom", 0x080000, 0x04c5acbd, 2 | BRF_GRA },
	{ "ps_gfx6.rom", 0x080000, 0xa544f4cc, 2 | BRF_GRA },
	{ "ps_gfx8.rom", 0x080000, 0x8f02f436, 2 | BRF_GRA },

	{ "9.512",       0x010000, 0xb8367eb5, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "18.010",      0x020000, 0x375c66e7, 4 | BRF_SND },
	{ "19.010",      0x020000, 0xeb5ca884, 4 | BRF_SND },
};

STD_ROM_PICK(Punisherh) STD_ROM_FN(Punisherh)

struct BurnDriver BurnDrvCpsPunisherh = {
	"punisherh", "punisher", NULL, "2002",
	"The Punisher (hack)\0", NULL, "bootleg", "CPS1",
	L"\uFEFF\u98C6\u98A8\u6230\u8B66 (hack)\0The Punisher (hack)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,PunisherhRomInfo,PunisherhRomName,punisherhInputInfo, punisherhDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II - The World Warrior (bootleg)

static struct BurnRomInfo Sf2bRomDesc[] = {
	{ "u3.bin",       0x020000, 0x2c9ece7c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "u4.bin",       0x020000, 0x4efb4c7a, 1 | BRF_ESS | BRF_PRG },
//	{ "sf2-4.bin",    0x020000, 0x76f9f91f, 1 | BRF_ESS | BRF_PRG },
//	{ "sf2-3.bin",    0x020000, 0xe8f14362, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_31a.bin",  0x020000, 0xa673143d, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_38a.bin",  0x020000, 0x4c2ccef7, 1 | BRF_ESS | BRF_PRG },
	{ "u6.bin",       0x020000, 0x5cfc3f39, 1 | BRF_ESS | BRF_PRG },
	{ "u5.bin",       0x020000, 0x47dd24b6, 1 | BRF_ESS | BRF_PRG },
//	{ "sf2-1.bin",    0x020000, 0x6de44671, 1 | BRF_ESS | BRF_PRG },
//	{ "sf2-2.bin",    0x020000, 0xbf0cd819, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_29a.bin",  0x020000, 0xbb4af315, 1 | BRF_ESS | BRF_PRG },
	{ "sf2_36a.bin",  0x020000, 0xc02a13eb, 1 | BRF_ESS | BRF_PRG },

	{ "sf2_06.bin",   0x080000, 0x22c9cc8e, 2 | BRF_GRA },
	{ "sf2_08.bin",   0x080000, 0x57213be8, 2 | BRF_GRA },
	{ "sf2_05.bin",   0x080000, 0xba529b4f, 2 | BRF_GRA },
	{ "sf2_07.bin",   0x080000, 0x4b1b33a8, 2 | BRF_GRA },
	{ "sf2_15.bin",   0x080000, 0x2c7e2229, 2 | BRF_GRA },
	{ "sf2_17.bin",   0x080000, 0xb5548f17, 2 | BRF_GRA },
	{ "sf2_14.bin",   0x080000, 0x14b84312, 2 | BRF_GRA },
	{ "sf2_16.bin",   0x080000, 0x5e9cd89a, 2 | BRF_GRA },
	{ "sf2_25.bin",   0x080000, 0x994bfa58, 2 | BRF_GRA },
	{ "sf2_27.bin",   0x080000, 0x3e66ad9d, 2 | BRF_GRA },
	{ "sf2_24.bin",   0x080000, 0xc1befaa8, 2 | BRF_GRA },
	{ "sf2_26.bin",   0x080000, 0x0627c831, 2 | BRF_GRA },

	{ "sf2_09.bin",   0x010000, 0xa4823a1b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sf2_18.bin",   0x020000, 0x7f162009, 4 | BRF_SND },
	{ "sf2_19.bin",   0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2b) STD_ROM_FN(Sf2b)

struct BurnDriver BurnDrvCpsSf2b = {
	"sf2b", "sf2", NULL, "1991",
	"Street Fighter II - The World Warrior (bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2bRomInfo,Sf2bRomName,sf2InputInfo, sf2DIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (Turbo hack, bootleg)

static struct BurnRomInfo Sf2thRomDesc[] = {
	{"stf2th-7.bin" ,0x080000, 0x03991fba, 1 | BRF_ESS | BRF_PRG }, // 0
	{"stf2th-5.bin" ,0x080000, 0x3127302c, 1 | BRF_ESS | BRF_PRG }, // 1
	{"stf2th-6.bin" ,0x020000, 0x64e6e091, 1 | BRF_ESS | BRF_PRG }, // 2
	{"sf2th-4.bin"  ,0x020000, 0xc95e4443, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   ,0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin"   ,0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   ,0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   ,0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   ,0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin"   ,0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   ,0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   ,0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   ,0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin"   ,0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   ,0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   ,0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   ,0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG },

	{"s92_18.bin"   ,0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin"   ,0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2th) STD_ROM_FN(Sf2th)

struct BurnDriver BurnDrvCpsSf2th = {
	"sf2th", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Turbo hack, bootleg)\0",NULL,"bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2thRomInfo,Sf2thRomName,Sf2yycInputInfo, sf2DIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (M8, bootleg)

static struct BurnRomInfo Sf2m8RomDesc[] = {
	{"k4-222"       , 0x080000, 0xdb567b66, 1 | BRF_ESS | BRF_PRG }, // 0
	{"k4-196"       , 0x080000, 0x95ea597e, 1 | BRF_ESS | BRF_PRG }, // 1
	{"u221.epr"     , 0x020000, 0xd8276822, 1 | BRF_ESS | BRF_PRG }, // 2
	{"u195.epr"     , 0x020000, 0x924c6ce2, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   , 0x080000, 0x03b0d852, 2 | BRF_GRA }, // 4
	{"s92_02.bin"   , 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   , 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   , 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   , 0x080000, 0xba8a2761, 2 | BRF_GRA }, // 8
	{"s92_06.bin"   , 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   , 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   , 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   , 0x080000, 0x960687d5, 2 | BRF_GRA }, // 12
	{"s92_11.bin"   , 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   , 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   , 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   , 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // 16

	{"s92_18.bin"   , 0x020000, 0x7f162009, 4 | BRF_SND }, // 17
	{"s92_19.bin"   , 0x020000, 0xbeade53f, 4 | BRF_SND }, // 18
};

STD_ROM_PICK(Sf2m8) STD_ROM_FN(Sf2m8)

static int sf2m8Init()
{
	Scroll1TileMask = 0x4fff;
	Scroll3TileMask = 0x1fff;

	return sf2m3Init();
}

struct BurnDriver BurnDrvCpsSf2m8 = {
	"sf2m8", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M8, bootleg)\0","Imperfect Graphic","bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2m8RomInfo,Sf2m8RomName,Sf2m3InputInfo, sf2DIPInfo,
	sf2m8Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (M9, bootleg)

static struct BurnRomInfo Sf2m9RomDesc[] = {
	{"sf3a-1.040"   , 0x080000, 0x9e6d058a, 1 | BRF_ESS | BRF_PRG }, // 0
	{"sf3a-3.040"   , 0x080000, 0x518d8404, 1 | BRF_ESS | BRF_PRG }, // 1
	{"sf3a-2"       , 0x020000, 0xfca4fc1e, 1 | BRF_ESS | BRF_PRG }, // 2
	{"sf3a-4"       , 0x020000, 0xcfdd6f54, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   , 0x080000, 0x03b0d852, 2 | BRF_GRA }, // 4
	{"s92_02.bin"   , 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   , 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   , 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   , 0x080000, 0xba8a2761, 2 | BRF_GRA }, // 8
	{"s92_06.bin"   , 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   , 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   , 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   , 0x080000, 0x960687d5, 2 | BRF_GRA }, // 12
	{"s92_11.bin"   , 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   , 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   , 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   , 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // 16

	{"s92_18.bin"   , 0x020000, 0x7f162009, 4 | BRF_SND }, // 17
	{"s92_19.bin"   , 0x020000, 0xbeade53f, 4 | BRF_SND }, // 18
};

STD_ROM_PICK(Sf2m9) STD_ROM_FN(Sf2m9)

struct BurnDriver BurnDrvCpsSf2m9 = {
	"sf2m9", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M9, bootleg)\0",NULL,"bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2m9RomInfo,Sf2m9RomName,Sf2yycInputInfo, sf2m2DIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (M10, bootleg)

static struct BurnRomInfo Sf2m10RomDesc[] = {
	{"sfu9-1.040"   , 0x080000, 0x95306baf, 1 | BRF_ESS | BRF_PRG }, // 0
	{"sfu9-3.040"   , 0x080000, 0x21024d5f, 1 | BRF_ESS | BRF_PRG }, // 1
	{"sfu9-2.020"   , 0x040000, 0x0b3fe5dd, 1 | BRF_ESS | BRF_PRG }, // 2
	{"sfu9-4.020"   , 0x040000, 0xdbee7b18, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   , 0x080000, 0x03b0d852, 2 | BRF_GRA }, // 4
	{"s92_02.bin"   , 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   , 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   , 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   , 0x080000, 0xba8a2761, 2 | BRF_GRA }, // 8
	{"s92_06.bin"   , 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   , 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   , 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   , 0x080000, 0x960687d5, 2 | BRF_GRA }, // 12
	{"s92_11.bin"   , 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   , 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   , 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   , 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // 16

	{"s92_18.bin"   , 0x020000, 0x7f162009, 4 | BRF_SND }, // 17
	{"s92_19.bin"   , 0x020000, 0xbeade53f, 4 | BRF_SND }, // 18
};

STD_ROM_PICK(Sf2m10) STD_ROM_FN(Sf2m10)

struct BurnDriver BurnDrvCpsSf2m10 = {
	"sf2m10", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M10, bootleg)\0",NULL,"bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2m10RomInfo,Sf2m10RomName,Sf2yycInputInfo, sf2DIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (M11, bootleg)

static struct BurnRomInfo Sf2m11RomDesc[] = {
	{"sfu7-1.040"   , 0x080000, 0x866a9b31, 1 | BRF_ESS | BRF_PRG }, // 0
	{"sfu7-3.040"   , 0x080000, 0xf3a45593, 1 | BRF_ESS | BRF_PRG }, // 1
	{"sfu7-2.020"   , 0x020000, 0xd1707134, 1 | BRF_ESS | BRF_PRG }, // 2
	{"sfu7-4.020"   , 0x020000, 0xcd1d5666, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   , 0x080000, 0x03b0d852, 2 | BRF_GRA }, // 4
	{"s92_02.bin"   , 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   , 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   , 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   , 0x080000, 0xba8a2761, 2 | BRF_GRA }, // 8
	{"s92_06.bin"   , 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   , 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   , 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   , 0x080000, 0x960687d5, 2 | BRF_GRA }, // 12
	{"s92_11.bin"   , 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   , 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   , 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   , 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // 16

	{"s92_18.bin"   , 0x020000, 0x7f162009, 4 | BRF_SND }, // 17
	{"s92_19.bin"   , 0x020000, 0xbeade53f, 4 | BRF_SND }, // 18
};

STD_ROM_PICK(Sf2m11) STD_ROM_FN(Sf2m11)

struct BurnDriver BurnDrvCpsSf2m11 = {
	"sf2m11", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M11, bootleg)\0",NULL,"bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2m11RomInfo,Sf2m11RomName,Sf2yycInputInfo, sf2jDIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (M12, bootleg)

static struct BurnRomInfo Sf2m12RomDesc[] = {
	{"sfu8-1.040"   , 0x080000, 0x10ec67fe, 1 | BRF_ESS | BRF_PRG }, // 0
	{"sfu8-3.040"   , 0x080000, 0x92eb3a1c, 1 | BRF_ESS | BRF_PRG }, // 1
	{"sfu8-2.020"   , 0x020000, 0x1073b7b6, 1 | BRF_ESS | BRF_PRG }, // 2
	{"sfu8-4.020"   , 0x020000, 0x924c6ce2, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   , 0x080000, 0x03b0d852, 2 | BRF_GRA }, // 4
	{"s92_02.bin"   , 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   , 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   , 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   , 0x080000, 0xba8a2761, 2 | BRF_GRA }, // 8
	{"s92_06.bin"   , 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   , 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   , 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   , 0x080000, 0x960687d5, 2 | BRF_GRA }, // 12
	{"s92_11.bin"   , 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   , 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   , 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   , 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // 16

	{"s92_18.bin"   , 0x020000, 0x7f162009, 4 | BRF_SND }, // 17
	{"s92_19.bin"   , 0x020000, 0xbeade53f, 4 | BRF_SND }, // 18
};

STD_ROM_PICK(Sf2m12) STD_ROM_FN(Sf2m12)

struct BurnDriver BurnDrvCpsSf2m12 = {
	"sf2m12", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (M12, bootleg)\0",NULL,"bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2m12RomInfo,Sf2m12RomName,Sf2yycInputInfo, sf2jDIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' Turbo - Hyper Fighting (M13, bootleg)

static struct BurnRomInfo Sf2m13RomDesc[] = {
	{"222-040.13"   ,0x080000, 0xec6f5cb3, 1 | BRF_ESS | BRF_PRG }, // 0
	{"196-040.13"   ,0x080000, 0x0e9ac52b, 1 | BRF_ESS | BRF_PRG }, // 1
	{"221-010.13"   ,0x020000, 0x8226c11c, 1 | BRF_ESS | BRF_PRG }, // 2
	{"195-010.13"   ,0x020000, 0x924c6ce2, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   ,0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin"   ,0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   ,0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   ,0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   ,0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin"   ,0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   ,0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   ,0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s2t_10.bin"   ,0x080000, 0x3c042686, 2 | BRF_GRA },
	{"s2t_11.bin"   ,0x080000, 0x8b7e7183, 2 | BRF_GRA },
	{"s2t_12.bin"   ,0x080000, 0x293c888c, 2 | BRF_GRA },
	{"s2t_13.bin"   ,0x080000, 0x842b35a4, 2 | BRF_GRA },

	{"s92_09.bin"   ,0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG },

	{"s92_18.bin"   ,0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin"   ,0x020000, 0xbeade53f, 4 | BRF_SND },
};

STD_ROM_PICK(Sf2m13) STD_ROM_FN(Sf2m13)

static void sf2m13_patch()
{
	bootleg_progload();

	// Fix scroll
	*((unsigned short*)(CpsRom + 0x1d22a)) = 0x0120;
	//Fix bg layer
//	*((unsigned short*)(CpsRom + 0x1d270)) = 0x0166;
	// Fix gfx
//	*((unsigned short*)(CpsRom + 0x1d470)) = 0x0170;
	// Fix title gfx
	*((unsigned short*)(CpsRom + 0x21bec)) = 0x0083;
	*((unsigned short*)(CpsRom + 0x21cf8)) = 0x828e;
	*((unsigned short*)(CpsRom + 0x21cfa)) = 0x829e;
	*((unsigned short*)(CpsRom + 0x21cfc)) = 0x82ae;
	*((unsigned short*)(CpsRom + 0x21d06)) = 0x827f;
	*((unsigned short*)(CpsRom + 0x21d08)) = 0x828f;
	*((unsigned short*)(CpsRom + 0x21d0a)) = 0x829f;
	*((unsigned short*)(CpsRom + 0x21d0c)) = 0x82af;
	*((unsigned short*)(CpsRom + 0x21d16)) = 0x827e;
	*((unsigned short*)(CpsRom + 0x21d18)) = 0x82a0;
	*((unsigned short*)(CpsRom + 0x21d1a)) = 0x822c;
	*((unsigned short*)(CpsRom + 0x21d1c)) = 0x823c;
	*((unsigned short*)(CpsRom + 0x21d2a)) = 0x822d;
}

static int sf2m13Init()
{
	pCpsInitCallback = sf2m13_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsSf2m13 = {
	"sf2m13", "sf2hf", NULL, "1992",
	"Street Fighter II' Turbo - Hyper Fighting (M13, bootleg)\0",NULL,"bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2m13RomInfo,Sf2m13RomName,Sf2yycInputInfo, sf2jDIPInfo,
	sf2m13Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (Tu Long set 1, Chinese bootleg)

static struct BurnRomInfo Sf2tlonaRomDesc[] = {
	{"tl4mt.1"      , 0x080000, 0x158635ca, 1 | BRF_ESS | BRF_PRG }, // 0
	{"tl4m.2"       , 0x080000, 0x882cd1c4, 1 | BRF_ESS | BRF_PRG }, // 1
	{"u221t.1m"     , 0x020000, 0x1073b7b6, 1 | BRF_ESS | BRF_PRG }, // 2
	{"u195t.1m"     , 0x020000, 0xdb7e1f72, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   , 0x080000, 0x03b0d852, 2 | BRF_GRA }, // 4
	{"s92_02.bin"   , 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   , 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   , 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   , 0x080000, 0xba8a2761, 2 | BRF_GRA }, // 8
	{"s92_06.bin"   , 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   , 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   , 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   , 0x080000, 0x960687d5, 2 | BRF_GRA }, // 12
	{"s92_11.bin"   , 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   , 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   , 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   , 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // 16

	{"s92_18.bin"   , 0x020000, 0x7f162009, 4 | BRF_SND }, // 17
	{"s92_19.bin"   , 0x020000, 0xbeade53f, 4 | BRF_SND }, // 18
};

STD_ROM_PICK(Sf2tlona) STD_ROM_FN(Sf2tlona)

struct BurnDriver BurnDrvCpsSf2tlona={
	"sf2tlona", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Tu Long set 1, Chinese bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2tlonaRomInfo,Sf2tlonaRomName,Sf2yycInputInfo, sf2DIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II' - Champion Edition (Tu Long set 2, Chinese bootleg)

static struct BurnRomInfo Sf2tlonbRomDesc[] = {
	{"tl4m.1"       , 0x080000, 0xafc7bd18, 1 | BRF_ESS | BRF_PRG }, // 0
	{"tl4m.2"       , 0x080000, 0x882cd1c4, 1 | BRF_ESS | BRF_PRG }, // 1
	{"u221.1m"      , 0x020000, 0xd1707134, 1 | BRF_ESS | BRF_PRG }, // 2
	{"u195.1m"      , 0x020000, 0xcd1d5666, 1 | BRF_ESS | BRF_PRG }, // 3

	{"s92_01.bin"   , 0x080000, 0x03b0d852, 2 | BRF_GRA }, // 4
	{"s92_02.bin"   , 0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin"   , 0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin"   , 0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin"   , 0x080000, 0xba8a2761, 2 | BRF_GRA }, // 8
	{"s92_06.bin"   , 0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin"   , 0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin"   , 0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin"   , 0x080000, 0x960687d5, 2 | BRF_GRA }, // 12
	{"s92_11.bin"   , 0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin"   , 0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin"   , 0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin"   , 0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // 16

	{"s92_18.bin"   , 0x020000, 0x7f162009, 4 | BRF_SND }, // 17
	{"s92_19.bin"   , 0x020000, 0xbeade53f, 4 | BRF_SND }, // 18
};

STD_ROM_PICK(Sf2tlonb) STD_ROM_FN(Sf2tlonb)

struct BurnDriver BurnDrvCpsSf2tlonb={
	"sf2tlonb", "sf2ce", NULL, "1992",
	"Street Fighter II' - Champion Edition (Tu Long set 2, Chinese bootleg)\0", NULL, "bootleg", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,Sf2tlonbRomInfo,Sf2tlonbRomName,Sf2yycInputInfo, sf2DIPInfo,
	sf2m2Init,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: SanGuo YingXiongZhuan (set 1, Chinese bootleg)

static struct BurnRomInfo WofhRomDesc[] = {
	{ "sgyx.800",     0x100000, 0x3703a650, 1 | BRF_ESS | BRF_PRG }, // 0

	{ "sgyx-1.160",   0x200000, 0xa60be9f6, 2 | BRF_GRA },			 // 1
	{ "sgyx-2.160",   0x200000, 0x6ad9d048, 2 | BRF_GRA },			 // 2

	{ "tk2_qa.rom",   0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // 3

	{ "tk2_q1.rom",   0x080000, 0x611268cf, 5 | BRF_SND },			 // 4
	{ "tk2_q2.rom",   0x080000, 0x20f55ca9, 5 | BRF_SND },			 // 5
	{ "tk2_q3.rom",   0x080000, 0xbfcf6f52, 5 | BRF_SND },			 // 6
	{ "tk2_q4.rom",   0x080000, 0x36642e88, 5 | BRF_SND },			 // 7
};

STD_ROM_PICK(Wofh) STD_ROM_FN(Wofh)

static void wofh_decode()
{
	// Stage Order
	CpsRom[0x72A6] = 0x0;

	// Patch protection? check
	CpsRom[0xF11EC] = 0x71;
	CpsRom[0xF11ED] = 0x4E;
	CpsRom[0xF11EE] = 0x71;
	CpsRom[0xF11EF] = 0x4E;

	// Disable Sprite Recoding
	CpsRom[0x5D858] = 0x0;
	CpsRom[0x5D859] = 0x61;
	CpsRom[0x5D85A] = 0xE6;
	CpsRom[0x5D85B] = 0x1;
	CpsRom[0x5D890] = 0x6D;
	CpsRom[0x5D891] = 0xC;
	CpsRom[0x5D892] = 0x0;
	CpsRom[0x5D893] = 0x90;
	CpsRom[0x5D894] = 0xD2;
	CpsRom[0x5D895] = 0xE3;
	CpsRom[0x5D8E8] = 0x90;
	CpsRom[0x5D8E9] = 0x0;
	CpsRom[0x5D8EA] = 0x0;
	CpsRom[0x5D8EB] = 0x0;
	CpsRom[0x5D8F8] = 0x90;
	CpsRom[0x5D8F9] = 0x0;
	CpsRom[0x5D8FA] = 0x0;
	CpsRom[0x5D8FB] = 0x40;

	// Patch Q sound protection? check
	CpsRom[0x5A0E] = 0x39;
	CpsRom[0x5A0F] = 0x30;
	CpsRom[0x5A10] = 0xF1;
	CpsRom[0x5A12] = 0xFE;
	CpsRom[0x5A13] = 0x9F;
	CpsRom[0x5A14] = 0x0;
	CpsRom[0x5A15] = 0xC;
	CpsRom[0x5A16] = 0x77;
	CpsRom[0x5A17] = 0x0;
	CpsRom[0x5A18] = 0xF4;
	CpsRom[0x5A19] = 0x66;
	CpsRom[0x5A1A] = 0xF9;
	CpsRom[0x5A1B] = 0x4E;
	CpsRom[0x5A1C] = 0x0;
	CpsRom[0x5A1E] = 0xB2;
	CpsRom[0x5A1F] = 0x72;
	CpsRom[0x5A20] = 0x7C;
	CpsRom[0x5A21] = 0x20;
	CpsRom[0x5A22] = 0xF1;
	CpsRom[0x5A23] = 0x0;
	CpsRom[0x5A24] = 0x0;
	CpsRom[0x5A25] = 0x80;
	CpsRom[0x5A26] = 0x7C;
	CpsRom[0x5A27] = 0x11;
	CpsRom[0x5A29] = 0x0;
	CpsRom[0x5A2A] = 0x1;
	CpsRom[0x5A2C] = 0x7C;
	CpsRom[0x5A2D] = 0x11;
	CpsRom[0x5A2E] = 0x10;
	CpsRom[0x5A2F] = 0x0;
	CpsRom[0x5A30] = 0xF;
	CpsRom[0x5A31] = 0x0;
	CpsRom[0x5A32] = 0x7C;
	CpsRom[0x5A33] = 0x11;
	CpsRom[0x5A34] = 0x0;
	CpsRom[0x5A35] = 0x0;
	CpsRom[0x5A36] = 0x7;
	CpsRom[0x5A38] = 0x7C;
	CpsRom[0x5A39] = 0x11;
	CpsRom[0x5A3A] = 0x0;
	CpsRom[0x5A3B] = 0x0;
	CpsRom[0x5A3C] = 0x9;
	CpsRom[0x5A3D] = 0x0;
	CpsRom[0x5A3E] = 0x7C;
	CpsRom[0x5A3F] = 0x11;
	CpsRom[0x5A40] = 0x0;
	CpsRom[0x5A41] = 0x0;
	CpsRom[0x5A42] = 0xC;
	CpsRom[0x5A44] = 0x41;
	CpsRom[0x5A48] = 0x7C;
	CpsRom[0x5A4A] = 0x0;
	CpsRom[0x5A4C] = 0x11;
	CpsRom[0x5A4D] = 0x0;
	CpsRom[0x5A4E] = 0x7C;
	CpsRom[0x5A4F] = 0x11;
	CpsRom[0x5A50] = 0x0;
	CpsRom[0x5A51] = 0x0;
	CpsRom[0x5A52] = 0x17;
	CpsRom[0x5A54] = 0x7C;
	CpsRom[0x5A56] = 0x0;
	CpsRom[0x5A58] = 0x19;
	CpsRom[0x5A59] = 0x0;
	CpsRom[0x5A5A] = 0x7C;
	CpsRom[0x5A5B] = 0x11;
	CpsRom[0x5A5C] = 0x0;
	CpsRom[0x5A5D] = 0x0;
	CpsRom[0x5A5E] = 0x13;
	CpsRom[0x5A60] = 0x7C;
	CpsRom[0x5A62] = 0x0;
	CpsRom[0x5A64] = 0x15;
	CpsRom[0x5A65] = 0x0;
	CpsRom[0x5A66] = 0x7C;
	CpsRom[0x5A67] = 0x11;
	CpsRom[0x5A68] = 0x0;
	CpsRom[0x5A69] = 0x0;
	CpsRom[0x5A6A] = 0x1F;
	CpsRom[0x5A6C] = 0xF9;
	CpsRom[0x5A6D] = 0x4E;
	CpsRom[0x5A6E] = 0x5;
	CpsRom[0x5A70] = 0x2C;
	CpsRom[0x5A71] = 0xE8;
	CpsRom[0x729A] = 0xF9;
	CpsRom[0x729C] = 0x0;
	CpsRom[0x729D] = 0x0;
	CpsRom[0x729E] = 0xE;
	CpsRom[0x729F] = 0x5A;
	CpsRom[0x5E826] = 0xF9;
	CpsRom[0x5E827] = 0x4E;
	CpsRom[0x5E828] = 0x0;
	CpsRom[0x5E82A] = 0x20;
	CpsRom[0x5E82B] = 0x5A;

	wof_decode();
}

static int wofhInit()
{
	pCpsInitCallback = wofh_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWofh = {
	"wofh", "wof", NULL, "1999",
	"Sangokushi II: SanGuo YingXiongZhuan (set 1, Chinese bootleg)\0", "Imperfect Sound", "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u56FD\u82F1\u96C4\u4F20 (set 1, Chinese bootleg)\0Sangokushi II: SanGuo YingXiongZhuan (set 1, Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofhRomInfo,WofhRomName,wofhInputInfo, wofhDIPInfo,
	wofhInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: SanGuo YingXiongZhuan (set 2, Chinese bootleg)

static struct BurnRomInfo WofhaRomDesc[] = {
	{ "fg-c.040",     0x80000,  0xd046fc86, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "fg-a.040",     0x80000,  0xf176ee8f, 1 | BRF_ESS | BRF_PRG }, // 1

	{ "sgyx-1.160",   0x200000, 0xa60be9f6, 2 | BRF_GRA },			 // 2
	{ "sgyx-2.160",   0x200000, 0x6ad9d048, 2 | BRF_GRA },			 // 3

	{ "tk2_qa.rom",   0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // 4

	{ "tk2_q1.rom",   0x080000, 0x611268cf, 5 | BRF_SND },			 // 5
	{ "tk2_q2.rom",   0x080000, 0x20f55ca9, 5 | BRF_SND },			 // 6
	{ "tk2_q3.rom",   0x080000, 0xbfcf6f52, 5 | BRF_SND },			 // 7
	{ "tk2_q4.rom",   0x080000, 0x36642e88, 5 | BRF_SND },			 // 8
};

STD_ROM_PICK(Wofha) STD_ROM_FN(Wofha)

static void wofha_decode()
{
	bootleg_progload();

	wofh_decode();
}

static int wofhaInit()
{
	pCpsInitCallback = wofha_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWofha = {
	"wofha", "wof", NULL, "1999",
	"Sangokushi II: SanGuo YingXiongZhuan (set 2, Chinese bootleg)\0", "Imperfect Sound", "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u56FD\u82F1\u96C4\u4F20 (set 2, Chinese bootleg)\0Sangokushi II: SanGuo YingXiongZhuan (set 2, Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofhaRomInfo,WofhaRomName,wofhInputInfo, wofhDIPInfo,
	wofhaInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: Huo Fenghuang (Chinese bootleg)

static struct BurnRomInfo WofhfhRomDesc[] = {

	{ "23",  0x080000, 0x6ae4b312, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "22",  0x080000, 0x94e8d01a, 1 | BRF_ESS | BRF_PRG },

	{ "1",   0x080000, 0x0d9cb9bf, 2 | BRF_GRA },
	{ "2",   0x080000, 0x45227027, 2 | BRF_GRA },
	{ "3",   0x080000, 0xc5ca2460, 2 | BRF_GRA },
	{ "4",   0x080000, 0xe349551c, 2 | BRF_GRA },
	{ "5",   0x080000, 0x34949d7b, 2 | BRF_GRA },
	{ "6",   0x080000, 0xdfa70971, 2 | BRF_GRA },
	{ "7",   0x080000, 0x073686a6, 2 | BRF_GRA },
	{ "8",   0x080000, 0x5300f8db, 2 | BRF_GRA },

	{ "9",   0x020000, 0x86fe8a97, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "18",  0x020000, 0xc04be720, 4 | BRF_SND },
	{ "19",  0x020000, 0xfbb8d8c1, 4 | BRF_SND },
};

STD_ROM_PICK(Wofhfh) STD_ROM_FN(Wofhfh)

/*
static void wofhfh_decode()
{
	// Stage Order
	CpsRom[0xB214] = 0x0;
}
*/

static int WofhfhInit()
{
//	pCpsInitCallback = wofhfh_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWofhfh = {
	"wofhfh", "wof", NULL, "1999",
	"Sangokushi II: Huo Fenghuang (Chinese bootleg)\0", NULL, "bootleg", "CPS1",
	L"\u4E09\u56FD\u5FD7II: \u706B\u51E4\u51F0 (Chinese bootleg)\0Sangokushi II: Huo Fenghuang (Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1,
	NULL,WofhfhRomInfo,WofhfhRomName,wofhfhInputInfo, wofhfhDIPInfo,
	WofhfhInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: San Jian Sheng (Chinese bootleg)

static struct BurnRomInfo Wof3jsRomDesc[] = {
	{ "3js_23.rom",   0x80000,  0x1ebb76da, 1 | BRF_ESS | BRF_PRG }, // 0 swapped 68000 code
	{ "3js_22.rom",   0x80000,  0xf41d6153, 1 | BRF_ESS | BRF_PRG }, // 1 swapped
	{ "3js_24.rom",   0x20000,  0x06ead409, 1 | BRF_ESS | BRF_PRG },
	{ "3js_28.rom",   0x20000,  0x8ba934e6, 1 | BRF_ESS | BRF_PRG },

	{ "tk2_gfx1.rom", 0x80000,  0x0d9cb9bf, 2 | BRF_GRA }, // 4
	{ "tk2_gfx3.rom", 0x80000,  0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom", 0x80000,  0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom", 0x80000,  0xe349551c, 2 | BRF_GRA },
	{ "3js_gfx5.rom", 0x80000,  0x94b58f82, 2 | BRF_GRA }, // 8
	{ "3js_gfx7.rom", 0x80000,  0xdf4fb386, 2 | BRF_GRA },
	{ "3js_gfx6.rom", 0x80000,  0xc22c5bd8, 2 | BRF_GRA },
	{ "3js_gfx8.rom", 0x80000,  0xf9cfd08b, 2 | BRF_GRA },

	{ "tk2_qa.rom",   0x20000,  0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // 12
	{ "3js_09.rom",   0x10000,  0x21ce044c, 0 | BRF_PRG | BRF_OPT }, // 12

	{ "tk2_q1.rom",   0x80000, 0x611268cf, 5 | BRF_SND }, // 13
	{ "tk2_q2.rom",   0x80000, 0x20f55ca9, 5 | BRF_SND }, // 14
	{ "tk2_q3.rom",   0x80000, 0xbfcf6f52, 5 | BRF_SND }, // 15
	{ "tk2_q4.rom",   0x80000, 0x36642e88, 5 | BRF_SND }, // 16
	{ "3js_18.rom",   0x20000, 0xac6e307d, 0 | BRF_SND | BRF_OPT }, //13
	{ "3js_19.rom",   0x20000, 0x068741db, 0 | BRF_SND | BRF_OPT }, //14
};

STD_ROM_PICK(Wof3js) STD_ROM_FN(Wof3js)

static void wof3js_decode()
{
	BurnLoadRom(CpsRom + 0x0c0001, 2, 2);
	BurnLoadRom(CpsRom + 0x0c0000, 3, 2);

	// Patch Q sound protection? check
	CpsRom[0x554] = 0xB4;
	CpsRom[0x555] = 0x54;
	CpsRom[0x5A50] = 0x5C;
	CpsRom[0x5A51] = 0x31;
	CpsRom[0x5A52] = 0xC;
	CpsRom[0x5A53] = 0x0;
	CpsRom[0x5A54] = 0x5C;
	CpsRom[0x5A55] = 0x11;
	CpsRom[0x5A56] = 0xF;
	CpsRom[0x5A58] = 0x5C;
	CpsRom[0x5A59] = 0x11;
	CpsRom[0x5A5A] = 0x11;
	CpsRom[0x5A5B] = 0x0;

	wof_decode();
}

static int wof3jsInit()
{
	pCpsInitCallback = wof3js_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWof3js = {
	"wof3js", "wof", NULL, "1997",
	"Sangokushi II: San Jian Sheng (Chinese bootleg)\0",NULL,"bootleg","CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u5251\u5723 (Chinese bootleg)\0Sangokushi II: San Jian Sheng (Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,Wof3jsRomInfo,Wof3jsRomName,wof3jsInputInfo, wof3jsDIPInfo,
	wof3jsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: San Sheng Jian (set 1, Chinese bootleg)

static struct BurnRomInfo Wof3sjRomDesc[] = {
	{ "k6b.040"     , 0x080000, 0x7b365108, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "k6a.040"     , 0x080000, 0x10488a51, 1 | BRF_ESS | BRF_PRG }, // 1

	{ "k6a.160"     , 0x200000, 0xa121180d, 2 | BRF_GRA }, // 2
	{ "k6b.160"     , 0x200000, 0xa4db96c4, 2 | BRF_GRA }, // 3

	{ "tk2_qa.rom"  , 0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // 4

	{ "tk2_q1.rom"  , 0x080000, 0x611268cf, 5 | BRF_SND }, // 5
	{ "tk2_q2.rom"  , 0x080000, 0x20f55ca9, 5 | BRF_SND }, // 6
	{ "tk2_q3.rom"  , 0x080000, 0xbfcf6f52, 5 | BRF_SND }, // 7
	{ "tk2_q4.rom"  , 0x080000, 0x36642e88, 5 | BRF_SND }, // 8
};

STD_ROM_PICK(Wof3sj) STD_ROM_FN(Wof3sj)

static void wof3sja_patch()
{
	// Disable Sprite Recoding
	CpsRom[0x5DE96] = 0x0;
	CpsRom[0x5DE97] = 0x61;
	CpsRom[0x5DE98] = 0xE6;
	CpsRom[0x5DE99] = 0x1;
	CpsRom[0x5DECE] = 0x6D;
	CpsRom[0x5DECF] = 0xC;
	CpsRom[0x5DED0] = 0x0;
	CpsRom[0x5DED1] = 0x90;
	CpsRom[0x5DED2] = 0xD2;
	CpsRom[0x5DED3] = 0xE3;
	CpsRom[0x5DF26] = 0x90;
	CpsRom[0x5DF27] = 0x0;
	CpsRom[0x5DF28] = 0x0;
	CpsRom[0x5DF29] = 0x0;
	CpsRom[0x5DF36] = 0x90;
	CpsRom[0x5DF37] = 0x0;
	CpsRom[0x5DF38] = 0x0;
	CpsRom[0x5DF39] = 0x40;

	// Patch Q sound protection? check
	CpsRom[0x5A0E] = 0x39;
	CpsRom[0x5A0F] = 0x30;
	CpsRom[0x5A10] = 0xF1;
	CpsRom[0x5A12] = 0xFE;
	CpsRom[0x5A13] = 0x9F;
	CpsRom[0x5A14] = 0x0;
	CpsRom[0x5A15] = 0xC;
	CpsRom[0x5A16] = 0x77;
	CpsRom[0x5A17] = 0x0;
	CpsRom[0x5A18] = 0xF4;
	CpsRom[0x5A19] = 0x66;
	CpsRom[0x5A1A] = 0xF9;
	CpsRom[0x5A1B] = 0x4E;
	CpsRom[0x5A1C] = 0x0;
	CpsRom[0x5A1E] = 0xBE;
	CpsRom[0x5A1F] = 0x72;
	CpsRom[0x5A20] = 0x7C;
	CpsRom[0x5A21] = 0x20;
	CpsRom[0x5A22] = 0xF1;
	CpsRom[0x5A23] = 0x0;
	CpsRom[0x5A24] = 0x0;
	CpsRom[0x5A25] = 0x80;
	CpsRom[0x5A26] = 0x7C;
	CpsRom[0x5A27] = 0x11;
	CpsRom[0x5A29] = 0x0;
	CpsRom[0x5A2A] = 0x1;
	CpsRom[0x5A2C] = 0x7C;
	CpsRom[0x5A2D] = 0x11;
	CpsRom[0x5A2E] = 0x10;
	CpsRom[0x5A2F] = 0x0;
	CpsRom[0x5A30] = 0xF;
	CpsRom[0x5A31] = 0x0;
	CpsRom[0x5A32] = 0x7C;
	CpsRom[0x5A33] = 0x11;
	CpsRom[0x5A34] = 0x0;
	CpsRom[0x5A35] = 0x0;
	CpsRom[0x5A36] = 0x7;
	CpsRom[0x5A38] = 0x7C;
	CpsRom[0x5A39] = 0x11;
	CpsRom[0x5A3A] = 0x0;
	CpsRom[0x5A3B] = 0x0;
	CpsRom[0x5A3C] = 0x9;
	CpsRom[0x5A3D] = 0x0;
	CpsRom[0x5A3E] = 0x7C;
	CpsRom[0x5A3F] = 0x11;
	CpsRom[0x5A40] = 0x0;
	CpsRom[0x5A41] = 0x0;
	CpsRom[0x5A42] = 0xC;
	CpsRom[0x5A44] = 0x41;
	CpsRom[0x5A48] = 0x7C;
	CpsRom[0x5A4A] = 0x0;
	CpsRom[0x5A4C] = 0x11;
	CpsRom[0x5A4D] = 0x0;
	CpsRom[0x5A4E] = 0x7C;
	CpsRom[0x5A4F] = 0x11;
	CpsRom[0x5A50] = 0x0;
	CpsRom[0x5A51] = 0x0;
	CpsRom[0x5A52] = 0x17;
	CpsRom[0x5A54] = 0x7C;
	CpsRom[0x5A56] = 0x0;
	CpsRom[0x5A58] = 0x19;
	CpsRom[0x5A59] = 0x0;
	CpsRom[0x5A5A] = 0x7C;
	CpsRom[0x5A5B] = 0x11;
	CpsRom[0x5A5C] = 0x0;
	CpsRom[0x5A5D] = 0x0;
	CpsRom[0x5A5E] = 0x13;
	CpsRom[0x5A60] = 0x7C;
	CpsRom[0x5A62] = 0x0;
	CpsRom[0x5A64] = 0x15;
	CpsRom[0x5A65] = 0x0;
	CpsRom[0x5A66] = 0x7C;
	CpsRom[0x5A67] = 0x11;
	CpsRom[0x5A68] = 0x0;
	CpsRom[0x5A69] = 0x0;
	CpsRom[0x5A6A] = 0x1F;
	CpsRom[0x5A6C] = 0xF9;
	CpsRom[0x5A6D] = 0x4E;
	CpsRom[0x5A6E] = 0x5;
	CpsRom[0x5A70] = 0x6A;
	CpsRom[0x5A71] = 0xEE;
	CpsRom[0x72A6] = 0xF9;
	CpsRom[0x72A8] = 0x0;
	CpsRom[0x72A9] = 0x0;
	CpsRom[0x72AA] = 0xE;
	CpsRom[0x72AB] = 0x5A;
	CpsRom[0x5EE64] = 0xF9;
	CpsRom[0x5EE65] = 0x4E;
	CpsRom[0x5EE66] = 0x0;
	CpsRom[0x5EE68] = 0x20;
	CpsRom[0x5EE69] = 0x5A;

	wof_decode();
}

static void wof3sj_decode()
{
	bootleg_progload();

	wof3sja_patch();
}

static int wof3sjInit()
{
	pCpsInitCallback = wof3sj_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWof3sj = {
	"wof3sj", "wof", NULL, "1997",
	"Sangokushi II: San Sheng Jian (set 1, Chinese bootleg)\0","Imperfect Sound","bootleg","CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u5723\u5251 (set 1, Chinese bootleg)\0Sangokushi II: San Sheng Jian (set 1, Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,Wof3sjRomInfo,Wof3sjRomName,wof3sjInputInfo, wof3jsDIPInfo,
	wof3sjInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: San Sheng Jian (set 2, Chinese bootleg)

static struct BurnRomInfo Wof3sjaRomDesc[] = {
	{ "3js.800",    0x100000, 0x812f9200, 1 | BRF_ESS | BRF_PRG }, // 0

	{ "k6a.160",    0x200000, 0xa121180d, 2 | BRF_GRA }, // 1
	{ "k6b.160",    0x200000, 0xa4db96c4, 2 | BRF_GRA }, // 2

	{ "tk2_qa.rom", 0x20000,  0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // 3

	{ "tk2_q1.rom", 0x80000,  0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom", 0x80000,  0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom", 0x80000,  0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom", 0x80000,  0x36642e88, 5 | BRF_SND },
};

STD_ROM_PICK(Wof3sja) STD_ROM_FN(Wof3sja)

static int wof3sjaInit()
{
	pCpsInitCallback = wof3sja_patch;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWof3sja = {
	"wof3sja", "wof", NULL, "1997",
	"Sangokushi II: San Sheng Jian (set 2, Chinese bootleg)\0","Imperfect Sound","bootleg","CPS1",
	L"\u4E09\u56FD\u5FD7II: \u4E09\u5723\u5251 (set 2, Chinese bootleg)\0Sangokushi II: San Sheng Jian (set 2, Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,Wof3sjaRomInfo,Wof3sjaRomName,wof3sjInputInfo, wof3jsDIPInfo,
	wof3sjaInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: Sheng Jian (set 1, Chinese bootleg)

static struct BurnRomInfo WofsjRomDesc[] = {
	{ "c-c47b.040"  ,  0x80000, 0xb1809761, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "a-2402.040"  ,  0x80000, 0x4fab4232, 1 | BRF_ESS | BRF_PRG }, // 1

	{ "tk2_gfx1.rom",  0x80000, 0x0d9cb9bf, 2 | BRF_GRA },			 // 2
	{ "tk2_gfx3.rom",  0x80000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom",  0x80000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom",  0x80000, 0xe349551c, 2 | BRF_GRA },
	{ "tk2_gfx5.rom",  0x80000, 0x291f0f0b, 2 | BRF_GRA },			 // 6
	{ "tk2_gfx7.rom",  0x80000, 0x3edeb949, 2 | BRF_GRA },
	{ "tk2_gfx6.rom",  0x80000, 0x1abd14d6, 2 | BRF_GRA },
	{ "tk2_gfx8.rom",  0x80000, 0xb27948e3, 2 | BRF_GRA },

	{ "tk2_qa.rom",    0x20000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG },	 // 10

	{ "tk2_q1.rom",    0x80000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",    0x80000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",    0x80000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",    0x80000, 0x36642e88, 5 | BRF_SND },
};

STD_ROM_PICK(Wofsj) STD_ROM_FN(Wofsj)

struct BurnDriver BurnDrvCpsWofsj = {
	"wofsj", "wof", NULL, "1995",
	"Sangokushi II: Sheng Jian (set 1, Chinese bootleg)\0","Imperfect Sound","bootleg","CPS1",
	L"\u4E09\u56FD\u5FD7II: \u5723\u5251 (set 1, Chinese bootleg)\0Sangokushi II: Sheng Jian (set 1, Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofsjRomInfo,WofsjRomName,wofsjInputInfo, wofDIPInfo,
	wof3sjInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: Sheng Jian (set 2, Chinese bootleg)

static struct BurnRomInfo WofsjaRomDesc[] = {
	{ "one.800",       0x100000,0x0507584d, 1 | BRF_ESS | BRF_PRG }, // 0

	{ "tk2_gfx1.rom",  0x80000, 0x0d9cb9bf, 2 | BRF_GRA },			 // 1
	{ "tk2_gfx3.rom",  0x80000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom",  0x80000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom",  0x80000, 0xe349551c, 2 | BRF_GRA },
	{ "tk2_gfx5.rom",  0x80000, 0x291f0f0b, 2 | BRF_GRA },			 // 5
	{ "tk2_gfx7.rom",  0x80000, 0x3edeb949, 2 | BRF_GRA },
	{ "tk2_gfx6.rom",  0x80000, 0x1abd14d6, 2 | BRF_GRA },
	{ "tk2_gfx8.rom",  0x80000, 0xb27948e3, 2 | BRF_GRA },

	{ "tk2_qa.rom",    0x20000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG },	 // 9

	{ "tk2_q1.rom",    0x80000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",    0x80000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",    0x80000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",    0x80000, 0x36642e88, 5 | BRF_SND },
};

STD_ROM_PICK(Wofsja) STD_ROM_FN(Wofsja)

struct BurnDriver BurnDrvCpsWofsja = {
	"wofsja", "wof", NULL, "1995",
	"Sangokushi II: Sheng Jian (set 2, Chinese bootleg)\0","Imperfect Sound","bootleg","CPS1",
	L"\u4E09\u56FD\u5FD7II: \u5723\u5251 (set 2, Chinese bootleg)\0Sangokushi II: Sheng Jian (set 2, Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofsjaRomInfo,WofsjaRomName,wofsjInputInfo, wofDIPInfo,
	wof3sjaInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Sangokushi II: Sheng Jian (set 3, Chinese bootleg)

static struct BurnRomInfo WofsjbRomDesc[] = {
	{ "c-d140.040",    0x80000, 0xe6d933a6, 1 | BRF_ESS | BRF_PRG }, // 0 swapped 68000 code
	{ "a-0050.040",    0x80000, 0x403eaead, 1 | BRF_ESS | BRF_PRG }, // 1 swapped

	{ "tk2_gfx1.rom",  0x80000, 0x0d9cb9bf, 2 | BRF_GRA },			 // 2
	{ "tk2_gfx3.rom",  0x80000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom",  0x80000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom",  0x80000, 0xe349551c, 2 | BRF_GRA },
	{ "tk2_gfx5.rom",  0x80000, 0x291f0f0b, 2 | BRF_GRA },			 // 6
	{ "tk2_gfx7.rom",  0x80000, 0x3edeb949, 2 | BRF_GRA },
	{ "tk2_gfx6.rom",  0x80000, 0x1abd14d6, 2 | BRF_GRA },
	{ "tk2_gfx8.rom",  0x80000, 0xb27948e3, 2 | BRF_GRA },

	{ "tk2_qa.rom",    0x20000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG },	 // 10

	{ "tk2_q1.rom",    0x80000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",    0x80000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",    0x80000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",    0x80000, 0x36642e88, 5 | BRF_SND },
};

STD_ROM_PICK(Wofsjb) STD_ROM_FN(Wofsjb)

static void wofsjb_decode()
{
	bootleg_progload();

	// GFX can be nearly 100% by patching in 0x5B6-0x6BA

	// QSound Fixes
	*((unsigned short*)(CpsRom + 0x5A1A)) = 0x6700;
	*((unsigned short*)(CpsRom + 0x5A1C)) = 0x0056;
	*((unsigned short*)(CpsRom + 0x5A1E)) = 0x207C;
	*((unsigned short*)(CpsRom + 0x5A20)) = 0x00F1;
	*((unsigned short*)(CpsRom + 0x5A22)) = 0x8000;
	*((unsigned short*)(CpsRom + 0x5A24)) = 0x4A28;
	*((unsigned short*)(CpsRom + 0x5A26)) = 0x001F;
	*((unsigned short*)(CpsRom + 0x5A28)) = 0x6A00;
	*((unsigned short*)(CpsRom + 0x5A40)) = 0x115C;
	*((unsigned short*)(CpsRom + 0x5A42)) = 0x0001;
	*((unsigned short*)(CpsRom + 0x5A44)) = 0x115C;
	*((unsigned short*)(CpsRom + 0x5A46)) = 0x0003;
	*((unsigned short*)(CpsRom + 0x5A4A)) = 0x0007;
	*((unsigned short*)(CpsRom + 0x5A4C)) = 0x115C;
	*((unsigned short*)(CpsRom + 0x5A4E)) = 0x0009;
	*((unsigned short*)(CpsRom + 0x5A50)) = 0x315C;
	*((unsigned short*)(CpsRom + 0x5A52)) = 0x000C;
	*((unsigned short*)(CpsRom + 0x5A54)) = 0x115C;
	*((unsigned short*)(CpsRom + 0x5A56)) = 0x000F;
	*((unsigned short*)(CpsRom + 0x5A58)) = 0x115C;
	*((unsigned short*)(CpsRom + 0x72A6)) = 0x4E71;
	*((unsigned short*)(CpsRom + 0x72A8)) = 0x4E71;
	*((unsigned short*)(CpsRom + 0x72AA)) = 0x4E71;
	*((unsigned short*)(CpsRom + 0x72AC)) = 0x4E71;
	*((unsigned short*)(CpsRom + 0x72AE)) = 0x4E71;
	*((unsigned short*)(CpsRom + 0x72B0)) = 0x4E71;
	*((unsigned short*)(CpsRom + 0x72B2)) = 0x3039;
	*((unsigned short*)(CpsRom + 0x72B4)) = 0x00F1;
	*((unsigned short*)(CpsRom + 0x72B6)) = 0x9FFE;
	*((unsigned short*)(CpsRom + 0x72B8)) = 0x0C00;

	wof_decode();
}

static int wofsjbInit()
{
	pCpsInitCallback = wofsjb_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWofsjb = {
	"wofsjb", "wof", NULL, "1995",
	"Sangokushi II: Sheng Jian (set 3, Chinese bootleg)\0",NULL,"bootleg","CPS1",
	L"\u4E09\u56FD\u5FD7II: \u5723\u5251 (set 3, Chinese bootleg)\0Sangokushi II: Sheng Jian (set 3, Chinese bootleg)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofsjbRomInfo,WofsjbRomName,wofsjbInputInfo, wofDIPInfo,
	wofsjbInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Warriors of Fate (bootleg)

static struct BurnRomInfo WofbRomDesc[] = {
	{ "3-f2ab.040",    0x80000, 0x61fd0a01, 1 | BRF_ESS | BRF_PRG }, // 0
	{ "1-9207.040",    0x80000, 0x7f59e24c, 1 | BRF_ESS | BRF_PRG }, // 1
	{ "4-d4d2.010",    0x20000, 0xfe5eee87, 1 | BRF_ESS | BRF_PRG }, // 2
	{ "2-6c41.010",    0x20000, 0x739379be, 1 | BRF_ESS | BRF_PRG }, // 3

	{ "5-caf3.040",    0x80000, 0xc8dcaa95, 2 | BRF_GRA },			 // 4
	{ "6-034f.040",    0x80000, 0x1ab0000c, 2 | BRF_GRA },
	{ "7-b0fa.040",    0x80000, 0x8425ff6b, 2 | BRF_GRA },
	{ "8-a6b7.040",    0x80000, 0x24ce197b, 2 | BRF_GRA },
	{ "9-8a2c.040",    0x80000, 0x9d20ef9b, 2 | BRF_GRA },			 // 8
	{ "10-7d24.040",   0x80000, 0x90c93dd2, 2 | BRF_GRA },
	{ "11-4171.040",   0x80000, 0x219fd7e2, 2 | BRF_GRA },
	{ "12-f56b.040",   0x80000, 0xefc17c9a, 2 | BRF_GRA },

	{ "tk2_qa.rom",    0x20000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG },	 // 12

	{ "tk2_q1.rom",    0x80000, 0x611268cf, 5 | BRF_SND },			 // 13
	{ "tk2_q2.rom",    0x80000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",    0x80000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",    0x80000, 0x36642e88, 5 | BRF_SND },
};

STD_ROM_PICK(Wofb) STD_ROM_FN(Wofb)

static void wofb_patch()
{
	// Fix gfx
	CpsRom[0x506] = 0xE7;
	CpsRom[0x507] = 0x48;
	CpsRom[0x508] = 0xFE;
	CpsRom[0x509] = 0xFF;
	CpsRom[0x50A] = 0xF8;
	CpsRom[0x50B] = 0x4B;
	CpsRom[0x6ABC] = 0x6;
	CpsRom[0x6ABD] = 0x1;
	// Fix sound
	CpsRom[0x764E] = 0x71;
	CpsRom[0x7650] = 0x71;
	CpsRom[0x7651] = 0x4E;
	// Fix screen transitions
	CpsRom[0x5D236] = 0xFC;
	CpsRom[0x5D237] = 0x28;
	CpsRom[0x5D238] = 0x0;
	CpsRom[0x5D23B] = 0x1;

	wof_decode();
}

static void wofb_decode()
{
	bootleg_progload();

	wofb_patch();
}

static int wofbInit()
{
	pCpsInitCallback = wofb_decode;

	return CpsInit();
}

struct BurnDriver BurnDrvCpsWofb = {
	"wofb", "wof", NULL, "1992",
	"Warriors of Fate (bootleg)\0",NULL,"bootleg","CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,3,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofbRomInfo,WofbRomName,wofInputInfo, wofDIPInfo,
	wofbInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


//------------------------------------------------------------------------------------------
// CPS Changer hardware
//


// Tenchi wo Kurau II - Sekiheki no Tatakai (CPS Changer Japan 921031)

static struct BurnRomInfo WofchRomDesc[] = {
	{ "wofch_23.rom", 0x080000, 0x4e0b8dee, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "wofch_22.rom", 0x080000, 0xd0937a8d, 1 | BRF_ESS | BRF_PRG },

	{ "tk2_gfx1.rom", 0x080000, 0x0d9cb9bf, 2 | BRF_GRA },
	{ "tk2_gfx3.rom", 0x080000, 0x45227027, 2 | BRF_GRA },
	{ "tk2_gfx2.rom", 0x080000, 0xc5ca2460, 2 | BRF_GRA },
	{ "tk2_gfx4.rom", 0x080000, 0xe349551c, 2 | BRF_GRA },
	{ "tk205.bin",    0x080000, 0xe4a44d53, 2 | BRF_GRA },
	{ "tk206.bin",    0x080000, 0x58066ba8, 2 | BRF_GRA },
	{ "wofch_07.rom", 0x080000, 0xcc9006c9, 2 | BRF_GRA },
	{ "tk208.bin",    0x080000, 0xd4a19a02, 2 | BRF_GRA },

	{ "tk2_qa.rom",   0x020000, 0xc9183a0d, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "tk2_q1.rom",   0x080000, 0x611268cf, 5 | BRF_SND },
	{ "tk2_q2.rom",   0x080000, 0x20f55ca9, 5 | BRF_SND },
	{ "tk2_q3.rom",   0x080000, 0xbfcf6f52, 5 | BRF_SND },
	{ "tk2_q4.rom",   0x080000, 0x36642e88, 5 | BRF_SND },
};

STD_ROM_PICK(Wofch) STD_ROM_FN(Wofch)

struct BurnDriver BurnDrvCpsWofch = {
	"wofch", "wof", NULL, "1992",
	"Tenchi wo Kurau II - Sekiheki no Tatakai (CPS Changer Japan 921031)\0", NULL, "Capcom", "CPS Changer",
	L"\u8EE2\u5730\u3092\u55B0\u3089\u3046II - \u8D64\u58C1\u306E\u6226\u3044 (CPS Changer Japan 921031)\0Tenchi wo Kurau II - Sekiheki no Tatakai\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPS1_QSOUND,
	NULL,WofchRomInfo,WofchRomName,wofchInputInfo, NULL,
	wofInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter Zero (CPS Changer 951020)

static struct BurnRomInfo sfzchRomDesc[] = {
	{ "sfzch23", 0x080000, 0x1140743f, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sfza22",  0x080000, 0x8d9b2480, 1 | BRF_ESS | BRF_PRG },
	{ "sfzch21", 0x080000, 0x5435225d, 1 | BRF_ESS | BRF_PRG },
	{ "sfza20",  0x080000, 0x806e8f38, 1 | BRF_ESS | BRF_PRG },

	{ "sfz01",   0x080000, 0x0dd53e62, 2 | BRF_GRA },
	{ "sfz02",   0x080000, 0x94c31e3f, 2 | BRF_GRA },
	{ "sfz03",   0x080000, 0x9584ac85, 2 | BRF_GRA },
	{ "sfz04",   0x080000, 0xb983624c, 2 | BRF_GRA },
	{ "sfz05",   0x080000, 0x2b47b645, 2 | BRF_GRA },
	{ "sfz06",   0x080000, 0x74fd9fb1, 2 | BRF_GRA },
	{ "sfz07",   0x080000, 0xbb2c734d, 2 | BRF_GRA },
	{ "sfz08",   0x080000, 0x454f7868, 2 | BRF_GRA },
	{ "sfz10",   0x080000, 0x2a7d675e, 2 | BRF_GRA },
	{ "sfz11",   0x080000, 0xe35546c8, 2 | BRF_GRA },
	{ "sfz12",   0x080000, 0xf122693a, 2 | BRF_GRA },
	{ "sfz13",   0x080000, 0x7cf942c8, 2 | BRF_GRA },
	{ "sfz14",   0x080000, 0x09038c81, 2 | BRF_GRA },
	{ "sfz15",   0x080000, 0x1aa17391, 2 | BRF_GRA },
	{ "sfz16",   0x080000, 0x19a5abd6, 2 | BRF_GRA },
	{ "sfz17",   0x080000, 0x248b3b73, 2 | BRF_GRA },

	{ "sfz09",   0x010000, 0xc772628b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sfz18",   0x020000, 0x61022b2d, 4 | BRF_SND },
	{ "sfz19",   0x020000, 0x3b5886d5, 4 | BRF_SND },
};

STD_ROM_PICK(sfzch) STD_ROM_FN(sfzch)

struct BurnDriver BurnDrvCpsSfzch = {
	"sfzch", NULL, NULL, "1995",
	"Street Fighter Zero (CPS Changer 951020)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_CAPCOM_CPSCHANGER,
	NULL,sfzchRomInfo,sfzchRomName,SfzchInputInfo, NULL,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter Alpha (Publicity US 950727)

static struct BurnRomInfo SfachRomDesc[] = {
	{ "sfach23", 0x080000, 0x02a1a853, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "sfza22",  0x080000, 0x8d9b2480, 1 | BRF_ESS | BRF_PRG },
	{ "sfzch21", 0x080000, 0x5435225d, 1 | BRF_ESS | BRF_PRG },
	{ "sfza20",  0x080000, 0x806e8f38, 1 | BRF_ESS | BRF_PRG },

	{ "sfz01",   0x080000, 0x0dd53e62, 2 | BRF_GRA },
	{ "sfz02",   0x080000, 0x94c31e3f, 2 | BRF_GRA },
	{ "sfz03",   0x080000, 0x9584ac85, 2 | BRF_GRA },
	{ "sfz04",   0x080000, 0xb983624c, 2 | BRF_GRA },
	{ "sfz05",   0x080000, 0x2b47b645, 2 | BRF_GRA },
	{ "sfz06",   0x080000, 0x74fd9fb1, 2 | BRF_GRA },
	{ "sfz07",   0x080000, 0xbb2c734d, 2 | BRF_GRA },
	{ "sfz08",   0x080000, 0x454f7868, 2 | BRF_GRA },
	{ "sfz10",   0x080000, 0x2a7d675e, 2 | BRF_GRA },
	{ "sfz11",   0x080000, 0xe35546c8, 2 | BRF_GRA },
	{ "sfz12",   0x080000, 0xf122693a, 2 | BRF_GRA },
	{ "sfz13",   0x080000, 0x7cf942c8, 2 | BRF_GRA },
	{ "sfz14",   0x080000, 0x09038c81, 2 | BRF_GRA },
	{ "sfz15",   0x080000, 0x1aa17391, 2 | BRF_GRA },
	{ "sfz16",   0x080000, 0x19a5abd6, 2 | BRF_GRA },
	{ "sfz17",   0x080000, 0x248b3b73, 2 | BRF_GRA },

	{ "sfz09",   0x010000, 0xc772628b, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "sfz18",   0x020000, 0x61022b2d, 4 | BRF_SND },
	{ "sfz19",   0x020000, 0x3b5886d5, 4 | BRF_SND },
};

STD_ROM_PICK(Sfach) STD_ROM_FN(Sfach)

struct BurnDriver BurnDrvCpsSfach = {
	"sfach", "sfzch", NULL, "1995",
	"Street Fighter Alpha (Publicity US 950727)\0", NULL, "Capcom", "CPS Changer",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE,2,HARDWARE_CAPCOM_CPSCHANGER,
	NULL,SfachRomInfo,SfachRomName,SfzchInputInfo, NULL,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


//------------------------------------------------------------------------------------------
// Region Hacks
//


// Pnickies (US 940608)

static struct BurnRomInfo PnickuRomDesc[] = {
	{ "pnij36.bin", 0x020000, 0x2d4ffb2b, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "pniu42.bin", 0x020000, 0x22d20227, 1 | BRF_ESS | BRF_PRG }, // 1 odd

	{ "pnij09.bin", 0x020000, 0x48177b0a, 2 | BRF_GRA },
	{ "pnij01.bin", 0x020000, 0x01a0f311, 2 | BRF_GRA },
	{ "pnij13.bin", 0x020000, 0x406451b0, 2 | BRF_GRA },
	{ "pnij05.bin", 0x020000, 0x8c515dc0, 2 | BRF_GRA },
	{ "pnij26.bin", 0x020000, 0xe2af981e, 2 | BRF_GRA },
	{ "pnij18.bin", 0x020000, 0xf17a0e56, 2 | BRF_GRA },
	{ "pnij38.bin", 0x020000, 0xeb75bd8c, 2 | BRF_GRA },
	{ "pnij32.bin", 0x020000, 0x84560bef, 2 | BRF_GRA },
	{ "pnij10.bin", 0x020000, 0xc2acc171, 2 | BRF_GRA },
	{ "pnij02.bin", 0x020000, 0x0e21fc33, 2 | BRF_GRA },
	{ "pnij14.bin", 0x020000, 0x7fe59b19, 2 | BRF_GRA },
	{ "pnij06.bin", 0x020000, 0x79f4bfe3, 2 | BRF_GRA },
	{ "pnij27.bin", 0x020000, 0x83d5cb0e, 2 | BRF_GRA },
	{ "pnij19.bin", 0x020000, 0xaf08b230, 2 | BRF_GRA },
	{ "pnij39.bin", 0x020000, 0x70fbe579, 2 | BRF_GRA },
	{ "pnij33.bin", 0x020000, 0x3ed2c680, 2 | BRF_GRA },

	{ "pnij17.bin", 0x010000, 0xe86f787a, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "pnij24.bin", 0x020000, 0x5092257d, 4 | BRF_SND },
	{ "pnij25.bin", 0x020000, 0x22109aaa, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Pnicku) STD_ROM_FN(Pnicku)

struct BurnDriver BurnDrvCpsPnicku = {
	"pnicku", "pnickj", NULL, "1994",
	"Pnickies (US 940608)\0", NULL, "Compile (Capcom license)", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,2,HARDWARE_CAPCOM_CPS1,
	NULL,PnickuRomInfo,PnickuRomName,pnickjInputInfo, pnickjDIPInfo,
	CpsInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Street Fighter II': Champion Edition (Hispanic 990804)

static struct BurnRomInfo Sf2cehRomDesc[] = {
	{"sf2ceh.23",   0x080000, 0x25dc14c8, 1 | BRF_ESS | BRF_PRG }, //  68k program
	{"sf2ceh.22",   0x080000, 0x1c9dd91c, 1 | BRF_ESS | BRF_PRG }, //
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92_10.bin",  0x080000, 0x960687d5, 2 | BRF_GRA },
	{"s92_11.bin",  0x080000, 0x978ecd18, 2 | BRF_GRA },
	{"s92_12.bin",  0x080000, 0xd6ec9a0a, 2 | BRF_GRA },
	{"s92_13.bin",  0x080000, 0xed2c67f6, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ceh) STD_ROM_FN(Sf2ceh)

struct BurnDriver BurnDrvCpsSf2ceh = {
	"sf2ceh", "sf2ce", NULL, "1992",
	"Street Fighter II': Champion Edition (Hispanic 990804)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2cehRomInfo, Sf2cehRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 19, 14
};


// Street Fighter II': Champion Edition (Brasil 920313)

static struct BurnRomInfo Sf2cebRomDesc[] = {
	{"sf2ceb.23",   0x080000, 0x74e848ee, 1 | BRF_ESS | BRF_PRG }, //  68k program
	{"sf2ceb.22",   0x080000, 0xc3c49626, 1 | BRF_ESS | BRF_PRG }, //
	{"s92_21a.bin", 0x080000, 0x925a7877, 1 | BRF_ESS | BRF_PRG },

	{"s92_01.bin",  0x080000, 0x03b0d852, 2 | BRF_GRA },
	{"s92_02.bin",  0x080000, 0x840289ec, 2 | BRF_GRA },
	{"s92_03.bin",  0x080000, 0xcdb5f027, 2 | BRF_GRA },
	{"s92_04.bin",  0x080000, 0xe2799472, 2 | BRF_GRA },
	{"s92_05.bin",  0x080000, 0xba8a2761, 2 | BRF_GRA },
	{"s92_06.bin",  0x080000, 0xe584bfb5, 2 | BRF_GRA },
	{"s92_07.bin",  0x080000, 0x21e3f87d, 2 | BRF_GRA },
	{"s92_08.bin",  0x080000, 0xbefc47df, 2 | BRF_GRA },
	{"s92b_10.bin", 0x080000, 0xb3e1dd5f, 2 | BRF_GRA },
	{"s92b_11.bin", 0x080000, 0xf13af812, 2 | BRF_GRA },
	{"s92b_12.bin", 0x080000, 0x10ce42af, 2 | BRF_GRA },
	{"s92b_13.bin", 0x080000, 0x32cf5af3, 2 | BRF_GRA },

	{"s92_09.bin",  0x010000, 0x08f6b60e, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{"s92_18.bin",  0x020000, 0x7f162009, 4 | BRF_SND },
	{"s92_19.bin",  0x020000, 0xbeade53f, 4 | BRF_SND },

	A_BOARD_PLDS
};

STD_ROM_PICK(Sf2ceb) STD_ROM_FN(Sf2ceb)

struct BurnDriver BurnDrvCpsSf2ceb = {
	"sf2ceb", "sf2ce", NULL, "1992",
	"Street Fighter II': Champion Edition (Brasil 920313)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_CAPCOM_CPS1,
	NULL, Sf2cebRomInfo, Sf2cebRomName, sf2InputInfo, sf2DIPInfo,
	CpsInit, CpsExit, Cps1Frame, CpsRedraw, CpsAreaScan,
	&CpsRecalcPal, 384, 224, 19, 14
};


// Saturday Night Slam Masters (Asia 930301)

static struct BurnRomInfo SlammastaRomDesc[] = {
	{ "mba_23e.rom",  0x080000, 0x027e49db, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbe_24b.rom",  0x020000, 0x95d5e729, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_28b.rom",  0x020000, 0xb1c7cbcb, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_25b.rom",  0x020000, 0xa50d3fd4, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_29b.rom",  0x020000, 0x08e32e56, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_21a.rom",  0x080000, 0xd5007b05, 1 | BRF_ESS | BRF_PRG },
	{ "mbu-20a.rom",  0x080000, 0xfc848af5, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_qa.rom",    0x020000, 0xe21a03c4, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Slammasta) STD_ROM_FN(Slammasta)

struct BurnDriver BurnDrvCpsSlammasta = {
	"slammasta", "slammast", NULL, "1993",
	"Saturday Night Slam Masters (Asia 930301)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,4,HARDWARE_CAPCOM_CPS1,
	NULL,SlammastaRomInfo,SlammastaRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Saturday Night Slam Masters (Hispanic 930713)

static struct BurnRomInfo SlammasthRomDesc[] = {
	{ "mbh_23e.rom",  0x080000, 0xb0220f4a, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbe_24b.rom",  0x020000, 0x95d5e729, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_28b.rom",  0x020000, 0xb1c7cbcb, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_25b.rom",  0x020000, 0xa50d3fd4, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_29b.rom",  0x020000, 0x08e32e56, 1 | BRF_ESS | BRF_PRG },
	{ "mbe_21a.rom",  0x080000, 0xd5007b05, 1 | BRF_ESS | BRF_PRG },
	{ "mbu-20a.rom",  0x080000, 0xfc848af5, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_qa.rom",    0x020000, 0xe21a03c4, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Slammasth) STD_ROM_FN(Slammasth)

struct BurnDriver BurnDrvCpsSlammasth = {
	"slammasth", "slammast", NULL, "1993",
	"Saturday Night Slam Masters (Hispanic 930713)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,4,HARDWARE_CAPCOM_CPS1,
	NULL,SlammasthRomInfo,SlammasthRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Muscle Bomber Duo - Ultimate Team Battle (USA 931206)

static struct BurnRomInfo MbombrduRomDesc[] = {
	{ "mbd_26.bin",   0x020000, 0x72b7451c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbdu_30.rom",  0x020000, 0x5c234403, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_27.bin",   0x020000, 0x4086f534, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_31.bin",   0x020000, 0x085f47f0, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_24.bin",   0x020000, 0xc20895a5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_28.bin",   0x020000, 0x2618d5e1, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_25.bin",   0x020000, 0x9bdb6b11, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_29.bin",   0x020000, 0x3f52d5e5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_21.bin",   0x080000, 0x690c026a, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_20.bin",   0x080000, 0xb8b2139b, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_q.bin",     0x020000, 0xd6fa76d1, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Mbombrdu) STD_ROM_FN(Mbombrdu)

struct BurnDriver BurnDrvCpsMbombrdu = {
	"mbombrdu", "mbombrd", NULL, "1993",
	"Muscle Bomber Duo - Ultimate Team Battle (USA 931206)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,4,HARDWARE_CAPCOM_CPS1,
	NULL,MbombrduRomInfo,MbombrduRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Muscle Bomber Duo - Ultimate Team Battle (Asia 931206)

static struct BurnRomInfo MbombrdaRomDesc[] = {
	{ "mbd_26.bin",   0x020000, 0x72b7451c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbda_30.rom",  0x020000, 0x42eaa9da, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_27.bin",   0x020000, 0x4086f534, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_31.bin",   0x020000, 0x085f47f0, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_24.bin",   0x020000, 0xc20895a5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_28.bin",   0x020000, 0x2618d5e1, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_25.bin",   0x020000, 0x9bdb6b11, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_29.bin",   0x020000, 0x3f52d5e5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_21.bin",   0x080000, 0x690c026a, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_20.bin",   0x080000, 0xb8b2139b, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_q.bin",     0x020000, 0xd6fa76d1, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Mbombrda) STD_ROM_FN(Mbombrda)

struct BurnDriver BurnDrvCpsMbombrda = {
	"mbombrda", "mbombrd", NULL, "1993",
	"Muscle Bomber Duo - Ultimate Team Battle (Asia 931206)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,4,HARDWARE_CAPCOM_CPS1,
	NULL,MbombrdaRomInfo,MbombrdaRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Muscle Bomber Duo - Ultimate Team Battle (Hispanic 931206)

static struct BurnRomInfo MbombrdhRomDesc[] = {
	{ "mbd_26.bin",   0x020000, 0x72b7451c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbdh_30.rom",  0x020000, 0x836cea7d, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_27.bin",   0x020000, 0x4086f534, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_31.bin",   0x020000, 0x085f47f0, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_24.bin",   0x020000, 0xc20895a5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_28.bin",   0x020000, 0x2618d5e1, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_25.bin",   0x020000, 0x9bdb6b11, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_29.bin",   0x020000, 0x3f52d5e5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_21.bin",   0x080000, 0x690c026a, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_20.bin",   0x080000, 0xb8b2139b, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_q.bin",     0x020000, 0xd6fa76d1, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Mbombrdh) STD_ROM_FN(Mbombrdh)

struct BurnDriver BurnDrvCpsMbombrdh = {
	"mbombrdh", "mbombrd", NULL, "1993",
	"Muscle Bomber Duo - Ultimate Team Battle (Hispanic 931206)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,4,HARDWARE_CAPCOM_CPS1,
	NULL,MbombrdhRomInfo,MbombrdhRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};


// Muscle Bomber Duo - Ultimate Team Battle (Japan E 931206)

static struct BurnRomInfo MbombrdjeRomDesc[] = {
	{ "mbd_26.bin",   0x020000, 0x72b7451c, 1 | BRF_ESS | BRF_PRG }, // 68k program
	{ "mbdje_30.rom", 0x020000, 0x61b09fb1, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_27.bin",   0x020000, 0x4086f534, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_31.bin",   0x020000, 0x085f47f0, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_24.bin",   0x020000, 0xc20895a5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_28.bin",   0x020000, 0x2618d5e1, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_25.bin",   0x020000, 0x9bdb6b11, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_29.bin",   0x020000, 0x3f52d5e5, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_21.bin",   0x080000, 0x690c026a, 1 | BRF_ESS | BRF_PRG },
	{ "mbd_20.bin",   0x080000, 0xb8b2139b, 1 | BRF_ESS | BRF_PRG },

	{ "mb_gfx01.rom", 0x080000, 0x41468e06, 2 | BRF_GRA },
	{ "mb_gfx03.rom", 0x080000, 0xf453aa9e, 2 | BRF_GRA },
	{ "mb_gfx02.rom", 0x080000, 0x2ffbfea8, 2 | BRF_GRA },
	{ "mb_gfx04.rom", 0x080000, 0x1eb9841d, 2 | BRF_GRA },
	{ "mb_05.bin",    0x080000, 0x506b9dc9, 2 | BRF_GRA },
	{ "mb_07.bin",    0x080000, 0xaff8c2fb, 2 | BRF_GRA },
	{ "mb_06.bin",    0x080000, 0xb76c70e9, 2 | BRF_GRA },
	{ "mb_08.bin",    0x080000, 0xe60c9556, 2 | BRF_GRA },
	{ "mb_10.bin",    0x080000, 0x97976ff5, 2 | BRF_GRA },
	{ "mb_12.bin",    0x080000, 0xb350a840, 2 | BRF_GRA },
	{ "mb_11.bin",    0x080000, 0x8fb94743, 2 | BRF_GRA },
	{ "mb_13.bin",    0x080000, 0xda810d5f, 2 | BRF_GRA },

	{ "mb_q.bin",     0x020000, 0xd6fa76d1, 3 | BRF_ESS | BRF_PRG }, // z80 program

	{ "mb_q1.bin",    0x080000, 0x0630c3ce, 5 | BRF_SND },
	{ "mb_q2.bin",    0x080000, 0x354f9c21, 5 | BRF_SND },
	{ "mb_q3.bin",    0x080000, 0x7838487c, 5 | BRF_SND },
	{ "mb_q4.bin",    0x080000, 0xab66e087, 5 | BRF_SND },
	{ "mb_q5.bin",    0x080000, 0xc789fef2, 5 | BRF_SND },
	{ "mb_q6.bin",    0x080000, 0xecb81b61, 5 | BRF_SND },
	{ "mb_q7.bin",    0x080000, 0x041e49ba, 5 | BRF_SND },
	{ "mb_q8.bin",    0x080000, 0x59fe702a, 5 | BRF_SND },

	A_BOARD_QSOUND_PLDS
};

STD_ROM_PICK(Mbombrdje) STD_ROM_FN(Mbombrdje)

struct BurnDriver BurnDrvCpsMbombrdje = {
	"mbombrdje", "mbombrd", NULL, "1993",
	"Muscle Bomber Duo - Ultimate Team Battle (Japan E 931206)\0", NULL, "Capcom", "CPS1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG,4,HARDWARE_CAPCOM_CPS1,
	NULL,MbombrdjeRomInfo,MbombrdjeRomName,slammastInputInfo, slammastDIPInfo,
	SlammastInit,CpsExit,Cps1Frame,CpsRedraw,CpsAreaScan,
	&CpsRecalcPal,384,224,4,3
};
