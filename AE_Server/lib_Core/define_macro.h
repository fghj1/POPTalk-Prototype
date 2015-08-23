//-------------------------------------------------------------------------------
//							Define_Macro.h
//-------------------------------------------------------------------------------



#ifndef __DEFINE_MACRO__H__
#define __DEFINE_MACRO__H__


#ifndef SAFE_DELETE
#define SAFE_DELETE(pointer)				if ( (pointer) != NULL ) { delete (pointer);	(pointer) = NULL; }
#endif 

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(pointer)			if ( (pointer) != NULL ) { delete [] (pointer);	(pointer) = NULL; }
#endif 


#ifndef SAFE_CLOSE_HANDLE
#define SAFE_CLOSE_HANDLE(handle)			if ( (handle) != NULL ) { ::CloseHandle(handle);	(handle) = NULL; }
#endif 



#ifndef PTHREAD_H
	#define pthread_mutex_t					CRITICAL_SECTION
	#define pthread_mutex_init(mutex, attr) InitializeCriticalSection(mutex)
	#define pthread_mutex_destroy(mutex)	DeleteCriticalSection(mutex)
	#define pthread_mutex_lock(mutex)		EnterCriticalSection(mutex)
	#define pthread_mutex_unlock(mutex)		LeaveCriticalSection(mutex)
#endif 



#endif 