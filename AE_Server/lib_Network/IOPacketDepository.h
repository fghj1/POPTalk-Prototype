#pragma once


class CIOPDDataUnit
{
public:
	BYTE* Data;
	DWORD DataLength;
	NetAddress RemoteAddr;
	DWORD Protocol;
	VOID* Object;

	CIOPDDataUnit() {}
	virtual ~CIOPDDataUnit() {}
};

class CPacketPool
{
public:
	CPacketPool() {}
	virtual ~CPacketPool() {}

	// Memory pool���� ������ ���� ���� Data�� �Ҵ� ��, Memory pool ���� �ּҺ��� �Ҵ�� �������� Offset ��ȯ
	bool BorrowFromMemoryPool( __in PVOID pObject, __in BYTE* pbyData, __in DWORD dwDataLength,
							   __out PULONG_PTR pulOffset, __out BYTE** ppbyBorrowedAddr );

protected:
	TCriticalSection m_CS;

	// Memory pool���� ���� ������ ��ȯ
	bool ReturnToMemoryPool( __in ULONG_PTR ulOffsetFromMemoryPool );
};

// ����! CIOPacketDepository�� ��ü�� � Thread�� ���ؼ� ó���� �Ǵ��� �׻� �ľ��ϰ� �־�� �Ѵ�.
//		 CIOPacketDepository�� ��ü�� Packet�� �ٷ�� �Ǿ� ���� Thread�κ��� ���ٵǹǷ� Thread-Safety�� ����Ǿ� �Ѵ�.
class CIOPacketDepository : public CPacketPool
{
public:
	typedef CAtlMap<ULONG_PTR, INT> mapIOProcessing;
	typedef std::queue<ULONG_PTR> queueIOProcessing;

	CIOPacketDepository();
	~CIOPacketDepository();

	bool StartIOPD( VOID );
	VOID EndIOPD( VOID );

	// map�� ���� ����
	VOID InsertIntoMap( __in ULONG_PTR ulKey, __in INT iValue );
	// map���� ���Ҹ� ã�� ���ο� ������ ����
	VOID UpdateMap( __in CONST ULONG_PTR ulFindKey, __in INT iUpdateValue );
	// map���� �ش� ���� ���� ���� ����
	VOID RemoveFromMap( __in INT iValue, __in bool bBeErased = false );
	// map���� Ű�� �ش��ϴ� ���� ����
	VOID RemoveFromMap( __in ULONG_PTR ulKey );

	// queue�� ���� ����
	VOID PushIntoQueue( __in ULONG_PTR ulElement );
	// queue�� ���ؼ� ���� ����
	bool GetFromQueue( __out PULONG_PTR pulOffset,
					   __out VOID** ppObject,
					   __out BYTE** ppbyData, __out DWORD* pdwDataLength );
	// Memory pool�κ��� Offset�� �ش��ϴ� ���� ����
	bool GetElementAtMemoryPool( __in ULONG_PTR ulOffset,
								 __out VOID** ppObject,
								 __out BYTE** ppbyData, __out DWORD* pdwDataLength );
	// Memory pool���� ���� ������ ��ȯ
	inline bool ReturnToMemoryPool( __in ULONG_PTR ulOffsetFromMemoryPool ) { return CPacketPool::ReturnToMemoryPool( ulOffsetFromMemoryPool ); }
	// queue�� front���� ���� ����
	VOID PopFromQueue( VOID );

	inline size_t GetSizemapTransceivingIOPacket( VOID ) { return m_mapTransceivingIOPacket.GetCount(); }

private:
	mapIOProcessing m_mapTransceivingIOPacket;
	queueIOProcessing m_queueWaitingToProcess;
};
