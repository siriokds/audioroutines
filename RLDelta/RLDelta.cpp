#include "RLDelta.h"
#include <math.h>

//int		RLDelta::LOGtab[16] = { 0, -127,-63,-31,-15,-7,-3,-1, 0, 1,3,7,15,31,63,127 };

SBYTE		RLDelta::LOGtab[16] = { 0,-2,-4,-8,-16,-32,-64, -127, 0,2,4,8,16,32,64,127 };

/*---------------------------------------------------------------*/
int		RLDelta::clip(int val)
/*---------------------------------------------------------------*/
{
	return (val < -127) ? -127 : ((val > 127) ? 127 : val);
}


/*---------------------------------------------------------------*/
int		RLDelta::GetDiff(SBYTE newval)
/*---------------------------------------------------------------*/
{
int i;
SBYTE	tval;

	for (i = 1; i < 16; i++) 
	{
		tval = oldval+LOGtab[i];
		if (tval == newval) return i;
	}


int	lastdelta = 10000;
int	lastindex, delta;
int	fault = 4;


	if (newval > oldval)
	{
		for (i = 9; i < 16; i++)
		{
			delta = newval - (oldval+LOGtab[i]); 
			if (delta < 0) delta = -delta;

			if (delta < lastdelta)
			{
				lastdelta = delta;
				lastindex = i;
			}
		}

		if (lastdelta > fault) return 0;
		if ((oldval+LOGtab[lastindex]) > 127) 
		{
			lastindex--;
			if ((oldval+LOGtab[lastindex]) > 127) return 0;
		}

		return lastindex;
	}


	for (i = 1; i < 8; i++)
	{
		delta = newval - (oldval+LOGtab[i]); 
		if (delta < 0) delta = -delta;

		if (delta < lastdelta)
		{
			lastdelta = delta;
			lastindex = i;
		}
	}

	if (lastdelta > fault) return 0;
	if ((oldval+LOGtab[lastindex]) < -127) 
	{
		lastindex--;
		if ((oldval+LOGtab[lastindex]) > 127) return 0;
	}

	return lastindex;
}
	


/*---------------------------------------------------------------*/
int		RLDelta::pack(SBYTE *dst, SBYTE *src, int nWord)
/*---------------------------------------------------------------*/
{
SBYTE	val1, val2;
int		valN, idx1, idx2;
int		packedsize = 0;
int		_ok = 0, _no = 0;
int		size = nWord*2;

SBYTE	lastpacked = 0x00, packed;
int		pack_acc = 0;
int		pack_blk = 0;
int		readed = 0;

	while (nWord--)
	{
		valN = 0;

		val1 = *src++;	val2 = *src++;	readed += 2;
		
		if ((idx1 = GetDiff(val1)) > 0) { valN++; oldval += LOGtab[idx1]; }
		if ((idx2 = GetDiff(val2)) > 0) { valN++; oldval += LOGtab[idx2]; }

		packed = (SBYTE)((idx1 << 4) | (idx2 & 15));

		if (valN < 2)
		{
			dst[0] = 0x00;
			dst[1] = (SBYTE)val2;
			oldval = dst[1];
			dst += 2;
			packedsize += 2;
		}
		else
		{

			if (packed == lastpacked) 
			{ 
				pack_acc++; 
			}
			else
			{
				if (lastpacked == 0x00) lastpacked = packed;

				if (pack_acc > 0)
				{
	
					if (pack_acc <= 256)
					{
						dst[0] = 0x01;
						dst[1] = (SBYTE)(pack_acc-1);
						dst += 2;
						packedsize += 2;
					}
					else
					{
						dst[0] = 0x02;
						dst[1] = (SBYTE)((pack_acc-1)&0xFF);
						dst[2] = (SBYTE)(((pack_acc-1)>>8)&0xFF);
						dst += 3;
						packedsize += 3;
					}
				}

				*dst++ = lastpacked;
				packedsize++;
					
				lastpacked = packed;
				pack_acc = 0;

			}

		}

	}

	return packedsize;
}









/*---------------------------------------------------------------*/
void	RLDelta::unpack(SBYTE *dst, SBYTE *src, int nWord)
/*---------------------------------------------------------------*/
{
SBYTE	val;

int		nBytes = nWord * 2;
SBYTE	lastval = 0x00;
int		counter;

	while (nBytes)
	{
		val = *src++;

		if (val == 0x00)
		{
			val = src[0];
			dst[0] = val;
			dst[1] = val;
			lastval = val;
			dst += 2;
			nBytes -= 2;
			src++;
			continue;
		}

		if (val == 0x01)
		{
			counter = (((int)src[0])&255)+1;
			src++;
			nBytes -= counter;
			while (counter--) *dst++ = lastval;
			continue;
		}
	
		if (val == 0x02)
		{
			counter = (int(src[0])&255) + ((int(src[1])<<8)&0xFF00);
			src += 2;
			nBytes -= counter;
			while (counter--) *dst++ = lastval;
			continue;
		}
	
		lastval += LOGtab[ ((val&0xF0)>>4) & 15 ];
		*dst++ = lastval;

		lastval += LOGtab[ val & 15 ];
		*dst++ = lastval;

		nBytes -= 2;
	
	}


}
