#include <windows.h>

#define CDXINCLUDEALL           // include all CDX headers
#include <CDX.h>

#include "KTSC.h"

#include "FC6_Thalion.h"

#include "Mixer.h"
#include "DOut.h"
#include "Replayer.h"
#include "Writer4.h"
#include "Font4pal.h"


BOOL			bActive     = TRUE; // Is the program running?
CDXScreen		* Screen    = 0;    // The screen object, every program must have one
CDXInput		* Input     = 0;    // The input object
PALETTEENTRY	Palette[256];

TWriter4		Wrt;
tDataInfo		Info;
int				StressReset = 0;
KTSC			cpuBench;
float			cpuMS = 0;

void	DoUpdate(void);
void	RunOnce(SBYTE *Dst, int Width, int Height);



/*-----------------------------------------------------------------------------------------*/
void	GeneratePalette(void)
/*-----------------------------------------------------------------------------------------*/
{

	Palette[0].peRed	= 4;
	Palette[0].peGreen	= 74;
	Palette[0].peBlue	= 104;
	Palette[0].peFlags	= NULL;

	for (int i = 1; i < 8; i++)
	{
		Palette[i].peRed	= Font4pal[(i*3)+0] << 2;
		Palette[i].peGreen	= Font4pal[(i*3)+1] << 2;
		Palette[i].peBlue	= Font4pal[(i*3)+2] << 2;
		Palette[i].peFlags	= NULL;
	}
}


// ------------------------------------------------------------------------------------------
// FiniApp - Destroy the CDX objects
// ------------------------------------------------------------------------------------------
/*-----------------------------------------------------------------------------------------*/
void FiniApp(void)
/*-----------------------------------------------------------------------------------------*/
{
	FC6_Close();

    SAFEDELETE( Input );
	SAFEDELETE( Screen );
}



/*-----------------------------------------------------------------------------------------*/
long APIENTRY WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
/*-----------------------------------------------------------------------------------------*/
{
	switch(message)
	{
		case WM_ACTIVATEAPP:    bActive = wParam;   // set if app is active or not
			                    break;

		case WM_KEYDOWN:		switch(wParam)      // if ESC key is hit, quit program
			                    {
				                    case VK_ESCAPE:
					                PostMessage(hWnd, WM_CLOSE, 0, 0);
				                    break;
			                    }
		                        break;

		case WM_DESTROY:		FiniApp();			// destroy all objects
								PostQuitMessage(0); // terminate program
		                        break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}




/*-----------------------------------------------------------------------------------------*/
BOOL InitApp(HINSTANCE hInst, int nCmdShow)
/*-----------------------------------------------------------------------------------------*/
{
	HWND hWnd;
	WNDCLASS WndClass;

	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WinProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInst;
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = "Example 5";
	RegisterClass(&WndClass);

    // create the main window
	hWnd = CreateWindowEx(
		WS_EX_TOPMOST,
		"Example 5",
		"Example 5",
		WS_POPUP,
		0,0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		hInst,
		NULL);

	if(!hWnd) 
        CDXError( NULL , "could not create the main window" );


	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	Screen = new CDXScreen();

    if( Screen->CreateFullScreen( hWnd , 640 , 480 , 8 ) == FALSE )
        CDXError( Screen , "could not set video mode 640x480x8" );


	GeneratePalette();
	Screen->SetPalette(0,8,Palette);



	Input = new CDXInput;
    if( Input == NULL )	
		CDXError( Screen , "Could not create CDXInput object!" );

	if( Input->Create(hInst, hWnd) == FALSE )
        CDXError( Screen , "Could not create CDXInput object!" );


	Sleep(500);


	if (FC6_Open((DWORD)GetActiveWindow(),4,4) == 0)
	{
			FC6_Init((SBYTE*)Modulo);
			FC6_Start();
	}



	return TRUE;
}



/*-----------------------------------------------------------------------------------------*/
/* WinMain																				   */
/*-----------------------------------------------------------------------------------------*/
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
/*-----------------------------------------------------------------------------------------*/
{
	MSG     msg;

	if(!InitApp(hInst, nCmdShow))
        CDXError( NULL , "Could not initialize application!" );

	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(!GetMessage(&msg, NULL, 0, 0 )) return msg.wParam;
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
		else if(bActive)                            // if application has the focus, process
		{
			DoUpdate();
		}
		else WaitMessage();
	}
}











