template<class T>
class CLockFreeQueue
{
	struct UnitNode;

#ifdef WIN64
	struct NodePointer
	{
		UnitNode* pNode;
		LONGLONG nCount;

		NodePointer() : pNode( NULL ), nCount( 0 ) {}
		NodePointer( UnitNode* pTempNode, CONST LONGLONG nTempCount ) : pNode( pTempNode ), nCount( nTempCount ) {}
		NodePointer( volatile CONST NodePointer& rTempP )
		{
			InterlockedExchange64( &nCount, rTempP.nCount );
			InterlockedExchangePointer( ( PVOID* )( &pNode ), rTempP.pNode );
		}
		~NodePointer() { Init(); }

		NodePointer& operator=( volatile CONST NodePointer& rTempP )
		{
			if( this != &rTempP )
			{
				InterlockedExchange64( &( this->nCount ), rTempP.nCount );
				InterlockedExchangePointer( ( PVOID* )( &( this->pNode ) ), rTempP.pNode );
			}

			return *this;
		}

		VOID Init( VOID )
		{
			pNode = NULL;
			nCount = 0;
		}
	};
#else
	struct NodePointer
	{
		UnitNode* pNode;
		LONG nCount;

		NodePointer() : pNode( NULL ), nCount( 0 ) {}
		NodePointer( UnitNode* pTempNode, CONST LONG nTempCount ) : pNode( pTempNode ), nCount( nTempCount ) {}
		NodePointer( volatile CONST NodePointer& rTempP )
		{
			InterlockedExchange( &nCount, rTempP.nCount );
			InterlockedExchangePointer( ( PVOID* )( &pNode ), rTempP.pNode );
		}
		~NodePointer() { Init(); }

		NodePointer& operator=( volatile CONST NodePointer& rTempP )
		{
			if( this != &rTempP )
			{
				InterlockedExchange( &( this->nCount ), rTempP.nCount );
				InterlockedExchangePointer( ( PVOID* )( &( this->pNode ) ), rTempP.pNode );
			}

			return *this;
		}

		VOID Init( VOID )
		{
			pNode = NULL;
			nCount = 0;
		}
	};
#endif

	struct UnitNode
	{
		T tValue;
		NodePointer NextP;
	};

	struct HazardPointer
	{
		INT iIsUsing;
		UnitNode* pHazardP;
		HazardPointer* pNextP;

		HazardPointer() : iIsUsing( 0 ), pHazardP( NULL ), pNextP( NULL ) {}
	};

	typedef typename std::vector<UnitNode*> vectDelWaitingPtrList;
	typedef typename vectDelWaitingPtrList::const_iterator vectitorDelWaitingPtrList;
	typedef typename std::map<DWORD, vectDelWaitingPtrList*> mapDWPListMgr;
	typedef typename mapDWPListMgr::const_iterator mapitorDWPListMgr;

public:
	CLockFreeQueue();
	~CLockFreeQueue();

	VOID Enqueue( __in CONST T& tPushItem );
	bool Dequeue( __out T& tPopItem );
#ifdef WIN64
	LONGLONG Size( VOID ) CONST;  // 주의! 결과값이 1자릿수 단위일 때, 부정확할 수 있다.
#else
	LONG Size( VOID ) CONST;  // 주의! 결과값이 1자릿수 단위일 때, 부정확할 수 있다.
#endif

private:
	volatile NodePointer m_HeadP;
	volatile NodePointer m_TailP;
	HazardPointer* m_pHPHead;
	mapDWPListMgr m_mapDWPListMgr;

	HazardPointer* AllocateHazardPointer( VOID );
	VOID DeallocateHazardPointer( __in HazardPointer* pHP );
	bool CompareAndSet( __inout volatile NodePointer& rDestination, __in NodePointer& rComparand, __in NodePointer& rExchange );
	vectDelWaitingPtrList* GetEachThdDelWaitingPtrList( __in DWORD dwThreadID );
	VOID UpdateEachThdDelWaitingPtrList( __in UnitNode* pUnusefulNode );
	VOID BatchDeleting( __in vectDelWaitingPtrList* pvtDWPList );
};

#include "./LockFreeQueue.inl"
