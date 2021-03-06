#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "Mod2Chp.h"

struct TModuleHeader		ModuleHeader;
struct TSampleDescriptor	SampleDescriptor[32];



class	ADPCM
{
static	int		LOGtab[16];
		int		DIFtab[256];

		int		GetDiff(int v);
public:
				ADPCM();
		int		clip(int val);
		void	packDATA(SBYTE *dp, SBYTE *sp, int nWord);
		void	unpackDATA(SBYTE *dp, SBYTE *sp, int nWord);
		int		packDATA2(SBYTE *dp, SBYTE *sp, int nWord);
		void	unpackDATA2(SBYTE *dp, SBYTE *sp, int nWord);
};


UBYTE	*PackedPatternData, *SrcSampleData, *DstSampleData;
ULONG	PackedPatternLen, UnPackedPatternLen;
UBYTE	Positions[128];		// Position data
ULONG	SampleDataPos, SampleDataSize, unpSampleDataSize;
UBYTE	*lpMod;
FILE	*fp;
SLONG	ModSize = 0, Readed = 0, Writed = 0;

ADPCM	pkr;

/*---------------------------------------------------------------*/
// Prototypes

void	initHeader(void);
void	GetSampleINFO(void);
void	GetOrderDATA(void);
void	GetPatternDATA(void);
UBYTE*	PackRLE_CHNPatternDATA(UBYTE *src, UBYTE *dst);
/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
void	initHeader(void)
/*---------------------------------------------------------------*/
{
int i;

		strcpy(ModuleHeader.ID, HEADER_ID);

		ModuleHeader.Version = HEADER_VER;

		ModuleHeader.Restart = 0;
		
		ModuleHeader.NumOfPat = 0;

		ModuleHeader.lptrSampleData = sizeof(struct TModuleHeader);

		ModuleHeader.Unused1 = 
		ModuleHeader.Unused2 = 
		ModuleHeader.Unused1 = 0;

		ModuleHeader.NumOfPos = 0;

		for (i = 0; i < HEADER_POSNUM; i++)	Positions[i] = 0;
	


		for (i = 0; i < 32; i++)
		{
			SampleDescriptor[i].SampleSize = 0;	// 2
			SampleDescriptor[i].Finetune = 0;	// 1
			SampleDescriptor[i].Volume = 0;		// 1
			SampleDescriptor[i].LoopStart = 0;	// 2
			SampleDescriptor[i].LoopSize = 0;	// 2
		}
}


/*---------------------------------------------------------------*/
void	GetOrderDATA(UBYTE *mp)
/*---------------------------------------------------------------*/
{
	ModuleHeader.NumOfPos = mp[950];
	ModuleHeader.Restart = mp[951];
	ModuleHeader.NumOfPat = 0;

	for (int i = 0; i < HEADER_POSNUM; i++)
	{
		Positions[i] = mp[952+i];
		if (Positions[i] > ModuleHeader.NumOfPat) 
			ModuleHeader.NumOfPat = Positions[i];
	}

	ModuleHeader.NumOfPat++;
	ModuleHeader.lptrSampleData += ModuleHeader.NumOfPos;
}


/*---------------------------------------------------------------*/
void	GetSampleINFO(UBYTE *mp)
/*---------------------------------------------------------------*/
{
int	i,k;

	SampleDataSize = 0;
	ModuleHeader.NumOfSmp = 0;


	for (i = 1; i < 32; i++)
	{
		// skip first 22 bytes (samplename)
		SampleDescriptor[i].SampleSize = SWAP_WORD(mp,22);	// 2

		if (SampleDescriptor[i].SampleSize) 
		{

			ModuleHeader.NumOfSmp++;

			k = mp[24]; //if (k > 7) k = k - 16;
			SampleDescriptor[i].Finetune = k;			// 1
			SampleDescriptor[i].Volume = mp[25];			// 1
			SampleDescriptor[i].LoopStart = SWAP_WORD(mp,26);	// 2
			SampleDescriptor[i].LoopSize = SWAP_WORD(mp,28);	// 2
		}
		
		mp += (22 + 2 + 1 + 1 + 2 + 2);
		
		SampleDataSize += (SampleDescriptor[i].SampleSize * 2);


		printf("  %02d - %04X   | ",i,SampleDescriptor[i].SampleSize);
	}

	ModuleHeader.lptrSampleData += (8 * ModuleHeader.NumOfSmp);
}



