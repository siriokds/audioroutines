//§§§
#include <windows.h>
#include <memory.h>
#include "Mixer.h"
#include "Replayer.h"

#define SEQSIZE	 16
#define	SEQSTART 0

#define	CH00_ON 1
#define	CH01_ON 1
#define	CH02_ON 1
#define	CH03_ON 1

#define	FC_GET_SWORD_AT(p) *((SWORD*)(p))
#define	FC_GET_SLONG_AT(p) *((SLONG*)(p))


SWORD*		FC6_GetMixBuffer();
SBYTE*		FC6_GetSEQtable(void);
SBYTE*		FC6_GetPATtable(int voc);
SBYTE*		FC6_GetVOLtable(int voc);
SBYTE*		FC6_GetFRQtable(int voc);
int			FC6_GetPATrow(void);
SBYTE		FC6_GetOfs(int voc, int frq);
SBYTE		FC6_GetOfsVAL(int voc, int frq);
SBYTE		FC6_GetOfsSPD(int voc, int frq);
ULONG		FC6_GetPeriod(int voc);
void		FC6_GoNextSeq(void);


#pragma pack(1)
typedef struct tagChannelSTR 
{
	SBYTE	ON;
	UWORD	PER;
	UWORD	PER2;


	SBYTE	*PatPtr;
	SWORD	Period;
	SBYTE	Volume;
	SBYTE	NoteTR;
	SBYTE	SampTR;
	UBYTE	VoiceN;
	UBYTE	Note;
	SBYTE	Info;
	SBYTE	PortaV;
	SBYTE	PortaX;
	SBYTE	VolTspd;
	SBYTE	VolTcnt;
	SBYTE	VolSus;
	SBYTE	*VolPtr;
	ULONG	VolOfs;
	SBYTE	*FrqPtr;
	ULONG	FrqOfs;
	SBYTE	FrqSus;
	SBYTE	VibSpd;
	SBYTE	VibAmp;
	SBYTE	VibAmpC;
	SBYTE	VibDel;
	SBYTE	VibXOR;
	SBYTE	PBend;
	SBYTE	PBendC;
	SBYTE	PBendX;
	SBYTE	FreqTR;
	SBYTE	VBendS;
	SBYTE	VBendC;
	SBYTE	VBendX;
	SBYTE	SamplN;

}	tVOCSTR;
#pragma pack()

#pragma pack(1)
typedef struct tagSOUNDINFO
{
	SLONG	Start;
	SWORD	Length;
	SWORD	LoopS;
//	SLONG	Unused;

}	tSOUNDINFO;

#pragma pack()


#pragma pack(1)
typedef struct tagFC16INFO
{
	UWORD	onoff;

	SBYTE	REPcnt;
	SBYTE	REPspd;

	SBYTE	*MODaddr;
	SBYTE	*SMPaddr;
//	SLONG	SMPsize;
//	SLONG	WAVsize;

	SBYTE	*SEQinfo;
	SLONG	SEQendp;
	SBYTE	*PATinfo;
	SBYTE	*VOLinfo;
	SBYTE	*FRQinfo;

	SWORD	SEQnum;
	SWORD	PATnum;
	SWORD	VOLnum;
	SWORD	FRQnum;

	SLONG	SEQoffs;
	SLONG	PAToffs;

}	tFC16INFO;
#pragma pack()





static tDataInfo		DataInfo[80];
static int				DataInfoPos = 0;
static int				DataPlayPos = 16;
static CRITICAL_SECTION InfoCS;



static	tSOUNDINFO	SoundInfo[10+80];
static	tVOCSTR		VXdata[4];
static	tFC16INFO	FC16Mod;
static  SBYTE		TabNULL[80];


