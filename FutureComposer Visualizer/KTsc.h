#ifndef KTSC_H
#define KTSC_H

#include <windows.h>


#define KTSC_CPUSPEED 450


/*-----------------------------------------------------------------*/
class	KTSC
/*-----------------------------------------------------------------*/
{
private:
		union
		{
                struct
                {
	                unsigned __int32 m_decimal;
        	        unsigned __int32 m_integer;
                };
        		signed __int64 m_value;
		};

public:
		void	SetTSC(void);
		__int64	GetTSC(void);
		float	GetCPUms(void);
};


#endif