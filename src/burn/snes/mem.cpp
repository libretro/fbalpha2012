#include <stdio.h>
#include "snem.h"

extern int pal;
unsigned short srammask=0;

unsigned char *sram;

unsigned char *ram;
unsigned char *rom;
unsigned char *memlookup[2048];
unsigned char memread[2048],memwrite[2048];
unsigned char accessspeed[2048];

int lorom;

void allocmem()
{
	//        FILE *f=fopen("finalf~1.srm","rb");
	ram=(unsigned char *)malloc(128*1024);
	memset(ram,0x55,128*1024);
	sram=(unsigned char *)malloc(8192);
	memset(sram,0,8192);
	//        fread(sram,8192,1,f);
	//        fclose(f);
}

void freemem()
{
	for (int i=0;i<2048;i++)
	{
		memlookup[i]=NULL;
		memread[i]=0;
		memwrite[i]=0;
		accessspeed[i]=0;

	}
	if (ram !=NULL)
	{
		free(ram);
	}
	if (sram!=NULL)
	{
		free(sram);
	}
	if (rom!=NULL)
	{
		free(rom);
	}
}

#if 0
void loadrom(char *fn)
{
	int c=0;
	char name[22];

	FILE *f=fopen(fn,"rb");
	int len;
	unsigned short temp,temp2;
	spccycles=-10000;
	if (!f)
	{
		snemlog("File %s not found\n",fn);
		//exit(-1);
	}
	fseek(f,-1,SEEK_END);
	len=ftell(f)+1;
	fseek(f,len&512,SEEK_SET);
	printf("%i %i\n",len,ftell(f));
	rom=(unsigned char *)malloc(4096*1024);
	//        fread(rom,512,1,f);
	/*        for (c=0;c<0x40000;c+=0x8000)
	{
	fread(&rom[c+0x40000],32768,1,f);
	fread(&rom[c],32768,1,f);
	}*/
	while (!feof(f) && c<0x400000)
	{
		//                printf("Read %06X\n",c);
		fread(&rom[c],65536,1,f);
		c+=0x10000;
	}
	fclose(f);
	temp=rom[0x7FDC]|(rom[0x7FDD]<<8);
	temp2=rom[0x7FDE]|(rom[0x7FDF]<<8);
	if ((temp|temp2)==0xFFFF) lorom=1;
	else                      lorom=0;
	//        lorom=0;
	initmem();
	if (((readmem(0xFFFD)<<8)|readmem(0xFFFC))==0xFFFF) { lorom^=1; initmem(); }

	len=c;//-0x10000;
	for (c=0;c<21;c++)
		name[c]=readmem(0xFFC0+c);
	name[21]=0;
	printf("ROM name : %s\n",name);
	printf("ROM size : %i megabits (%i kbytes)\n",1<<(readmem(0xFFD7)-7),len>>10);
	srammask=(1<<(readmem(0xFFD8)+10))-1;
	if (!readmem(0xFFD8)) srammask=0;
	printf("SRAM size : %i kilobits (%i kbytes) %04X\n",1<<(readmem(0xFFD8)+3),(1<<(readmem(0xFFD8)+3))>>3,srammask);
	if (readmem(0xFFD9)>1) pal=1;
	else                   pal=0;
	printf("Country code : %i (%s)\n",readmem(0xFFD9),(readmem(0xFFD9)>1)?"PAL":"NTSC");
	printf("NMI vector : %02X%02X\n",readmem(0xFFEB),readmem(0xFFEA));
	printf("IRQ vector : %02X%02X\n",readmem(0xFFEF),readmem(0xFFEE));
	printf("Reset vector : %02X%02X\n",readmem(0xFFFD),readmem(0xFFFC));
	printf("Memory map : %s\n",(lorom)?"LoROM":"HiROM");
	if (srammask)
	{
		memset(sram,0,srammask+1);
	}
	memset(ram,0x55,128*1024);
}
#endif

