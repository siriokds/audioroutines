#ifndef MOD2CHP_H
#define MOD2CHP_H

typedef signed char     SBYTE;
typedef unsigned char   UBYTE;
typedef signed short    SWORD;
typedef unsigned short  UWORD;
typedef signed long     SLONG;
typedef unsigned long   ULONG;


#define	HEADER_ID 		"ChP!\0"
#define HEADER_VER		0x13
#define HEADER_POSNUM	128

#define	SWAP_WORD(p,off) ((((SWORD)p[off+0]) << 8) | p[off+1])

#pragma pack(1)

struct TSampleDescriptor	// Size = 8
{
	UWORD	SampleSize;		// Sample Size / 2
	UBYTE	Finetune;		// Finetune (0 -> F)
	UBYTE	Volume;			// Volume (0 - 40h)
	UWORD	LoopStart;		// Loop Start / 2
	UWORD	LoopSize;		// Loop Size / 2
};

struct TModuleHeader		// Size = 16
{
	char	ID[5];			// ID = "CHiP\0"
	UBYTE	Version;		// Version
	UBYTE	Restart;		// NoiseTracker restart byte
	UBYTE	NumOfPat;		// Number Of Pattern
	ULONG	lptrSampleData; // Pointer to the sample data
	UBYTE	Unused1;		// Unused 1
	UBYTE	Unused2;		// Unused 2
	UBYTE	NumOfSmp;		// Number Of Samples
	UBYTE	NumOfPos;		// Number Of Positions
};

#pragma pack()

#endif
