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

	// Memory pool에서 공간을 빌려 인자 Data를 할당 후, Memory pool 시작 주소부터 할당된 공간까지 Offset 반환
	bool BorrowFromMemoryPool( __in PVOID pObject, __in BYTE* pbyData, __in DWORD dwDataLength,
							   __out PULONG_PTR pulOffset, __out BYTE** ppbyBorrowedAddr );

protected:
	TCriticalSection m_CS;

	// Memory pool에서 빌린 공간을 반환
	bool ReturnToMemoryPool( __in ULONG_PTR ulOffsetFromMemoryPool );
};

// 주의! CIOPacketDepository형 객체가 어떤 Thread에 의해서 처리가 되는지 항상 파악하고 있어야 한다.
//		 CIOPacketDepository형 객체는 Packet을 다루게 되어 다중 Thread로부터 접근되므로 Thread-Safety가 보장되야 한다.
class CIOPacketDepository : public CPacketPool
{
public:
	typedef CAtlMap<ULONG_PTR, INT> mapIOProcessing;
	typedef std::queue<ULONG_PTR> queueIOProcessing;

	CIOPacketDepository();
	~CIOPacketDepository();

	bool StartIOPD( VOID );
	VOID EndIOPD( VOID );

	// map에 원소 삽입
	VOID InsertIntoMap( __in ULONG_PTR ulKey, __in INT iValue );
	// map에서 원소를 찾아 새로운 값으로 갱신
	VOID UpdateMap( __in CONST ULONG_PTR ulFindKey, __in INT iUpdateValue );
	// map에서 해당 값을 가진 원소 제거
	VOID RemoveFromMap( __in INT iValue, __in bool bBeErased = false );
	// map에서 키에 해당하는 원소 제거
	VOID RemoveFromMap( __in ULONG_PTR ulKey );

	// queue에 원소 삽입
	VOID PushIntoQueue( __in ULONG_PTR ulElement );
	// queue를 통해서 원소 추출
	bool GetFromQueue( __out PULONG_PTR pulOffset,
					   __out VOID** ppObject,
					   __out BYTE** ppbyData, __out DWORD* pdwDataLength );
	// Memory pool로부터 Offset에 해당하는 원소 추출
	bool GetElementAtMemoryPool( __in ULONG_PTR ulOffset,
								 __out VOID** ppObject,
								 __out BYTE** ppbyData, __out DWORD* pdwDataLength );
	// Memory pool에서 빌린 공간을 반환
	inline bool ReturnToMemoryPool( __in ULONG_PTR ulOffsetFromMemoryPool ) { return CPacketPool::ReturnToMemoryPool( ulOffsetFromMemoryPool ); }
	// queue에 front에서 원소 제거
	VOID PopFromQueue( VOID );

	inline size_t GetSizemapTransceivingIOPacket( VOID ) { return m_mapTransceivingIOPacket.GetCount(); }

private:
	mapIOProcessing m_mapTransceivingIOPacket;
	queueIOProcessing m_queueWaitingToProcess;
};
