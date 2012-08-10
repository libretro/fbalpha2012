/*
  File: fm.h -- header file for software emulation for FM sound generator

*/
#ifndef _H_FM_FM_
#define _H_FM_FM_

/* --- select emulation chips --- */
#define BUILD_YM2610  (HAS_YM2610)		/* build YM2610(OPNB)  emulator */
#define BUILD_YM2610B (HAS_YM2610B)		/* build YM2610B(OPNB?)emulator */

/* select bit size of output : 8 or 16 */
#define FM_SAMPLE_BITS 16

/* select timer system internal or external */
#define FM_INTERNAL_TIMER 0

/* --- speedup optimize --- */
/* busy flag enulation , The definition of FM_GET_TIME_NOW() is necessary. */
#define FM_BUSY_FLAG_SUPPORT 1

/* --- external SSG(YM2149/AY-3-8910)emulator interface port */
/* used by YM2203,YM2608,and YM2610 */

/* SSGClk   : Set SSG Clock      */
/* int n    = chip number        */
/* int clk  = MasterClock(Hz)    */
/* int rate = sample rate(Hz) */
#define SSGClk(chip,clock) AY8910_set_clock((chip)+ay8910_index_ym,clock)

/* SSGWrite : Write SSG port     */
/* int n    = chip number        */
/* int a    = address            */
/* int v    = data               */
#define SSGWrite(n,a,v) AY8910Write((n)+ay8910_index_ym,a,v)

/* SSGRead  : Read SSG port */
/* int n    = chip number   */
/* return   = Read data     */
#define SSGRead(n) AY8910Read((n)+ay8910_index_ym)

/* SSGReset : Reset SSG chip */
/* int n    = chip number   */
#define SSGReset(chip) AY8910Reset((chip)+ay8910_index_ym)


/* --- external callback funstions for realtime update --- */

/* for busy flag emulation , function FM_GET_TIME_NOW() should */
/* return present time in seconds with "double" precision  */
  /* in timer.c */
  #define FM_GET_TIME_NOW() timer_get_time()

#if BUILD_YM2610
  /* in 2610intf.c */
void BurnYM2610UpdateRequest(void);
#define YM2610UpdateReq(chip) BurnYM2610UpdateRequest()
#endif

/* compiler dependence */
#ifndef INLINE
#define INLINE static __inline__
#endif





#if (FM_SAMPLE_BITS==16)
typedef INT16 FMSAMPLE;
#endif
#if (FM_SAMPLE_BITS==8)
typedef unsigned char  FMSAMPLE;
#endif

typedef void (*FM_TIMERHANDLER)(int n,int c,int cnt,double stepTime);
typedef void (*FM_IRQHANDLER)(int n,int irq);
/* FM_TIMERHANDLER : Stop or Start timer         */
/* int n          = chip number                  */
/* int c          = Channel 0=TimerA,1=TimerB    */
/* int count      = timer count (0=stop)         */
/* doube stepTime = step time of one count (sec.)*/

/* FM_IRQHHANDLER : IRQ level changing sense     */
/* int n       = chip number                     */
/* int irq     = IRQ level 0=OFF,1=ON            */

#if (BUILD_YM2610||BUILD_YM2610B)
/* -------------------- YM2610(OPNB) Interface -------------------- */
int YM2610Init(int num, int baseclock, int rate,
               void **pcmroma,int *pcmasize,void **pcmromb,int *pcmbsize,
               FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler);
void YM2610SetRom(int num,
				void *pcmroma,int pcmsizea,void *pcmromb,int pcmsizeb);
void YM2610Shutdown(void);
void YM2610ResetChip(int num);
void YM2610UpdateOne(int num, INT16 **buffer, int length);
#if BUILD_YM2610B
void YM2610BUpdateOne(int num, INT16 **buffer, int length);
#endif

int YM2610Write(int n, int a,unsigned char v);
unsigned char YM2610Read(int n,int a);
int YM2610TimerOver(int n, int c );
#endif /* BUILD_YM2610 */

#endif /* _H_FM_FM_ */