void initmem()
{

	int c,d;
	for (c=0;c<256;c++)
	{
		for (d=0;d<8;d++)
		{
			memread[(c<<3)|d]=memwrite[(c<<3)|d]=0;
		}
	}
	if (lorom)
	{
		for (c=0;c<96;c++)
		{
			for (d=0;d<4;d++)
			{
				memread[(c<<3)|(d+4)]=1;
				memlookup[(c<<3)|(d+4)]=&rom[((d*0x2000)+(c*0x8000))&0x3FFFFF];
				memread[(c<<3)|(d+4)|0x400]=1;
				memlookup[(c<<3)|(d+4)|0x400]=&rom[((d*0x2000)+(c*0x8000))&0x3FFFFF];
			}
		}
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0]=memwrite[(c<<3)|0]=1;
			memlookup[(c<<3)|0]=ram;
		}
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0x400]=memwrite[(c<<3)|0x400]=1;
			memlookup[(c<<3)|0x400]=ram;
		}
		for (c=0;c<8;c++)
		{
			memread[(0x7E<<3)|c]=memwrite[(0x7E<<3)|c]=1;
			memlookup[(0x7E<<3)|c]=&ram[c*0x2000];
			memread[(0x7F<<3)|c]=memwrite[(0x7F<<3)|c]=1;
			memlookup[(0x7F<<3)|c]=&ram[(c*0x2000)+0x10000];
		}
		//                printf("%08X\n",memlookup[(0x7F<<3)|4]);
		/*                for (c=0;c<96;c++)
		{
		for (d=0;d<4;d++)
		{
		memread[((c+64)<<3)|(d+4)]=1;
		memlookup[((c+64)<<3)|(d+4)]=&rom[((d*0x2000)+(c*0x8000))&0x3FFFFF];
		}
		}
		for (c=0;c<0x60<<3;c++)
		{
		memread[c+0x400]=memread[c];
		memwrite[c+0x400]=memwrite[c];
		memlookup[c+0x400]=memlookup[c];
		}
		/*                for (c=0;c<16;c++)
		{
		memread[(0x70<<3)+c]=memwrite[(0x70<<3)+c]=0;
		memlookup[(0x70<<3)+c]=sram;
		}*/
		//                printf("%08X\n",memlookup[(0x7F<<3)|4]);
	}
	else
	{
		for (c=0;c<2048;c++)
		{
			memread[c]=1;
			memwrite[c]=0;
			memlookup[c]=&rom[(c*0x2000)&0x3FFFFF];
		}
		for (c=0;c<64;c++)
		{
			for (d=1;d<4;d++)
			{
				memread[(c<<3)+d]=memwrite[(c<<3)+d]=0;
				memread[(c<<3)+d+1024]=memwrite[(c<<3)+d+1024]=0;
			}
		}
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0]=memwrite[(c<<3)|0]=1;
			memlookup[(c<<3)|0]=ram;
			memread[(c<<3)|1024]=memwrite[(c<<3)|1024]=1;
			memlookup[(c<<3)|1024]=ram;
		}
		for (c=0;c<8;c++)
		{
			memread[(0x7E<<3)|c]=memwrite[(0x7E<<3)|c]=1;
			memlookup[(0x7E<<3)|c]=&ram[c*0x2000];
			memread[(0x7F<<3)|c]=memwrite[(0x7F<<3)|c]=1;
			memlookup[(0x7F<<3)|c]=&ram[(c*0x2000)+0x10000];
		}
		/*                for (c=0;c<0x40<<3;c++)
		{
		memread[c+0x400]=memread[c];
		memwrite[c+0x400]=memwrite[c];
		memlookup[c+0x400]=memlookup[c];
		}*/
		for (c=0;c<16;c++)
		{
			memread[(0x70<<3)+c]=memwrite[(0x70<<3)+c]=1;
			memlookup[(0x70<<3)+c]=sram;
		}
	}
	/*Set up access speed table*/
	for (c=0;c<64;c++)
	{
		accessspeed[(c<<3)|0]=8;
		accessspeed[(c<<3)|1]=6;
		accessspeed[(c<<3)|2]=6;
		accessspeed[(c<<3)|3]=6;
		accessspeed[(c<<3)|4]=accessspeed[(c<<3)|5]=8;
		accessspeed[(c<<3)|6]=accessspeed[(c<<3)|7]=8;
	}
	for (c=64;c<128;c++)
	{
		for (d=0;d<8;d++)
		{
			accessspeed[(c<<3)|d]=8;
		}
	}
	for (c=128;c<192;c++)
	{
		accessspeed[(c<<3)|0]=8;
		accessspeed[(c<<3)|1]=6;
		accessspeed[(c<<3)|2]=6;
		accessspeed[(c<<3)|3]=6;
		accessspeed[(c<<3)|4]=accessspeed[(c<<3)|5]=8;
		accessspeed[(c<<3)|6]=accessspeed[(c<<3)|7]=8;
	}
	for (c=192;c<256;c++)
	{
		for (d=0;d<8;d++)
		{
			accessspeed[(c<<3)|d]=8;
		}
	}
}

