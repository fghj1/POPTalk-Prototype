#include "NetworkPCH.h"
#include "./IOPacketDepository.h"


bool CPacketPool::BorrowFromMemoryPool( __in PVOID pObject, __in BYTE* pbyData, __in DWORD dwDataLength,
										__out PULONG_PTR pulOffset, __out BYTE** ppbyBorrowedAddr )
{
	if( pObject == NULL ||
		pbyData == NULL ||
		( dwDataLength == 0 || PACKET_BUFFER_SIZE <= dwDataLength ) )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] Parameter(%#p, %#p, %lu(0 < Length < %d)) is invalid." ),
			  pObject, pbyData, dwDataLength, PACKET_BUFFER_SIZE );
		return false;
	}

	CIOPDDataUnit* pUnitData = NULL;

	pUnitData = new CIOPDDataUnit();  // Memory pool에서 1차 빌림
	if( pUnitData == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as CIOPDDataUnit type." ) );
		return false;
	}

	pUnitData->Object = pObject;
	pUnitData->DataLength = dwDataLength;
	pUnitData->Data = new BYTE[dwDataLength];
	if( pUnitData->Data == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Failed to create the object as BYTE*%d type." ), dwDataLength );
		return false;
	}

	ZeroMemory( pUnitData->Data, dwDataLength );
	memcpy_s( pUnitData->Data, dwDataLength, pbyData, dwDataLength );

	*pulOffset = ( ULONG_PTR )pUnitData;
	*ppbyBorrowedAddr = pUnitData->Data;

	return true;
}

bool CPacketPool::ReturnToMemoryPool( __in ULONG_PTR ulOffsetFromMemoryPool )
{
	// 주의! 다중 Thread 동기화는 파생 Class에 Member 함수에서 한다.

	if( ulOffsetFromMemoryPool == 0 )
	{
		TLOG( LOG_DEBUG, _T( "[ERROR] Variable(%#p) is invalid." ), ulOffsetFromMemoryPool );
		return false;
	}

	CIOPDDataUnit* pUnitData = NULL;

	pUnitData = ( CIOPDDataUnit* )ulOffsetFromMemoryPool;
	if( pUnitData == NULL )
	{
		TLOG( LOG_DEBUG, _T( "[WARNING] Variable(%#p, %#p) is invalid." ), ulOffsetFromMemoryPool, pUnitData );
		return false;
	}

	SAFE_DELETE_ARRAY( pUnitData->Data );
	SAFE_DELETE( pUnitData );  // Memory pool에서 1차 빌림에 대한 반환

	return true;
}

CIOPacketDepository::CIOPacketDepository()
{
}

CIOPacketDepository::~CIOPacketDepository()
{
}

VOID CIOPacketDepository::InsertIntoMap( __in ULONG_PTR ulKey, __in INT iValue )
{
	SCOPED_LOCK( m_CS )
	{
		m_mapTransceivingIOPacket.SetAt( ulKey, iValue );
	}
}

VOID CIOPacketDepository::UpdateMap( __in CONST ULONG_PTR ulFindKey, __in INT iUpdateValue )
{
	mapIOProcessing::CPair* pElement = NULL;

	SCOPED_LOCK( m_CS )
	{
		pElement = m_mapTransceivingIOPacket.Lookup( ulFindKey );
		if( pElement == NULL )
		{
			TLOG( LOG_DEBUG, _T( "[WARNING] Because there is no matching key, failed to update the map." ) );
			return;
		}
		pElement->m_value = iUpdateValue;
	}
}

VOID CIOPacketDepository::RemoveFromMap( __in INT iValue, __in bool bBeErased/* = false*/ )
{
	POSITION CurtPos;
	mapIOProcessing::CPair* pElement = NULL;

	SCOPED_LOCK( m_CS )
	{
		if( bBeErased )  // 모두 제거
		{
			CurtPos = m_mapTransceivingIOPacket.GetStartPosition();
			while( CurtPos )
			{
				pElement = m_mapTransceivingIOPacket.GetNext( CurtPos );
				CPacketPool::ReturnToMemoryPool( pElement->m_key );
				m_mapTransceivingIOPacket.RemoveKey( pElement->m_key );
			}
		}
		else  // 조건부 제거
		{
			CurtPos = m_mapTransceivingIOPacket.GetStartPosition();
			while( CurtPos )
			{
				pElement = m_mapTransceivingIOPacket.GetNext( CurtPos );
				if( pElement->m_value == iValue )
				{
					CPacketPool::ReturnToMemoryPool( pElement->m_key );
					m_mapTransceivingIOPacket.RemoveKey( pElement->m_key );
				}
			}
		}
	}
}

