extern int TaitoIC_SupermanCChipInUse;
extern int TaitoIC_MegabCChipInUse;
extern int TaitoIC_RainbowCChipInUse;
extern int TaitoIC_OpwolfCChipInUse;
extern int TaitoIC_VolfiedCChipInUse;

extern int TaitoIC_PC080SNInUse;
extern int TaitoIC_PC090OJInUse;
extern int TaitoIC_TC0100SCNInUse;
extern int TaitoIC_TC0110PCRInUse;
extern int TaitoIC_TC0140SYTInUse;
extern int TaitoIC_TC0150RODInUse;
extern int TaitoIC_TC0180VCUInUse;
extern int TaitoIC_TC0220IOCInUse;
extern int TaitoIC_TC0280GRDInUse;
extern int TaitoIC_TC0360PRIInUse;
extern int TaitoIC_TC0430GRWInUse;
extern int TaitoIC_TC0480SCPInUse;
extern int TaitoIC_TC0510NIOInUse;
extern int TaitoIC_TC0640FIOInUse;

extern int TaitoWatchdog;

void TaitoICReset();
void TaitoICExit();
void TaitoICScan(int nAction);

// C-Chip
UINT16 SupermanCChipCtrlRead();
UINT16 SupermanCChipRamRead(unsigned int Offset, unsigned char Input1, unsigned char Input2, unsigned char Input3);
void SupermanCChipCtrlWrite();
void SupermanCChipBankWrite(UINT16 Data);
void SupermanCChipRamWrite(unsigned int Offset, UINT16 Data);
void SupermanCChipReset();
void SupermanCChipInit();
void SupermanCChipExit();
void SupermanCChipScan(int nAction);

UINT16 MegabCChipRead(unsigned int Offset);
void MegabCChipWrite(unsigned int Offset, UINT16 Data);
void MegabCChipReset();
void MegabCChipInit();
void MegabCChipExit();
void MegabCChipScan(int nAction);

void RainbowCChipUpdate(unsigned char Input1, unsigned char Input2, unsigned char Input3, unsigned char Input4);
UINT16 RainbowCChipCtrlRead();
UINT16 RainbowCChipRamRead(unsigned int Offset);
void RainbowCChipCtrlWrite(UINT16);
void RainbowCChipBankWrite(UINT16 Data);
void RainbowCChipRamWrite(unsigned int Offset, UINT16 Data);
void RainbowCChipReset();
void RainbowCChipInit(int Version);
void RainbowCChipExit();
void RainbowCChipScan(int nAction);

void OpwolfCChipUpdate(unsigned char Input1, unsigned char Input2);
UINT16 OpwolfCChipStatusRead();
UINT16 OpwolfCChipDataRead(unsigned int Offset);
void OpwolfCChipStatusWrite();
void OpwolfCChipBankWrite(UINT16 Data);
void OpwolfCChipDataWrite(unsigned char *p68kRom, unsigned int Offset, UINT16 Data );
void OpwolfCChipReset();
void OpwolfCChipInit(int Region);
void OpwolfCChipExit();
void OpwolfCChipScan(int nAction);

void BonzeWriteCChipRam(int offset, int data);
void BonzeWriteCChipBank(int data);
unsigned short BonzeReadCChipRam(int offset);
void BonzeCChipReset();
void BonzeCChipScan(int nAction);

void VolfiedCChipBankWrite(UINT16 data);
void VolfiedCChipRamWrite(int offset, UINT8 data);
UINT8 VolfiedCChipCtrlRead();
UINT8 VolfiedCChipRamRead(int offset);
void VolfiedCChipInit();
void VolfiedCChipReset();
void VolfiedCChipExit();
void VolfiedCChipScan(int nAction);

// PC080SN
#define PC080SN_MAX_CHIPS 2

extern unsigned char *PC080SNRam[PC080SN_MAX_CHIPS];

