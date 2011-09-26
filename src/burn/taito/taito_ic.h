#ifndef TAITO_IC_H
#define TAITO_IC_H

extern int TaitoIC_SupermanCChipInUse;
extern int TaitoIC_MegabCChipInUse;
extern int TaitoIC_RainbowCChipInUse;
extern int TaitoIC_OpwolfCChipInUse;

extern int TaitoIC_PC080SNInUse;
extern int TaitoIC_PC090OJInUse;
extern int TaitoIC_TC0100SCNInUse;
extern int TaitoIC_TC0110PCRInUse;
extern int TaitoIC_TC0140SYTInUse;
extern int TaitoIC_TC0150RODInUse;
extern int TaitoIC_TC0220IOCInUse;
extern int TaitoIC_TC0280GRDInUse;
extern int TaitoIC_TC0360PRIInUse;
extern int TaitoIC_TC0430GRWInUse;
extern int TaitoIC_TC0480SCPInUse;
extern int TaitoIC_TC0510NIOInUse;

extern void TaitoICReset();
extern void TaitoICExit();
extern void TaitoICScan(int nAction);

// C-Chip
extern UINT16 SupermanCChipCtrlRead();
extern UINT16 SupermanCChipRamRead(unsigned int Offset, unsigned char Input1, unsigned char Input2, unsigned char Input3);
extern void SupermanCChipCtrlWrite();
extern void SupermanCChipBankWrite(UINT16 Data);
extern void SupermanCChipRamWrite(unsigned int Offset, UINT16 Data);
extern void SupermanCChipReset();
extern void SupermanCChipInit();
extern void SupermanCChipExit();
extern void SupermanCChipScan(int nAction);

extern UINT16 MegabCChipRead(unsigned int Offset);
extern void MegabCChipWrite(unsigned int Offset, UINT16 Data);
extern void MegabCChipReset();
extern void MegabCChipInit();
extern void MegabCChipExit();
extern void MegabCChipScan(int nAction);

extern void RainbowCChipUpdate(unsigned char Input1, unsigned char Input2, unsigned char Input3, unsigned char Input4);
extern UINT16 RainbowCChipCtrlRead();
extern UINT16 RainbowCChipRamRead(unsigned int Offset);
extern void RainbowCChipCtrlWrite(UINT16);
extern void RainbowCChipBankWrite(UINT16 Data);
extern void RainbowCChipRamWrite(unsigned int Offset, UINT16 Data);
extern void RainbowCChipReset();
extern void RainbowCChipInit(int Version);
extern void RainbowCChipExit();
extern void RainbowCChipScan(int nAction);

extern void OpwolfCChipUpdate(unsigned char Input1, unsigned char Input2);
extern UINT16 OpwolfCChipStatusRead();
extern UINT16 OpwolfCChipDataRead(unsigned int Offset);
extern void OpwolfCChipStatusWrite();
extern void OpwolfCChipBankWrite(UINT16 Data);
extern void OpwolfCChipDataWrite(unsigned char *p68kRom, unsigned int Offset, UINT16 Data );
extern void OpwolfCChipReset();
extern void OpwolfCChipInit(int Region);
extern void OpwolfCChipExit();
extern void OpwolfCChipScan(int nAction);

extern void BonzeWriteCChipRam(int offset, int data);
extern void BonzeWriteCChipBank(int data);
extern unsigned short BonzeReadCChipRam(int offset);
extern void BonzeCChipReset();
extern void BonzeCChipScan(int nAction);

// PC080SN
extern unsigned char *PC080SNRam;

extern void PC080SNDrawBgLayer(int Opaque, unsigned char *pSrc);
extern void PC080SNDrawFgLayer(unsigned char *pSrc);
extern void PC080SNSetScrollX(unsigned int Offset, UINT16 Data);
extern void PC080SNSetScrollY(unsigned int Offset, UINT16 Data);
extern void PC080SNCtrlWrite(unsigned int Offset, UINT16 Data);
extern void PC080SNOverrideFgScroll(int xScroll, int yScroll);
extern void PC080SNReset();
extern void PC080SNInit(int nNumTiles, int xOffset, int yOffset, int yInvert, int DblWidth);
extern void PC080SNSetFgTransparentPen(int Pen);
extern void PC080SNExit();
extern void PC080SNScan(int nAction);

