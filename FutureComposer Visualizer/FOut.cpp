#include <windows.h>
#include <dsound.h>
#include <cguid.h>
#include <stdio.h>
#include "DOut.h"


#define DOUT_MIN_BUFSIZE	(882*2)
#define DOUT_MIN_BUFFER		2
#define DOUT_MAX_BUFFER		16
#define	DOUT_RASTERSIZE		((44100*2*2)/50)

static	int			nBufferLen, nBufferNum, nBuffer;
static	void		(*UserFunction)(void);

static	int			Times = 0;
static	BOOL		Opened;
FILE	*fp;

/*--------------------------------------------------------------*/
int	 DOut_Open(void (*UserF)(void),
			   int BufferLen,
			   int BufferNum, 
			   DWORD User)
/*--------------------------------------------------------------*/
{
	UserFunction = UserF;

	BufferLen -= (BufferLen % (44100 / 50));

	if (BufferLen < DOUT_MIN_BUFSIZE) BufferLen = DOUT_MIN_BUFSIZE;
	if (BufferNum < DOUT_MIN_BUFFER)  BufferNum = DOUT_MIN_BUFFER;
	if (BufferNum > DOUT_MAX_BUFFER)  BufferNum = DOUT_MAX_BUFFER;
	nBufferLen = BufferLen;
	nBufferNum = BufferNum;
	Times = (int)User;

	fp = fopen("F:\\OUT.WAV","wb");
	return 0;
}



/*--------------------------------------------------------------*/
int	 DOut_Start(void)
/*--------------------------------------------------------------*/
{
int	i;

	for (i = 0; i < 10; i++)
//	while (Times--)	
	{

		if (UserFunction != NULL) (*UserFunction)();

	}
	
	return 0;
}



/*--------------------------------------------------------------*/
void	DOut_Close(void)
/*--------------------------------------------------------------*/
{
	fclose(fp);
}



/*--------------------------------------------------------------*/
void	DOut_BufferCopy(void *src)
/*--------------------------------------------------------------*/
{
	fwrite(src, 1, nBufferNum * nBufferLen * DOUT_RASTERSIZE, fp);
}