void PC080SNDrawBgLayer(int Chip, int Opaque, unsigned char *pSrc, unsigned short *pDest);
void PC080SNDrawFgLayer(int Chip, int Opaque, unsigned char *pSrc, unsigned short *pDest);
void PC080SNSetScrollX(int Chip, unsigned int Offset, UINT16 Data);
void PC080SNSetScrollY(int Chip, unsigned int Offset, UINT16 Data);
void PC080SNCtrlWrite(int Chip, unsigned int Offset, UINT16 Data);
void PC080SNOverrideFgScroll(int Chip, int xScroll, int yScroll);
void PC080SNReset();
void PC080SNInit(int Chip, int nNumTiles, int xOffset, int yOffset, int yInvert, int DblWidth);
void PC080SNSetFgTransparentPen(int Chip, int Pen);
void PC080SNExit();
void PC080SNScan(int nAction);
void TopspeedDrawBgLayer(int Chip, unsigned char *pSrc, unsigned short *pDest, UINT16 *ColourCtrlRam);
void TopspeedDrawFgLayer(int Chip, unsigned char *pSrc, unsigned short *pDest, UINT16 *ColourCtrlRam);

// PC090OJ
extern unsigned char *PC090OJRam;
extern int PC090OJSpriteCtrl;

void PC090OJDrawSprites(unsigned char *pSrc);
void PC090OJReset();
void PC090OJInit(int nNumTiles, int xOffset, int yOffset, int UseBuffer);
void PC090OJSetPaletteOffset(int Offset);
void PC090OJExit();
void PC090OJScan(int nAction);

// TC0100SCN
#define TC0100SCN_MAX_CHIPS 3

extern unsigned char *TC0100SCNRam[TC0100SCN_MAX_CHIPS];
extern UINT16 TC0100SCNCtrl[TC0100SCN_MAX_CHIPS][8];
extern unsigned char TC0100SCNBgLayerUpdate[TC0100SCN_MAX_CHIPS];
extern unsigned char TC0100SCNFgLayerUpdate[TC0100SCN_MAX_CHIPS];

void TC0100SCNCtrlWordWrite(int Chip, unsigned int Offset, UINT16 Data);
int TC0100SCNBottomLayer(int Chip);
void TC0100SCNRenderBgLayer(int Chip, int Opaque, unsigned char *pSrc);
void TC0100SCNRenderFgLayer(int Chip, int Opaque, unsigned char *pSrc);
void TC0100SCNRenderCharLayer(int Chip);
void TC0100SCNReset();
void TC0100SCNInit(int Chip, int nNumTiles, int xOffset, int yOffset, int xFlip, unsigned char *PriorityMap);
void TC0100SCNSetColourDepth(int Chip, int ColourDepth);
void TC0100SCNSetGfxMask(int Chip, int Mask);
void TC0100SCNSetGfxBank(int Chip, int Bank);
void TC0100SCNSetClipArea(int Chip, int ClipWidth, int ClipHeight, int ClipStartX);
void TC0100SCNSetPaletteOffset(int Chip, int PaletteOffset);
void TC0100SCNExit();
void TC0100SCNScan(int nAction);

// TC0110PCR
extern unsigned int *TC0110PCRPalette;
extern int TC0110PCRTotalColours;

UINT16 TC0110PCRWordRead(int Chip);
void TC0110PCRWordWrite(int Chip, int Offset, UINT16 Data);
void TC0110PCRStep1WordWrite(int Chip, int Offset, UINT16 Data);
void TC0110PCRStep1RBSwapWordWrite(int Chip, int Offset, UINT16 Data);
void TC0110PCRStep14rbgWordWrite(int Chip, int Offset, UINT16 Data);
void TC0110PCRReset();
void TC0110PCRInit(int Num, int nNumColours);
void TC0110PCRExit();
void TC0110PCRScan(int nAction);

// TC0140SYT
void TC0140SYTPortWrite(UINT8 Data);
UINT8 TC0140SYTCommRead();
void TC0140SYTCommWrite(UINT8 Data);
void TC0140SYTSlavePortWrite(UINT8 Data);
UINT8 TC0140SYTSlaveCommRead();
void TC0140SYTSlaveCommWrite(UINT8 Data);
void TC0140SYTReset();
void TC0140SYTInit();
void TC0140SYTExit();
void TC0140SYTScan(int nAction);

