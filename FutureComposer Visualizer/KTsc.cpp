#include "KTSC.h"


/*---------------------------------------------*/
void	KTSC::SetTSC(void)
/*---------------------------------------------*/
{
__int64	val;

	__asm	
	{
		push	edi
		rdtsc
		lea		edi,val
		mov		[edi],eax
		mov		[edi+4],edx
		pop		edi
	}

	m_value = val;
}


/*---------------------------------------------*/
__int64		KTSC::GetTSC(void)
/*---------------------------------------------*/
{
__int64	val;

	__asm	
	{
		push	edi
		rdtsc
		lea		edi,val
		mov		[edi],eax
		mov		[edi+4],edx
		pop		edi
	}

	m_value = val - m_value;

	return m_value;
}


/*---------------------------------------------*/
float	KTSC::GetCPUms(void)
/*---------------------------------------------*/
{
	GetTSC();
	return float(double(m_value) / (KTSC_CPUSPEED * 1000));
}