/*-----------------------------------------------------------------------------------------*/
void	DoUpdate(void)
/*-----------------------------------------------------------------------------------------*/
{
		Input->Update();

		cpuBench.SetTSC();

		Screen->Fill(0);

		Screen->GetBack()->Lock( );
		SBYTE *pixels = (SBYTE*) Screen->GetBack()->GetSurfaceBytesPointer( );

//------------------------

		RunOnce(pixels, Screen->GetBack()->GetPitch() , Screen->GetBack()->GetHeight());

//------------------------
		Screen->GetBack()->UnLock();

		cpuMS = cpuBench.GetCPUms();

		Screen->Flip(TRUE);

}



















/*-----------------------------------------------------------------------*/
void	DrawSEQ(int x, int y, int rows)
/*-----------------------------------------------------------------------*/
{
int	 i,j,lx,col;
SBYTE	*p = Info.SeqPtr;
int		rw = Info.SeqRow;

	if (!FC6_IsPlaying()) return;
	if (!p) return;

	for (j = 0; j < rows; j++)
	{
		col = (j == 0) ? 0 : 1;
		lx = x;

		Wrt.WriteHexByte(x, y+j, rw++, col); x += 3;
		x ++;


		for (i = 0; i < 4; i++)
		{
			if (VGF1_GetChanMasterVol(i))
			{
				Wrt.WriteHexByte(x,   y+j, p[0], col);
				Wrt.WriteHexByte(x+3, y+j, p[1], col);
				Wrt.WriteHexByte(x+6, y+j, p[2], col);
			}

			p+=3;
			x+=10;
		}

		
		Wrt.WriteHexByte(x, y+j, *p++, col);

		x = lx;
		p +=3;
	}
}



/*-----------------------------------------------------------------------*/
void	DrawQUAD(int x, int y, int rows, int dec, int chn)
/*-----------------------------------------------------------------------*/
{
int	 i,j,k,col;

SBYTE *p;
SBYTE cnt;
SBYTE val;
SBYTE spd;


	if (!FC6_IsPlaying()) return;

	if (!VGF1_GetChanMasterVol(chn)) return;

	if (dec)
	{
		p   = Info.Chan[chn].VolPtr;
		cnt = Info.Chan[chn].VolOfs;
		val = Info.Chan[chn].VolOfsVal;
		spd = Info.Chan[chn].VolOfsSpd;

		p -= 5; 
		cnt += 5;
	} 
	else
	{
		p   = Info.Chan[chn].FrqPtr;
		cnt = Info.Chan[chn].FrqOfs;
		val = Info.Chan[chn].FrqOfsVal;
		spd = Info.Chan[chn].FrqOfsSpd;
	}

	if (!p) return;

//		if (dec) { p -= 5; cnt += 5; }


	Wrt.WriteHexByte(x ,y  , cnt, 0);
	Wrt.WriteHexByte(x ,y+1, spd, 0);
	Wrt.WriteHexByte(x ,y+2, val, 0);

	x += 4;

	k = 0;
	for (j = 0; j < rows; j++)
	{
		for (i = 0; i < 16; i++)
		{
			col = 0;

			if (k == cnt) col = 2;
			else
				if ((dec) && (k < 5)) col = 1;

			Wrt.WriteHexByte(x+(i*3) ,y+j, *p++, col);

			k++;
		}
	}
}


/*-----------------------------------------------------------------------*/
void	DrawPAT(int x, int y, int chn)
/*-----------------------------------------------------------------------*/
{
int		j,c;
int		row = Info.ActRow;
SBYTE	*p = Info.Chan[chn].PatPtr;

		if (!FC6_IsPlaying()) return;
		if (!VGF1_GetChanMasterVol(chn)) return;

		if (!p) return;

		for (j = 0; j < 32; j++)
		{
			c = (j == row) ? 2 : 0;

			Wrt.WriteHexByte(x  ,y+j,*p++, c);
			Wrt.WriteHexByte(x+3,y+j,*p++, c);
		}
}


/*-----------------------------------------------------------------------*/
void	DrawPER(int x, int y)
/*-----------------------------------------------------------------------*/
{
int		i,v;


	if (!FC6_IsPlaying()) return;
		for (i = 0; i < 4; i++)
		{
			if (!VGF1_GetChanMasterVol(i)) continue;
			v = Info.Chan[i].Period;

			Wrt.WriteHexWord(x+(i*10),y  ,v >> 16, 0);
			Wrt.WriteHexWord(x+(i*10),y+1,v & 0xFFFF, 0);
		}
}


/*-----------------------------------------------------------------------*/
void	DrawBUF(SBYTE *dst, int w, int x, int y)
/*-----------------------------------------------------------------------*/
{
int	i,j,val;
SWORD *p = Info.MixBuffer;
static wpos = 0;

	if (!FC6_IsPlaying()) return;
	if (!p) return;

	p += (wpos*882*2);
	wpos++; wpos &= 3;

	for (i = 0, j = 0; i < 300; i++, j+=4)
	{
		val = (p[j] >> 8) ^ 128;
		val = ((255 - val) & 0xFF) >> 2;
		dst[(x+i)+((y+val)*w)] = 2;

		val = (p[j+1] >> 8) ^ 128;
		val = ((255 - val) & 0xFF) >> 2;
		dst[(x+i+320)+((y+val)*w)] = 2;
	}
}


