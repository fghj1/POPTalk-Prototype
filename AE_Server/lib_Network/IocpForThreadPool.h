#pragma once


class CIocpForThreadPool : private CIocp
{
	class COverlappedEXDestructor
	{
	public:
		COverlappedEXDestructor( LPOVERLAPPED pstOverlapped ) : m_pOverlappedEX( reinterpret_cast<OVERLAPPED_EX*>( pstOverlapped ) ) {}
		~COverlappedEXDestructor( VOID )
		{
			if( NULL != m_pOverlappedEX )
			{
				if( NULL != m_pOverlappedEX->pParameter )
				{
					TSPacket* pWorkPacket = static_cast<TSPacket*>( m_pOverlappedEX->pParameter );
					delete pWorkPacket;
					pWorkPacket = NULL;
				}

				delete m_pOverlappedEX;
				m_pOverlappedEX = NULL;
			}
		}

	private:
		OVERLAPPED_EX* m_pOverlappedEX;
	};

public:
	CIocpForThreadPool( VOID ) {}
	virtual ~CIocpForThreadPool( VOID ) {}

	bool RegisterSocketToIocp( SOCKET socket, ULONG_PTR completionKey );
	virtual BOOL PostQueuedCompletionStatus( DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped );

	virtual VOID WorkerThreadFunc( VOID );

protected:
	bool Begin( VOID );
	bool End( VOID );

	virtual VOID WorkProcessing( LPVOID pRequestor, LPVOID pWork ) = 0;

	virtual VOID StartShutdown( VOID ) = 0;
};