/*---------------------------------------------------------------*/
void	GetPatternDATA(UBYTE *mp)
/*---------------------------------------------------------------*/
{
UBYTE	*dp = PackedPatternData; // ChP ptr of packed pattern data

	for (int pat = 0; pat < ModuleHeader.NumOfPat; pat++)
	{
		dp = PackRLE_CHNPatternDATA(mp+ 0, dp);	// Pack Chn 0
		dp = PackRLE_CHNPatternDATA(mp+ 4, dp);	// Pack Chn 1
		dp = PackRLE_CHNPatternDATA(mp+ 8, dp);	// Pack Chn 2
		dp = PackRLE_CHNPatternDATA(mp+12, dp);	// Pack Chn 3
		mp += (64 * 4 * 4);	// Size of a pattern
	}

	SrcSampleData = mp;
	ModuleHeader.lptrSampleData += PackedPatternLen;
}


/*---------------------------------------------------------------*/
UBYTE	Period2Tab(UWORD PeriodVal)
/*---------------------------------------------------------------*/
{
static UWORD freqtab[12*3] =
{
	 856,808,762,720,678,640,604,570,538,508,480,453
	,428,404,381,360,339,320,302,285,269,254,240,226
	,214,202,190,180,170,160,151,143,135,127,120,113
};

UBYTE	NNum = 0;

	for (int j = 0; j < 36; j++)
	{
		if (freqtab[j] == PeriodVal) 
		{ 
			NNum = (j+1) & 63; 
			break; 
		}
	}
	return NNum;
}

/*---------------------------------------------------------------*/
UBYTE*	PackRLE_CHNPatternDATA(UBYTE *src, UBYTE *dst)
/*---------------------------------------------------------------*/
{
struct	unpkNte
{
	UBYTE	SampleNum;
	UBYTE	NoteNum;
	UBYTE	EfxNum;
	UBYTE	EfxData;
};

unpkNte	NoteBuf, NoteAct;
UBYTE	RepVal = 0;
int		period;

	memset(&NoteBuf, 0, sizeof(unpkNte));
	
	
	for (int row = 0; row < 64; row++)
	{
		NoteAct.SampleNum = ((src[0] & 0xF0) | (src[2] >> 4)) & 31;
		NoteAct.EfxNum    = src[2] & 0x0F;
		NoteAct.EfxData   = src[3];

		period = ((src[0] & 0x0F) << 8) | src[1];
		NoteAct.NoteNum   = Period2Tab( period );
		
		if ((NoteAct.EfxNum == 0x00) && (NoteAct.EfxData != 0x00)) NoteAct.EfxNum = 0x08;
		
		
		src += (4*4);
		UnPackedPatternLen += 4;

		if (row > 0)
			if ( memcmp(&NoteAct, &NoteBuf, sizeof(unpkNte)) == 0 ) { RepVal++; continue; }


		if (RepVal)
		{
			*dst++ = 0x80 | (RepVal & 0x3F);
			PackedPatternLen++;
			RepVal = 0;
		}

		memcpy(&NoteBuf, &NoteAct, sizeof(unpkNte));

		dst[0] = ((NoteAct.NoteNum << 1) | (NoteAct.SampleNum >> 4)) & 0x7F;
		dst[1] = (NoteAct.SampleNum << 4) | NoteAct.EfxNum;
		dst[2] = NoteAct.EfxData;
		dst += 3;
		PackedPatternLen += 3;
	}

	if (RepVal)
	{
		*dst++ = 0x80 | (RepVal & 0x3F);
		PackedPatternLen++;
		RepVal = 0;
	}

	return dst;
}



