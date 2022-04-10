//-----------------------------------------------------------------------
// DOUT.H - DirectSound Streaming routines.
//-----------------------------------------------------------------------
//  This file is part of the SiRioKD audio library.
// 
//  Copyright (C) SiRioKD.  All rights reserved.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

#ifndef DOUT_H_
#define DOUT_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "Types.h"

int	__stdcall DOut_Open(void (*UserFunc)(void), int MixFreq, int BuffLen, int BuffNum, DWORD User);
int	__stdcall DOut_Prepare(void);
int	__stdcall DOut_Begin(void);
int	__stdcall DOut_Start(void);
void	__stdcall DOut_Stop(void);
void	__stdcall DOut_Close(void);
void	__stdcall DOut_BufferCopy(void *src);



#define	DOUT_ERR_OK				0

#define	DOUT_ERR_DS_OBJCREATE			100
#define	DOUT_ERR_DS_SETCOOPLEVEL		101

#define	DOUT_ERR_DS_PRIBUF_CREATE		110
#define	DOUT_ERR_DS_PRIBUF_SETFORMAT		111

#define	DOUT_ERR_DS_SECBUF_CREATE		120
#define	DOUT_ERR_DS_SECBUF_EVENTCREATE		121
#define	DOUT_ERR_DS_SECBUF_EVENTQUERY		122
#define	DOUT_ERR_DS_SECBUF_EVENTSET		123

#define	DOUT_ERR_MAINTHREAD_CREATE		130



#ifdef __cplusplus
};//extern "C"
#endif


#endif	//DOUT_H_