//+§+
SWORD	PERIODS[] = 
{
	 0x06b0,0x0650,0x05f4,0x05a0,0x054c,0x0500,0x04b8,0x0474,0x0434,0x03f8,0x03c0,0x038a
	,0x0358,0x0328,0x02fa,0x02d0,0x02a6,0x0280,0x025c,0x023a,0x021a,0x01fc,0x01e0,0x01c5
	,0x01ac,0x0194,0x017d,0x0168,0x0153,0x0140,0x012e,0x011d,0x010d,0x00fe,0x00f0,0x00e2
	,0x00d6,0x00ca,0x00be,0x00b4,0x00aa,0x00a0,0x0097,0x008f,0x0087,0x007f,0x0078,0x0071
	,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071

	,0x0d60,0x0ca0,0x0be8,0x0b40,0x0a98,0x0a00,0x0970,0x08e8,0x0868,0x07f0,0x0780,0x0714
	,0x1ac0,0x1940,0x17d0,0x1680,0x1530,0x1400,0x12e0,0x11d0,0x10d0,0x0fe0,0x0f00,0x0e28
	,0x06b0,0x0650,0x05f4,0x05a0,0x054c,0x0500,0x04b8,0x0474,0x0434,0x03f8,0x03c0,0x038a
	,0x0358,0x0328,0x02fa,0x02d0,0x02a6,0x0280,0x025c,0x023a,0x021a,0x01fc,0x01e0,0x01c5
	,0x01ac,0x0194,0x017d,0x0168,0x0153,0x0140,0x012e,0x011d,0x010d,0x00fe,0x00f0,0x00e2
	,0x00d6,0x00ca,0x00be,0x00b4,0x00aa,0x00a0,0x0097,0x008f,0x0087,0x007f,0x0078,0x0071

	,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071
	,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071,0x0071
};


/*--- Function prototypes -----------------------------------------*/
void	NEW_NOTE(int voc);
void	EFFECTS(int voc);
void	CheckSequence(void);
void	setSample(int voc, int reinit);
/*-----------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
void	setSample(int voc, int Kick)
/*-----------------------------------------------------------------------*/
{
int	SmpNum = VXdata[voc].SamplN;

	
//	if (SmpNum == 0x29) SmpNum--;

//	if (SoundInfo[SmpNum].Length < 4) { VGF1_StopVoice(voc); return; }


	VGF1_SetVoiceL(voc
		,SoundInfo[SmpNum].Start
		,SoundInfo[SmpNum].Length);


	if ((VXdata[voc].ON) && (Kick))
	{
		VGF1_PlayVoice(voc);
/*
		if (SoundInfo[SmpNum].LoopS >= SoundInfo[SmpNum].Length)
		{
			VGF1_SetVoiceL(voc
				,SoundInfo[SmpNum].Start
				,2);

		}
*/
		if (SoundInfo[SmpNum].LoopS < SoundInfo[SmpNum].Length)
		{
			VGF1_SetVoiceL(voc
				,SoundInfo[SmpNum].Start
				,SoundInfo[SmpNum].Length - SoundInfo[SmpNum].LoopS);

		}
	}


}




/*-----------------------------------------------------------------------*/
int		FC6_Open(DWORD User, int m, int n)
/*-----------------------------------------------------------------------*/
{
		FC16Mod.onoff = 0;

		InitializeCriticalSection(&InfoCS);
		return VGF1_Open(FC6_PLAY_MUSIC, User, m, n);
}


/*-----------------------------------------------------------------------*/
void	FC6_Close(void)
/*-----------------------------------------------------------------------*/
{
		FC16Mod.onoff = 0;

		VGF1_Close();
		DeleteCriticalSection(&InfoCS);
}

/*-----------------------------------------------------------------------*/
void	FC6_Start(void)
/*-----------------------------------------------------------------------*/
{
		FC16Mod.onoff = 1;
		DataInfoPos = 0;
		VGF1_Reset();
		VGF1_Start();
		DataPlayPos = 0;
}

/*-----------------------------------------------------------------------*/
void	FC6_Stop(void)
/*-----------------------------------------------------------------------*/
{
		if (!FC6_IsPlaying()) return;

		FC16Mod.onoff = 0;
		VGF1_Stop();
}

/*-----------------------------------------------------------------------*/
int		FC6_IsPlaying(void)
/*-----------------------------------------------------------------------*/
{
		return FC16Mod.onoff;
}



