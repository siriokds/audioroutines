//-----------------------------------------------------------------------
// PAULA.CPP - 'Paula' chip like mixing routines.
//-----------------------------------------------------------------------
//  This file is part of the SiRioKD audio library.
// 
//  Copyright (C) SiRioKD.  All rights reserved.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

#include "Paula.h"
#include "DOut.h"


static	BOOL	Opened;
static	BOOL	Filter;

static	KMixChn	MixChn[NUMCHN];

static	SWORD	VolTab[VOLTABLEN];
static	SBYTE	nullBytes[16];


static	SLONG	MixingFrequency;	//(44100)
static	PFUNC	UserFunction;
static	SLONG	UserFunctionRate;	//(50)
static	SLONG	BufferSize;
static	SLONG	BufferNum;

static	SLONG	nbSample;			//(882)
static	SLONG	innerSampleCount;

#define PAULA_BUFSIZE_MAX 16384

static	SWORD	LocalBuffer[PAULA_BUFSIZE_MAX*2];





/*-----------------------------------------------------------------------*/
static	int	ValidVoice(int voc)
/*-----------------------------------------------------------------------*/
{
	return ((voc >= 0) && (voc < NUMCHN));
}


/*-----------------------------------------------------------------------*/
static	void	FirMemory(SWORD *d, int len)
/*-----------------------------------------------------------------------*/
{
int	i, j;

	for (i = 1; i < len-1; i++)
	{
		j = i*2;
		d[j] = (d[j-2] / 4) + (d[j] / 2) + (d[j+2] / 4);
		j++;
		d[j] = (d[j-2] / 4) + (d[j] / 2) + (d[j+2] / 4);

	}
}


/*-----------------------------------------------------------------------*/
static	void	InitVolTab(void)
/*-----------------------------------------------------------------------*/
{
SWORD	*pVolTab = VolTab;
int		vol,byt;

int		max,val;

		MixChn[0].PanVolumeL = 64;
		MixChn[0].PanVolumeR = 16;

		MixChn[1].PanVolumeL = 16;
		MixChn[1].PanVolumeR = 64;

		MixChn[2].PanVolumeL = 16;
		MixChn[2].PanVolumeR = 64;

		MixChn[3].PanVolumeL = 64;
		MixChn[3].PanVolumeR = 16;


		max = MixChn[0].PanVolumeL + MixChn[1].PanVolumeL + 
			  MixChn[2].PanVolumeL + MixChn[3].PanVolumeL;

		val = MixChn[0].PanVolumeR + MixChn[1].PanVolumeR + 
			  MixChn[2].PanVolumeR + MixChn[3].PanVolumeR;

		if (val > max) max = val;





		for (byt = 0; byt < 256; byt++) *pVolTab++ = 0;

		for (vol = 1; vol < (MAXVOL+1); vol++, pVolTab += 256)
		{
  			for (byt = -128; byt < 128; byt++)
			  pVolTab[(UBYTE)byt] = ((byt * vol * 256) / MAXVOL * 64) / max;
		}
}




/*---------------------------------------------------------------------------------------------------------------------*/
static	SLONG	MixStereoNormal(SBYTE *src, SWORD *dst,
			ULONG idx, ULONG inc, ULONG todo,
                        SWORD *Vol_L, SWORD *Vol_R)
