//---------------------------------------------------------------------------
#ifndef Writer4H
#define Writer4H
//---------------------------------------------------------------------------
#include <windows.h>
#include "Types.h"
	
#define FONTIMAGEWIDTH 128

#define FONTWIDTH	4
#define FONTHEIGHT	6

class	TWriter4 
{

private:
	SBYTE	*Frame;
	int		Width, Height;
	char	CharMatrix[18];
	
	int		GetCharMatrixPos(char c);
public:
	TWriter4(void);

	void	SetDest(SBYTE *dst, int w, int h);
	void	CharCopy(int DstX, int DstY, char c, int col);
	void	WriteFloat(int DstX, int DstY, float n, int c);
	void	WriteHexByte(int DstX, int DstY, int n, int col);
	void	WriteHexWord(int DstX, int DstY, int n, int col);
	void	WriteHexLong(int DstX, int DstY, int n, int col);
	void	WriteHexLongCut(int DstX, int DstY, int n, int col);
};


#endif
