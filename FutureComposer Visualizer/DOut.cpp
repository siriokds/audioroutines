// DirectSound WaveStreamer 99%
// with (optional) diskwriter (for debug)

#include <windows.h>
#include <dsound.h>
#include <cguid.h>
#include "DOut.h"
#include <stdio.h>

static	int	DiskWriter = 0;

#define DOUT_MIN_BUFSIZE	(882*2)
#define DOUT_MIN_BUFFER		2
#define DOUT_MAX_BUFFER		16

#define DOUT_MAX_POSNOTIFY	(DOUT_MAX_BUFFER)
#define	DOUT_NUMEVENTS		(nBufferNum * 10)

#define DOUT_SAMPLESIZE		(sizeof(SWORD)*2)

static	HRESULT				hr;
static	LPDIRECTSOUND		lpDSOUND = 0;

static	DSBUFFERDESC		dsBufferDescr;
static	LPDIRECTSOUNDBUFFER	lpdsPrimary;
static	LPDIRECTSOUNDBUFFER	lpdsBuffer;
static	LPDIRECTSOUNDNOTIFY	lpdsNotify = 0;
static	WAVEFORMATEX		wfxPrimaryFORM;
static	WAVEFORMATEX		wfxBufferFORM;


static	DSBPOSITIONNOTIFY	dsbPosNotify[DOUT_MAX_POSNOTIFY];

static	int					nBufferLen, nBufferNum, nBuffer;
static	void				(*UserFunction)(void);

static	BOOL				bUserStop;
static	BOOL				bUserQuit;
static	HANDLE				hEvent;

static	CRITICAL_SECTION	ThreadCS;
static  HANDLE				ThreadHandle;
static  DWORD				ThreadID;
static	DWORD	WINAPI		ThreadProc(LPVOID Data);

static	BOOL				Opened;
static	int					LastError = 0;


static	FILE	*fp;
/*------------------------------------------------------------*/
DWORD WINAPI ThreadProcDS(LPVOID Data)
/*------------------------------------------------------------*/
{
  while(bUserQuit == FALSE)
  {
	if ( WaitForSingleObject(hEvent,500) != WAIT_TIMEOUT )
	{
		if (nBuffer == DOut_GetBufferWritePos())
        {
		  EnterCriticalSection(&ThreadCS);
		  if (UserFunction != NULL) (*UserFunction)();
		  if (++nBuffer >= nBufferNum) nBuffer = 0;
		  LeaveCriticalSection(&ThreadCS);
    	}
	}
  }
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
int	 DOut_Open(void (*UserF)(void),
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

	BufferLen -= (BufferLen % (44100 / 50));

	if (BufferLen < DOUT_MIN_BUFSIZE) BufferLen = DOUT_MIN_BUFSIZE;
	if (BufferNum < DOUT_MIN_BUFFER)  BufferNum = DOUT_MIN_BUFFER;
	if (BufferNum > DOUT_MAX_BUFFER)  BufferNum = DOUT_MAX_BUFFER;
	nBufferLen = BufferLen;
	nBufferNum = BufferNum;


	//----------------------------------------------------------------
	// Primary Buffer
	//----------------------------------------------------------------

	ZeroMemory(&wfxPrimaryFORM, sizeof(WAVEFORMATEX));

	wfxPrimaryFORM.wFormatTag = WAVE_FORMAT_PCM;
	wfxPrimaryFORM.nSamplesPerSec = 44100;
	wfxPrimaryFORM.wBitsPerSample = 16;
	wfxPrimaryFORM.nChannels = 2;

	wfxPrimaryFORM.nAvgBytesPerSec = 
							(wfxPrimaryFORM.nSamplesPerSec) * 
							(wfxPrimaryFORM.wBitsPerSample >> 3) *
							(wfxPrimaryFORM.nChannels);
	
	wfxPrimaryFORM.nBlockAlign = (WORD)
    	((wfxPrimaryFORM.wBitsPerSample *
		  wfxPrimaryFORM.nChannels) >> 3);


	CopyMemory(&wfxBufferFORM,&wfxPrimaryFORM,sizeof(WAVEFORMATEX));

	//----------------------------------------------------------------

	ZeroMemory(&dsBufferDescr, sizeof(DSBUFFERDESC));

	dsBufferDescr.dwSize = sizeof(DSBUFFERDESC);

	dsBufferDescr.dwFlags = DSBCAPS_PRIMARYBUFFER;



	if FAILED(lpDSOUND->CreateSoundBuffer(
		&dsBufferDescr, &lpdsPrimary, NULL)) 
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_PRIBUF_CREATE);
	}



	hr = lpdsPrimary->SetFormat(&wfxPrimaryFORM); 

    if ((hr != DS_OK) && (hr != DSERR_BADFORMAT))
    {
		DOut_DSClose();
        return (LastError = DOUT_ERR_DS_PRIBUF_SETFORMAT);
    }


	//----------------------------------------------------------------
	// Secondary Buffer
	//----------------------------------------------------------------

	ZeroMemory(&dsBufferDescr, sizeof(DSBUFFERDESC));

	dsBufferDescr.dwSize = sizeof(DSBUFFERDESC);

    dsBufferDescr.dwFlags = DSBCAPS_GETCURRENTPOSITION2
				            | DSBCAPS_GLOBALFOCUS
							| DSBCAPS_CTRLPOSITIONNOTIFY;

    dsBufferDescr.dwBufferBytes = 	nBufferNum *
    								nBufferLen * 
									DOUT_SAMPLESIZE;
                                    
    dsBufferDescr.lpwfxFormat = &wfxBufferFORM;



	if FAILED(lpDSOUND->CreateSoundBuffer(
		&dsBufferDescr, &lpdsBuffer, NULL))
	{
		DOut_DSClose();
		return (LastError = DOUT_ERR_DS_SECBUF_CREATE);
	}

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



	if (!SetThreadPriority(ThreadHandle,THREAD_PRIORITY_TIME_CRITICAL))
	{
		(LastError = DOUT_ERR_MAINTHREAD_SETPRIORITY);
	}

	//----------------------------------------------------------------

	InitializeCriticalSection(&ThreadCS);
	Opened = true;

	if (DiskWriter) fp = fopen("F:\\OUT.RAW","wb");
	return DOUT_ERR_OK;
}



