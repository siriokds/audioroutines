#ifndef PAU_H_
#define PAU_H_

#ifdef __cplusplus
extern "C" {
#endif

// #include "Types.h"

void	__stdcall paulaOpen(void (*UserFunc)(void), DWORD User);
void	__stdcall paulaMasterVol(int mastervol);	// mastervol 128 = 100%
void	__stdcall paulaStart(void);
void	__stdcall paulaClose(void);

void	__stdcall paulaSelectVoice(int voc);
void	__stdcall paulaSetPeriod(int per);
void	__stdcall paulaSetVolume(int vol);
void	__stdcall paulaSetSamplePos(char *pos);
void	__stdcall paulaSetSampleLen(int len);
void	__stdcall paulaSetVoice(int onoff);

#define	paulaPlayVoice(voc) paulaSetVoice(voc, 1);
#define	paulaStopVoice(voc) paulaSetVoice(voc, 0);

#ifdef __cplusplus
}
#endif

#endif
