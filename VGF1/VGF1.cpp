// 16bit 44100Hz Stereo Mixer with high precision
// 16bit decimal part of increment
// and linear interpolation (optional)

#include <windows.h>
#include "VGF1.h"


#define FRACBITS	16
#define FRACNUM		(1<<FRACBITS)
#define FRACMASK	(FRACNUM-1)

#define	NUMCHN		4
#define	MAXVOL		64

#define	MIXFREQ		44100
#define	BUF_SIZE	(MIXFREQ/50)	// 882 samples every 1/50 sec.
#define BUF_WAIT	1


//*************************************************************
//
//                  CLOCK VALUES
//                   NTSC   PAL        UNITS
//
//Clock Constant   3579545 3546895   ticks per second
//Clock Interval  0.279365 0.281937  microseconds per interval
//
//*************************************************************


#define Period2Freq(p) (3546895/period) // Amiga PAL  (50Hz)
//#define Period2Freq(p) (3579545/period) // Amiga NTSC (60Hz)




#define	VDRAM_LEN	(256 * 1024)		// 256Kb DRAM
#define VOLTABLEN	((MAXVOL+1) * 256 * sizeof(SWORD))

typedef struct tagMixerChan
{
	UBYTE		MasterVol;

	BOOL		Active;

	SLONG		SampleSize;
	SLONG		SamplePos;
	BOOL		Loop;
	SLONG		LoopS;


	UBYTE		Volume;
	SLONG		Freq;

	SLONG		PosIndexHI;
	SLONG		PosIndexLO;

	// External buffered data
	SLONG		_SampleSize;
	SLONG		_SamplePos;
	BOOL		_Loop;
	SLONG		_LoopS;

}	MIXCHAN;

static	int		Filter = 1;

static HGLOBAL	hMixTotalMem;
static UBYTE	*MixTotalMem;

static MIXCHAN	MixChn[NUMCHN];
static SBYTE	*GF1Mem;
static SWORD	*VolTab;


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
			  pVolTab[(UBYTE)byt] = (byt * vol * 256) / MAXVOL / 2;
		}
}