/*---------------------------------------------------------------------------------------------------------------------*/
{
UBYTE	sample1, sample2, sample3, sample4;
ULONG	remain;

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





/*---------------------------------------------------------------------------------------------------------------------*/
static	void	AddChannel(int voc, SWORD *dst, ULONG todo)
/*---------------------------------------------------------------------------------------------------------------------*/
{
SWORD 	*pVolTab_L, *pVolTab_R;
ULONG	inc, lim, vol_L, vol_R;
ULONG	samplesUntilEnd, stepsUntilEnd, mixNow;

KMixChn	*chan = &MixChn[voc];


	if ((chan->Active == FALSE)	|| (chan->SampleLen_i < 4) || (chan->Freq < 16) ) return;

	vol_L = (chan->Volume * chan->PanVolumeL * chan->MasterVol) / (64*64);
	vol_R = (chan->Volume * chan->PanVolumeR * chan->MasterVol) / (64*64);

	inc = ((MixChn[voc].Freq << FRACBITS) / MixingFrequency);

	pVolTab_L = VolTab + (vol_L * 256);
	pVolTab_R = VolTab + (vol_R * 256);


	lim = chan->SampleLen_i << FRACBITS;

	while(todo > 0)
	{
		while(chan->PosIndex >= lim)
		{
			chan->PosIndex -= lim;
			chan->SamplePos_i = chan->SamplePos;
			chan->SampleLen_i = chan->SampleLen;
			lim = chan->SampleLen_i << FRACBITS;
		}

		if (chan->SampleLen_i < 3) { todo = 0; chan->Active = FALSE; return; } 
		



		samplesUntilEnd = lim - chan->PosIndex;


		stepsUntilEnd = samplesUntilEnd / inc;
		if (samplesUntilEnd % inc) stepsUntilEnd++;


		mixNow = (stepsUntilEnd > todo) ? todo : stepsUntilEnd;
		todo -= mixNow;


		if (mixNow <= 0) return;


		if ( (vol_L == 0) && (vol_R == 0) ) chan->PosIndex += (mixNow * inc); else
		{
			chan->PosIndex = MixStereoNormal(	chan->SamplePos_i, 
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


//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************


/*-----------------------------------------------------------------------*/
void	__stdcall paulaReset(void)
/*-----------------------------------------------------------------------*/
{
int	voc;

	for (voc = 0; voc < NUMCHN; voc++)
	{
		MixChn[voc].Num = voc;

		MixChn[voc].MasterVol = 64;

		MixChn[voc].Active = FALSE;

		MixChn[voc].SampleLen = 0;
		MixChn[voc].SampleLen_i = 0;

		MixChn[voc].SamplePos = &nullBytes[0];
		MixChn[voc].SamplePos_i = &nullBytes[0];

		MixChn[voc].Volume = 0;
		MixChn[voc].Freq = 0;
		MixChn[voc].PosIndex = 0;

	}

	innerSampleCount = 0;

}










/*-----------------------------------------------------------------------*/
void	__stdcall paulaSetFreq(int voc, int freq)
/*-----------------------------------------------------------------------*/
{
	if (!ValidVoice(voc)) return;

	MixChn[voc].Freq = freq & 0xFFFF;

	if (MixChn[voc].Freq < 2)
	{
		MixChn[voc].Freq = 0;
		MixChn[voc].Active = FALSE;
	}

}


/*-----------------------------------------------------------------------*/
void	__stdcall paulaSetPeriod(int voc, int period)
/*-----------------------------------------------------------------------*/
{
	if (!ValidVoice(voc)) return;

	if (period == 0)
	{
		MixChn[voc].Active = FALSE;
		return;
	}

	if (period < 0x40)  period = 0x40;
	if (period > 0x6B00) period = 0x6B00;

//	if (period < 113) period = 113;
//	if (period > 856) period = 856;
	paulaSetFreq(voc, Period2Freq(period)<<2);
}



/*-----------------------------------------------------------------------*/
void	__stdcall paulaSetVolume(int voc, int vol)
/*-----------------------------------------------------------------------*/
{
	if (!ValidVoice(voc)) return;

//	vol = ((((vol < 0) ? 0 : vol) > MAXVOL) ? MAXVOL : vol);

	if (vol < 0) vol = 0;	if (vol > MAXVOL) vol = MAXVOL;
	MixChn[voc].Volume = (UBYTE)(vol&0xFF);
}


/*-----------------------------------------------------------------------*/
void	__stdcall paulaSetSamplePos(int voc, SBYTE *Address)
/*-----------------------------------------------------------------------*/
{
	if (!ValidVoice(voc)) return;

	if (Address != NULL) MixChn[voc].SamplePos = Address;
}



/*-----------------------------------------------------------------------*/
void	__stdcall paulaSetSampleLen(int voc, UWORD Length)
/*-----------------------------------------------------------------------*/
{
	if (!ValidVoice(voc)) return;

	if (Length > 0) 
		MixChn[voc].SampleLen = Length * 2;
	else
		paulaStopVoice(voc);
}




/*-----------------------------------------------------------------------*/
void	__stdcall paulaPlayVoice(int voc)
/*-----------------------------------------------------------------------*/
{
	if (!ValidVoice(voc)) return;

	MixChn[voc].SamplePos_i = MixChn[voc].SamplePos;
	MixChn[voc].SampleLen_i = MixChn[voc].SampleLen;
	MixChn[voc].PosIndex = 0;

	if (MixChn[voc].SampleLen_i > 0)
		MixChn[voc].Active = TRUE;
	else
		MixChn[voc].Active = FALSE;
}


/*-----------------------------------------------------------------------*/
void	__stdcall paulaStopVoice(int voc)
/*-----------------------------------------------------------------------*/
{
	if (!ValidVoice(voc)) return;

	MixChn[voc].Active = FALSE;
}


/*-----------------------------------------------------------------------*/
void	__stdcall paulaWriteDmaCon(UWORD amigaWord)
/*-----------------------------------------------------------------------*/
{
int	i;

	// Set channel DMA.
	if (amigaWord & 0x8000)
	{
		for(i = 0; i < NUMCHN; i++)
		   if ( amigaWord & (1<<i) ) paulaPlayVoice(i);
	}
	// Clear channel DMA.
	else
	{
		for(i = 0; i < NUMCHN; i++)
		   if ( amigaWord & (1<<i) ) paulaStopVoice(i);
	}

}




// nbSample = mixingFrequency / userRate; (882)
// innerSampleCount = 0;
/*--------------------------------------------------------------*/
void	__stdcall paulaUpdate(SWORD *dst, int nbSampleToCompute)
/*--------------------------------------------------------------*/
{

//		if (PaulaOpened == FALSE) return;

		if ((dst == NULL) || (nbSampleToCompute <= 0)) return;


	// 1) Clear BUFFER

		ZeroMemory(dst, nbSampleToCompute * sizeof(SWORD) * 2);


		do
		{
			int nbs = nbSample - innerSampleCount;
			
			if (nbs > nbSampleToCompute) nbs = nbSampleToCompute;
			
			innerSampleCount += nbs;
			
			if (innerSampleCount >= nbSample)
			{
				if (UserFunction)
				{
					UserFunction();
				}
				innerSampleCount -= nbSample;
			}

		// 2) Mix Channels

			AddChannel(0, dst, nbs);
			AddChannel(1, dst, nbs);
			AddChannel(2, dst, nbs);
			AddChannel(3, dst, nbs);


		// 3) Filter BUFFER

			if (Filter) FirMemory(dst, nbs);
			
			nbSampleToCompute -= nbs;
			dst += (nbs*2);

			Sleep(0);
		}
		while (nbSampleToCompute > 0);
}



/*-----------------------------------------------------------------------*/
void	paulaRunOnce(void)
/*-----------------------------------------------------------------------*/
{
		paulaUpdate(LocalBuffer, BufferSize);	
		DOut_BufferCopy(LocalBuffer);
}


/*-----------------------------------------------------------------------*/
void	__stdcall paulaOpen(int MixFreq, 
				  PFUNC UserFunc, int UserFuncRate, 
				  int p_BufferSize, int p_BufferNum)
/*-----------------------------------------------------------------------*/
{
int	err;

	MixingFrequency = MixFreq;
	if (MixingFrequency > 48000) MixingFrequency = 48000;
	if (MixingFrequency <  8000) MixingFrequency =  8000;

	UserFunction = UserFunc;
	UserFunctionRate = UserFuncRate;
	
	BufferSize = p_BufferSize;
	if (BufferSize > PAULA_BUFSIZE_MAX) BufferSize = PAULA_BUFSIZE_MAX;

	BufferNum  = p_BufferNum;

	nbSample = MixingFrequency / UserFunctionRate;
	
	InitVolTab();
	paulaReset();


	err = DOut_Open(paulaRunOnce, MixingFrequency, BufferSize, BufferNum, (DWORD)GetForegroundWindow());
	if (err != DOUT_ERR_OK) return;


	Opened = TRUE;
	Filter = TRUE;
}

