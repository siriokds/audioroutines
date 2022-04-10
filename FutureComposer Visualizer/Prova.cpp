#include <stdlib.h>
#include "Prova.h"
#include "Mixer.h"
#include "DOut.h"
#include "Replayer.h"
#include "Writer4.h"
#include "Font4pal.h"



TWriter4	Wrt;
tDataInfo	Info;
Console		console;

int			StressReset = 0;

int			KeyState[10];


/*----------------------------------------------------------*/
int APIENTRY WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,
					 LPSTR lpCmdLine,int nCmdShow)
/*----------------------------------------------------------*/
{
int		ExitFlag = 0;

    try
    {
		for (int i = 0; i < 10; i++) KeyState[i] = 0;

		Font4pal[0] = 0;
		Font4pal[1] = 6;
		Font4pal[2] = 10;

        Format format(8);

        Palette palette;
        generate(palette,Font4pal,8);

		console.option("DirectX");
/*
		console.option("windowed output");
	    console.option("fixed window");
	    console.option("center window");
*/

        console.open(WNDNAME,RESXdev,RESYdev,format);
        console.palette(palette);

        Surface surface(RESX,RESY,format);
        surface.palette(palette);

		if (FC6_Open((DWORD)GetActiveWindow(),4,4) == 0)
		{
				FC6_Init((SBYTE*)Modulo);
				FC6_Start();
		}




        while(!ExitFlag)
        {

            if (console.key())
            {
                // read console key press
                const Key key = console.read();

                // handle cursor keys
                switch (key.code())
                {
                    case Key::F1:	VGF1_SetChanMasterVol(0,-1); break;
                    case Key::F2:	VGF1_SetChanMasterVol(1,-1); break;
                    case Key::F3:	VGF1_SetChanMasterVol(2,-1); break;
                    case Key::F4:	VGF1_SetChanMasterVol(3,-1); break;

                    case Key::F5:	FC6_GoPrevSeq();  break;
					case Key::F6:	FC6_GoNextSeq();  break;
                    case Key::F7:	VGF1_FilterONOFF(); break;
                    case Key::F8:	FC6_MusicONOFF(); break;

                    case Key::ONE:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo);
						FC6_Start();
						Sleep(100);
						break;

                    case Key::TWO:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo2);
						FC6_Start();
						Sleep(100);
						break;

                    case Key::THREE:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo3);
						FC6_Start();
						Sleep(100);
						break;

                    case Key::FOUR:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo4);
						FC6_Start();
						Sleep(100);
						break;

                    case Key::FIVE:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo5);
						FC6_Start();
						Sleep(100);
						break;

                    case Key::SIX:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo6);
						FC6_Start();
						Sleep(100);
						break;

                    case Key::SEVEN:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo7);
						FC6_Start();
						Sleep(100);
						break;

                    case Key::EIGHT:
						while (console.key()) console.read();
						Sleep(400);
						FC6_Init((SBYTE*)Modulo8);
						FC6_Start();
						Sleep(100);
						break;


                    case Key::NUMPAD1:
						StressReset = 1;
						FC6_Close();
						if (FC6_Open((DWORD)GetActiveWindow(),4,4) == 0)
						{
							FC6_Init((SBYTE*)Modulo);
							FC6_Start();
						}
						break;

                    case Key::NUMPAD2:
						StressReset = 1;
						FC6_Close();
						if (FC6_Open((DWORD)GetActiveWindow(),4,8) == 0)
						{
							FC6_Init((SBYTE*)Modulo);
							FC6_Start();
						}
						break;

                    case Key::NUMPAD3:
						StressReset = 1;
						FC6_Close();
						if (FC6_Open((DWORD)GetActiveWindow(),8,8) == 0)
						{
							FC6_Init((SBYTE*)Modulo);
							FC6_Start();
						}
						break;
                }

                // exit when escape is pressed
                if (key.code()==Key::ESCAPE) ExitFlag = 1;

            }


			surface.clear();

            char8 *pixels = (char8*) surface.lock();
//------------------------

			RunOnce(pixels, surface.width(), surface.height());

//------------------------
            surface.unlock();

            Area areaSrc(0,0,RESX,RESY);
            Area areaDst(0,((RESYdev-RESY)/2),RESX,((RESYdev-RESY)/2)+RESY);

            surface.copy(console,areaSrc,areaDst);

            console.update();

        }

		console.close();


		FC6_Close();


    }
    catch (Error &error)
    {
        // report error
        error.report();
    }


    // exit
    return 0;
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
int	vol = (Info.Chan[chn].Volume > 64) ? 64 : Info.Chan[chn].Volume;
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
void	RunOnce(char8 *Dst, int Width, int Height)
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



