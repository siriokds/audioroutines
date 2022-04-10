//-----------------------------------------------------------------------
// DOUT.CPP - DirectSound Streaming routines.
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

// DirectSound Threaded WaveStreamer 99%. Requires dsound.lib, (DirectX 7+)


#include <windows.h>
#include <dsound.h>
#include <cguid.h>
#include "DOut.h"


#define DOUT_MIN_BUFFER		2
#define DOUT_MAX_BUFFER		16

#define DOUT_MAX_POSNOTIFY	(DOUT_MAX_BUFFER)

#define DOUT_SAMPLESIZE		(sizeof(SWORD)*2)

static	HRESULT			hr;
static	LPDIRECTSOUND		lpDSOUND = 0;

static	DSBUFFERDESC		dsBufferDescr;
static	LPDIRECTSOUNDBUFFER	lpdsPrimary;
static	LPDIRECTSOUNDBUFFER	lpdsBuffer;
static	LPDIRECTSOUNDNOTIFY	lpdsNotify = 0;
static	WAVEFORMATEX		wfxPrimaryFORM;
static	WAVEFORMATEX		wfxBufferFORM;


static	DSBPOSITIONNOTIFY	dsbPosNotify[DOUT_MAX_POSNOTIFY];

static	int			nBufferLen, nBufferNum, nBuffer;
static	void			(*UserFunction)(void);

static	BOOL			bUserStop;
static	BOOL			bUserQuit;
static	HANDLE			hEvent;

static	CRITICAL_SECTION	ThreadCS;
static  HANDLE			ThreadHandle;
static  DWORD			ThreadID;
static	DWORD WINAPI		ThreadProc(LPVOID Data);

static	BOOL			Opened;
static	int			LastError = 0;




/*------------------------------------------------------------*/
void	DOut_CorrectWritePos(void)
/*------------------------------------------------------------*/
{
DWORD	playpos,writepos;
DWORD	playbuf,writebuf;
int	BufferSize = DOUT_SAMPLESIZE * nBufferLen;

	lpdsBuffer->GetCurrentPosition(&playpos,&writepos);

	playbuf = (playpos / BufferSize) % nBufferNum;
	writebuf = (writepos / BufferSize) % nBufferNum;

	if (playbuf == writebuf) nBuffer = (playbuf - 1) % nBufferNum;
}


/*------------------------------------------------------------*/
DWORD WINAPI ThreadProcDS(LPVOID Data)
/*------------------------------------------------------------*/
{
// Thread Init Routines
  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);	
	

// Thread Main Routines	
  while(bUserQuit == FALSE)
  {
	if ( WaitForSingleObject(hEvent,500) != WAIT_TIMEOUT )
	{
//		DOut_CorrectWritePos();

		EnterCriticalSection(&ThreadCS);
		if (UserFunction != NULL) (*UserFunction)();
		if (++nBuffer >= nBufferNum) nBuffer = 0;
		LeaveCriticalSection(&ThreadCS);
	}
  }

// Thread End Routines	

  return 0;
}



/*--------------------------------------------------------------*/
void	DOut_DSClose(void)
/*--------------------------------------------------------------*/
{
		if (lpdsNotify)	lpdsNotify->Release();
		if (lpDSOUND)	lpDSOUND->Release();
}



/*--------------------------------------------------------------*/
int		__stdcall 
		DOut_Open(void (*UserF)(void),
			int MixFreq,
			int BufferLen,
			int BufferNum, 
			DWORD User)
