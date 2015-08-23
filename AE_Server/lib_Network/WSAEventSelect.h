#pragma once


class CWSAEventSelect
{
public:
	CWSAEventSelect();
	virtual ~CWSAEventSelect();

	bool WSAESBegin( SOCKET uni64Socket );
	VOID WSAESEnd( VOID );

	DWORD WSAESThreadCallback( LPCTSTR lpszThreadName );

protected:
	HANDLE m_hStartupEvent;
	HANDLE m_hSelectEvent;
	HANDLE m_hDestroyEvent;

	HANDLE m_hWSAESThread;

	SOCKET m_uni64WSAESSocket;

	virtual bool ReceivePacket( __out BYTE* pbyReceivedPacket = NULL ) = 0;
	virtual bool SendPacket( __in WORD wLength = 0, __in BYTE* pbyBuffer = NULL ) = 0;
	virtual VOID OnIOConnect( VOID ) = 0;
	virtual VOID OnIODisconnect( VOID ) = 0;
};
