#pragma once

#include "./Define_HT.h"
#include "../lib_Common/CommonPCH.h"


template<class T>
class NumberHash
{
public:
	NumberHash( T val ) { m_val = ( U64 )val; }

	inline CONST U64 GetValue( VOID ) CONST
	{
		return m_val;
	}

private:
	U64 m_val;
};

// TODO: 스레드 동기화 코드가 들어가면서 성능이 떨어지진 않았는지 확인해야 한다.
template<class KEY, class VALUE, class HASH>
class TMap
{
	template<class KEY, class VALUE, class HASH>
	friend class MapIterator;

	typedef struct Element
	{
		KEY key;
		VALUE value;
		struct Element* pNext;
	};

	enum { MODULUS_EXPANSION = 4 };

public:
	TMap( __in U32 startSize = 64, __in U32 growingTrigger = 60 );
	~TMap();

	bool Add( __in CONST KEY& rKey, __in CONST VALUE& rValue );
	bool Remove( __in CONST KEY& rKey );
	bool Find( __in CONST KEY& rKey, __inout VALUE* pValue );
	inline CONST int Count( VOID ) CONST
	{
		return m_totalElement;
	}

private:
	TCriticalSection m_CS;
	U32 m_sizeBuckets;
	Element** m_ppBuckets;
	U32 m_growingTrigger;
	volatile U32 m_totalElement;

	VOID Grow( __in unsigned newSize );
};

template<class KEY, class VALUE, class HASH>
TMap<KEY, VALUE, HASH>::TMap( __in U32 startSize/* = 64*/, __in U32 growingTrigger/* = 60*/ ) : m_sizeBuckets( 0 ), m_ppBuckets( NULL ), m_growingTrigger( 0 )
{
	m_sizeBuckets = HASH_T( startSize, 1 );
	m_ppBuckets = new Element*[m_sizeBuckets];
	if( m_ppBuckets )
		memset( m_ppBuckets, 0, sizeof( Element* ) * m_sizeBuckets );
	m_growingTrigger = growingTrigger;
	m_totalElement = 0;
}

template<class KEY, class VALUE, class HASH>
TMap<KEY, VALUE, HASH>::~TMap()
{
	U32 index = 0;
	Element* pNext = NULL;

	SCOPED_LOCK( m_CS )
	{
		if( m_ppBuckets )
		{
			for( index = 0; index < m_sizeBuckets; ++index )
			{
				while( m_ppBuckets[index] )
				{
					pNext = ( m_ppBuckets[index] )->pNext;
					SAFE_DELETE( m_ppBuckets[index] );
					m_ppBuckets[index] = pNext;
				}
			}

			SAFE_DELETE_ARRAY( m_ppBuckets );
		}

		m_sizeBuckets = 0;
		m_growingTrigger = 0;
		m_totalElement = 0;
	}
}

template<class KEY, class VALUE, class HASH>
bool TMap<KEY, VALUE, HASH>::Add( __in CONST KEY& rKey, __in CONST VALUE& rValue )
{
	VALUE dummy;
	Element* pItem = NULL;

	SCOPED_LOCK( m_CS )
	{
		if( Find( rKey, &dummy ) )
			return false;

		if( ( ( m_totalElement + 1 ) * 100 / m_sizeBuckets ) > m_growingTrigger )
		{
			Grow( max( m_totalElement * MODULUS_EXPANSION, m_sizeBuckets * MODULUS_EXPANSION ) );
		}

		pItem = new Element;
		if( pItem == NULL )
			return false;
		pItem->key = rKey;
		pItem->value = rValue;
		HASH hash( rKey );
		U32 which = hash.GetValue() % m_sizeBuckets;
		pItem->pNext	 = m_ppBuckets[which];
		m_ppBuckets[which] = pItem;
		++m_totalElement;
	}

	return true;
}

template<class KEY, class VALUE, class HASH>
bool TMap<KEY, VALUE, HASH>::Remove( __in CONST KEY& rKey )
{
	SCOPED_LOCK( m_CS )
	{
		HASH hash( rKey );
		U64 which = hash.GetValue() % m_sizeBuckets;
		Element* pItem = m_ppBuckets[which];
		Element* pPrev = NULL;

		while( pItem )
		{
			if( pItem->key == rKey )
			{
				if( pPrev )
					pPrev->pNext = pItem->pNext;
				else
					m_ppBuckets[which] = pItem->pNext;

				SAFE_DELETE( pItem );
				--m_totalElement;

				return true;
			}

			pPrev = pItem;
			pItem = pItem->pNext;
		}
	}

	return false;
}

template<class KEY, class VALUE, class HASH>
bool TMap<KEY, VALUE, HASH>::Find( __in CONST KEY& rKey, __out VALUE* pValue )
{
	if( pValue == NULL )
		return false;

	SCOPED_LOCK( m_CS )
	{
		HASH hash( rKey );
		U32 which = hash.GetValue() % m_sizeBuckets;
		Element* pItem = m_ppBuckets[which];

		while( pItem )
		{
			if( pItem->key == rKey )
			{
				*pValue = pItem->value;
				return true;
			}

			pItem = pItem->pNext;
		}
	}

	return false;
}

