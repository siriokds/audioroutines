#include <Windows.h>

#define WNDNAME	"F u t u r e   C o m p o s e r"
#define RESXdev 640
#define RESYdev 480

#define RESX RESXdev
#define RESY 480

#include "Types.h"
#include "ptc.h"


#include "FC6_Thalion.h"
//#include "FC6_Astaroth.h"
#include "FC6_Shaolin1.h"
#include "FC6_SkidRow5.h"
#include "FC6_Blaizer.h"
#include "FC6_Trsdcs.h"
#include "FC6_Monty.h"
#include "FC6_Paradox.h"
#include "FC6_Krnfpow.h"


//--------------------------------------------------------------

void	RunOnce(char8 *Dst, int Width, int Height);

//--------------------------------------------------------------


/*----------------------------------------------------------*/
int32 pack(int32 r,int32 g,int32 b)
/*----------------------------------------------------------*/
{
    // pack color integer
    return (r<<16) | (g<<8) | b; 
}

/*----------------------------------------------------------*/
void generate(Palette &palette, char8 *p, int n)
/*----------------------------------------------------------*/
{
	int32	*data = palette.lock();

   	for (int i = 0; i < n; i++)
	{
		data[i] = pack((int32)p[0]<<2, (int32)p[1]<<2, (int32)p[2]<<2);
		p += 3;
	}

    palette.unlock();
}