// TC0150ROD
extern unsigned char *TC0150RODRom;
extern unsigned char *TC0150RODRam;

void TC0150RODDraw(int yOffs, int pOffs, int Type, int RoadTrans, int LowPriority, int HighPriority);
void TC0150RODReset();
void TC0150RODInit(int nRomSize, int xFlip);
void TC0150RODExit();
void TC0150RODScan(int nAction);

// TC0180VCU
extern unsigned char *TC0180VCURAM;
extern unsigned char *TC0180VCUScrollRAM;
extern unsigned char *TC0180VCUFbRAM;

void TC0180VCUInit(unsigned char *gfx0, int mask0, unsigned char *gfx1, int mask1, int global_x, int global_y);
void TC0180VCUReset();
void TC0180VCUExit();
void TC0180VCUScan(int nAction);

void TC0180VCUDrawCharLayer(int colorbase);
void TC0180VCUDrawLayer(int colorbase, int ctrl_offset, int transparent);

void TC0180VCUFramebufferDraw(int priority, int color_base);
void TC0180VCUDrawSprite(unsigned short *dest);
void TC0180VCUBufferSprites();

unsigned short TC0180VCUFramebufferRead(int offset);
void TC0180VCUFramebufferWrite(int offset);

unsigned char TC0180VCUReadRegs(int offset);
void TC0180VCUWriteRegs(int offset, int data);
unsigned char TC0180VCUReadControl();

// TC0220IOC
extern unsigned char TC0220IOCInputPort0[8];
extern unsigned char TC0220IOCInputPort1[8];
extern unsigned char TC0220IOCInputPort2[8];
extern unsigned char TC0220IOCDip[2];
extern unsigned char TC0220IOCInput[3];

UINT8 TC0220IOCPortRead();
UINT8 TC0220IOCHalfWordPortRead();
UINT8 TC0220IOCPortRegRead();
UINT8 TC0220IOCHalfWordRead(int Offset);
UINT8 TC0220IOCRead(UINT8 Port);
void TC0220IOCWrite(UINT8 Port, UINT8 Data);
void TC0220IOCHalfWordPortRegWrite(UINT16 Data);
void TC0220IOCHalfWordPortWrite(UINT16 Data);
void TC0220IOCHalfWordWrite(int Offset, UINT16 Data);
void TC0220IOCReset();
void TC0220IOCInit();
void TC0220IOCExit();
void TC0220IOCScan(int nAction);

// TC0280GRD
extern unsigned char *TC0280GRDRam;
extern int TC0280GRDBaseColour;

void TC0280GRDRenderLayer();
void TC0280GRDCtrlWordWrite(unsigned int Offset, UINT16 Data);
void TC0280GRDReset();
void TC0280GRDInit(int xOffs, int yOffs, unsigned char *pSrc);
void TC0430GRWInit(int xOffs, int yOffs, unsigned char *pSrc);
void TC0280GRDExit();
void TC0280GRDScan(int nAction);

#define TC0430GRWRam		TC0280GRDRam
#define TC0430GRWRenderLayer	TC0280GRDRenderLayer
#define TC0430GRWCtrlWordWrite	TC0280GRDCtrlWordWrite
#define TC0430GRWReset		TC0280GRDReset
#define TC0430GRWExit		TC0280GRDExit
#define TC0430GRWScan		TC0280GRDScan

// TC0360PRI
extern UINT8 TC0360PRIRegs[16];

void TC0360PRIWrite(unsigned int Offset, unsigned char Data);
void TC0360PRIHalfWordWrite(unsigned int Offset, unsigned short Data);
void TC0360PRIHalfWordSwapWrite(unsigned int Offset, unsigned short Data);
void TC0360PRIReset();
void TC0360PRIInit();
void TC0360PRIExit();
void TC0360PRIScan(int nAction);

// TC0480SCP
extern unsigned char *TC0480SCPRam;
extern UINT16 TC0480SCPCtrl[0x18];