// PC090OJ
extern unsigned char *PC090OJRam;
extern int PC090OJSpriteCtrl;

extern void PC090OJDrawSprites(unsigned char *pSrc);
extern void PC090OJReset();
extern void PC090OJInit(int nNumTiles, int xOffset, int yOffset, int UseBuffer);
extern void PC090OJExit();
extern void PC090OJScan(int nAction);

// TC0100SCN
#define TC0100SCN_MAX_CHIPS 3

extern unsigned char *TC0100SCNRam[TC0100SCN_MAX_CHIPS];
extern UINT16 TC0100SCNCtrl[TC0100SCN_MAX_CHIPS][8];

extern void TC0100SCNCtrlWordWrite(unsigned int Offset, UINT16 Data);
extern int TC0100SCNBottomLayer();
extern void TC0100SCNRenderFgLayerHack (unsigned char *pSrc);
extern void TC0100SCNRenderBgLayer(int Opaque, unsigned char *pSrc);
extern void TC0100SCNRenderBgLayerHack(unsigned char *pSrc);
extern void TC0100SCNRenderFgLayer(int Opaque, unsigned char *pSrc);
extern void TC0100SCNRenderCharLayer();
extern void TC0100SCNRenderCharLayerHack();
extern void TC0100SCNReset();
extern void TC0100SCNInit(int Num, int nNumTiles, int xOffset, int yOffset, int xFlip, unsigned char *PriorityMap);
extern void TC0100SCNSetColourDepth(int ColourDepth);
extern void TC0100SCNSetGfxMask(int Mask);
extern void TC0100SCNSetGfxBank(int Bank);
extern void TC0100SCNExit();
extern void TC0100SCNScan(int nAction);

// TC0110PCR
extern unsigned int *TC0110PCRPalette;
extern int TC0110PCRTotalColours;

extern UINT16 TC0110PCRWordRead(int Chip);
extern void TC0110PCRWordWrite(int Chip, int Offset, UINT16 Data);
extern void TC0110PCRStep1WordWrite(int Chip, int Offset, UINT16 Data);
extern void TC0110PCRStep1RBSwapWordWrite(int Chip, int Offset, UINT16 Data);
extern void TC0110PCRStep14rbgWordWrite(int Chip, int Offset, UINT16 Data);
extern void TC0110PCRReset();
extern void TC0110PCRInit(int Num, int nNumColours);
extern void TC0110PCRExit();
void TC0110PCRScan(int nAction);

// TC0140SYT
extern void TC0140SYTPortWrite(UINT8 Data);
extern UINT8 TC0140SYTCommRead();
extern void TC0140SYTCommWrite(UINT8 Data);
extern void TC0140SYTSlavePortWrite(UINT8 Data);
extern UINT8 TC0140SYTSlaveCommRead();
extern void TC0140SYTSlaveCommWrite(UINT8 Data);
extern void TC0140SYTReset();
extern void TC0140SYTInit();
extern void TC0140SYTExit();
extern void TC0140SYTScan(int nAction);

// TC0150ROD
extern unsigned char *TC0150RODRom;
extern unsigned char *TC0150RODRam;

extern void TC0150RODDraw(int yOffs, int pOffs, int Type, int RoadTrans, int LowPriority, int HighPriority);
extern void TC0150RODReset();
extern void TC0150RODInit(int nRomSize, int xFlip);
extern void TC0150RODExit();
extern void TC0150RODScan(int nAction);

// TC0220IOC
extern unsigned char TC0220IOCInputPort0[8];
extern unsigned char TC0220IOCInputPort1[8];
extern unsigned char TC0220IOCInputPort2[8];
extern unsigned char TC0220IOCDip[2];
extern unsigned char TC0220IOCInput[3];