/*-----------------------------------------------------------------------*/
int		FC6_Init(SBYTE *ModAddr)
/*-----------------------------------------------------------------------*/
{
SBYTE	*p;
SLONG	Start;
int		i;

		FC6_Stop();

		FC16Mod.onoff   = 0;

		FC16Mod.MODaddr = ModAddr;
		FC16Mod.SMPaddr = ModAddr + FC_GET_SLONG_AT(ModAddr+12);
//		FC16Mod.SMPsize = FC_GET_SLONG_AT(ModAddr+16);
//		FC16Mod.WAVsize = FC_GET_SLONG_AT(ModAddr+20);

		FC16Mod.SEQnum	= FC_GET_SWORD_AT(ModAddr+0x04);
		FC16Mod.PATnum	= FC_GET_SWORD_AT(ModAddr+0x06);
		FC16Mod.VOLnum	= FC_GET_SWORD_AT(ModAddr+0x08);
		FC16Mod.FRQnum	= FC_GET_SWORD_AT(ModAddr+0x0A);

		FC16Mod.SEQendp = (FC16Mod.SEQnum * SEQSIZE);

		FC16Mod.SEQinfo = ModAddr + 0x90;
		FC16Mod.PATinfo = FC16Mod.SEQinfo + FC16Mod.SEQendp;
		FC16Mod.VOLinfo = FC16Mod.PATinfo + (FC16Mod.PATnum * 64);
		FC16Mod.FRQinfo = FC16Mod.VOLinfo + (FC16Mod.VOLnum * 64);

//--------------------------------------------------------

		Start = 0;
		for (p = ModAddr + 0x18, i = 0; i < 10; i++, p += 4)
		{
			SoundInfo[i].Start  = Start;
			SoundInfo[i].Length = FC_GET_SWORD_AT(p);
			SoundInfo[i].LoopS  = SoundInfo[i].Length;
			Start += SoundInfo[i].Length;
		}

		Start += 16;
		for (p = ModAddr + 0x40, i = 10; i < 90; i++, p++)
		{
			SoundInfo[i].Start  = Start;
			SoundInfo[i].Length = (*p << 1);
			SoundInfo[i].LoopS = 0;
			Start += SoundInfo[i].Length;
			Start += 2;
		}

//--------------------------------------------------------

		p = FC16Mod.SEQinfo;

		// get replay speed, if 0 use default value (3);
//		FC16Mod.REPspd = 5;
		FC16Mod.REPspd = (!p[12]) ? 3 : p[12];
		FC16Mod.REPcnt = 1;
		FC16Mod.PAToffs = 64;
//		FC16Mod.SEQoffs = FC16Mod.SEQendp;
		FC16Mod.SEQoffs = (SEQSIZE * (SEQSTART-1));


		for (p = FC16Mod.SEQinfo, i = 0; i < 4; i++, p += 3)
		{
			memset((VXdata+i), 0, sizeof(tVOCSTR));

			VXdata[i].VoiceN = i;
			VXdata[i].VolTspd = 1;
			VXdata[i].VolTcnt = 1;

			VXdata[i].PatPtr = FC16Mod.PATinfo + (p[0] * 64);
			VXdata[i].NoteTR = p[1];	// Transpose value
			VXdata[i].SampTR = p[2];	// SoundTranspose value

			VXdata[i].VolPtr = &TabNULL[5];
			VXdata[i].FrqPtr = &TabNULL[5];
			VGF1_StopVoice(i);
		}
		TabNULL[5] = (SBYTE)0xE1;

		VXdata[0].ON = CH00_ON;
		VXdata[1].ON = CH01_ON;
		VXdata[2].ON = CH02_ON;
		VXdata[3].ON = CH03_ON;

//---------------------------------------------------------

		p = FC16Mod.SMPaddr;
		for (i = 0; i < 10; i++)
		{
		  if (SoundInfo[i].Length > 0)
		  {
			VGF1_PokeBlock(SoundInfo[i].Start,(char*)p,SoundInfo[i].Length);
			p += SoundInfo[i].Length;
		  }	
		}


//		p = FC16Mod.SMPaddr + FC16Mod.SMPsize;
		for (i = 10; i < 90; i++)
		{
		  if (SoundInfo[i].Length > 0)
		  {
			VGF1_PokeBlock(SoundInfo[i].Start,(char*)p,SoundInfo[i].Length);
			p += SoundInfo[i].Length;
		  }
		}

		return 0;
}



