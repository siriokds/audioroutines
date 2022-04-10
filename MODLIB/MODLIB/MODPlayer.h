#ifndef _MODPL_H
#define _MODPL_H

#ifdef __cplusplus
extern "C" {
#endif


#include "Windows.h"
#include "Types.h"


int	__stdcall MOD_Open(int MixFreq, int BufferSize, int BufferNum);
int	__stdcall MOD_InitModuleCHP(SBYTE *Module);
void	__stdcall MOD_Prepare(void);
void	__stdcall MOD_Begin(void);
void	__stdcall MOD_Start(void);
void	__stdcall MOD_Stop(void);
void	__stdcall MOD_Close(void);



#ifdef __cplusplus
};//extern "C"
#endif

#endif //_MODPL_H
