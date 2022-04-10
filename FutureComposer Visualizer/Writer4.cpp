#include "Writer4.h"
#include "Font4.h"
#include <stdio.h>

/*-----------------------------------------------------------------*/
int		TWriter4::GetCharMatrixPos(char c)
/*-----------------------------------------------------------------*/
{
int		i,pos = -1;
char	*p = CharMatrix;

	for (i = 0; p[i] != 0; i++)
	{
		if (p[i] == c) 
		{
			pos = i;
			break;
		}
	}
	return pos;
}

/*-----------------------------------------------------------------*/
TWriter4::TWriter4(void)
/*-----------------------------------------------------------------*/
{
	Frame = NULL;
	Width = Height = 0;
	memcpy(CharMatrix,"0123456789ABCDEF# ",18);
}


/*-----------------------------------------------------------------*/
void	TWriter4::SetDest(SBYTE *dst, int w, int h)
/*-----------------------------------------------------------------*/
{
	Frame = dst;
	Width = w;
	Height = h;
}


/*-----------------------------------------------------------------*/
void	TWriter4::CharCopy(int DstX, int DstY, char c, int col)
/*-----------------------------------------------------------------*/
{
int	pos, x;
SBYTE	*Src, *Dst;

	if (Frame == NULL) return;

	if ((pos = GetCharMatrixPos(c)) == -1)	x = 0;
	else								x = pos % FONTIMAGEWIDTH;


	Src = ((SBYTE*)Font4) +	(col * FONTHEIGHT * FONTIMAGEWIDTH) + 
							(x * FONTWIDTH);

	Dst = Frame +	(DstY * FONTHEIGHT * Width) + 
					(DstX * FONTWIDTH);

	x = Width-4;

/*
DWORD	*pSrc, *pDst;
	pSrc = (DWORD*)Src;
	pDst = (DWORD*)Dst;

	for (pos = 0; pos < 6; pos++)
	{
		*pDst = *pSrc;
		pSrc += (FONTIMAGEWIDTH/4);
		pDst += (Width/4) ;
	}
*/

	_asm {
		push ecx
		push esi
		push edi

		mov	esi,Src
		mov	edi,Dst
		mov	ecx,FONTHEIGHT
		cld
copyf4:
		movsd
		add	esi,FONTIMAGEWIDTH-4
		add	edi,x
		
		dec	ecx
		jnz	copyf4

		pop edi
		pop esi
		pop ecx
	}



}


/*-----------------------------------------------------------------*/
void	TWriter4::WriteFloat(int DstX, int DstY, float n, int col)
/*-----------------------------------------------------------------*/
{
char	buf[16],c;
char	*s = buf;

	sprintf(buf,"%4f",n);

	while (c = *s++)
	{
		CharCopy(DstX, DstY, CharMatrix[c & 0xF], col&3);
		DstX++;
	}
}


/*-----------------------------------------------------------------*/
void	TWriter4::WriteHexByte(int DstX, int DstY, int n, int c)
/*-----------------------------------------------------------------*/
{
	CharCopy(DstX,   DstY, CharMatrix[(n >> 4) & 0xF], c&3);
	CharCopy(DstX+1, DstY, CharMatrix[n & 0xF], c&3);
}


/*-----------------------------------------------------------------*/
void	TWriter4::WriteHexWord(int DstX, int DstY, int n, int c)
/*-----------------------------------------------------------------*/
{
	WriteHexByte(DstX,   DstY, (n >> 8) & 0xFF, c);
	WriteHexByte(DstX+2, DstY, n & 0xFF, c);
}

/*-----------------------------------------------------------------*/
void	TWriter4::WriteHexLong(int DstX, int DstY, int n, int c)
/*-----------------------------------------------------------------*/
{
	WriteHexWord(DstX,   DstY, (n >> 16) & 0xFFFF, c);
	WriteHexWord(DstX+4, DstY, (n  & 0xFFFF), c);
}

/*-----------------------------------------------------------------*/
void	TWriter4::WriteHexLongCut(int DstX, int DstY, int n, int c)
/*-----------------------------------------------------------------*/
{
	WriteHexByte(DstX,   DstY, (n >> 16) & 0xFF, c);
	WriteHexWord(DstX+2, DstY, (n  & 0xFFFF), c);
}