/*--------------------------------------------------------------*/
{
	Opened = false;

	lpdsNotify = 0;
	lpDSOUND = 0;
	LastError = 0;

	nBuffer = 0;
	bUserStop = TRUE;
	bUserQuit = FALSE;

	//----------------------------------------------------------------
	// DirectSound Object create and set
	//----------------------------------------------------------------

	if FAILED(DirectSoundCreate(NULL,&lpDSOUND,NULL)) 
		return DOUT_ERR_DS_OBJCREATE;


	if FAILED(lpDSOUND->SetCooperativeLevel(
			(HWND)User, DSSCL_PRIORITY))
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_SETCOOPLEVEL);
	}


	//----------------------------------------------------------------
	// Assign ext variables
	//----------------------------------------------------------------

	UserFunction = UserF;

	if (BufferNum < DOUT_MIN_BUFFER)  BufferNum = DOUT_MIN_BUFFER;
	if (BufferNum > DOUT_MAX_BUFFER)  BufferNum = DOUT_MAX_BUFFER;
	nBufferLen = BufferLen;
	nBufferNum = BufferNum;

	

	//----------------------------------------------------------------
	// Primary Buffer
	//----------------------------------------------------------------

	ZeroMemory(&wfxPrimaryFORM, sizeof(WAVEFORMATEX));

	wfxPrimaryFORM.wFormatTag = WAVE_FORMAT_PCM;
	wfxPrimaryFORM.nSamplesPerSec = MixFreq;
	wfxPrimaryFORM.wBitsPerSample = 16;
	wfxPrimaryFORM.nChannels = 2;

	wfxPrimaryFORM.nAvgBytesPerSec = 
				(wfxPrimaryFORM.nSamplesPerSec) * 
				(wfxPrimaryFORM.wBitsPerSample >> 3) *
				(wfxPrimaryFORM.nChannels);
	
	wfxPrimaryFORM.nBlockAlign = (WORD)
	    	((wfxPrimaryFORM.wBitsPerSample *
			  wfxPrimaryFORM.nChannels) >> 3);



	ZeroMemory(&dsBufferDescr, sizeof(DSBUFFERDESC));

	dsBufferDescr.dwSize = sizeof(DSBUFFERDESC);

	dsBufferDescr.dwFlags = DSBCAPS_PRIMARYBUFFER;


	if FAILED(lpDSOUND->CreateSoundBuffer(
		&dsBufferDescr, &lpdsPrimary, NULL)) 
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_PRIBUF_CREATE);
	}



	if FAILED(lpdsPrimary->SetFormat(&wfxPrimaryFORM))
	{
		DOut_DSClose();
       	return (LastError = DOUT_ERR_DS_PRIBUF_SETFORMAT);
	}

	//----------------------------------------------------------------
	// Secondary Buffer
	//----------------------------------------------------------------

	ZeroMemory(&wfxBufferFORM, sizeof(WAVEFORMATEX));

	wfxBufferFORM.wFormatTag = WAVE_FORMAT_PCM;
	wfxBufferFORM.nSamplesPerSec = MixFreq;
	wfxBufferFORM.wBitsPerSample = 16;
	wfxBufferFORM.nChannels = 2;

	wfxBufferFORM.nAvgBytesPerSec = 
						(wfxBufferFORM.nSamplesPerSec) * 
						(wfxBufferFORM.wBitsPerSample >> 3) *
						(wfxBufferFORM.nChannels);
	
	wfxBufferFORM.nBlockAlign = (WORD)
    	((wfxBufferFORM.wBitsPerSample *
		  wfxBufferFORM.nChannels) >> 3);


	ZeroMemory(&dsBufferDescr, sizeof(DSBUFFERDESC));

	dsBufferDescr.dwSize = sizeof(DSBUFFERDESC);

	dsBufferDescr.dwFlags =   DSBCAPS_GETCURRENTPOSITION2
							| DSBCAPS_GLOBALFOCUS
							| DSBCAPS_CTRLPOSITIONNOTIFY;

    	dsBufferDescr.dwBufferBytes =	nBufferNum * nBufferLen * DOUT_SAMPLESIZE;
                                    
    	dsBufferDescr.lpwfxFormat = &wfxBufferFORM;


	if FAILED(lpDSOUND->CreateSoundBuffer(
		&dsBufferDescr, &lpdsBuffer, NULL))
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_SECBUF_CREATE);
	}



	//----------------------------------------------------------------
	// Events
	//----------------------------------------------------------------

	if ((hEvent = CreateEvent(NULL,FALSE,FALSE,NULL)) == NULL)
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_SECBUF_EVENTCREATE);
	}


	for (int i = 0; i < nBufferNum; i++)
	{
		dsbPosNotify[i].dwOffset = 
			((i+1) * nBufferLen * DOUT_SAMPLESIZE) - 1;

		dsbPosNotify[i].hEventNotify = hEvent;
	}




    	if FAILED(IDirectSoundBuffer_QueryInterface(
		lpdsBuffer,IID_IDirectSoundNotify,(VOID **)&lpdsNotify))
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_SECBUF_EVENTQUERY);
	}




	if FAILED(lpdsNotify->SetNotificationPositions(
			nBufferNum, dsbPosNotify))
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_SECBUF_EVENTSET);
	}




	//----------------------------------------------------------------
	// Create MainThread
	//----------------------------------------------------------------

	if ((ThreadHandle = CreateThread(NULL, 0, 
		ThreadProcDS, NULL, CREATE_SUSPENDED, &ThreadID)) == NULL)
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_MAINTHREAD_CREATE);
	}


	//----------------------------------------------------------------

	InitializeCriticalSection(&ThreadCS);
	Opened = true;

	return DOUT_ERR_OK;
}