unsigned char readmeml(unsigned long addr)
{
	addr&=~0xFF000000;
	if (((addr>>16)&0x7F)<0x40)
	{
		switch (addr&0xF000)
		{
		case 0x2000:
			return readppu(addr);
		case 0x4000:
			if ((addr&0xE00)==0x200)
			{
				return readio(addr);
			}
			if ((addr&0xFFFE)==0x4016)
			{
				return readjoyold(addr);
			}
			snemlog("Bad Read %06X\n",addr);
			return 0;
		case 0x6000: case 0x7000:
			//                        printf("Read SRAM %04X %02X %06X\n",addr,sram[addr&0x1FFF],pbr|pc);
			if (!lorom) 
			{
				return sram[addr&srammask];
			}
		default:
			snemlog("Bad read %06X\n",addr);
			return 0xFF;
		}
	}
	if ((addr>>16)>=0xD0 && (addr>>16)<=0xFE) 
	{
		return 0;
	}
	if ((addr>>16)==0x70)
	{
		//                return 0;
		//                printf("Read SRAM %04X %02X\n",addr,sram[addr&0x1FFF]);
		if (srammask)
		{
			//                        printf("Read SRAM %04X %04X %02X %04X\n",addr,addr&srammask,sram[addr&srammask],srammask);
			return sram[addr&srammask];
		}
		//                printf("Read SRAM zero\n");
		return 0;
	}
	if ((addr>>16)==0x60) 
	{
		return 0;
	}
	snemlog("Bad read %06X\n",addr);
	return 0xff;
}

void writememl(unsigned long addr, unsigned char val)
{
	addr&=~0xFF000000;
	if (((addr>>16)&0x7F)<0x40)
	{
		switch (addr&0xF000)
		{
		case 0x2000:
			writeppu(addr,val);
			return;
		case 0x3000:
			return;
		case 0x4000:
			if ((addr&0xE00)==0x200)
				writeio(addr,val);
			if ((addr&0xFFFE)==0x4016)
				writejoyold(addr,val);
			return;
		case 0x5000: return;
		case 0x6000: case 0x7000:
			//                        printf("Write SRAM %04X %02X %06X\n",addr,val,pbr|pc);
			if (!lorom) sram[addr&srammask]=val;
			return;
		case 0x8000: case 0x9000: case 0xA000: case 0xB000:
		case 0xC000: case 0xD000: case 0xE000: case 0xF000:
			return;
		default:
			snemlog("Bad write %06X %02X\n",addr,val);

		}
	}
	if ((addr>>16)>=0xD0 && (addr>>16)<=0xFE) return;
	//        if ((addr>>16)==0xD0) return;
	if ((addr>>16)==0x70)
	{
		//                printf("Write SRAM %04X %04X %02X\n",addr,addr&srammask,val);
		sram[addr&srammask]=val;
		return;
	}
	if ((addr>>16)==0x60) return;
	if ((addr>=0xC00000 && addr<0xFE0000)) return;
	if ((addr>=0x710000 && addr<0x7E0000)) return;        
	snemlog("Bad write %06X %02X\n",addr,val);

}

