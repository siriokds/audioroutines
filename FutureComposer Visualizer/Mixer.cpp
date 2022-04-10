// 16bit 44100Hz Stereo Mixer with high precision
// 16bit decimal part of increment
// and linear interpolation (optional)

#include <windows.h>
#include "Mixer.h"
#include "DOut.h"


#define FRACBITS	12

#define	NUMCHN		4
#define	MAXVOL		64

#define	MIXFREQ		44100
#define	BUF_SIZE	(MIXFREQ/50)	// 882 samples every 1/50 sec.
#define BUF_WAIT	1

/*
                  CLOCK VALUES
                   NTSC   PAL        UNITS

Clock Constant   3579545 3546895   ticks per second
Clock Interval  0.279365 0.281937  microseconds per interval
*/

#define Period2Freq(p) (3546895/period) // Amiga PAL  (50Hz)
//#define Period2Freq(p) (3579545/period) // Amiga NTSC (60Hz)

//#define Period2Freq(p) (((8363L*1712L)/period)/4) // Amiga PAL (50Hz)

#define	VDRAM_LEN	(256 * 1024)		// 256Kb DRAM
#define VOLTABLEN	((MAXVOL+1) * 256 * sizeof(SWORD))
#define MIXBUFLEN	(BUF_SIZE * sizeof(SWORD))

typedef struct tagMixerChan
{
	UBYTE		MasterVol;
	UBYTE		PanVolumeL;
	UBYTE		PanVolumeR;

	BOOL		Active;

	SLONG		SampleSize;
	SLONG		SamplePos;


	UBYTE		Volume;
	SLONG		Freq;

	SLONG		PosIndex;

	// External buffered data
	SLONG		_SampleSize;
	SLONG		_SamplePos;

}	MIXCHAN;

static	int		Filter = 1;
static	int		BUF_MULT;
static	int		BUF_NUM;


static void		(*CallBackFunc)(void);

static HGLOBAL	hMixTotalMem;
static UBYTE	*MixTotalMem;

static MIXCHAN	MixChn[NUMCHN];
static SBYTE	*GF1Mem;
static SWORD	*VolTab;
static SWORD	*MixBuf;

static CRITICAL_SECTION	csVGF1;	// to protect external data access!

static int		MixerStress = 0;	
static BOOL		MixerRunning = FALSE;
static BOOL		Started = FALSE;	
//-------------------------------------------------------------
//-------------------------------------------------------------
static  void	MixFillBuffer(void);
static  void	AddStereoNormal(int voc, SWORD *dst, SLONG todo);
//-------------------------------------------------------------
//-------------------------------------------------------------

