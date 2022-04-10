#ifndef MIXER_H_
#define MIXER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "Types.h"

int		VGF1_Open(void (*callb)(void), DWORD User, int m, int n);
void	VGF1_Start(void);
void	VGF1_Stop(void);
int		VGF1_Close(void);
void	VGF1_Reset(void);

void	VGF1_FilterONOFF(void);
void	VGF1_SetChanMasterVol(int chan, int vol);
int		VGF1_GetChanMasterVol(int chan);

char	VGF1_Peek(int addr);
void	VGF1_Poke(int addr, char val);
void	VGF1_PokeBlock(int Dst, char *Src, int Size);
void	VGF1_PokeBlockD(int Dst, char *Src, int Size);
void	VGF1_PokeBlockQ(int Dst, char *Src, int Size);
void	VGF1_PokeBlockX(int Dst, char *Src, int Size, int XORval);
void	VGF1_PokeBlock16(int Dst, short *Src, int Size);
void	VGF1_SetFreq(int voc, int freq);
void	VGF1_SetPeriod(int voc, int period);
void	VGF1_SetVol(int voc, int vol);
int		VGF1_GetVol(int voc);
void	VGF1_SetVoice(int voc, int md, int Start, int LoopStart, int LoopLen);
void	VGF1_SetVoiceL(int voc, int LoopStart, int LoopLen);
void	VGF1_PlayVoice(int voc);
void	VGF1_StopVoice(int voc);

int		VGF1_GetSampleOfs(int voc);
int		VGF1_GetSampleSize(int voc);

SWORD*	VGF1_GetMixBuffer(void);
int		VGF1_GetMixStress(void);
int		VGF1_GetActiveChannels(void);

#define VGF1_LOOP 1
#define VGF1_NOLOOP 1


#ifdef __cplusplus
}
#endif

#endif