VOID CIOPacketDepository::RemoveFromMap( __in ULONG_PTR ulKey )
{
	mapIOProcessing::CPair* pElement = NULL;

	SCOPED_LOCK( m_CS )
	{
		pElement = m_mapTransceivingIOPacket.Lookup( ulKey );
		if( pElement == NULL )
		{
			TLOG( LOG_DEBUG, _T( "[WARNING] Because there is no matching key, failed to delete the element in map." ) );
			return;
		}
		CPacketPool::ReturnToMemoryPool( pElement->m_key );
		m_mapTransceivingIOPacket.RemoveKey( pElement->m_key );
	}
}

VOID CIOPacketDepository::PushIntoQueue( __in ULONG_PTR ulElement )
{
	SCOPED_LOCK( m_CS )
	{
		m_queueWaitingToProcess.push( ulElement );
	}
}

bool CIOPacketDepository::GetFromQueue( __out PULONG_PTR pulOffset,
										__out VOID** ppObject,
										__out BYTE** ppbyData, __out DWORD* pdwDataLength )
{
	if( pulOffset == NULL || ppObject == NULL || ppbyData == NULL || pdwDataLength == NULL )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] Parameter(%#p, %#p, %#p, %#p) is invalid." ),
			  pulOffset, ppObject, ppbyData, pdwDataLength );
		return false;
	}

	SCOPED_LOCK( m_CS )
	{
		if( m_queueWaitingToProcess.empty() )
			return false;

		CIOPDDataUnit* pUnitData = NULL;
		*pulOffset = m_queueWaitingToProcess.front();

		pUnitData = ( CIOPDDataUnit* )( ( VOID* )( *pulOffset ) );
		if( pUnitData )
		{
			*ppObject = pUnitData->Object;
			*pdwDataLength = pUnitData->DataLength;
			*ppbyData = pUnitData->Data;
		}
		else
		{
			m_queueWaitingToProcess.pop();
			return false;
		}
	}

	return true;
}

bool CIOPacketDepository::GetElementAtMemoryPool( __in ULONG_PTR ulOffset,
												  __out VOID** ppObject,
												  __out BYTE** ppbyData, __out DWORD* pdwDataLength )
{
	if( ulOffset == 0 || ppObject == NULL || ppbyData == NULL || pdwDataLength == NULL )
	{
		TLOG( LOG_DEBUG,
			  _T( "[ERROR] Parameter(%#p, %#p, %#p, %#p) is invalid." ),
			  ulOffset, ppObject, ppbyData, pdwDataLength );
		return false;
	}

	CIOPDDataUnit* pUnitData = NULL;

	SCOPED_LOCK( m_CS )
	{
		pUnitData = ( CIOPDDataUnit* )( ( VOID* )ulOffset );
		if( pUnitData == NULL )
			return false;

		*ppObject = pUnitData->Object;
		*pdwDataLength = pUnitData->DataLength;
		*ppbyData = pUnitData->Data;
	}

	return true;
}

VOID CIOPacketDepository::PopFromQueue( VOID )
{
	SCOPED_LOCK( m_CS )
	{
		m_queueWaitingToProcess.pop();
	}
}

bool CIOPacketDepository::StartIOPD( VOID )
{
	// TODO: 필요한 처리 구현할 것
	return true;
}

VOID CIOPacketDepository::EndIOPD( VOID )
{
	// 주의! m_queueWaitingToProcess은 DWORD형 객체를 원소로 가지므로 별도 처리 없이 소멸자에 의존하며
	//		 m_mapTransceivingIOPacket은 Memory pool과 관계가 있으므로 직접 처리해야 한다.
	RemoveFromMap( 0, true );
}
