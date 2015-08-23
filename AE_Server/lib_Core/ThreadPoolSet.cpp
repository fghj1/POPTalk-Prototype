// ThreadPool.cpp: implementation of the ThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#include "CorePCH.h"
#include "Ijob.h"
#include "threadpoolSet.h"
#include <algorithm>
#include "threadEx.h"



	void remove_job(IJob* p) { delete p; }
	void delete_thread(Thread* pthread) { 	delete pthread; }

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

	ThreadPoolSet::ThreadPoolSet()
	{
		Thread::get_threadcount(false, true);
		m_fstop = false;
		pthread_mutex_init(&m_cs, NULL);
		#ifndef PTHREAD_H
		m_hport = NULL;
		#endif
	}

	ThreadPoolSet::~ThreadPoolSet()
	{
		#ifndef PTHREAD_H
		if (m_hport)
			CloseHandle(m_hport);
		#else
		pthread_cond_destroy(&m_condition);
		#endif
		pthread_mutex_destroy(&m_cs);
	}

	void ThreadPoolSet::stop() 
	{
		m_fstop = true;
		pthread_mutex_lock(&m_cs);

		

		//std::for_each(m_jobs.begin(), m_jobs.end(), remove_job);
		m_jobs.erase(m_jobs.begin(), m_jobs.end());
		
		#ifndef PTHREAD_H
		for (int index  = 0; index < m_nthreadcount; ++index) 
		{
			::PostQueuedCompletionStatus(m_hport, 0, 0, 0);
		}
		#else
		pthread_cond_broadcast(&m_condition);
		#endif
		
		std::for_each(m_threads.begin(), m_threads.end(), delete_thread);
		
		m_threads.erase(m_threads.begin(), m_threads.end());

		#ifndef PTHREAD_H
			CloseHandle(m_hport);
			m_hport = NULL;
		#else
			pthread_cond_destroy(&m_condition);
		#endif

		
		pthread_mutex_unlock(&m_cs);
	}

	void ThreadPoolSet::add_thread(Thread *pthread) 
	{
		m_threads.push_back(pthread);
	}

	void ThreadPoolSet::set_threadcount(int nthreads) 
	{
		m_nthreadcount = nthreads;
		#ifndef PTHREAD_H
			m_hport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, nthreads);
		#else
			pthread_cond_init(&m_condition, NULL);
		#endif
		
		for (int index = 0; index < nthreads; ++index) 
		{
			Thread::create_thread();
		}
	}

