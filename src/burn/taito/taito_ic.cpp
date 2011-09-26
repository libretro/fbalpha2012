#include "burnint.h"
#include "taito_ic.h"

int TaitoIC_SupermanCChipInUse = 0;
int TaitoIC_MegabCChipInUse = 0;
int TaitoIC_RainbowCChipInUse = 0;
int TaitoIC_OpwolfCChipInUse = 0;

int TaitoIC_PC080SNInUse = 0;
int TaitoIC_PC090OJInUse = 0;
int TaitoIC_TC0100SCNInUse = 0;
int TaitoIC_TC0110PCRInUse = 0;
int TaitoIC_TC0140SYTInUse = 0;
int TaitoIC_TC0150RODInUse = 0;
int TaitoIC_TC0220IOCInUse = 0;
int TaitoIC_TC0280GRDInUse = 0;
int TaitoIC_TC0360PRIInUse = 0;
int TaitoIC_TC0430GRWInUse = 0;
int TaitoIC_TC0480SCPInUse = 0;
int TaitoIC_TC0510NIOInUse = 0;

void TaitoICReset()
{
	if (TaitoIC_SupermanCChipInUse) SupermanCChipReset();
	if (TaitoIC_MegabCChipInUse) MegabCChipReset();
	if (TaitoIC_RainbowCChipInUse) RainbowCChipReset();
	if (TaitoIC_OpwolfCChipInUse) OpwolfCChipReset();

	if (TaitoIC_PC080SNInUse) PC080SNReset();
	if (TaitoIC_PC090OJInUse) PC090OJReset();
	if (TaitoIC_TC0100SCNInUse) TC0100SCNReset();
	if (TaitoIC_TC0110PCRInUse) TC0110PCRReset();
	if (TaitoIC_TC0140SYTInUse) TC0140SYTReset();
	if (TaitoIC_TC0150RODInUse) TC0150RODReset();
	if (TaitoIC_TC0220IOCInUse) TC0220IOCReset();
	if (TaitoIC_TC0280GRDInUse) TC0280GRDReset();
	if (TaitoIC_TC0360PRIInUse) TC0360PRIReset();
	if (TaitoIC_TC0430GRWInUse) TC0430GRWReset();
	if (TaitoIC_TC0480SCPInUse) TC0480SCPReset();
	if (TaitoIC_TC0510NIOInUse) TC0510NIOReset();
}

void TaitoICExit()
{
	TaitoIC_SupermanCChipInUse = 0;
	TaitoIC_MegabCChipInUse = 0;
	TaitoIC_RainbowCChipInUse = 0;
	TaitoIC_OpwolfCChipInUse = 0;

	TaitoIC_PC080SNInUse = 0;
	TaitoIC_PC090OJInUse = 0;
	TaitoIC_TC0100SCNInUse = 0;
	TaitoIC_TC0110PCRInUse = 0;
	TaitoIC_TC0140SYTInUse = 0;
	TaitoIC_TC0150RODInUse = 0;
	TaitoIC_TC0220IOCInUse = 0;
	TaitoIC_TC0280GRDInUse = 0;
	TaitoIC_TC0360PRIInUse = 0;
	TaitoIC_TC0430GRWInUse = 0;
	TaitoIC_TC0480SCPInUse = 0;
	TaitoIC_TC0510NIOInUse = 0;
	
	SupermanCChipExit();
	MegabCChipExit();
	RainbowCChipExit();
	OpwolfCChipExit();
	
	PC080SNExit();
	PC090OJExit();
	TC0100SCNExit();
	TC0110PCRExit();
	TC0140SYTExit();
	TC0150RODExit();
	TC0220IOCExit();
	TC0280GRDExit();
	TC0360PRIExit();
	TC0430GRWExit();
	TC0480SCPExit();
	TC0510NIOExit();	
}

void TaitoICScan(int nAction)
{
	if (TaitoIC_SupermanCChipInUse) SupermanCChipScan(nAction);
	if (TaitoIC_MegabCChipInUse) MegabCChipScan(nAction);
	if (TaitoIC_RainbowCChipInUse) RainbowCChipScan(nAction);
	if (TaitoIC_OpwolfCChipInUse) OpwolfCChipScan(nAction);

	if (TaitoIC_PC080SNInUse) PC080SNScan(nAction);
	if (TaitoIC_PC090OJInUse) PC090OJScan(nAction);
	if (TaitoIC_TC0100SCNInUse) TC0100SCNScan(nAction);
	if (TaitoIC_TC0110PCRInUse) TC0110PCRScan(nAction);
	if (TaitoIC_TC0140SYTInUse) TC0140SYTScan(nAction);
	if (TaitoIC_TC0150RODInUse) TC0150RODScan(nAction);
	if (TaitoIC_TC0220IOCInUse) TC0220IOCScan(nAction);
	if (TaitoIC_TC0280GRDInUse) TC0280GRDScan(nAction);
	if (TaitoIC_TC0360PRIInUse) TC0360PRIScan(nAction);
	if (TaitoIC_TC0430GRWInUse) TC0430GRWScan(nAction);
	if (TaitoIC_TC0480SCPInUse) TC0480SCPScan(nAction);
	if (TaitoIC_TC0510NIOInUse) TC0510NIOScan(nAction);
}
