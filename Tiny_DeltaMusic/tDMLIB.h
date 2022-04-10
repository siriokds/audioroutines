#ifndef _tDMLIB_H
#define _tDMLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

void	__stdcall DM2_Open(DWORD User);
void	__stdcall DM2_InitModule(char *Module);
void	__stdcall DM2_Start(void);
void	__stdcall DM2_Close(void);

#ifdef __cplusplus
}
#endif
 
#endif