/*-----------------------------------------------------------------------*/
void	FC6_PLAY_MUSIC(void)
/*-----------------------------------------------------------------------*/
{
int	i;
		if (!FC16Mod.onoff) return;

		if (--FC16Mod.REPcnt) goto noNewNote;

		if (FC16Mod.PAToffs < 64) goto noNewPatt;
		FC16Mod.PAToffs = 0;
		FC16Mod.SEQoffs += SEQSIZE;
		CheckSequence();
noNewPatt:
		FC16Mod.REPcnt = FC16Mod.REPspd;	// Restore replayspeed counter
		NEW_NOTE(0);
		NEW_NOTE(1);
		NEW_NOTE(2);
		NEW_NOTE(3);

		FC16Mod.PAToffs += 2;
noNewNote:
		EFFECTS(0);
		EFFECTS(1);
		EFFECTS(2);
		EFFECTS(3);

//-------------------

		EnterCriticalSection(&InfoCS);

		DataInfo[DataInfoPos].ActRow = FC6_GetPATrow();
		DataInfo[DataInfoPos].RepCnt = FC6_GetREPcnt();
		DataInfo[DataInfoPos].SeqPtr = FC6_GetSEQtable();
		DataInfo[DataInfoPos].SeqRow = FC16Mod.SEQoffs / SEQSIZE;

		DataInfo[DataInfoPos].MixBuffer = FC6_GetMixBuffer();
		for (i = 0; i < 4; i++)
		{
			DataInfo[DataInfoPos].Chan[i].ON = (VXdata[i].ON & FC16Mod.onoff);

			DataInfo[DataInfoPos].Chan[i].PatPtr = FC6_GetPATtable(i);

			DataInfo[DataInfoPos].Chan[i].VolPtr = FC6_GetVOLtable(i);
			DataInfo[DataInfoPos].Chan[i].VolOfs = FC6_GetOfs(i,0);
			DataInfo[DataInfoPos].Chan[i].VolOfsVal = FC6_GetOfsVAL(i,0);
			DataInfo[DataInfoPos].Chan[i].VolOfsSpd = FC6_GetOfsSPD(i,0);

			DataInfo[DataInfoPos].Chan[i].FrqPtr = FC6_GetFRQtable(i);
			DataInfo[DataInfoPos].Chan[i].FrqOfs = FC6_GetOfs(i,1);
			DataInfo[DataInfoPos].Chan[i].FrqOfsVal = FC6_GetOfsVAL(i,1);
			DataInfo[DataInfoPos].Chan[i].FrqOfsSpd = FC6_GetOfsSPD(i,1);

			DataInfo[DataInfoPos].Chan[i].Period = FC6_GetPeriod(i);
			DataInfo[DataInfoPos].Chan[i].Volume = VXdata[i].Volume;
			DataInfo[DataInfoPos].Chan[i].DevVolume = VGF1_GetVol(i);

			DataInfo[DataInfoPos].Chan[i].SampleOfs = VGF1_GetSampleOfs(i);
			DataInfo[DataInfoPos].Chan[i].SampleSiz = VGF1_GetSampleSize(i);
		}

		DataPlayPos++; DataPlayPos %= 80;
		DataInfoPos++; DataInfoPos %= 80;

		LeaveCriticalSection(&InfoCS);
}


/*-----------------------------------------------------------------------*/
void	CheckSequence(void)
/*-----------------------------------------------------------------------*/
{
int		i;
SBYTE	*p;

	if (FC16Mod.SEQoffs >= FC16Mod.SEQendp) FC16Mod.SEQoffs = 0;

	if ((p = FC16Mod.SEQinfo + FC16Mod.SEQoffs) == NULL) return;

	if (p[12])	FC16Mod.REPspd = 
				FC16Mod.REPcnt = p[12];		// Get replay speed

	for (i = 0; i < 4; i++, p += 3)
	{
		VXdata[i].PatPtr = 
			FC16Mod.PATinfo + (p[0] * 64);	// Pattern to play
			VXdata[i].NoteTR = p[1];		// Transpose value
			VXdata[i].SampTR = p[2];		// SoundTranspose value
	}
}










