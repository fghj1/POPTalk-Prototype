//--------------------------------------------------------------------
//
// ThreadPool.h: interface for the ThreadPool class.
//
//--------------------------------------------------------------------

#ifndef __THREADPOOL__H__
#define __THREADPOOL__H__

#include <vector>
#include <queue>
#include "../lib_Common/CommonPCH.h"
#include "./IJob.h"
#include "./define_macro.h"

//using namespace UTIL;
#pragma warning(disable:4786)	// debug information truncated to 255 characters
#pragma warning(disable:4503)	// decorated name length exceeded, name was truncated



	using namespace std;
	class Thread;


	class ThreadPoolSet : public TSingleton< ThreadPoolSet >
	{
	public:
		inline bool is_stop() { return m_fstop; }
		void		add_thread(Thread* pthread);
		inline long get_threadcount() {return m_nthreadcount;}
		
		inline long get_jobcount() 
		{
			pthread_mutex_lock(&m_cs);
			int count = static_cast<int>(m_jobs.size());
			pthread_mutex_unlock(&m_cs);
			return count;
		}

		void stop();
		void set_threadcount(int nthreads);

		inline void add_job(IJob* pjob) 
		{
			if (m_fstop) 
			{
				delete pjob;
				return;
			}
			pthread_mutex_lock(&m_cs);
			m_jobs.push_back(pjob);
			pthread_mutex_unlock(&m_cs);
			#ifndef PTHREAD_H
				::PostQueuedCompletionStatus(m_hport, 0, 0, NULL);
			#else
				pthread_cond_signal(&m_condition);
			#endif
		}

		#ifndef PTHREAD_H
			inline HANDLE get_object() { return m_hport; }
		#else
			inline pthread_cond_t* get_object() { return &m_condition; }
		#endif

		ThreadPoolSet();
		virtual ~ThreadPoolSet();
		
		inline IJob* get_next() 
		{
			pthread_mutex_lock(&m_cs);
			IJob* p = NULL;
			if (m_jobs.size() > 0) 
			{
				p = m_jobs.front();
				m_jobs.pop_front();
			}
			pthread_mutex_unlock(&m_cs);
			return p;
		}



	protected:
		#ifndef PTHREAD_H
			HANDLE m_hport;
		#else
			pthread_cond_t m_condition;
		#endif
		std::deque< IJob* > m_jobs;
		std::vector<Thread* > m_threads;
		pthread_mutex_t m_cs;
		int m_nthreadcount;
		bool m_fstop;
	};

	inline static ThreadPoolSet* WORKPOOL() 
	{
		return ThreadPoolSet::Instance();
	}

	



#endif 