/*-----------------------------------------------------------------------*/
void	VGF1_FilterONOFF(void)
/*-----------------------------------------------------------------------*/
{
	Filter = (!Filter) ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
int		VGF1_GetSampleOfs(int voc)
/*-----------------------------------------------------------------------*/
{
	if ((voc < 0) || (voc >= NUMCHN)) return 0;

	return MixChn[voc].SamplePos;
}

/*-----------------------------------------------------------------------*/
int		VGF1_GetSampleSize(int voc)
/*-----------------------------------------------------------------------*/
{
	if ((voc < 0) || (voc >= NUMCHN)) return 0;

	return MixChn[voc].SampleSize;
}

/*-----------------------------------------------------------------------*/
void	MixInitVolTab(void)
/*-----------------------------------------------------------------------*/
{
SWORD	*pVolTab = VolTab;
int		vol,byt;

		for (byt = 0; byt < 256; byt++) *pVolTab++ = 0;

		for (vol = 1; vol < (MAXVOL+1); vol++, pVolTab += 256)
		{
  			for (byt = -128; byt < 128; byt++)
			  pVolTab[(UBYTE)byt] = (byt * vol * 256) / MAXVOL / 4;
		}
}

/*-----------------------------------------------------------------------*/
int		VGF1_Open(void (*callb)(void), DWORD User, int _MULT, int _NUM)
/*-----------------------------------------------------------------------*/
{
int		err,voc;
UBYTE	*p;

	BUF_MULT = (_MULT < 4) ? 4 : _MULT;
	BUF_NUM  = (_NUM  < 4) ? 4 : _NUM;

	if (
		(hMixTotalMem = GlobalAlloc(
			  GPTR,
			  VOLTABLEN				// Allocate Voltab
			+ VDRAM_LEN				// Allocate DRAM
			+ (MIXBUFLEN*BUF_MULT*2)// Allocate MixBuf
			+ 2048)					// +2Kb more to be sure...
		) == NULL) return 2;
	
	if ((MixTotalMem = (UBYTE*) GlobalLock(hMixTotalMem)) == NULL)
	{
		GlobalFree(hMixTotalMem);
		return 2;
	}

//-------------------------------------------------------------------------
	p = MixTotalMem + 1024;

	VolTab = (SWORD*)(p);
	GF1Mem = (SBYTE*)(p + VOLTABLEN);
	MixBuf = (SWORD*)(p + VOLTABLEN + VDRAM_LEN);

	MixInitVolTab();

//-------------------------------------------------------------------------
	CallBackFunc = callb;

	for (voc = 0; voc < NUMCHN; voc++)
	{
		MixChn[voc].MasterVol = 64;
		MixChn[voc].Active = FALSE;

		MixChn[voc].SampleSize = 0;
		MixChn[voc].SamplePos = 0;

		MixChn[voc]._SampleSize = 0;
		MixChn[voc]._SamplePos = 0;

		MixChn[voc].Volume = 0;
		MixChn[voc].Freq = 0;
		MixChn[voc].PosIndex = 0;
	}


	MixChn[0].PanVolumeL = 0x40;
	MixChn[0].PanVolumeR = 0x00;

	MixChn[1].PanVolumeL = 0x40;
	MixChn[1].PanVolumeR = 0x00;

	MixChn[2].PanVolumeL = 0x40;
	MixChn[2].PanVolumeR = 0x00;

	MixChn[3].PanVolumeL = 0x40;
	MixChn[3].PanVolumeR = 0x00;


//-------------------------------------------------------------------------
	err = DOut_Open(MixFillBuffer, BUF_SIZE*BUF_MULT, BUF_NUM, User);

	if (!err) InitializeCriticalSection(&csVGF1);

	return err;
}

/*-----------------------------------------------------------------------*/
void	VGF1_Start(void)
/*-----------------------------------------------------------------------*/
{
		MixerRunning = TRUE;
//		if (Started == FALSE) DOut_Start();
		DOut_Start();
		Started = TRUE;
}

/*-----------------------------------------------------------------------*/
void	VGF1_Stop(void)
/*-----------------------------------------------------------------------*/
{
		MixerRunning = FALSE;
		DOut_Stop();
}

/*-----------------------------------------------------------------------*/
int		VGF1_Close(void)
/*-----------------------------------------------------------------------*/
{
		MixerRunning = FALSE;
		DOut_Close();

		DeleteCriticalSection(&csVGF1);

		GlobalUnlock(hMixTotalMem);
		GlobalFree(hMixTotalMem);
		return 0;
}



/*-----------------------------------------------------------------------*/
char	VGF1_Peek(int addr)
/*-----------------------------------------------------------------------*/
{
		if (GF1Mem == NULL) return 0;
		addr &= VDRAM_LEN-1;
		return GF1Mem[addr];
}

/*-----------------------------------------------------------------------*/
void	VGF1_Poke(int addr, char val)
/*-----------------------------------------------------------------------*/
{
		if (GF1Mem == NULL) return;
		addr &= VDRAM_LEN-1;
		GF1Mem[addr] = val;
}

/*-----------------------------------------------------------------------*/
void	VGF1_PokeBlock(int Dst, char *Src, int Size)
/*-----------------------------------------------------------------------*/
{
int	i;
		for (i = 0; i < Size-1; i++)
			VGF1_Poke(Dst++,Src[i]);

		VGF1_Poke(Dst++,(Src[Size-1] - Src[0]) / 2);

		VGF1_Poke(Dst++,Src[0]);
		VGF1_Poke(Dst++,Src[1]);
}


/*-----------------------------------------------------------------------*/
void	VGF1_PokeBlock16(int Dst, short *Src, int Size)
/*-----------------------------------------------------------------------*/
{
int	i;
		for (i = 0; i < Size; i++)
			VGF1_Poke(Dst++, (char)(Src[i]>>8) );
}


/*-----------------------------------------------------------------------*/
void	VGF1_PokeBlockD(int Dst, char *Src, int Size)
/*-----------------------------------------------------------------------*/
{
int	i;
		for (i = 0; i < Size; i++)
		{
			VGF1_Poke(Dst++,Src[i]);
			VGF1_Poke(Dst++,Src[i]);
		}

		VGF1_Poke(Dst++,Src[0]);
		VGF1_Poke(Dst++,Src[0]);
}

/*-----------------------------------------------------------------------*/
void	VGF1_PokeBlockQ(int Dst, char *Src, int Size)
/*-----------------------------------------------------------------------*/
{
int	i;
		for (i = 0; i < Size; i++)
		{
			VGF1_Poke(Dst++,Src[i]);
			VGF1_Poke(Dst++,Src[i]);
			VGF1_Poke(Dst++,Src[i]);
			VGF1_Poke(Dst++,Src[i]);
		}

		VGF1_Poke(Dst++,Src[0]);
		VGF1_Poke(Dst++,Src[0]);
		VGF1_Poke(Dst++,Src[0]);
		VGF1_Poke(Dst++,Src[0]);
}

/*-----------------------------------------------------------------------*/
void	VGF1_PokeBlockX(int Dst, char *Src, int Size, int XOR)
/*-----------------------------------------------------------------------*/
{
int	i;
		for (i = 0; i < Size; i++)
			VGF1_Poke(Dst++, Src[i] + XOR);
}


/*-----------------------------------------------------------------------*/
void	VGF1_SetFreq(int voc, int freq)
/*-----------------------------------------------------------------------*/
{
		MixChn[voc].Freq = freq & 0xFFFF;
		if (MixChn[voc].Freq < 2) MixChn[voc].Freq = 2;
}

/*-----------------------------------------------------------------------*/
void	VGF1_SetPeriod(int voc, int period)
/*-----------------------------------------------------------------------*/
{
	if (!period) { VGF1_StopVoice(voc); return; }
	
	if (period < 0x40)  period = 0x40;
	if (period > 0x6B00) period = 0x6B00;
	
	VGF1_SetFreq(voc, Period2Freq(period));
}


/*-----------------------------------------------------------------------*/
void	VGF1_SetVol(int voc, int vol)
/*-----------------------------------------------------------------------*/
{
		if (vol < 0) vol = 0;
		if (vol > MAXVOL) vol = MAXVOL;
		MixChn[voc].Volume = vol;
}


/*-----------------------------------------------------------------------*/
int		VGF1_GetVol(int voc)
/*-----------------------------------------------------------------------*/
{
		return MixChn[voc].Volume;
}



/*-----------------------------------------------------------------------*/
void	VGF1_SetVoiceL(int voc, int VB, int VL)
/*-----------------------------------------------------------------------*/
{
		if (VB >= VDRAM_LEN) return;

		MixChn[voc]._SamplePos = VB;
		MixChn[voc]._SampleSize = VL;
}


/*-----------------------------------------------------------------------*/
void	VGF1_PlayVoice(int voc)
/*-----------------------------------------------------------------------*/
{
		MixChn[voc].Active = TRUE;
		MixChn[voc].PosIndex = 0;

		MixChn[voc].SamplePos = MixChn[voc]._SamplePos;
		MixChn[voc].SampleSize = MixChn[voc]._SampleSize;

}

/*-----------------------------------------------------------------------*/
void	VGF1_StopVoice(int voc)
/*-----------------------------------------------------------------------*/
{
		MixChn[voc].Active = FALSE;
}


/*--------------------------------------------------------------*/
static void	FirMemory(SWORD *d, int len)
/*--------------------------------------------------------------*/
{
int	i, j;

	for (i = 1; i < len-1; i++)
	{
		j = i*2;
		d[j] = (d[j-2] / 4) + (d[j] / 2) + (d[j+2] / 4);
		j++;
		d[j] = (d[j-2] / 4) + (d[j] / 2) + (d[j+2] / 4);

	}

/*
	for (i = 2; i < len-2; i++)
	{
		j = i*2;
		d[j] = (d[j-4] / 16) + (d[j-2] / 8) + (d[j] / 2) + (d[j+2] / 8) + (d[j+4] / 16);
		j++;
		d[j] = (d[j-4] / 16) + (d[j-2] / 8) + (d[j] / 2) + (d[j+2] / 8) + (d[j+4] / 16);

	}
*/
}


/*--------------------------------------------------------------*/
static void	MixFillBuffer(void)
/*--------------------------------------------------------------*/
{
int		i, chn, TotStress, Stress;
SWORD	*p;

		if (MixerRunning == FALSE) return;

		p = MixBuf;
		TotStress = 0;

		for (i = 0; i < BUF_MULT; i++)
		{
			EnterCriticalSection(&csVGF1);
			Stress = timeGetTime();

			// Call External PlayRoutine
			if (CallBackFunc) (*CallBackFunc)();

			// 1) clear BUFFER
			ZeroMemory(p, MIXBUFLEN*2);

			// 2) Mix Channels
			for (chn = 0; chn < NUMCHN; chn++) 
				AddStereoNormal(chn, p, BUF_SIZE);


			p += (BUF_SIZE*2);	// 882 * BUF_NUM

			Stress = timeGetTime() - Stress;
			TotStress += Stress;
			LeaveCriticalSection(&csVGF1);

			Sleep(BUF_WAIT);

		}



Stress = timeGetTime();

		if (Filter) FirMemory(MixBuf,   (MIXBUFLEN/2)*BUF_MULT );

Stress = timeGetTime() - Stress;
TotStress += Stress;

		EnterCriticalSection(&csVGF1);
MixerStress = TotStress;
		LeaveCriticalSection(&csVGF1);

		DOut_BufferCopy((void *)MixBuf);
}




/*---------------------------------------------------------------------------------------------------------------------*/
SLONG	MixStereoNormal(SBYTE *src, SWORD *dst, SLONG idx, SLONG inc, 
						SLONG todo, SWORD *Vol_L, SWORD *Vol_R)
/*---------------------------------------------------------------------------------------------------------------------*/
{
    SBYTE  sample1, sample2, sample3, sample4;
    int    remain;

	if ((src == NULL) || (dst == NULL) || 
		(Vol_L == NULL) || (Vol_R == NULL)) return idx;


    remain = todo & 3;
    
    for(todo>>=2; todo; todo--)
    {
        sample1 = src[idx >> FRACBITS];	idx  += inc;
        sample2 = src[idx >> FRACBITS];	idx  += inc;
        sample3 = src[idx >> FRACBITS];	idx  += inc;
        sample4 = src[idx >> FRACBITS];	idx  += inc;
        
        *dst++ += Vol_L[sample1];
        *dst++ += Vol_R[sample1];
        *dst++ += Vol_L[sample2];
        *dst++ += Vol_R[sample2];
        *dst++ += Vol_L[sample3];
        *dst++ += Vol_R[sample3];
        *dst++ += Vol_L[sample4];
        *dst++ += Vol_R[sample4];
    }


    while(remain--)
    {
        sample1 = src[idx >> FRACBITS];	idx += inc;
        *dst++ += Vol_L[sample1];
        *dst++ += Vol_R[sample1];
    }
    

    return idx;
}





void	AddStereoNormal(int voc, SWORD *dst, SLONG todo)
{
SWORD 	*pVolTab_L, *pVolTab_R;
SLONG	inc, lim, vol_L, vol_R;
SLONG	samplesUntilEnd, stepsUntilEnd, mixNow;

MIXCHAN	*chan = &MixChn[voc];


	if ((chan->Active == FALSE)	|| (chan->SampleSize < 4) || (chan->Freq < 16) ) return;

	vol_L = chan->Volume;
	vol_R = chan->Volume;

	vol_L = (vol_L * chan->PanVolumeL * chan->MasterVol) / 4096;
	vol_R = (vol_R * chan->PanVolumeR * chan->MasterVol) / 4096;


	
	inc = ((MixChn[voc].Freq << FRACBITS) / MIXFREQ);

	pVolTab_L = VolTab + (vol_L * 256);
	pVolTab_R = VolTab + (vol_R * 256);
	lim = chan->SampleSize << FRACBITS;

	while(todo > 0)
	{
		while(chan->PosIndex >= lim)
		{
			chan->PosIndex -= lim;
			chan->SamplePos = chan->_SamplePos;
			chan->SampleSize = chan->_SampleSize;
			lim = chan->SampleSize << FRACBITS;
		}

		if (chan->SampleSize < 4) return;

		


		samplesUntilEnd = lim - chan->PosIndex;



		stepsUntilEnd = samplesUntilEnd / inc;		
		if (samplesUntilEnd % inc) stepsUntilEnd++;


		mixNow = (stepsUntilEnd > todo) ? todo : stepsUntilEnd;
		todo -= mixNow;

		if (mixNow <= 0) return;


		if ((vol_L == 0) && (vol_R == 0)) chan->PosIndex += (mixNow * inc); else
		{
			chan->PosIndex = MixStereoNormal(	(SBYTE *) GF1Mem + MixChn[voc].SamplePos,
												dst, 
												chan->PosIndex, 
												inc, 
												mixNow, 
												pVolTab_L, 
												pVolTab_R);
		}



		dst += (mixNow * 2);
	}

}


/*
static void	AddStereoNormal(int voc, SWORD *dst, SLONG todo)
{
SWORD 	*pVolTab;
SBYTE	*src;
SLONG	idx, inc, lim;
int		vol;

	if ((MixChn[voc].SampleSize < 8) 
	||  (!MixChn[voc].Volume)
	||  (MixChn[voc].Freq < 128)
	||  (MixChn[voc].Active == FALSE)
	||	(MixChn[voc].MasterVol == 0) ) return;


	if ((voc == 1) || (voc == 2)) *dst++;

	src = (SBYTE *) GF1Mem + MixChn[voc].SamplePos;
	vol = (MixChn[voc].Volume * MixChn[voc].MasterVol) / 64;
	pVolTab = VolTab + (vol * 256);

	idx = MixChn[voc].PosIndex;
	inc = (MixChn[voc].Freq << FRACBITS) / MIXFREQ;
	lim = MixChn[voc].SampleSize << FRACBITS;


	while(todo--)
	{
		while(idx > lim) 
		{
			idx -= lim;
			MixChn[voc].SamplePos = MixChn[voc]._SamplePos;
			MixChn[voc].SampleSize = MixChn[voc]._SampleSize;

			if (MixChn[voc].SampleSize < 4)
			{
				VGF1_StopVoice(voc);
				todo = 0;
				break;
			}

			src = (SBYTE *) GF1Mem + MixChn[voc].SamplePos;
			lim = MixChn[voc].SampleSize << FRACBITS;
		}


		*dst += pVolTab[ src[idx >> FRACBITS] ];

		dst += 2;
		idx += inc;

	}
	MixChn[voc].PosIndex = idx;

}

*/







/*--------------------------------------------------------------*/
SWORD*	VGF1_GetMixBuffer(void)
/*--------------------------------------------------------------*/
{
	return (SWORD*)MixBuf;
}


/*--------------------------------------------------------------*/
void	VGF1_Reset(void)
/*--------------------------------------------------------------*/
{
int	voc;

	EnterCriticalSection(&csVGF1);
	for (voc = 0; voc < NUMCHN; voc++)
	{
		MixChn[voc].Active = FALSE;

		MixChn[voc].SampleSize = 0;
		MixChn[voc].SamplePos = 0;
		MixChn[voc].Volume = 0;
		MixChn[voc].Freq = 0;
		MixChn[voc].PosIndex = 0;
	}

	LeaveCriticalSection(&csVGF1);
}


/*-----------------------------------------------------------------------*/
void	VGF1_SetChanMasterVol(int chan, int vol)
/*-----------------------------------------------------------------------*/
{

	if ((chan < 0) || (chan > 3)) return;

	EnterCriticalSection(&csVGF1);
	if (vol == -1)
	{
		MixChn[chan].MasterVol = (MixChn[chan].MasterVol) ? 0 : 64;
	}
	else
	{
		vol = (vol<0) ? 0 : ((vol>64) ? 64 : vol);
		MixChn[chan].MasterVol = vol;
	}
	LeaveCriticalSection(&csVGF1);
}


/*-----------------------------------------------------------------------*/
int		VGF1_GetChanMasterVol(int chan)
/*-----------------------------------------------------------------------*/
{
int	vol;
	if ((chan < 0) || (chan > 3)) return 0;

	EnterCriticalSection(&csVGF1);
	vol = MixChn[chan].MasterVol;
	LeaveCriticalSection(&csVGF1);

	return vol;
}


/*-----------------------------------------------------------------------*/
int		VGF1_GetMixStress(void)
/*-----------------------------------------------------------------------*/
{
int	val;

	EnterCriticalSection(&csVGF1);
	val = MixerStress;
	LeaveCriticalSection(&csVGF1);
	return val;
}

/*-----------------------------------------------------------------------*/
int		VGF1_GetActiveChannels(void)
/*-----------------------------------------------------------------------*/
{
int	val = 0;
int	i;

	EnterCriticalSection(&csVGF1);
	for (i = 0; i < NUMCHN; i++)
		if ((MixChn[i].Active == TRUE)
		&& (MixChn[i].MasterVol>0)) val++;
	LeaveCriticalSection(&csVGF1);
	return val;
}