/*-------------------------------------------------------------------*/
void	NEW_NOTE(int voc)
/*-------------------------------------------------------------------*/
{
SBYTE	*p, note, info, temp;

	if (VXdata[voc].PatPtr == NULL) return;

	p = VXdata[voc].PatPtr + FC16Mod.PAToffs;
	info = p[1];	// Get info byte #1 (bl)
	note = p[0];	// Get note			(al)
	if (note) goto ww1;
	if (!(info &= 0xC0)) goto noport;
	goto ww11;
ww1:
	VXdata[voc].Period = 0;
ww11:
	VXdata[voc].PortaV = 0;
	if (!(info & 0x80)) goto noport;
	VXdata[voc].PortaV = p[3];
noport:
	if (!(note &= 0x7F)) return;
	VXdata[voc].Note = note;
	VXdata[voc].Info = info = p[1];

	VGF1_StopVoice(voc);

	info = (info & 0x3F);
	info += VXdata[voc].SampTR;

	//Control #1
//	if (info >= FC16Mod.VOLnum) info = FC16Mod.VOLnum-1;

	if (info >= FC16Mod.VOLnum)
	{
		VXdata[voc].VolTspd = 0;
		VXdata[voc].VolTcnt = 0;
		VXdata[voc].VibSpd = 0;
		VXdata[voc].VibXOR = 0;
		VXdata[voc].VibAmp = 0; 
		VXdata[voc].VibDel = 0;
		VXdata[voc].FrqOfs = 0;
		VXdata[voc].VolSus = 0; 
		VXdata[voc].FrqSus = 0;
		VXdata[voc].VolOfs = 0;

		VXdata[voc].VolPtr = &TabNULL[5];
		VXdata[voc].FrqPtr = &TabNULL[5];
		return;
	}

	p = FC16Mod.VOLinfo + (info * 64);
	VXdata[voc].VolOfs = 0;
	
	VXdata[voc].VolTspd = p[0];
	VXdata[voc].VolTcnt = VXdata[voc].VolTspd;

	temp = p[1];

	//Control #2
	if (temp >= FC16Mod.FRQnum) temp = FC16Mod.FRQnum-1;

	VXdata[voc].VibSpd = p[2];
	VXdata[voc].VibXOR = 0x40;
	VXdata[voc].VibAmp = VXdata[voc].VibAmpC = p[3];
	VXdata[voc].VibDel = p[4];
	p += 5;
	VXdata[voc].VolPtr = p;
	VXdata[voc].FrqPtr = FC16Mod.FRQinfo + (temp * 64);

	VXdata[voc].FrqOfs = VXdata[voc].VolSus = VXdata[voc].FrqSus = 0;


}






