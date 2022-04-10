//---------------------------------------------------------------------------
#ifndef FCpH
#define FCpH
//---------------------------------------------------------------------------

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

#include <windows.h>
#include "Types.h"
#include "paula.h"

int		FCp_Open(void);
void	FCp_Close(void);
void	FCp_Start(void);
void	FCp_Stop(void);
int		FCp_InitModule(SBYTE *Module);
int		FCp_Update(void);






#define	CH00_ON	TRUE
#define	CH01_ON	TRUE
#define	CH02_ON	TRUE
#define	CH03_ON	TRUE

#define	SEQSTART 0
#define	SMP_BUFSIZE ((60*1024)+(80*256))	// 60Kb+20Kb = 80Kb

#define STR_ASMPREFIX chn
//#define STR_ASMPREFIX


typedef struct tKInstr
{
	ULONG	Start;
	UWORD	Len;
	UWORD	RepSta;
	UWORD	RepLen;
} KInstr;


typedef struct tKChan
{
	BOOL	ON;

    SWORD	PER1,PER2;

	SBYTE	*PatPtr;
    ULONG	PatOfs;

	SWORD	Period;
	SBYTE	Volume;
	SBYTE	NoteTR;
	SBYTE	SampTR;
	UBYTE	VoiceN;
	UBYTE	Note;
	SBYTE	Info;
	SBYTE	PortaV;
	SBYTE	PortaX;
	SBYTE	VolTspd;
	SBYTE	VolTcnt;
	SBYTE	VolSus;
	SBYTE	*VolPtr;
	ULONG	VolOfs;
	SBYTE	*FrqPtr;
	ULONG	FrqOfs;
	SBYTE	FrqSus;
	SBYTE	VibSpd;
	SBYTE	VibAmp;
	SBYTE	VibAmpC;
	SBYTE	VibDel;
	SBYTE	VibXOR;
	SBYTE	PBend;
	SBYTE	PBendC;
	SBYTE	PBendX;
	SBYTE	FreqTR;
	SBYTE	VBendS;
	SBYTE	VBendC;
	SBYTE	VBendX;
	SBYTE	SamplN;

} KChan;




/*
#ifdef __cplusplus
}
#endif
*/



#endif