void TC0480SCPCtrlWordWrite(int Offset, UINT16 Data);
void TC0480SCPTilemapRender(int Layer, int Opaque, unsigned char *pSrc);
void TC0480SCPRenderCharLayer();
void TC0480SCPReset();
int TC0480SCPGetBgPriority();
void TC0480SCPInit(int nNumTiles, int Pixels, int xOffset, int yOffset, int xTextOffset, int yTextOffset, int VisYOffset);
void TC0480SCPSetColourBase(int Base);
void TC0480SCPExit();
void TC0480SCPScan(int nAction);

// TC0510NIO
extern unsigned char TC0510NIOInputPort0[8];
extern unsigned char TC0510NIOInputPort1[8];
extern unsigned char TC0510NIOInputPort2[8];
extern unsigned char TC0510NIODip[2];
extern unsigned char TC0510NIOInput[3];

UINT16 TC0510NIOHalfWordRead(int Offset);
UINT16 TC0510NIOHalfWordSwapRead(int Offset);
void TC0510NIOHalfWordWrite(int Offset, UINT16 Data);
void TC0510NIOHalfWordSwapWrite(int Offset, UINT16 Data);
void TC0510NIOReset();
void TC0510NIOInit();
void TC0510NIOExit();
void TC0510NIOScan(int nAction);

// TC0640FIO
extern unsigned char TC0640FIOInputPort0[8];
extern unsigned char TC0640FIOInputPort1[8];
extern unsigned char TC0640FIOInputPort2[8];
extern unsigned char TC0640FIOInputPort3[8];
extern unsigned char TC0640FIOInputPort4[8];
extern unsigned char TC0640FIOInput[5];

UINT8 TC0640FIORead(unsigned int Offset);
void TC0640FIOWrite(unsigned int Offset, UINT8 Data);
UINT16 TC0640FIOHalfWordRead(unsigned int Offset);
void TC0640FIOHalfWordWrite(unsigned int Offset, UINT16 Data);
UINT16 TC0640FIOHalfWordByteswapRead(unsigned int Offset);
void TC0640FIOHalfWordByteswapWrite(unsigned int Offset, UINT16 Data);
void TC0640FIOReset();
void TC0640FIOInit();
void TC0640FIOExit();
void TC0640FIOScan(int nAction);

#define TC0100SCN0CtrlWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0100SCNCtrlWordWrite(0, (a - base_address) >> 1, d);	\
		return;							\
	}
	
#define TC0100SCN1CtrlWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0100SCNCtrlWordWrite(1, (a - base_address) >> 1, d);	\
		return;							\
	}
	
#define TC0100SCN2CtrlWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0100SCNCtrlWordWrite(2, (a - base_address) >> 1, d);	\
		return;							\
	}

