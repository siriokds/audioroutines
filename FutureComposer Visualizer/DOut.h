#ifndef DOUT_H_
#define DOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"

int		DOut_Open(void (*UserFunc)(void), int BuffLen, int BuffNum, DWORD User);
int		DOut_Start(void);
void	DOut_Stop(void);
void	DOut_Close(void);

void	DOut_BufferCopy(void *src);
int		DOut_GetBufferN(void);
int		DOut_GetBufferWritePos(void);
int		DOut_GetBufferPlayPos(void);
//char*  WOut_GetDeviceName(void);



#define	DOUT_ERR_OK						0

#define	DOUT_ERR_DS_OBJCREATE			100
#define	DOUT_ERR_DS_SETCOOPLEVEL		101

#define	DOUT_ERR_DS_PRIBUF_CREATE		110
#define	DOUT_ERR_DS_PRIBUF_SETFORMAT	111

#define	DOUT_ERR_DS_SECBUF_CREATE		120
#define	DOUT_ERR_DS_SECBUF_EVENTCREATE	121
#define	DOUT_ERR_DS_SECBUF_EVENTQUERY	122
#define	DOUT_ERR_DS_SECBUF_EVENTSET		123

#define	DOUT_ERR_MAINTHREAD_CREATE		130
#define	DOUT_ERR_MAINTHREAD_SETPRIORITY	131



#ifdef __cplusplus
}
#endif

#endif
