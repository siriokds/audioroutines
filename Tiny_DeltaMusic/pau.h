#ifndef PAU_H_
#define PAU_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "Types.h"

void	__stdcall paulaOpen(void (*UserFunc)(void), DWORD User);
void	__stdcall paulaStart(void);
void	__stdcall paulaClose(void);

void	__stdcall paulaSetPeriod(int voc, int per);
void	__stdcall paulaSetVolume(int voc, int vol);
void	__stdcall paulaSetSamplePos(int voc, char *pos);
void	__stdcall paulaSetSampleLen(int voc, int len);
void	__stdcall paulaSetVoice(int voc, int onoff);

#define	paulaPlayVoice(voc) paulaSetVoice(voc, 1);
#define	paulaStopVoice(voc) paulaSetVoice(voc, 0);

#ifdef __cplusplus
}
#endif

#endif
