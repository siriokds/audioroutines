#ifndef _RLDELTA_H
#define _RLDELTA_H


#include "Types.h"

class	RLDelta
{
static	SBYTE	LOGtab[16];

		int		GetDiff(SBYTE newval);
		
		SBYTE	oldval;
		int		clip(int val);
public:

		int		pack(SBYTE *dp, SBYTE *sp, int nWord);
		void	unpack(SBYTE *dp, SBYTE *sp, int nWord);
};


#endif
