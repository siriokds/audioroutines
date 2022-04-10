#ifndef GF1_H_
#define GF1_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"


void	__stdcall gf1Open(void(*func)(void), ULONG User);
void	__stdcall gf1MasterVol(int mastervol);	// mastervol 128 = 100%
void	__stdcall gf1Start(void);
void	__stdcall gf1Close(void);

void	__stdcall gf1SelectVoice(int voc);
void	__stdcall gf1SetPeriod(int per);
void	__stdcall gf1SetVolume(int vol);
void	__stdcall gf1SetSample(int sta, int end, int ls);
void	__stdcall gf1PokeBlock(int ofs, SBYTE *src, int len);
void	__stdcall gf1SetVoice(int onoff);

#ifdef __cplusplus
}
#endif

#endif