#define TC0100SCN0ByteWrite_Map(start, end)				\
	if (a >= start && a <= end) {					\
		int Offset = (a - start) ^ 1;				\
		if (TC0100SCNRam[0][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[0] = 1;			\
			TC0100SCNFgLayerUpdate[0] = 1;			\
		}							\
		TC0100SCNRam[0][Offset] = d;				\
		return;							\
	}

#define TC0100SCN0WordWrite_Map(start, end)				\
	if (a >= start && a <= end) {					\
		UINT16 *Ram = (UINT16*)TC0100SCNRam[0];			\
		int Offset = (a - start) >> 1;				\
		if (Ram[Offset] != d) {					\
			TC0100SCNBgLayerUpdate[0] = 1;			\
			TC0100SCNFgLayerUpdate[0] = 1;			\
		}							\
		Ram[Offset] = d;					\
		return;							\
	}
	
#define TC0100SCN1ByteWrite_Map(start, end)				\
	if (a >= start && a <= end) {					\
		int Offset = (a - start) ^ 1;				\
		if (TC0100SCNRam[1][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[1] = 1;			\
			TC0100SCNFgLayerUpdate[1] = 1;			\
		}							\
		TC0100SCNRam[1][Offset] = d;				\
		return;							\
	}

#define TC0100SCN1WordWrite_Map(start, end)				\
	if (a >= start && a <= end) {					\
		UINT16 *Ram = (UINT16*)TC0100SCNRam[1];			\
		int Offset = (a - start) >> 1;				\
		if (Ram[Offset] != d) {					\
			TC0100SCNBgLayerUpdate[1] = 1;			\
			TC0100SCNFgLayerUpdate[1] = 1;			\
		}							\
		Ram[Offset] = d;					\
		return;							\
	}
	
#define TC0100SCN2ByteWrite_Map(start, end)				\
	if (a >= start && a <= end) {					\
		int Offset = (a - start) ^ 1;				\
		if (TC0100SCNRam[2][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[2] = 1;			\
			TC0100SCNFgLayerUpdate[2] = 1;			\
		}							\
		TC0100SCNRam[2][Offset] = d;				\
		return;							\
	}

#define TC0100SCN2WordWrite_Map(start, end)				\
	if (a >= start && a <= end) {					\
		UINT16 *Ram = (UINT16*)TC0100SCNRam[2];			\
		int Offset = (a - start) >> 1;				\
		if (Ram[Offset] != d) {					\
			TC0100SCNBgLayerUpdate[2] = 1;			\
			TC0100SCNFgLayerUpdate[2] = 1;			\
		}							\
		Ram[Offset] = d;					\
		return;							\
	}
	
#define TC0100SCNDualScreenByteWrite_Map(start, end)			\
	if (a >= start && a <= end) {					\
		int Offset = (a - start) ^ 1;				\
		if (TC0100SCNRam[0][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[0] = 1;			\
			TC0100SCNFgLayerUpdate[0] = 1;			\
		}							\
		if (TC0100SCNRam[1][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[1] = 1;			\
			TC0100SCNFgLayerUpdate[1] = 1;			\
		}							\
		TC0100SCNRam[0][Offset] = d;				\
		TC0100SCNRam[1][Offset] = d;				\
		return;							\
	}
	
#define TC0100SCNDualScreenWordWrite_Map(start, end)			\
	if (a >= start && a <= end) {					\
		UINT16 *Ram0 = (UINT16*)TC0100SCNRam[0];		\
		UINT16 *Ram1 = (UINT16*)TC0100SCNRam[1];		\
		int Offset = (a - start) >> 1;				\
		if (Ram0[Offset] != d) {				\
			TC0100SCNBgLayerUpdate[0] = 1;			\
			TC0100SCNFgLayerUpdate[0] = 1;			\
		}							\
		if (Ram1[Offset] != d) {				\
			TC0100SCNBgLayerUpdate[1] = 1;			\
			TC0100SCNFgLayerUpdate[1] = 1;			\
		}							\
		Ram0[Offset] = d;					\
		Ram1[Offset] = d;					\
		return;							\
	}

#define TC0100SCNTripleScreenByteWrite_Map(start, end)			\
	if (a >= start && a <= end) {					\
		int Offset = (a - start) ^ 1;				\
		if (TC0100SCNRam[0][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[0] = 1;			\
			TC0100SCNFgLayerUpdate[0] = 1;			\
		}							\
		if (TC0100SCNRam[1][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[1] = 1;			\
			TC0100SCNFgLayerUpdate[1] = 1;			\
		}							\
		if (TC0100SCNRam[2][Offset] != d) {			\
			TC0100SCNBgLayerUpdate[2] = 1;			\
			TC0100SCNFgLayerUpdate[2] = 1;			\
		}							\
		TC0100SCNRam[0][Offset] = d;				\
		TC0100SCNRam[1][Offset] = d;				\
		TC0100SCNRam[2][Offset] = d;				\
		return;							\
	}
	
#define TC0100SCNTripleScreenWordWrite_Map(start, end)			\
	if (a >= start && a <= end) {					\
		UINT16 *Ram0 = (UINT16*)TC0100SCNRam[0];		\
		UINT16 *Ram1 = (UINT16*)TC0100SCNRam[1];		\
		UINT16 *Ram2 = (UINT16*)TC0100SCNRam[2];		\
		int Offset = (a - start) >> 1;				\
		if (Ram0[Offset] != d) {				\
			TC0100SCNBgLayerUpdate[0] = 1;			\
			TC0100SCNFgLayerUpdate[0] = 1;			\
		}							\
		if (Ram1[Offset] != d) {				\
			TC0100SCNBgLayerUpdate[1] = 1;			\
			TC0100SCNFgLayerUpdate[1] = 1;			\
		}							\
		if (Ram2[Offset] != d) {				\
			TC0100SCNBgLayerUpdate[2] = 1;			\
			TC0100SCNFgLayerUpdate[2] = 1;			\
		}							\
		Ram0[Offset] = d;					\
		Ram1[Offset] = d;					\
		Ram2[Offset] = d;					\
		return;							\
	}

#define TC0220IOCHalfWordRead_Map(base_address)				\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		return TC0220IOCHalfWordRead((a - base_address) >> 1);	\
	}
	
#define TC0220IOCHalfWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0220IOCHalfWordWrite((a - base_address) >> 1, d);	\
		return;							\
	}

#define TC0280GRDCtrlWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0280GRDCtrlWordWrite((a - base_address) >> 1, d);	\
		return;							\
	}

#define TC0360PRIHalfWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x1f) {		\
		TC0360PRIHalfWordWrite((a - base_address) >> 1, d);	\
		return;							\
	}
	
#define TC0360PRIHalfWordSwapWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x1f) {		\
		TC0360PRIHalfWordSwapWrite((a - base_address) >> 1, d);	\
		return;							\
	}

#define TC0430GRWCtrlWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0430GRWCtrlWordWrite((a - base_address) >> 1, d);	\
		return;							\
	}

#define TC0480SCPCtrlWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x2f) {		\
		TC0480SCPCtrlWordWrite((a - base_address) >> 1, d);	\
		return;							\
	}