/*-----------------------------------------------------------------------*/
void	EFFECTS(int voc)
/*-----------------------------------------------------------------------*/
{
tVOCSTR	*v = &VXdata[voc];

UBYTE	val;
SBYTE	*p, tmp;
SLONG	TabOffset;
SWORD	Period;



	if (v == NULL) return;



//<<<<<<<<<<<<<<<<<<<<<<<<<<
// FREQUENCY EFFECTS
//<<<<<<<<<<<<<<<<<<<<<<<<<<
testsustain:
	if (VXdata[voc].FrqSus == 0) goto sustzero;
	VXdata[voc].FrqSus--;
	goto VOLUfx;
sustzero:
	p = VXdata[voc].FrqPtr + VXdata[voc].FrqOfs;
testeffects:
	val = p[0];
	if (val == 0xE1) goto VOLUfx;

	if (val != 0xE0) goto testnewsound;	// E0 = loop to point of FRQtable

	VXdata[voc].FrqOfs = (SLONG)(p[1] & 0x3F);
	p = VXdata[voc].FrqPtr + VXdata[voc].FrqOfs;
	val = p[0];

testnewsound:
	if (val != 0xE2) goto testE4;		// E2 = set waveform (kick!)

	VXdata[voc].SamplN = p[1] & 0x3F;
	setSample(voc,1);
	VXdata[voc].VolOfs = 0;
	VXdata[voc].VolTcnt = 1;
	VXdata[voc].FrqOfs += 2;
	goto transpose;

testE4:
	if (val != 0xE4) goto testE9;		// E4 = change waveform (no kick!)
	VXdata[voc].SamplN = p[1] & 0x3F;
	setSample(voc,0);
	VXdata[voc].FrqOfs += 2;
	goto transpose;

testE9:
	if (val != 0xE9) goto testpatjmp;
	VXdata[voc].FrqOfs += 3;

testpatjmp:
	if (val != 0xE7) goto testpitchbend;
	VXdata[voc].FrqOfs = 0;
	p = VXdata[voc].FrqPtr = FC16Mod.FRQinfo + (p[1] * 64);
	goto testeffects;

testpitchbend:
	if (val != 0xEA) goto testnewsustain;
	VXdata[voc].PBend  = p[1];
	VXdata[voc].PBendC = p[2];
	VXdata[voc].FrqOfs += 3;
	goto transpose;

testnewsustain:
	if (val != 0xE8) goto testnewvib;
	VXdata[voc].FrqSus = p[1];
	VXdata[voc].FrqOfs += 2;
	goto testsustain;

testnewvib:
	if (val != 0xE3) goto transpose;
	VXdata[voc].FrqOfs += 3;
	VXdata[voc].VibSpd = p[1];
	VXdata[voc].VibAmp = p[2];

transpose:
	p = VXdata[voc].FrqPtr + VXdata[voc].FrqOfs;
	VXdata[voc].FreqTR = p[0];
	VXdata[voc].FrqOfs++;



//<<<<<<<<<<<<<<<<<<<<<<<<<<
// VOLUME EFFECTS
//<<<<<<<<<<<<<<<<<<<<<<<<<<
VOLUfx:
	if (!VXdata[voc].VolSus) goto volsustzero;
	VXdata[voc].VolSus--;
	goto calcperiod;

volsustzero:
	if (VXdata[voc].VBendC) goto do_VOLbend;
	if (--VXdata[voc].VolTcnt) goto calcperiod;
	VXdata[voc].VolTcnt = VXdata[voc].VolTspd;

volu_cmd:
	p = VXdata[voc].VolPtr + VXdata[voc].VolOfs;
	val = p[0];

//testvoluend:
	if (val == 0xE1) goto calcperiod;		// VolSeq END
	if (val != 0xEA) goto testVOLsustain;	// VolSeq NEWBEND
	VXdata[voc].VBendS = p[1];
	VXdata[voc].VBendC = p[2];
	VXdata[voc].VolOfs += 3;
	goto do_VOLbend;

testVOLsustain:
	if (val != 0xE8) goto testVOLloop;		// VolSeq SUSTAIN
	VXdata[voc].VolSus = p[1];
	VXdata[voc].VolOfs += 2;
	goto calcperiod;

testVOLloop:
	if (val != 0xE0) goto setvolume;		// VolSeq LOOP
	VXdata[voc].VolOfs = (p[1] & 0x3F) - 5;
	goto volu_cmd;

do_VOLbend:
	VXdata[voc].VBendX ^= 0xFF;				// DO VOLBend
	if (!VXdata[voc].VBendX) goto calcperiod;
	VXdata[voc].VBendC--;
	VXdata[voc].Volume += VXdata[voc].VBendS;

	if (!(VXdata[voc].VBendS & 0x80)) goto calcperiod;
	VXdata[voc].Volume = VXdata[voc].VBendC = 0;
	goto calcperiod;

setvolume:
	VXdata[voc].Volume = p[0];
	VXdata[voc].VolOfs++;



//<<<<<<<<<<<<<<<<<<<<<<<<<<
// CALCULATE PERIOD
//<<<<<<<<<<<<<<<<<<<<<<<<<<
calcperiod:

	tmp = VXdata[voc].FreqTR;
	if (tmp < 0) goto lockednote;		// is < 0? -> locked note

	tmp += VXdata[voc].Note;			// +Note
	tmp += VXdata[voc].NoteTR;			// +NoteTR

lockednote:
	tmp &= 0x7F;						// use only lower 7bit
	TabOffset = (tmp * 2);				// (TABOFS OF NOTE*2)
	Period = PERIODS[tmp];

	VXdata[voc].PER = Period;	// DEBUG


//<<<<<<<<<<<<<<<<<<<<<<<<<<
// VIBRATOR
//<<<<<<<<<<<<<<<<<<<<<<<<<<


	__asm
	{
		mov	 edi,v
		xor	 ecx,ecx

		mov	 ebx,TabOffset			; BX = d5 = TabOffset (idx*2)
		mov	 cl,[edi]v.VibXOR		; CL = d7 = VibXORval
		cmp	 [edi]v.VibDel,0
		jz	 vibrator
		dec	 [edi]v.VibDel
		jmp	 novibrato
vibrator:
		xor	 edx,edx
		mov	 ch,[edi]v.VibAmp
		add	 ch,ch					; CH = d4 = VibAmp * 2
		mov	 dl,[edi]v.VibAmpC		; DL = d1 = VibAmpCounter
		test cl,80h
		jz	 vib1
		test cl,1
		jnz	 vib4
vib1:	
		test cl,20h
		jnz	 vib2
		sub	 dl,[edi]v.VibSpd
		jnc	 vib3
		or	 cl,20h
		xor	 edx,edx
		jmp	 vib3
vib2:	
		add	 dl,[edi]v.VibSpd
		cmp	 dl,ch
		jc	 vib3
		and	 cl,0DFh
		mov	 dl,ch
vib3:
		mov	 [edi]v.VibAmpC,dl
vib4:
		shr  ch,1
		sub	 dl,ch
		jnc	 vib5
		sub	 dx,0100h
vib5:
		add	 bl,0A0h
		jc	 vib7
vib6:
		add	 dx,dx
		add	 bl,018h
		jnc	 vib6
vib7:
		add  Period,dx
novibrato:
		xor	 cl,1
		mov	 [edi]v.VibXOR,cl
	}



//<<<<<<<<<<<<<<<<<<<<<<<<<<
// PORTAMENTO
//<<<<<<<<<<<<<<<<<<<<<<<<<<
	__asm 
	{
		mov	 edi,v
		xor	 [edi]v.PortaX,1		; every 2 frames!!!
		jz	 noporta

		xor	 ebx,ebx
		mov	 bl,[edi]v.PortaV
		or	 bl,bl
		jz	 noporta
		cmp	 bl,1Fh
		jbe	 portaup
//portadown: 
		and	 bl,1Fh
		neg	 bx
portaup:
		sub  [edi]v.Period,bx
noporta:

	}



//<<<<<<<<<<<<<<<<<<<<<<<<<<
// PITCH BEND
//<<<<<<<<<<<<<<<<<<<<<<<<<<
	__asm
	{
		mov	 edi,v
		xor	 [edi]v.PBendX,1		; every 2 frames!!!
		jz	 addporta

		cmp	 [edi]v.PBendC,0
		jz	 addporta
		dec	 [edi]v.PBendC
		xor	 ebx,ebx
		mov	 bl,[edi]v.PBend
		test bl,80h
		jz	 pitchup
		and  bx,0FFh
pitchup:
		sub	 [edi]v.Period,bx
addporta:
	}

//<<<<<<<<<<<<<<<<<<<<<<<<<<
// CHECK LIMITS
//<<<<<<<<<<<<<<<<<<<<<<<<<<

		Period += VXdata[voc].Period;
		if (Period <= 0x0070) Period = 0x0071;
		if (Period >  0x0d60) Period = 0x0d60;

		VGF1_SetPeriod(voc, Period);
		VGF1_SetVol(voc, VXdata[voc].Volume);

		VXdata[voc].PER2 = Period;	// DEBUG
}




