//-------------------------------------------------------------------------
// ThreadJob.h: interface for the ThreadJob class.
//-------------------------------------------------------------------------

#ifndef __IJOB__H__
#define __IJOB__H__


#include "../lib_Common/CommonPCH.h"

class reference_counted
{
    LONG m_count;
    inline void release() 
    { 
        if (0 == ::InterlockedDecrement(&m_count))
        {
            delete this; 
        }            
    }

    inline void add_ref() 
    { 
        ::InterlockedIncrement(&m_count); 
    }

    inline reference_counted()  : m_count(0) 
    {
    }

	inline virtual ~reference_counted() 
    {
    }
};
    
    
	class IJob  
	{
	public:
		IJob() 	{}
		virtual ~IJob() {}

		virtual bool execute()
		{
			delete this;
			return true;
		}
		
		virtual bool remove()
		{
			delete this;
			return true;
		}
	};



#endif 