#define TC0510NIOHalfWordRead_Map(base_address)				\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		return TC0510NIOHalfWordRead((a - base_address) >> 1);	\
	}

#define TC0510NIOHalfWordSwapRead_Map(base_address)				\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		return TC0510NIOHalfWordSwapRead((a - base_address) >> 1);	\
	}

#define TC0510NIOHalfWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0510NIOHalfWordWrite((a - base_address) >> 1, d);	\
		return;							\
	}
	
#define TC0510NIOHalfWordSwapWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0510NIOHalfWordSwapWrite((a - base_address) >> 1, d);	\
		return;							\
	}

#define TC0180VCUHalfWordWrite_Map(base_address)					\
	if (a >= (base_address + 0x40000) && a <= (base_address+0x7ffff)) {		\
		TC0180VCUFbRAM[(a & 0x3ffff)^1] = d;					\
		TC0180VCUFramebufferWrite(a);						\
		return;									\
	}										\
											\
	if (a >= (base_address + 0x18000) && a <= (base_address+0x1801f)) {		\
		TC0180VCUWriteRegs(a, d);						\
		return;									\
	}

#define TC0180VCUWordWrite_Map(base_address)						\
	if (a >= (base_address + 0x40000) && a <= (base_address+0x7ffff)) {		\
		*((unsigned short*)(TC0180VCUFbRAM + (a & 0x3fffe))) = d;		\
		TC0180VCUFramebufferWrite(a);						\
		return;									\
	}										\
											\
	if (a >= (base_address + 0x18000) && a <= (base_address+0x1801f)) {		\
		TC0180VCUWriteRegs(a, d >> 8);						\
		return;									\
	}

#define TC0180VCUHalfWordRead_Map(base_address)						\
	if (a >= (base_address + 0x40000) && a <= (base_address+0x7ffff)) {		\
		if (a & 1) return TC0180VCUFramebufferRead(a) >> 8;			\
		return TC0180VCUFramebufferRead(a);					\
	}										\
											\
	if (a >= (base_address + 0x18000) && a <= (base_address+0x1801f)) {		\
		return TC0180VCUReadRegs(a);						\
	}

// TC0180VCU doesn't seem to use word access at all