//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------


int		ADPCM::LOGtab[16] = { 0, -127,-63,-31,-15,-7,-3,-1, 0, 1,3,7,15,31,63,127 };

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
int	min_index = 1;
int	min_val = abs(v - LOGtab[min_index]);

	for (int i = 2; i < 16; i++)
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
int	Delta1, Delta2;
int	LastVal = 0;

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
int	ADPCM::clip(int val)
/*---------------------------------------------------------------*/
{
	return (val < -127) ? -127 : ((val > 127) ? 127 : val);
}



/*---------------------------------------------------------------*/
int	ADPCM::packDATA2(SBYTE *dst, SBYTE *src, int nWord)
/*---------------------------------------------------------------*/
{
int		val;
int		Delta1, Delta2;
SBYTE	LastVal = 0;
SBYTE	RLE_last;
int		RLE_len;
int		RLE_block16;
int		packedsize = 0;

/*
		Delta1 = DIFtab[(*src++ - LastVal) & 0xFF];
		LastVal += LOGtab[Delta1 & 15];

		Delta2 = DIFtab[(*src++ - LastVal) & 0xFF];
		LastVal += LOGtab[Delta2 & 15];
*/

int		dif;
int		out;

		dif = (*src++ - LastVal) & 0xFF;
		out = LOGtab[ DIFtab[dif] ];
		if ((out > 0) && ((int)LastVal + out) >  127) dif--;
		if ((out < 0) && ((int)LastVal + out) < -127) dif++;
		Delta1 = DIFtab[dif];
		LastVal += LOGtab[Delta1 & 15];

		dif = (*src++ - LastVal) & 0xFF;
		out = LOGtab[ DIFtab[dif] ];
		if ((out > 0) && ((int)LastVal + out) >  127) dif--;
		if ((out < 0) && ((int)LastVal + out) < -127) dif++;
		Delta2 = DIFtab[dif];
		LastVal += LOGtab[Delta2 & 15];

	
	val = (SBYTE) ( ((Delta1 << 4) & 0xF0) | (Delta2 & 0x0F) ); nWord--;
	RLE_last = *dst++ = val; packedsize++;
	RLE_len = 0;
	RLE_block16 = 0;
	
	while (nWord > 0)
	{
		dif = (*src++ - LastVal) & 0xFF;
		out = LOGtab[ DIFtab[dif] ];
		if ((out > 0) && ((int)LastVal + out) >  127) dif--;
		if ((out < 0) && ((int)LastVal + out) < -127) dif++;
		Delta1 = DIFtab[dif];
		LastVal += LOGtab[Delta1 & 15];

		dif = (*src++ - LastVal) & 0xFF;
		out = LOGtab[ DIFtab[dif] ];
		if ((out > 0) && ((int)LastVal + out) >  127) dif--;
		if ((out < 0) && ((int)LastVal + out) < -127) dif++;
		Delta2 = DIFtab[dif];
		LastVal += LOGtab[Delta2 & 15];

		val = (SBYTE) ( ((Delta1 << 4) & 0xF0) | (Delta2 & 0x0F) ); nWord--;


		if (val == RLE_last) 
		{
			if (++RLE_len == 16) 
			{ 
				RLE_block16++;
				RLE_len = 0; 
			}
		}
		else
		{
			*dst++ = RLE_last; packedsize++;

			while (RLE_block16 > 0)
			{
				*dst++ = 0x0F;
				RLE_block16--;
					packedsize++;
			}

			if (RLE_len == 1) { *dst++ = RLE_last; packedsize++; }
			else if (RLE_len > 1)
			{
				*dst++ = ((RLE_len - 1) & 0x0F);
				packedsize++;
			}

			RLE_last = val;
			RLE_len = 0;

		}
		
	}

	if (RLE_len > 0)
	{
		*dst++ = ((RLE_len - 1) & 0x0F); packedsize++; 
		RLE_len = 0;
	}


	return packedsize++;
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
void	ADPCM::unpackDATA2(SBYTE *dp, SBYTE *sp, int nWord)
/*---------------------------------------------------------------*/
{
int LastRLE = 0;
SBYTE LastVal = 0;

	while (nWord > 0)
	{
		int actVal = *sp++ & 0xFF;

		if (actVal > 0x0F)
		{
			LastRLE = actVal;

			LastVal += LOGtab[(actVal >> 4) & 0x0F];
			*dp++ = (SBYTE) LastVal;
			
			LastVal += LOGtab[actVal & 0x0F];
			*dp++ = (SBYTE) LastVal;

			nWord--;
		}
		else
		{
			int todo = actVal + 1;
			nWord -= todo;

			int d1 = LOGtab[(LastRLE >> 4) & 0x0F];
			int d2 = LOGtab[(LastRLE     ) & 0x0F];

			while (todo--)
			{
				LastVal += d1;
				*dp++ = (SBYTE)LastVal;

				LastVal += d2;
				*dp++ = (SBYTE)LastVal;

			}
		}

	}
}






//********
//**. . .

/*---------------------------------------------------------------*/
int	pakGetSampleDATA(SBYTE *dp, SBYTE *sp)
/*---------------------------------------------------------------*/
{
int	i, paksize;
int	dataSize = 0;
SBYTE	tmp[1024];

	for (i = 1; i < 32; i++)
	{
		if (SampleDescriptor[i].SampleSize)		// SampleSize (is in words and not in bytes...)
		{
//			pkr.packDATA(dp, sp, paksize = SampleDescriptor[i].SampleSize);

			paksize = pkr.packDATA2(dp, sp, SampleDescriptor[i].SampleSize);

			if (i == 1)
			{
				FILE *fp;
				fp = fopen("c:\\ark1_a.raw","wb");
				fwrite(sp,2,SampleDescriptor[i].SampleSize,fp);
				fclose(fp);

				pkr.unpackDATA2(tmp, dp, SampleDescriptor[i].SampleSize);
//				pkr.unpackDATA(tmp, dp, SampleDescriptor[i].SampleSize);

				fp = fopen("c:\\ark1_b.raw","wb");
				fwrite(tmp,2,SampleDescriptor[i].SampleSize,fp);
				fclose(fp);

			}

			dp += paksize;	dataSize += paksize;

			sp += (SampleDescriptor[i].SampleSize * 2);
		}
	}
	printf("\n");
	
	return dataSize;
}

/*---------------------------------------------------------------*/
void	GetSampleDATA(void)
/*---------------------------------------------------------------*/
{
//	SampleDataSize = pakGetSampleDATA((SBYTE*)(DstSampleData + 4), (SBYTE*)SrcSampleData);
//	*((ULONG*)DstSampleData) = SampleDataSize;

	SampleDataSize = pakGetSampleDATA((SBYTE*)(DstSampleData), (SBYTE*)SrcSampleData);
}





/*---------------------------------------------------------------*/
void	writeAll(FILE *fp)
/*---------------------------------------------------------------*/
{
int	size;


		size = sizeof(struct TModuleHeader);
		fwrite( &ModuleHeader, size, 1,	fp);
		printf("Header:\t\t\t%04Xh\n",size);
		Writed += size;


//		fprintf(fp,"POS_");
		fwrite(Positions, 1, ModuleHeader.NumOfPos, fp);
		printf("Positions:\t\t%04Xh (Unpacked: 0080h - %d%%)\n"
			,ModuleHeader.NumOfPos
			,(ModuleHeader.NumOfPos * 100) / 0x80
			);
		Writed += ModuleHeader.NumOfPos;


//		fprintf(fp,"INS_");
		size = sizeof(struct TSampleDescriptor);
		fwrite( &SampleDescriptor[1], size, ModuleHeader.NumOfSmp,	fp);
		printf("SampleDescriptor:\t%04Xh (Unpacked: 03A2h - %d%%)\n"
			,(size * ModuleHeader.NumOfSmp)
			,((size * ModuleHeader.NumOfSmp) * 100) / 0x3A2
			);

		Writed += (size * ModuleHeader.NumOfSmp);


//		fprintf(fp,"PAT_");
		fwrite( PackedPatternData, 1, PackedPatternLen,	fp);
		printf("PackedPatternLen:\t%04Xh (Unpacked: %04Xh - %d%%)\n"
			,PackedPatternLen
			,UnPackedPatternLen
			,((PackedPatternLen * 100) / UnPackedPatternLen)
			);
		Writed += PackedPatternLen;


		fprintf(fp,"PKPKPKPK");
		fwrite(DstSampleData, 1, SampleDataSize, fp);

		printf("SampleDataSize:\t\t%04Xh (Unpacked: %04Xh - %d%%)\n"
			,SampleDataSize
			,unpSampleDataSize
			,(((4+SampleDataSize) * 100) / unpSampleDataSize)
			);

		Writed += (SampleDataSize);

}











/*---------------------------------------------------------------*/
int		main(int argc, char *argv[])
/*---------------------------------------------------------------*/
{

	
ULONG	MK_ID;


		printf("\nMOD2CHP Module Converter! (ChP! v1.3)\n");
		printf("Copyleft(C) 2oo2 - SiRioKD / Lockless\n");
		printf("usage: MOD2CHP <MODname> <CHPname>\n\n");


		if (argc != 3) return 1;
		if (!strcmp(argv[1],argv[2])) return 1;

		fp = fopen(argv[1],"rb"); if (fp == NULL) return 2;

		fseek(fp, 0, SEEK_END);	
		ModSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (ModSize == 0) {	fclose(fp);	return 3; }

		lpMod = (UBYTE *)malloc(ModSize);
		if (lpMod == NULL) { fclose(fp); return 4; }

		Readed = fread(lpMod, 1, ModSize, fp);
		fclose(fp);

		if (Readed != ModSize) { free(lpMod); return 5;	}
		
		// Check for "M.K." string (4chans mod)
		MK_ID = * ((ULONG*)(lpMod + 0x438));
		if (MK_ID != 0x2E4B2E4D) { free(lpMod); return 6; }

		printf("Processing M.K. module...\n\n",ModSize);
		
		initHeader();

//- 1 ------------------
		GetOrderDATA(lpMod);
//- 2 ------------------
		GetSampleINFO(lpMod + 20);
//- 3 ------------------
		PackedPatternData = (UBYTE *)malloc( ModuleHeader.NumOfPat * ((4 * 64) * 3) );
		if (PackedPatternData == NULL) { free(lpMod); return 4; }
		memset(PackedPatternData,0,ModuleHeader.NumOfPat * ((4 * 64) * 3));

		GetPatternDATA(lpMod + 1084);
//- 4 ------------------
		unpSampleDataSize = SampleDataSize;

		if ( (DstSampleData = (UBYTE *)malloc(SampleDataSize)) == NULL ) 
		{ 
			free(PackedPatternData); 
			free(lpMod); 
			return 4; 
		}
		memset(DstSampleData, 0, (SampleDataSize));

		GetSampleDATA();
//----------------------
		fp = fopen(argv[2],"wb"); if (fp == NULL) return 2;
		writeAll(fp);
//----------------------
//		Writed += 16;

		printf("\nChP! Module size:\t%d (Original: %d - %3d%%)\n\n"
			,Writed,ModSize,(Writed*100)/ModSize);


		fclose(fp);

		free(DstSampleData); 
		free(PackedPatternData); 
		free(lpMod);

		
		return 0;
}

