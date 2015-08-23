//-----------------------------------------------------------------------
//								AEWorker.h
//-----------------------------------------------------------------------



#ifndef __WORKER_THREAD__H__
#define __WORKER_THREAD__H__

#include <windows.h>

class WorkerThread
{
public:
	WorkerThread(bool autoQuit = true);
	~WorkerThread();

	bool Start(LPTHREAD_START_ROUTINE theFunc, void *param);
	void RegisterOnCompleteRoutine(LPTHREAD_START_ROUTINE theFunc, void *param);
	DWORD Join(DWORD timeout = INFINITE) const;
	bool ReExecute(void *param = NULL);
	bool End();

	enum Status {
		NotCreated,
		Created,
		Started,
		Restarted,
		Complete
	};

protected:
	static DWORD WINAPI ThreadProc(void *param);
	bool PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	LPTHREAD_START_ROUTINE mThreadFunc;
	void *mParam;
	LPTHREAD_START_ROUTINE mOnCompleteFunc;
	void *mOnCompleteParam;

	HANDLE mThreadHandle;
	HANDLE mReadyEvent;
	DWORD  mThreadID;
	Status mStatus;
public:
	bool mAutoQuit;
	bool mMessageQueueCreated;
};

#endif 