#pragma once


class CIocp
{
public:
	CIocp( VOID );
	virtual ~CIocp( VOID ) {}

	BOOL RegisterSocketToIocp( SOCKET socket, ULONG_PTR completionKey );
	BOOL PostQueuedCompletionStatus( DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped );

	virtual VOID WorkerThreadFunc( VOID ) = 0;

protected:
	HANDLE m_hStartupEvent;
	HANDLE m_hIOCP;

	bool Begin( VOID );
	BOOL End( VOID );

	virtual VOID StartShutdown( VOID ) = 0;

private:
	DWORD m_dwWorkerThreadCount;
	std::vector<HANDLE> m_vectWorkerThreadHandle;
};
