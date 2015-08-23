#pragma once


class CIocpForNetwork : private CIocp
{
public:
	CIocpForNetwork( VOID ) {}
	virtual ~CIocpForNetwork( VOID ) {}

	bool RegisterSocketToIocp( SOCKET socket, ULONG_PTR completionKey );
	VOID PostQueuedCompletionStatus( DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped );

	virtual VOID WorkerThreadFunc( VOID );

protected:
	bool Begin( VOID );
	bool End( VOID );

	virtual VOID OnIOConnected( VOID* object ) = 0;
	virtual VOID OnIODisconnected( VOID* object, bool bIsForcedDisconnection = false ) = 0;
	virtual VOID OnIORead( VOID* object, U32 dataLength ) = 0;
	virtual VOID OnIOWrote( VOID* object ) = 0;

	virtual VOID StartShutdown( VOID ) = 0;
};
