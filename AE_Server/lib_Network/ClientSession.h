#pragma once

#define WAIT_TIMEOUT_INTERVAL 5000


class CClientSession : public CWSAEventSelect
{
public:
	CClientSession();
	virtual ~CClientSession();

	static HANDLE m_hOperatingEventOfCS;

	bool m_IsConnection;

	bool StartCS( NetAddress& stConnectionAddr );
	bool EndCS( VOID );

	bool SendPacket( __in WORD wLength = 0, __in BYTE* pbyBuffer = NULL );

protected:
	bool ReceivePacket( __out BYTE* pbyReceivedPacket = NULL );

private:
	CPeer m_Session;
};
