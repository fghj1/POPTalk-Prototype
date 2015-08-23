// Thread.h: interface for the Thread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __THREAD__H__
#define __THREAD__H__



	

	template <class T>
	struct ThreadData
	{
	public:
		Thread*			thread;		///< Pointer to this object


		
		HANDLE hEvent;
		DWORD	threadId;		///< Id of the thread
		T* pThreadObject;

		typedef void (T::*TFunc)();
		TFunc	pThreadFunc;
		

		static DWORD _ThreadFunc(ThreadData<T>* pThis)
		{
			//copying data, because after SetEvent is called,
			//the caller thread could restart and delete the
			//local data
			ThreadData<T> td=*pThis;
			SetEvent(td.hEvent);
			((*(td.pThreadObject)).*(td.pThreadFunc))();

			return 0;
		}
	};

	template <class T>
	inline HANDLE CreateMemberThread(T* p, void (T::*func)())
	{
		ThreadData<T> td;
		td.pThreadObject=p;
		td.pThreadFunc=func;
		td.hEvent=CreateEvent(NULL,0,0,NULL);
		DWORD Dummy;   //To make win 9x Happy with the lpThreadId param
		HANDLE ThreadHandle=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ThreadData<T>::_ThreadFunc,&td,NULL,&Dummy);
		WaitForSingleObject(td.hEvent,INFINITE);
		::CloseHandle(td.hEvent);
		return ThreadHandle;
	}


	class Thread  
	{
	public:
		Thread();
		virtual ~Thread();
		typedef void (Thread::*tFUNC)();

	public:
		static long get_threadcount(bool finc, bool finit = false);
		static Thread* create_thread();
		template <class T>
		//Thread* CreateMemberThread(T* p,void (T::*func)());
		Thread* Create_MThread(T* p, tFUNC func);
	
	public:
		void _ThreadFunc();
		


	protected:

		ThreadData<Thread> td;
		unsigned int m_threadid;
	#ifdef PTHREAD_H
			pthread_t m_thread;
			pthread_mutex_t m_mutex;
	#endif
		bool m_fquit;
	};


	
	


#endif