template<class KEY, class VALUE, class HASH>
VOID TMap<KEY, VALUE, HASH>::Grow( __in unsigned newSize )
{
	U32 index = 0;
	Element* pNext = NULL;
	Element* pRoot = NULL;

	for( index = 0; index < m_sizeBuckets; ++index )
	{	
		while( m_ppBuckets[index] )
		{
			pNext = ( m_ppBuckets[index] )->pNext;
			( m_ppBuckets[index] )->pNext = pRoot;
			pRoot = m_ppBuckets[index];
			m_ppBuckets[index] = pNext;
		}
	}

	pNext = NULL;
	SAFE_DELETE( m_ppBuckets );

	m_sizeBuckets = HASH_T( newSize, 1 );
	m_ppBuckets = new Element*[m_sizeBuckets];
	if( m_ppBuckets )
		memset( m_ppBuckets, 0, sizeof( Element* ) * m_sizeBuckets );

	while( pRoot )
	{
		pNext = pRoot->pNext;

		HASH hash( pRoot->key );
		U32 which = hash.GetValue() % m_sizeBuckets;

		pRoot->pNext = m_ppBuckets[which];
		m_ppBuckets[which] = pRoot;
		pRoot = pNext;
	}
}

template<class KEY, class VALUE, class HASH>
class MapIterator
{
public:
	MapIterator( __in TMap<KEY, VALUE, HASH>& rMap );
	~MapIterator();

	VOID Begin( VOID );
	bool IsContinuous( VOID );
	VOID Next( VOID );

	VOID CurrentItem( __out KEY* pKey, __out VALUE* pValue );
	VALUE CurrentItemValue( VOID );

private:
	TCriticalSection* m_pCS;
	bool m_bUnlock;
	TMap<KEY, VALUE, HASH>* m_pOriginalMap;
	int m_curtBucketIndex;
	typename TMap<KEY, VALUE, HASH>::Element* m_pItem;

	VOID FindValid( VOID );
};

template<class KEY, class VALUE, class HASH>
MapIterator<KEY, VALUE, HASH>::MapIterator( __in TMap<KEY, VALUE, HASH>& rMap ) : m_pCS( NULL ), m_bUnlock( false ), m_pOriginalMap( NULL ), m_curtBucketIndex( -1 ), m_pItem( NULL )
{
	m_pOriginalMap = &rMap;
	m_pCS = &( m_pOriginalMap->m_CS );

	m_pCS->lock();
}

template<class KEY, class VALUE, class HASH>
MapIterator<KEY, VALUE, HASH>::~MapIterator()
{
	if( m_bUnlock == false )
		m_pCS->unlock();
}

template<class KEY, class VALUE, class HASH>
VOID MapIterator<KEY, VALUE, HASH>::Begin( VOID )
{
	m_curtBucketIndex = 0;
	m_pItem = m_pOriginalMap->m_ppBuckets[m_curtBucketIndex];

	FindValid();
}

template<class KEY, class VALUE, class HASH>
bool MapIterator<KEY, VALUE, HASH>::IsContinuous( VOID )
{
	if( 0 <= m_curtBucketIndex )
		return true;
	else
	{
		m_bUnlock = true;
		m_pCS->unlock();
		return false;
	}
}

template<class KEY, class VALUE, class HASH>
VOID MapIterator<KEY, VALUE, HASH>::Next( VOID )
{
	m_pItem = m_pItem->pNext;

	FindValid();
}

template<class KEY, class VALUE, class HASH>
VOID MapIterator<KEY, VALUE, HASH>::CurrentItem( __out KEY* pKey, __out VALUE* pValue )
{
	if( pKey == NULL || pValue == NULL )
		return;

	if( m_pItem )
	{
		*pKey = m_pItem->key;
		*pValue = m_pItem->value;
	}
}

template<class KEY, class VALUE, class HASH>
VALUE MapIterator<KEY, VALUE, HASH>::CurrentItemValue( VOID )
{
	if( m_pItem )
		return m_pItem->value;

	return ( VALUE )NULL;
}

template<class KEY, class VALUE, class HASH>
VOID MapIterator<KEY, VALUE, HASH>::FindValid( VOID )
{
	if( m_curtBucketIndex < 0 || ( m_pItem && m_curtBucketIndex >= 0 ) )
		return;

	for( ++m_curtBucketIndex; ( U32 )m_curtBucketIndex < m_pOriginalMap->m_sizeBuckets; ++m_curtBucketIndex )
	{
		if( m_pOriginalMap->m_ppBuckets[m_curtBucketIndex] )
		{
			m_pItem = m_pOriginalMap->m_ppBuckets[m_curtBucketIndex];
			return;
		}
	}

	m_curtBucketIndex = -1;
	m_pItem = 0;
}
