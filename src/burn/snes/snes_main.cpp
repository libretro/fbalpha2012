#include <stdio.h>
#include "snem.h"

static int frames;
int intthisline;
int framenum;
int oldnmi=0;
int spcclck;
double spcclck2,spcclck3;

void __cdecl snemlog(char *,...)
{}


void SnesReset()
{
	resetppu();
	resetspc();
	resetdsp();
	reset65816();
	frames=0;
}


int SnesInit()
{
	int nret = 0;

	allocmem();
	initppu();
	initspc();
	makeopcodetable();
	initdsp();

	SnesReset();
	int c=0;
	char name[22];

	int len;
	unsigned short temp,temp2;

	spccycles=-10000;

	rom=(unsigned char *)malloc(4096*1024);
	BurnLoadRom(rom,0,0);


	temp=rom[0x7FDC]|(rom[0x7FDD]<<8);
	temp2=rom[0x7FDE]|(rom[0x7FDF]<<8);
	if ((temp|temp2)==0xFFFF) 
		lorom=1;
	else
		lorom=0;
	//        lorom=0;
	initmem();
	if (((readmem(0xFFFD)<<8)|readmem(0xFFFC))==0xFFFF) 
	{ 
		lorom^=1; 
		initmem(); 
	}

	len=c;//-0x10000;
	for (c=0;c<21;c++)
	{
		name[c]=readmem(0xFFC0+c);
	}
	name[21]=0;
	srammask=(1<<(readmem(0xFFD8)+10))-1;
	if (!readmem(0xFFD8)) 
	{
		srammask=0;
	}
	if (readmem(0xFFD9)>1) 
	{
		pal=1;
	}
	else
	{
		pal=0;
	}
	if (srammask)
	{
		memset(sram,0,srammask+1);
	}
	memset(ram,0x55,128*1024);
    SnesReset();
	return nret;
}

int SnesExit()
{
	freemem();
	return 0;
}

int SnesFrame()
{
	if (DoSnesReset)
	{
		SnesReset();
	}

	nmi=vbl=0;
	framenum++;
	if (framenum==50)
	{
		spcclck=spctotal;
		spcclck2=spctotal2;
		spcclck3=spctotal3;
		spctotal=0;
		spctotal2=0.0f;
		spctotal3=0.0f;
	}
	for (lines=0;lines<((pal)?312:262);lines++)
	{
		//                snemlog("%i %02X:%04X %i %i %i\n",lines,pbr>>16,pc,irqenable,xirq,yirq);
		if ((irqenable==2/* || irqenable==1*/) && (lines==yirq)) { irq=1; /*snemlog("Raise IRQ line %i %02X\n",lines,lines);*/ }
		if (lines<225) 
		{
			drawline(lines);
		}
		cycles+=1364;
		intthisline=0;
		while (cycles>0)
		{
			opcode=readmem(pbr|pc); pc++;
			opcodes[opcode][cpumode]();
			if ((((irqenable==3) && (lines==yirq)) || (irqenable==1)) && !intthisline)
			{
				if (((1364-cycles)>>2)>=xirq)
				{
					irq=1;
					intthisline=1;
					//                                        snemlog("Raise IRQ horiz %i %i\n",lines,irqenable);
				}
			}
			ins++;
			if (oldnmi!=nmi && nmienable && nmi)
			{
				nmi65816();
			}
			else if (irq && (!p.i || inwai))
			{
				irq65816();
			}
			oldnmi=nmi;
		}
		if (lines==0xE0) 
		{
			nmi=1;
		}
		if (lines==0xE0)
		{
			vbl=joyscan=1;
			readjoy();
			//                        snemlog("Enter VBL\n");
		}
		if (lines==0xE3) 
		{
			joyscan=0;
		}

	}
	frames++;

	return 0;
}

int SnesScan(int nAction,int *pnMin)
{
	return 0;
}
