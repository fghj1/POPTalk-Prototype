// Thread.cpp: implementation of the Thread class.
//
//////////////////////////////////////////////////////////////////////

#include "CorePCH.h"
#include "Ijob.h"
#include "ThreadPoolSet.h"
#include <algorithm>
#include "threadEx.h"



	
		unsigned int _stdcall __thread_proc(void *p) 
		{
			((Thread*)p)->_ThreadFunc();
			return 0;
		}
	
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

		

		
	Thread::Thread()
	{
		m_fquit = false;
		#ifndef PTHREAD_H
	
		//_beginthreadex(NULL, 0, __thread_proc, this, 0, (unsigned int *)&m_threadid);
		Create_MThread<Thread >(this, (tFUNC) &Thread::_ThreadFunc);
		
		#else
			m_threadid = pthread_create(&m_thread, NULL, __thread_proc, this);
			pthread_mutex_init(&m_mutex, NULL);
		#endif

	}

	Thread::~Thread()
	{
		while (!m_fquit) 
		{ 
			Sleep(10); 
		}
		#ifdef PTHREAD_H
			pthread_mutex_destroy(&m_mutex);
		#endif
	}

	void Thread::_ThreadFunc(void)
	{
		//HRESULT hResult;

		while (true) 
		{
			DWORD dwcount = 0;
			LPVOID pCompletionKey = NULL;

			OVERLAPPED* pol;
			if (!GetQueuedCompletionStatus( WORKPOOL()->get_object(), 
								&dwcount,
								reinterpret_cast<PULONG_PTR>( &pCompletionKey ),
								(OVERLAPPED**)&pol,
								INFINITE) ) 
			{
				DWORD err = GetLastError();
				break;
			}
		
			
			if (WORKPOOL()->is_stop() ) {
				break;
			}

			

			IJob* pjob = WORKPOOL()->get_next();
			if (pjob != NULL) 
			{
				if( !pjob->execute() )
					pjob->remove();
			}
			
			
			//Sleep(1);
			
		}
		m_fquit = true;
		
	}

	Thread* Thread::create_thread()
	{
		long threads = get_threadcount(false);
		if (threads < WORKPOOL()->get_threadcount()) 
		{
			Thread* pthread = new Thread;
			WORKPOOL()->add_thread(pthread);
			get_threadcount(true);
			return pthread;
		}
		return NULL;
	}

	long Thread::get_threadcount(bool finc, bool finit)
	{
		static long nthreads;
		if (finit)
			nthreads = 0;
		if (finc) {
			nthreads++;
		}
		return nthreads;
	}

	
	
	template <class T>
	Thread* Thread::Create_MThread(T* p, tFUNC func )
	{
		
		td.pThreadObject	=	p;
		td.pThreadFunc		=	func;
		td.hEvent	=	CreateEvent(NULL,0,0,NULL);
		
		HANDLE	ThreadHandle	=	CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ThreadData<T>::_ThreadFunc,&td,NULL,&td.threadId);
		WaitForSingleObject(td.hEvent,INFINITE);
		::CloseHandle(td.hEvent);
		return this;
	}


