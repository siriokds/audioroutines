#ifndef DOUT_H_
#define DOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"

int	__stdcall DOut_Open(void (*UserFunc)(void), DWORD User);
int	__stdcall DOut_Start(void);
void	__stdcall DOut_Close(void);
void	__stdcall DOut_BufferCopy(void *src);


#ifdef __cplusplus
}
#endif

#endif
