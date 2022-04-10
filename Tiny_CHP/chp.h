#ifndef _CHP_H
#define _CHP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

void	__stdcall CHP_Open(DWORD User, int mastervol);	// Mastervol: 128 = 100%
void	__stdcall CHP_InitModule(char *Module);
void	__stdcall CHP_Start(void);
void	__stdcall CHP_Close(void);
void	__stdcall CHP_Update(void);
char*	__stdcall CHP_GetSamplesPtr(void);

#ifdef __cplusplus
}
#endif

#endif
