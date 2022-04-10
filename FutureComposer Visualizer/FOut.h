#ifndef DOUT_H_
#define DOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"

int		DOut_Open(void (*UserFunc)(void), int BuffLen, int BuffNum, DWORD User);
int		DOut_Start(void);
void	DOut_Close(void);

void	DOut_BufferCopy(void *src);

#ifdef __cplusplus
}
#endif

#endif
