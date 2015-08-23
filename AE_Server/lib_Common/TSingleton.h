//--------------------------------------------------------------------
//							TSingleton.h
//--------------------------------------------------------------------

#ifndef __TSINGLETON__H__
#define __TSINGLETON__H__

#include "AEType.h"
#include "AEMacro.h"



template <typename T> 
class TSingleton
{
public:

	static T* Instance()
	{
		if( m_instance == NULL )
		{
			m_instance = new T;

			ASSERT( m_instance != NULL );
			ScheduleForDestruction( TSingleton::DestroyInstance );
		}

		return m_instance;
	};
	
	static void DestroyInstance()
	{
		SAFE_DELETE(m_instance);
	};

protected:
	inline explicit TSingleton()
	{	
		ASSERT(TSingleton::m_instance == 0); 
		TSingleton::m_instance = static_cast<T*>(this); 
	};

	virtual ~TSingleton(){	};

	static void ScheduleForDestruction(void (*pFun)()) 
	{
		int iResult = std::atexit( pFun );
		if( iResult != 0 )
		{
			char szWarning[256] = {0,};
			sprintf_s( szWarning, 256, "[WARNING] Failed to call atexit() == %d\n", iResult );
			OutputDebugStringA( szWarning );
			printf_s( "%s", szWarning );
		}
	}


private:
	TSingleton(const TSingleton& source)	{	};
	static T* m_instance; //!< singleton class instance
};


template <typename T> T* TSingleton<T>::m_instance = NULL;




#endif 