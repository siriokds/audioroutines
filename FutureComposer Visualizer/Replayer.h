#ifndef _REPLAYER_H
#define _REPLAYER_H

#pragma pack(1)
typedef struct tagChanInfo
{
	int		ON;

	SBYTE	*PatPtr;

	SBYTE	*VolPtr;
	int		VolOfs;
	int		VolOfsVal;
	int		VolOfsSpd;

	SBYTE	*FrqPtr;
	int		FrqOfs;
	int		FrqOfsVal;
	int		FrqOfsSpd;
	
	ULONG	Period;
	SBYTE	Volume;
	SBYTE	DevVolume;

	ULONG	SampleOfs;
	ULONG	SampleSiz;

}	tChanInfo;
#pragma pack()


#pragma pack(1)
typedef struct tagDataInfo
{
	int			ActRow;
	int			RepCnt;
	SBYTE*		SeqPtr;
	int			SeqRow;
	SWORD*		MixBuffer;
	tChanInfo	Chan[4];
	
}	tDataInfo;
#pragma pack()

int			FC6_Open(DWORD User, int BuffMul, int BuffNum);
int			FC6_Init(SBYTE *ModAddr);
void		FC6_Start(void);
void		FC6_Stop(void);
void		FC6_Close(void);
int			FC6_IsPlaying(void);


void		FC6_GoNextSeq(void);
void		FC6_GoPrevSeq(void);
void		FC6_MusicONOFF();


void		FC6_PLAY_MUSIC(void);
SWORD*		FC6_GetMixBuffer(void);
tDataInfo*	FC6_GetInfo(void);
void		FC6_PutInfo(tDataInfo* dst);
tDataInfo*	FC6_GetInfoN(int n);
int			FC6_GetREPcnt(void);
void		FC6_ChanONOFF(int chn);



#endif //_REPLAYER_H