/*-----------------------------------------------------------------------*/
void	DrawSMP(SBYTE *dst, int w, int x, int y, int chn)
/*-----------------------------------------------------------------------*/
{
int	i,val;
int	ofs = Info.Chan[chn].SampleOfs;
int	siz = Info.Chan[chn].SampleSiz;
int	vol = (Info.Chan[chn].DevVolume > 64) ? 64 : Info.Chan[chn].DevVolume;
int	pos,inc;

	if (!FC6_IsPlaying()) return;
	if (!VGF1_GetChanMasterVol(chn)) return;

	pos = 0;
	inc = (siz * 1024) / 64;

	for (i = 0; i < 64; i++)
	{
		val = ((VGF1_Peek((pos/1024)+ofs) * vol) / 64) ^ 128;
		val = ((255 - val) & 0xFF) / 8;
		if (!(i&3)) 
		{
			dst[(x+i)+((y+6+0)*w)] = 3;
			dst[(x+i)+((y+6+15)*w)] = 3;
			dst[(x+i)+((y+6+31)*w)] = 3;
		}
		dst[(x+i)+((y+6+val)*w)] = 5;
		pos += inc;
	}

	Wrt.WriteHexLongCut(x/4, y/6, ofs, 1);
	Wrt.WriteHexByte((x+56)/4, y/6, vol, 1);
}


/*-----------------------------------------------------------------------*/
void	DrawStress(void)
/*-----------------------------------------------------------------------*/
{
int val, ActStress;
static	int StxMax = 0, StxMin = 0;
static	int	LastStress = 0;

	if (StressReset)
	{
		StressReset = 0;
		StxMax = StxMin = ActStress = LastStress = 0;
	}
	
	ActStress = VGF1_GetMixStress();
	val = (ActStress + LastStress) / 2;

	if (ActStress > StxMax) StxMax = ActStress;

	if (StxMin == 0) StxMin = ActStress;
	else
		if (ActStress < StxMin) StxMin = ActStress;

	Wrt.WriteHexWord(0,0, ActStress, 0);
	Wrt.WriteHexWord(5,0, val, 0);
	Wrt.WriteHexWord(12,0, StxMin, 0);
	Wrt.WriteHexWord(17,0, StxMax, 0);


}


/*----------------------------------------------------------*/
void	RunOnce(SBYTE *Dst, int Width, int Height)
/*----------------------------------------------------------*/
{
int yOffs = 5;

	if (!FC6_IsPlaying()) return;
	FC6_PutInfo(&Info);

	Wrt.SetDest((SBYTE*)Dst,Width,Height);


	DrawStress();

	Wrt.WriteHexWord(0,1, VGF1_GetActiveChannels(),0);

	Wrt.WriteHexByte(0,3, DOut_GetBufferPlayPos(),0);
	Wrt.WriteHexByte(0,4, DOut_GetBufferWritePos(),0);
	Wrt.WriteFloat(0,5, cpuMS,0);

	DrawPAT(20,yOffs + 12,0);
	DrawPAT(30,yOffs + 12,1);
	DrawPAT(40,yOffs + 12,2);
	DrawPAT(50,yOffs + 12,3);


	DrawQUAD(100,yOffs + 0, 4,1, 0);
	DrawQUAD(100,yOffs + 5, 4,0, 0);

	DrawQUAD(100,yOffs +15, 4,1, 1);
	DrawQUAD(100,yOffs +20, 4,0, 1);
	
	DrawQUAD(100,yOffs +30, 4,1, 2);
	DrawQUAD(100,yOffs +35, 4,0, 2);

	DrawQUAD(100,yOffs +45, 4,1, 3);
	DrawQUAD(100,yOffs +50, 4,0, 3);

	DrawSEQ(16, yOffs + 0, 7);
	DrawPER(20, yOffs + 8);

	DrawBUF((SBYTE*)Dst+8, Width, 0, 400);


	DrawSMP((SBYTE*)Dst, Width, 328, 32+yOffs + (6*0),  0);
	DrawSMP((SBYTE*)Dst, Width, 328, 32+yOffs + (6*15), 1);
	DrawSMP((SBYTE*)Dst, Width, 328, 32+yOffs + (6*30), 2);
	DrawSMP((SBYTE*)Dst, Width, 328, 32+yOffs + (6*45), 3);
}


