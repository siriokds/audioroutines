#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

typedef signed char SBYTE;



class	ADPCM
{
static	int		LOGtab[16];
		int		DIFtab[256];

		int		GetDiff(int v);
public:
				ADPCM();
		void	packDATA(SBYTE *dp, SBYTE *sp, int nWord);
		void	unpackDATA(SBYTE *dp, SBYTE *sp, int nWord);
};


int		ADPCM::LOGtab[16] = { -127,-63,-31,-15,-7,-3,-1,0,1,2,3,7,15,31,63,127 };


/*---------------------------------------------------------------*/
		ADPCM::ADPCM()
/*---------------------------------------------------------------*/
{
	for (int i = -128; i < 127; i++) DIFtab[i&0xFF] = GetDiff(i);
}

/*---------------------------------------------------------------*/
int		ADPCM::GetDiff(int v)
/*---------------------------------------------------------------*/
{
int	min_index = 0;
int	min_val = abs(v - LOGtab[min_index]);

	for (int i = 1; i < 16; i++)
	{
		int t = abs(v - LOGtab[i]);

		if (t < min_val) 
		{ 
			min_val = t;
			min_index = i; 
		}
	}
	return min_index;
}


/*---------------------------------------------------------------*/
void	ADPCM::packDATA(SBYTE *dst, SBYTE *src, int nWord)
/*---------------------------------------------------------------*/
{
int		Delta1, Delta2;
int		LastVal = 0;

	for (int i = 0; i < nWord; i++)
	{
		Delta1 = DIFtab[(*src++ - LastVal) & 0xFF];
		LastVal += LOGtab[Delta1 & 15];

		Delta2 = DIFtab[(*src++ - LastVal) & 0xFF];
		LastVal += LOGtab[Delta2 & 15];

		*dst++ = (SBYTE) ( ((Delta1 << 4) & 0xF0) | (Delta2 & 0x0F) );
	}
}





/*---------------------------------------------------------------*/
void	ADPCM::unpackDATA(SBYTE *dp, SBYTE *sp, int nWord)
/*---------------------------------------------------------------*/
{
int LastVal = 0;

	for (int i = 0; i < nWord; i++)
	{
		int actVal = *sp++;
	
		LastVal += LOGtab[(actVal >> 4) & 0x0F];	*dp++ = (SBYTE)LastVal;
		LastVal += LOGtab[actVal & 0x0F];			*dp++ = (SBYTE)LastVal;
	}
}




/*---------------------------------------------------------------*/
int		main(void)
/*---------------------------------------------------------------*/
{
FILE	*fp;
int		size;
SBYTE	*mp,*dp;

ADPCM	pkr;


		fp = fopen("d:\\data\\1_8bit.raw","rb");
		fseek(fp,0,2);
		size = ftell(fp);
		fseek(fp,0,0);
		mp = new SBYTE[size];
		fread(mp,1,size,fp);
		fclose(fp);

		dp = new SBYTE[size/2];

		pkr.packDATA(dp, mp, size/2);

		fp = fopen("d:\\data\\1_8bit_adpcm.raw","wb");
		fwrite(dp,1,size/2,fp);
		fclose(fp);

		delete[] dp;
		delete[] mp;




		fp = fopen("d:\\data\\1_8bit_adpcm.raw","rb");
		fseek(fp,0,2);
		size = ftell(fp);
		fseek(fp,0,0);
		mp = new SBYTE[size];
		fread(mp,1,size,fp);
		fclose(fp);

		dp = new SBYTE[size*2];

		pkr.unpackDATA(dp, mp, size);

		fp = fopen("d:\\data\\1_8bit_pcm.raw","wb");
		fwrite(dp,1,size*2,fp);
		fclose(fp);

		delete[] dp;
		delete[] mp;

		return 0;
}

