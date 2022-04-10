//-----------------------------------------------------------------------
// PAULA.H - 'Paula' chip like mixing routines.
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

#ifndef __PAULA_H
#define __PAULA_H

#ifdef __cplusplus
//extern "C" {
#endif
//---------------------------------------------------------------------------



#include <windows.h>
#include "Types.h"

typedef int (*PFUNC)(void);


void	__stdcall paulaOpen(int MixFreq, 
			  	PFUNC UserFunc, int UserFuncRate,
			  	int p_BufferSize, int p_BufferNum);

void	__stdcall paulaReset(void);
void	__stdcall paulaSetFreq(int voc, int freq);
void	__stdcall paulaSetPeriod(int voc, int period);
void	__stdcall paulaSetVolume(int voc, int vol);
void	__stdcall paulaSetSamplePos(int voc, SBYTE *Address);
void	__stdcall paulaSetSampleLen(int voc, UWORD Length);
void	__stdcall paulaPlayVoice(int voc);
void	__stdcall paulaStopVoice(int voc);
void	__stdcall paulaWriteDmaCon(UWORD amigaWord);
void	__stdcall paulaUpdate(SWORD *dst, int nbSampleToCompute);





#define FRACBITS	12	// 12 bit fixed point

#define	NUMCHN		4
#define	MAXVOL		64
#define VOLTABLEN	((MAXVOL+1) * 256 * sizeof(SWORD))


#define Period2Freq(p) (3546895/period) // Amiga PAL  (50Hz)
//#define Period2Freq(p) (3579545/period) // Amiga NTSC (60Hz)


typedef struct tKMixChn
{
	UBYTE	Num;

	BOOL	Active;
	UBYTE	MasterVol;

	UBYTE	PanVolumeL;
	UBYTE	PanVolumeR;

	ULONG	SampleLen;
	ULONG	SampleLen_i;

	SBYTE	*SamplePos;
	SBYTE	*SamplePos_i;

	ULONG	PosIndex;

	SLONG	Freq;
	UBYTE	Volume;

} KMixChn;


#ifdef __cplusplus
//};//extern "C"
#endif



#endif
