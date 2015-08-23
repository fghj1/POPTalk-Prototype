//----------------------------------------------------------------------------------------
//								AEThread.h
//----------------------------------------------------------------------------------------
#include "CommonPCH.h"
#include "AEThread.h"

// Custom messages
#define WM_REEXECUTE		(WM_USER + 100)
#define WM_EXECUTE			(WM_USER + 101)
//Add more messages here


WorkerThread::WorkerThread(bool autoQuit) : mThreadHandle(NULL), mReadyEvent(NULL), mThreadID(0),
		mParam(NULL), mThreadFunc(NULL), mOnCompleteFunc(NULL), mOnCompleteParam(NULL),
		mStatus(NotCreated), mAutoQuit(autoQuit), mMessageQueueCreated(false)
{
}

//
// Destructor
//
WorkerThread::~WorkerThread()
{
	CloseHandle(mThreadHandle);
}

//
// Start : to start the thread, this will keep waiting till mReadyEvent signaled by ThreadProc
//
bool WorkerThread::Start(LPTHREAD_START_ROUTINE theFunc, void *param)
{
	bool ret = false;
	mParam = param;
	mThreadFunc = theFunc;

	mReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	mThreadHandle = CreateThread(NULL, 0, ThreadProc, this, 0, &mThreadID);

	if (mThreadHandle != NULL) {
		mStatus = Created;
		ret = true;

		// Now wait for the thread's event loop to start.
		::WaitForSingleObject(mReadyEvent, INFINITE);
		::CloseHandle(mReadyEvent);
		mReadyEvent = NULL;
	}

	return ret;
}

//
// RegisterOnCompleteRoutine: set the OnComplete routine and its parameter value.
//
void WorkerThread::RegisterOnCompleteRoutine(LPTHREAD_START_ROUTINE theFunc, void *param)
{
	mOnCompleteFunc = theFunc;
	mOnCompleteParam = param;
}

//
// Join: cause calling thread to waint till worker thread complete its execution.
//
DWORD WorkerThread::Join(DWORD timeout) const
{
	return WaitForSingleObject(mThreadHandle, timeout);
}

//
// ThreadProc: static internal thread proc. this will call the user defined thread proc.
//				This function a thread message loop and this can be extened for user defined
//				messages.
//
DWORD WINAPI WorkerThread::ThreadProc(void *param)
{
	WorkerThread *pWorkerThread = (WorkerThread *)param;
	MSG msg = {0};
	DWORD ret = 0;

	// PeekMessage call just to force creation of a message queue.
	::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	::SetEvent(pWorkerThread->mReadyEvent);

	pWorkerThread->mMessageQueueCreated = true;

	pWorkerThread->mStatus = Started;
	ret = pWorkerThread->mThreadFunc(pWorkerThread->mParam);
	pWorkerThread->mStatus = Complete;

	if (pWorkerThread->mOnCompleteFunc != NULL) {
		pWorkerThread->mOnCompleteFunc(pWorkerThread->mOnCompleteParam);
	}

	if (pWorkerThread->mAutoQuit) {
		goto cleanup;
	}

	// Get the Task OR wait for the task.
	while (0 != GetMessage(&msg, NULL, 0, 0) ) // loop till WM_QUIT
	{
		switch(msg.message) {
			case WM_REEXECUTE:
				if (msg.wParam != NULL) {
					pWorkerThread->mParam = (void *)msg.wParam;
				}
				pWorkerThread->mStatus = Restarted;
				ret = pWorkerThread->mThreadFunc(pWorkerThread->mParam);
				pWorkerThread->mStatus = Complete;

				if (pWorkerThread->mOnCompleteFunc != NULL) {
					pWorkerThread->mOnCompleteFunc(pWorkerThread->mOnCompleteParam);
				}
				break;
			//
			// Add more messages here
			// case WM_YOURMSG:
			//
			default:
				break;
		} //switch
	} // while

cleanup:
	pWorkerThread->mThreadID = 0;
	return ret;
}

//
// ReExecute: To re execute the thread function with different data.
//			This can be used only if WorkerThread is created with autoQuit false.
//			This can overwrite the thread proc data. TODO: implement a queue.
//
//
bool WorkerThread::ReExecute(void *param)
{
	return PostMessage(WM_REEXECUTE, (WPARAM)param, 0);
}

//
// End: Send a WM_QUIT message to worker thread. This will end the thread only on the completion
// of current thread proc.
//
bool WorkerThread::End()
{
	return PostMessage(WM_QUIT, 0, 0);
}

//
// PostMessage: internal private function to post messages to worker thread.
//
bool WorkerThread::PostMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool ret = false;

	if (mMessageQueueCreated) {
		ret = PostThreadMessage(mThreadID, msg, wParam, lParam) ? true : false;
	}

	return ret;
}