/*-----------------------------------------------------------------------*/
int	VGF1_Open(void)
/*-----------------------------------------------------------------------*/
{
UBYTE	*p;

	if (
		(hMixTotalMem = GlobalAlloc(
			  GPTR,
			  VOLTABLEN				// Allocate Voltab
			+ VDRAM_LEN				// Allocate DRAM
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

	MixInitVolTab();

//-------------------------------------------------------------------------

	VGF1_Reset();

	return 0;
}


/*-----------------------------------------------------------------------*/
int		VGF1_Close(void)
/*-----------------------------------------------------------------------*/
{
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
void	VGF1_SetVoice(int voc, int md, int VB, int VS, int VL)
/*-----------------------------------------------------------------------*/
{
		if (VB >= VDRAM_LEN) return;

		MixChn[voc]._SamplePos = VB;
		MixChn[voc]._SampleSize = VL;
		MixChn[voc]._LoopS = VS;
		MixChn[voc]._Loop = ((md > 0) ? TRUE : FALSE);

}


/*-----------------------------------------------------------------------*/
void	VGF1_SetVoiceL(int voc, int VB, int VL)
/*-----------------------------------------------------------------------*/
{
		if (VB >= VDRAM_LEN) return;

		MixChn[voc]._SamplePos = VB;
		MixChn[voc]._SampleSize = VL;
		MixChn[voc]._LoopS = VB;
		MixChn[voc]._Loop = TRUE;
}


/*-----------------------------------------------------------------------*/
void	VGF1_PlayVoice(int voc)
/*-----------------------------------------------------------------------*/
{
		MixChn[voc].Active = TRUE;
		MixChn[voc].PosIndexHI = 0;
		MixChn[voc].PosIndexLO = 0;

		MixChn[voc].SamplePos = MixChn[voc]._SamplePos;
		MixChn[voc].SampleSize = MixChn[voc]._SampleSize;
		MixChn[voc].LoopS = MixChn[voc]._LoopS;
		MixChn[voc].Loop = MixChn[voc]._Loop;

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
void	VGF1_Update(SWORD *dst, int sampnum)
/*--------------------------------------------------------------*/
{
	// 1) Clear BUFFER
		
		ZeroMemory(dst, sampnum * sizeof(SWORD) * 2);



	// 2) Mix Channels
		
		AddStereoNormal(0, dst, sampnum);
		AddStereoNormal(1, dst, sampnum);
		AddStereoNormal(2, dst, sampnum);
		AddStereoNormal(3, dst, sampnum);


	// 3) Filter BUFFER
		
		if (Filter) FirMemory(dst, sampnum);

}






/*-----------------------------------------------------------------------*/
static void	AddStereoNormal(int voc, SWORD *dst, SLONG todo)
/*-----------------------------------------------------------------------*/
{
SWORD 	*pVolTab;
SBYTE	*src;
SLONG	idxHI, idxLO, incHI, incLO, lim;
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

	idxHI = MixChn[voc].PosIndexHI;
	idxLO = MixChn[voc].PosIndexLO;

	incHI = ((MixChn[voc].Freq << FRACBITS) / MIXFREQ);
	incLO = incHI << FRACBITS;
	incHI >>= FRACBITS; 

	if (MixChn[voc].Loop == TRUE)
	{
		lim = MixChn[voc].SampleSize;

		while(todo--)
		{
			while(idxHI > lim) 
			{
				idxHI -= (lim - MixChn[voc].LoopS);
				MixChn[voc].SamplePos = MixChn[voc]._SamplePos;
				MixChn[voc].SampleSize = MixChn[voc]._SampleSize;
				MixChn[voc].LoopS = MixChn[voc]._LoopS;
				MixChn[voc].Loop = MixChn[voc]._Loop;

				src = (SBYTE *) GF1Mem + MixChn[voc].SamplePos;
				lim = MixChn[voc].SampleSize;
			}

//			*dst += pVolTab	[ src[idxHI] + (((src[idxHI+1] - src[idxHI]) * (idxLO>>24)) / 256) ];

			*dst += pVolTab[ src[idxHI] ];

			dst += 2;

			// idxHI:idxLO += incHI:incLO
			_asm {
				mov	eax,idxLO
				mov	ebx,idxHI
				add	eax,incLO
				adc ebx,incHI
				mov	idxLO,eax
				mov	idxHI,ebx
			}
		}
	}
	else
	{
		lim = MixChn[voc].SampleSize - 1;

		while(todo--)
		{
			if (idxHI > lim)
			{ 
				MixChn[voc].Active = FALSE;
				MixChn[voc].PosIndexHI = 0;
				MixChn[voc].PosIndexLO = 0;
				todo = 0; 
				break; 
			}

//			*dst += pVolTab	[ src[idxHI] + (((src[idxHI+1] - src[idxHI]) * (idxLO>>24)) / 256) ];

			*dst += pVolTab[ src[idxHI] ];

			dst += 2;

			// idxHI:idxLO += incHI:incLO
			_asm {
				mov	eax,idxLO
				mov	ebx,idxHI
				add	eax,incLO
				adc ebx,incHI
				mov	idxLO,eax
				mov	idxHI,ebx
			}
		}
	}
	MixChn[voc].PosIndexHI = idxHI;
	MixChn[voc].PosIndexLO = idxLO;

}









/*--------------------------------------------------------------*/
void	VGF1_Reset(void)
/*--------------------------------------------------------------*/
{
int	voc;

	for (voc = 0; voc < NUMCHN; voc++)
	{
		MixChn[voc].MasterVol = 64;

		MixChn[voc].Active = FALSE;

		MixChn[voc].SampleSize = 0;
		MixChn[voc].SamplePos = 0;
		MixChn[voc].Loop = FALSE;
		MixChn[voc].LoopS = 0;
		
		MixChn[voc]._SampleSize = 0;
		MixChn[voc]._SamplePos = 0;
		MixChn[voc]._Loop = FALSE;
		MixChn[voc]._LoopS = 0;

		MixChn[voc].Volume = 0;
		MixChn[voc].Freq = 0;
		MixChn[voc].PosIndexHI = 0;
		MixChn[voc].PosIndexLO = 0;
	}
}


/*-----------------------------------------------------------------------*/
void	VGF1_SetChanMasterVol(int chan, int vol)
/*-----------------------------------------------------------------------*/
{

	if ((chan < 0) || (chan > 3)) return;

	if (vol == -1)
	{
		MixChn[chan].MasterVol = (MixChn[chan].MasterVol) ? 0 : 64;
	}
	else
	{
		vol = (vol<0) ? 0 : ((vol>64) ? 64 : vol);
		MixChn[chan].MasterVol = vol;
	}
}


/*-----------------------------------------------------------------------*/
int		VGF1_GetChanMasterVol(int chan)
/*-----------------------------------------------------------------------*/
{
int	vol;
	if ((chan < 0) || (chan > 3)) return 0;

	vol = MixChn[chan].MasterVol;

	return vol;
}



/*-----------------------------------------------------------------------*/
int		VGF1_GetActiveChannels(void)
/*-----------------------------------------------------------------------*/
{
int	val = 0;
int	i;

	for (i = 0; i < NUMCHN; i++)
		if ((MixChn[i].Active == TRUE)
		&& (MixChn[i].MasterVol>0)) val++;
	return val;
}

