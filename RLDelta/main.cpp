#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "Types.h"
#include "RLDelta.h"


RLDelta	RLD;
FILE *fp;

SBYTE	src[] = { 0x7E, 0x72, 0x7F, 0x7F, 0x81, 0x82, 0x81, 0x81 };
SBYTE	dst[8];

/*--------------------------------------------------*/
void	main(void)
/*--------------------------------------------------*/
{
	RLD.pack(dst,src,4);
}