extern UINT8 TC0220IOCPortRead();
extern UINT8 TC0220IOCHalfWordPortRead();
extern UINT8 TC0220IOCPortRegRead();
extern UINT8 TC0220IOCHalfWordRead(int Offset);
extern UINT8 TC0220IOCRead(UINT8 Port);
extern void TC0220IOCWrite(UINT8 Port, UINT8 Data);
extern void TC0220IOCHalfWordPortRegWrite(UINT16 Data);
extern void TC0220IOCHalfWordPortWrite(UINT16 Data);
extern void TC0220IOCHalfWordWrite(int Offset, UINT16 Data);
extern void TC0220IOCReset();
extern void TC0220IOCInit();
extern void TC0220IOCExit();
extern void TC0220IOCScan(int nAction);

// TC0280GRD
extern unsigned char *TC0280GRDRam;
extern int TC0280GRDBaseColour;

extern void TC0280GRDRenderLayer();
extern void TC0280GRDCtrlWordWrite(unsigned int Offset, UINT16 Data);
extern void TC0280GRDReset();
extern void TC0280GRDInit(int xOffs, int yOffs, unsigned char *pSrc);
extern void TC0430GRWInit(int xOffs, int yOffs, unsigned char *pSrc);
extern void TC0280GRDExit();
extern void TC0280GRDScan(int nAction);

#define TC0430GRWRam		TC0280GRDRam
#define TC0430GRWRenderLayer	TC0280GRDRenderLayer
#define TC0430GRWCtrlWordWrite	TC0280GRDCtrlWordWrite
#define TC0430GRWReset		TC0280GRDReset
#define TC0430GRWExit		TC0280GRDExit
#define TC0430GRWScan		TC0280GRDScan

// TC0360PRI
extern UINT8 TC0360PRIRegs[16];

extern void TC0360PRIWrite(unsigned int Offset, unsigned char Data);
extern void TC0360PRIHalfWordWrite(unsigned int Offset, unsigned short Data);
extern void TC0360PRIHalfWordSwapWrite(unsigned int Offset, unsigned short Data);
extern void TC0360PRIReset();
extern void TC0360PRIInit();
extern void TC0360PRIExit();
extern void TC0360PRIScan(int nAction);

// TC0480SCP
extern unsigned char *TC0480SCPRam;
extern UINT16 TC0480SCPCtrl[0x18];

extern void TC0480SCPCtrlWordWrite(int Offset, UINT16 Data);
extern void TC0480SCPTilemapRender(int Layer, int Opaque, unsigned char *pSrc);
extern void TC0480SCPRenderCharLayer();
extern void TC0480SCPReset();
extern int TC0480SCPGetBgPriority();
extern void TC0480SCPInit(int nNumTiles, int Pixels, int xOffset, int yOffset, int xTextOffset, int yTextOffset, int VisYOffset);
extern void TC0480SCPSetColourBase(int Base);
extern void TC0480SCPExit();
extern void TC0480SCPScan(int nAction);

// TC0510NIO
extern unsigned char TC0510NIOInputPort0[8];
extern unsigned char TC0510NIOInputPort1[8];
extern unsigned char TC0510NIOInputPort2[8];
extern unsigned char TC0510NIODip[2];
extern unsigned char TC0510NIOInput[3];

extern UINT16 TC0510NIOHalfWordRead(int Offset);
extern UINT16 TC0510NIOHalfWordSwapRead(int Offset);
extern void TC0510NIOHalfWordWrite(int Offset, UINT16 Data);
extern void TC0510NIOHalfWordSwapWrite(int Offset, UINT16 Data);
extern void TC0510NIOReset();
extern void TC0510NIOInit();
extern void TC0510NIOExit();
extern void TC0510NIOScan(int nAction);

#define TC0100SCNCtrlWordWrite_Map(base_address)			\
	if (a >= base_address && a <= base_address + 0x0f) {		\
		TC0100SCNCtrlWordWrite((a - base_address) >> 1, d);	\
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

#endif