/*--------------------------------------------------------------*/
int	 DOut_Start(void)
/*--------------------------------------------------------------*/
{
	if (Opened == false) return 1;

//	bUserQuit = TRUE;

	nBuffer = 0;

	for (int i = 0; i < nBufferNum; i++)
	{
		if (UserFunction != NULL) (*UserFunction)();
		if (++nBuffer >= nBufferNum) nBuffer = 0;
	}


/*
DWORD	playpos;

    lpdsBuffer->GetCurrentPosition(&playpos,NULL);
	playpos /= nBufferLen;
	playpos *= nBufferLen;
	lpdsBuffer->SetCurrentPosition(playpos);
*/
	
	lpdsBuffer->SetCurrentPosition(0);
	
	bUserStop = FALSE;
	ResumeThread(ThreadHandle);
	lpdsBuffer->Play(0, 0, DSBPLAY_LOOPING);

	return 0;

}

/*--------------------------------------------------------------*/
void DOut_Stop(void)
/*--------------------------------------------------------------*/
{
	bUserStop = TRUE;
	lpdsBuffer->Stop();
	SuspendThread(ThreadHandle);
}


/*--------------------------------------------------------------*/
void DOut_Close(void)
/*--------------------------------------------------------------*/
{
	if (Opened == false) return;

	if (DiskWriter) fclose(fp);

	lpdsBuffer->Stop();

	bUserQuit = TRUE;
	if (bUserStop == TRUE) ResumeThread(ThreadHandle);

	WaitForSingleObject(ThreadHandle,INFINITE);

	if (hEvent != NULL) CloseHandle(hEvent);

	DeleteCriticalSection(&ThreadCS);

	DOut_DSClose();
}



/*--------------------------------------------------------------*/
void	DOut_BufferCopy(void *src)
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

	if (DiskWriter) fwrite(src,1,size,fp);
}


/*--------------------------------------------------------------*/
int		DOut_GetBufferN(void)
/*--------------------------------------------------------------*/
{
		return nBuffer;
}

/*--------------------------------------------------------------*/
int		DOut_GetBufferWritePos(void)
/*--------------------------------------------------------------*/
{
DWORD	writepos;

    lpdsBuffer->GetCurrentPosition(NULL,&writepos);
    return ((writepos / DOUT_SAMPLESIZE / nBufferLen) - 1) % nBufferNum;
}

/*--------------------------------------------------------------*/
int		DOut_GetBufferPlayPos(void)
/*--------------------------------------------------------------*/
{
DWORD	playpos;

    lpdsBuffer->GetCurrentPosition(&playpos,NULL);
    return ((playpos / DOUT_SAMPLESIZE / nBufferLen) - 1) % nBufferNum;
}