/*-----------------------------------------------------------------------*/
SWORD*	FC6_GetMixBuffer(void)
/*-----------------------------------------------------------------------*/
{
	return VGF1_GetMixBuffer();
}



/*-----------------------------------------------------------------------*/
int		FC6_GetPATrow(void)
/*-----------------------------------------------------------------------*/
{
	return (FC16Mod.PAToffs / 2);
}

/*-----------------------------------------------------------------------*/
int		FC6_GetREPcnt(void)
/*-----------------------------------------------------------------------*/
{
	return (FC16Mod.REPcnt);
}

/*-----------------------------------------------------------------------*/
SBYTE*	FC6_GetPATtable(int voc)
/*-----------------------------------------------------------------------*/
{
	return VXdata[voc].PatPtr;
}

/*-----------------------------------------------------------------------*/
SBYTE*	FC6_GetVOLtable(int voc)
/*-----------------------------------------------------------------------*/
{
	return VXdata[voc].VolPtr;
}

/*-----------------------------------------------------------------------*/
SBYTE*	FC6_GetFRQtable(int voc)
/*-----------------------------------------------------------------------*/
{
	return VXdata[voc].FrqPtr;
}

/*-----------------------------------------------------------------------*/
SBYTE*	FC6_GetSEQtable(void)
/*-----------------------------------------------------------------------*/
{
	return (FC16Mod.SEQinfo + FC16Mod.SEQoffs);
}