/*--------------------------------------------------------------*/
int	 __stdcall DOut_Prepare(void)
/*--------------------------------------------------------------*/
{
	if (Opened == false) return 1;

	bUserStop = TRUE;

// * Fill Pre-Buffers
	
	nBuffer = 0;

	for (int i = 0; i < nBufferNum; i++)
	{
		if (UserFunction != NULL) (*UserFunction)();
		if (++nBuffer >= nBufferNum) nBuffer = 0;
	}


// * Rewing Position to 0, 

	lpdsBuffer->SetCurrentPosition(0);
	return 0;
}

/*--------------------------------------------------------------*/
int	 __stdcall DOut_Begin(void)
/*--------------------------------------------------------------*/
{
	if (Opened == false) return 1;

	ResumeThread(ThreadHandle);
	lpdsBuffer->Play(0, 0, DSBPLAY_LOOPING);

	return 0;

}

/*--------------------------------------------------------------*/
int	 __stdcall DOut_Start(void)
/*--------------------------------------------------------------*/
{
int	err;

	err = DOut_Prepare();
	DOut_Begin();

	return err;
}

/*--------------------------------------------------------------*/
void 	__stdcall DOut_Stop(void)
/*--------------------------------------------------------------*/
{
	if (Opened == false) return;

	bUserStop = TRUE;
	lpdsBuffer->Stop();
	SuspendThread(ThreadHandle);
}


/*--------------------------------------------------------------*/
void 	__stdcall DOut_Close(void)
/*--------------------------------------------------------------*/
{
	if (Opened == false) return;


	lpdsBuffer->Stop();

	bUserQuit = TRUE;
	if (bUserStop == TRUE) ResumeThread(ThreadHandle);

	WaitForSingleObject(ThreadHandle,INFINITE);

	if (hEvent != NULL) CloseHandle(hEvent);

	DeleteCriticalSection(&ThreadCS);

	DOut_DSClose();
}



/*--------------------------------------------------------------*/
void	__stdcall DOut_BufferCopy(void *src)
/*--------------------------------------------------------------*/
{
HRESULT	hr;
void*	lpWrite1;
DWORD	dwLen1;
int		size;

//	if (bUserStop == TRUE) return;


	size   = nBufferLen * DOUT_SAMPLESIZE;

	hr = lpdsBuffer->Lock(
		(nBuffer * size), size,
		&lpWrite1,&dwLen1,NULL,0,0);

    // If we got DSERR_BUFFERLOST, restore and retry lock.
	if (hr == DSERR_BUFFERLOST)
    {
    	lpdsBuffer->Restore();
		hr = lpdsBuffer->Lock(
			(nBuffer*size), size
			,&lpWrite1,&dwLen1,NULL,0,0);
    }

    if (hr == DS_OK) CopyMemory(lpWrite1,src,size);

	lpdsBuffer->Unlock(lpWrite1,dwLen1,NULL,0);

}