/*-----------------------------------------------------------------------*/
SBYTE	FC6_GetOfs(int voc, int frq)
/*-----------------------------------------------------------------------*/
{
	if (!frq) return (SBYTE)VXdata[voc].VolOfs;
	return (SBYTE)VXdata[voc].FrqOfs;
}

/*-----------------------------------------------------------------------*/
SBYTE	FC6_GetOfsVAL(int voc, int frq)
/*-----------------------------------------------------------------------*/
{
	if (!frq) 
		return (SBYTE)((VXdata[voc].VolPtr + VXdata[voc].VolOfs)[0]);

	return (SBYTE)((VXdata[voc].FrqPtr + VXdata[voc].FrqOfs)[0]);
}

/*-----------------------------------------------------------------------*/
SBYTE	FC6_GetOfsSPD(int voc, int frq)
/*-----------------------------------------------------------------------*/
{
	if (!frq) return (SBYTE)VXdata[voc].VolTspd;
	return (SBYTE)VXdata[voc].VolTspd;
}

/*-----------------------------------------------------------------------*/
ULONG	FC6_GetPeriod(int voc)
/*-----------------------------------------------------------------------*/
{
	return (((ULONG)VXdata[voc].PER) << 16) | (VXdata[voc].PER2);
}

/*-----------------------------------------------------------------------*/
void	FC6_GoNextSeq(void)
/*-----------------------------------------------------------------------*/
{
		FC16Mod.PAToffs = 64;
		FC16Mod.REPcnt = 1;
//		VGF1_Reset();
}


/*-----------------------------------------------------------------------*/
void	FC6_GoPrevSeq(void)
/*-----------------------------------------------------------------------*/
{
		FC16Mod.SEQoffs -= (SEQSIZE * 2);

		if (FC16Mod.SEQoffs < 0) 
			FC16Mod.SEQoffs = FC16Mod.SEQendp - SEQSIZE;

		FC6_GoNextSeq();
}



/*-----------------------------------------------------------------------*/
tDataInfo*	FC6_GetInfo(void)
/*-----------------------------------------------------------------------*/
{
tDataInfo* p;

		EnterCriticalSection(&InfoCS);
		p = &DataInfo[DataPlayPos];
		LeaveCriticalSection(&InfoCS);
		return p;
}


/*-----------------------------------------------------------------------*/
void	FC6_PutInfo(tDataInfo* dst)
/*-----------------------------------------------------------------------*/
{
		EnterCriticalSection(&InfoCS);
		memcpy(dst,&DataInfo[DataPlayPos],sizeof(tDataInfo));
		LeaveCriticalSection(&InfoCS);
}

/*-----------------------------------------------------------------------*/
tDataInfo*	FC6_GetInfoN(int n)
/*-----------------------------------------------------------------------*/
{
tDataInfo* p;

		EnterCriticalSection(&InfoCS);
		p = &DataInfo[n % 80];
		LeaveCriticalSection(&InfoCS);
		return p;
}

/*-----------------------------------------------------------------------*/
void	FC6_ChanONOFF(int chn)
/*-----------------------------------------------------------------------*/
{
	chn &= 3;

	if (VXdata[chn].ON)
	{
		VXdata[chn].ON = 0;
		VGF1_StopVoice(chn);
	}
	else
		VXdata[chn].ON = 1;
}

/*-----------------------------------------------------------------------*/
void	FC6_MusicONOFF()
/*-----------------------------------------------------------------------*/
{
	if (FC16Mod.onoff)
	{
		FC16Mod.onoff = 0;
		VGF1_SetChanMasterVol(0,0);
		VGF1_SetChanMasterVol(1,0);
		VGF1_SetChanMasterVol(2,0);
		VGF1_SetChanMasterVol(3,0);
	}
	else
	{
		FC16Mod.onoff = 1;
		VGF1_SetChanMasterVol(0,64);
		VGF1_SetChanMasterVol(1,64);
		VGF1_SetChanMasterVol(2,64);
		VGF1_SetChanMasterVol(3,64);
	}

}
