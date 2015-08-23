template<class T>
CLockFreeQueue<T>::CLockFreeQueue()
{
	//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  // TEST

	UnitNode* pNode = new UnitNode();
	if( NULL == pNode )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Failed to create the object as UnitNode type.\n" );
		OutputDebugStringA( szError );
		return;
	}

	m_HeadP.pNode = pNode;
	m_TailP.pNode = pNode;

	m_pHPHead = NULL;
}

template<class T>
CLockFreeQueue<T>::~CLockFreeQueue()
{
	mapitorDWPListMgr mapitor, mapitorStart, mapitorEnd;
	vectDelWaitingPtrList* pvtDWPList = NULL;
	vectitorDelWaitingPtrList vectitor, vectitorStart;
	UnitNode* pNode = NULL;
	HazardPointer* pHazardP = NULL;
	UnitNode* pNextNode = NULL;

	// Each thread deletion waiting pointer list 해제
	mapitorStart = m_mapDWPListMgr.begin();
	mapitorEnd = m_mapDWPListMgr.end();
	for( mapitor = mapitorStart; mapitor != mapitorEnd; ++mapitor )
	{
		pvtDWPList = mapitor->second;
		if( NULL != pvtDWPList )
		{
			vectitorStart = pvtDWPList->begin();
			for( vectitor = vectitorStart; vectitor != pvtDWPList->end(); )
			{
				pNode = *vectitor;

				vectitor = pvtDWPList->erase( vectitor );

				delete pNode;
				pNode = NULL;
			}
		}

		delete pvtDWPList;
	}
	m_mapDWPListMgr.clear();

	// Hazard pointer 해제
	for( pHazardP = m_pHPHead; NULL != pHazardP; pHazardP = m_pHPHead )
	{
		m_pHPHead = pHazardP->pNextP;
		delete pHazardP;
	}

	// Lock-free queue 원소 해제
	pNode = m_HeadP.pNode;
	while( NULL != pNode )
	{
		pNextNode = pNode->NextP.pNode;

		delete pNode;
		pNode = pNextNode;
	}

	//_CrtDumpMemoryLeaks();  // TEST
}

template<class T>
VOID CLockFreeQueue<T>::Enqueue( __in CONST T& tPushItem )
{
	UnitNode* pNewNode = NULL;
	bool bEnqueueNotDone = true;

	pNewNode = new UnitNode();
	if( NULL == pNewNode )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Failed to create the object as UnitNode type.\n" );
		OutputDebugStringA( szError );
		return;
	}

	pNewNode->tValue = tPushItem;

	HazardPointer* pHP_TailP = AllocateHazardPointer();
	HazardPointer* pHP_TNextP = AllocateHazardPointer();
	if( NULL == pHP_TailP || NULL == pHP_TNextP )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Failed to borrow the hazard pointer(%#p, %#p).\n", pHP_TailP, pHP_TNextP );
		OutputDebugStringA( szError );

		DeallocateHazardPointer( pHP_TNextP );
		DeallocateHazardPointer( pHP_TailP );
		delete pNewNode;
		return;
	}

	while( bEnqueueNotDone )
	{
		NodePointer TempTailP( m_TailP );
		pHP_TailP->pHazardP = TempTailP.pNode;
		NodePointer TempTNextP( TempTailP.pNode->NextP );
		pHP_TNextP->pHazardP = TempTNextP.pNode;

		if( ( TempTailP.nCount == m_TailP.nCount && TempTailP.pNode == m_TailP.pNode ) &&
			( TempTNextP.nCount == TempTailP.pNode->NextP.nCount && TempTNextP.pNode == TempTailP.pNode->NextP.pNode ) )
		{
			if( NULL == TempTNextP.pNode )
			{
				if( CompareAndSet( TempTailP.pNode->NextP, TempTNextP, NodePointer( pNewNode, ( TempTailP.nCount + 1 ) ) ) )
				{
					pNewNode = NULL;
					bEnqueueNotDone = false;
				}
			}
			else
				CompareAndSet( m_TailP, TempTailP, NodePointer( TempTNextP.pNode, ( TempTailP.nCount + 1 ) ) );
		}
	}

	DeallocateHazardPointer( pHP_TNextP );
	DeallocateHazardPointer( pHP_TailP );
}

template<class T>
bool CLockFreeQueue<T>::Dequeue( __out T& tPopItem )
{
	NodePointer TempHeadP;
	bool bDequeNotDone = true;

	HazardPointer* pHP_HeadP = AllocateHazardPointer();
	HazardPointer* pHP_TailP = AllocateHazardPointer();
	HazardPointer* pHP_HNextP = AllocateHazardPointer();
	if( NULL == pHP_HeadP || NULL == pHP_TailP || NULL == pHP_HNextP )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Failed to borrow the hazard pointer(%#p, %#p, %#p).\n", pHP_HeadP, pHP_TailP, pHP_HNextP );
		OutputDebugStringA( szError );

		DeallocateHazardPointer( pHP_HNextP );
		DeallocateHazardPointer( pHP_TailP );
		DeallocateHazardPointer( pHP_HeadP );
		return false;
	}

	while( bDequeNotDone )
	{
		TempHeadP.Init();
		TempHeadP = m_HeadP;
		pHP_HeadP->pHazardP = TempHeadP.pNode;
		NodePointer TempTailP( m_TailP );
		pHP_TailP->pHazardP = TempTailP.pNode;
		NodePointer TempHNextP( TempHeadP.pNode->NextP );
		pHP_HNextP->pHazardP = TempHNextP.pNode;

		if( ( TempHeadP.nCount == m_HeadP.nCount && TempHeadP.pNode == m_HeadP.pNode ) &&
			( TempTailP.nCount == m_TailP.nCount && TempTailP.pNode == m_TailP.pNode ) &&
			( TempHNextP.nCount == TempHeadP.pNode->NextP.nCount && TempHNextP.pNode == TempHeadP.pNode->NextP.pNode ) )
		{
			if( TempHeadP.pNode == TempTailP.pNode )
			{
				if( NULL == TempHNextP.pNode )
				{
#ifdef WIN64
					if( TempTailP.nCount == ( InterlockedCompareExchange64( &m_TailP.nCount, 0, TempTailP.nCount ) ) )
						InterlockedCompareExchange64( &m_HeadP.nCount, 0, TempHeadP.nCount );
#else
					if( TempTailP.nCount == ( InterlockedCompareExchange( &m_TailP.nCount, 0, TempTailP.nCount ) ) )
						InterlockedCompareExchange( &m_HeadP.nCount, 0, TempHeadP.nCount );
#endif

					DeallocateHazardPointer( pHP_HNextP );
					DeallocateHazardPointer( pHP_TailP );
					DeallocateHazardPointer( pHP_HeadP );
					return false;
				}

				CompareAndSet( m_TailP, TempTailP, NodePointer( TempHNextP.pNode, ( TempTailP.nCount + 1 ) ) );
			}
			else
			{
				tPopItem = TempHNextP.pNode->tValue;

				if( CompareAndSet( m_HeadP, TempHeadP, NodePointer( TempHNextP.pNode, ( TempHeadP.nCount + 1 ) ) ) )
					bDequeNotDone = false;
			}
		}
	}

	DeallocateHazardPointer( pHP_HNextP );
	DeallocateHazardPointer( pHP_TailP );
	DeallocateHazardPointer( pHP_HeadP );
	UpdateEachThdDelWaitingPtrList( TempHeadP.pNode );

	return true;
}

#ifdef WIN64
template<class T>
LONGLONG CLockFreeQueue<T>::Size( VOID ) CONST
{
	LONGLONG llTCount = 0, llHCount = 0;

	while( true )
	{
		NodePointer TempTailP( m_TailP );
		NodePointer TempTNextP( TempTailP.pNode->NextP );

		if( NULL == TempTNextP.pNode )
		{
			if( TempTNextP.pNode == m_TailP.pNode->NextP.pNode )
			{
				llTCount = TempTailP.nCount;
				break;
			}
		}
		else
		{
			if( TempTNextP.pNode ==  m_TailP.pNode->NextP.pNode )
			{
				llTCount = TempTNextP.nCount;
				break;
			}
		}

		TempTNextP.Init();
		TempTailP.Init();
	}
	InterlockedExchange64( &llHCount, m_HeadP.nCount );

	return ( llTCount - llHCount );
}
#else
template<class T>
LONG CLockFreeQueue<T>::Size( VOID ) CONST
{
	LONG lTCount = 0, lHCount = 0;

	while( true )
	{
		NodePointer TempTailP( m_TailP );
		NodePointer TempTNextP( TempTailP.pNode->NextP );

		if( NULL == TempTNextP.pNode )
		{
			if( TempTNextP.pNode == m_TailP.pNode->NextP.pNode )
			{
				lTCount = TempTailP.nCount;
				break;
			}
		}
		else
		{
			if( TempTNextP.pNode ==  m_TailP.pNode->NextP.pNode )
			{
				lTCount = TempTNextP.nCount;
				break;
			}
		}

		TempTNextP.Init();
		TempTailP.Init();
	}
	InterlockedExchange( &lHCount, m_HeadP.nCount );

	return ( lTCount - lHCount );
}
#endif

template<class T>
typename CLockFreeQueue<T>::HazardPointer* CLockFreeQueue<T>::AllocateHazardPointer( VOID )
{
	HazardPointer* pHazardP = NULL;

	for( pHazardP = m_pHPHead; NULL != pHazardP; pHazardP = pHazardP->pNextP )
	{
#ifdef WIN64
		if( ( 0 == pHazardP->iIsUsing ) &&
			( 0 == InterlockedCompareExchange64( ( LONGLONG volatile* )&( pHazardP->iIsUsing ), 1, 0 ) ) )
#else
		if( ( 0 == pHazardP->iIsUsing ) &&
			( 0 == InterlockedCompareExchange( ( LONG volatile* )&( pHazardP->iIsUsing ), 1, 0 ) ) )
#endif
			return pHazardP;
	}

	pHazardP = new HazardPointer();  // ~CLockFreeQueue()의 'Hazard pointer 해제'에서 해제
	if( NULL == pHazardP )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Failed to create the object as HazardPointer type.\n" );
		OutputDebugStringA( szError );
		return NULL;
	}
	pHazardP->iIsUsing = 1;

	HazardPointer* pPreHazardP = NULL;

	do
	{
		pPreHazardP = m_pHPHead;
		pHazardP->pNextP = pPreHazardP;
	}
	while( pPreHazardP != InterlockedCompareExchangePointer( ( PVOID volatile* )&m_pHPHead, pHazardP, pPreHazardP ) );

	return pHazardP;
}

template<class T>
VOID CLockFreeQueue<T>::DeallocateHazardPointer( __in HazardPointer* pHP )
{
	if( NULL != pHP && 1 == pHP->iIsUsing )
	{
		pHP->iIsUsing = 0;
		pHP->pHazardP = NULL;
	}
}

template<class T>
bool CLockFreeQueue<T>::CompareAndSet( __inout volatile NodePointer& rDestination, __in NodePointer& rComparand, __in NodePointer& rExchange )
{
	if( rComparand.pNode == InterlockedCompareExchangePointer( ( ( PVOID volatile* )&rDestination.pNode ),
															   rExchange.pNode,
															   rComparand.pNode ) )
	{
#ifdef WIN64
		InterlockedCompareExchange64( &rDestination.nCount, rExchange.nCount, rComparand.nCount );
#else
		InterlockedCompareExchange( &rDestination.nCount, rExchange.nCount, rComparand.nCount );
#endif
		return true;
	}

	return false;
}

template<class T>
typename CLockFreeQueue<T>::vectDelWaitingPtrList* CLockFreeQueue<T>::GetEachThdDelWaitingPtrList( __in DWORD dwThreadID )
{
	vectDelWaitingPtrList* pvtDWPList = NULL;

	mapitorDWPListMgr itor = m_mapDWPListMgr.find( dwThreadID );
	if( itor == m_mapDWPListMgr.end() )
	{
		pvtDWPList = new vectDelWaitingPtrList;  // ~CLockFreeQueue()의 'Each thread deletion waiting pointer list 해제'에서 해제
		if( NULL == pvtDWPList )
		{
			char szError[256] = {0,};
			sprintf_s( szError, 256, "[ERROR] Failed to create the object as vectDelWaitingPtrList type.\n" );
			OutputDebugStringA( szError );
			return NULL;
		}

		m_mapDWPListMgr.insert( mapDWPListMgr::value_type( dwThreadID, pvtDWPList ) );
	}
	else
		pvtDWPList = itor->second;

	return pvtDWPList;
}

template<class T>
VOID CLockFreeQueue<T>::UpdateEachThdDelWaitingPtrList( __in UnitNode* pUnusefulNode )
{
	if( pUnusefulNode == NULL )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Parameter(%#p) is invalid.\n", pUnusefulNode );
		OutputDebugStringA( szError );
		return;
	}

	DWORD dwThreadID = GetCurrentThreadId();
	vectDelWaitingPtrList* pvtDWPList = GetEachThdDelWaitingPtrList( dwThreadID );
	if( NULL == pvtDWPList )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Not found a list that matches a specified thread ID(%lu). Memory leak(%#p) may occur.\n", dwThreadID, pUnusefulNode );
		OutputDebugStringA( szError );
		return;
	}

	pvtDWPList->push_back( pUnusefulNode );

	if( 100 < pvtDWPList->size() )  // TODO: 적절한 한계치로 바꿔야 한다.
		BatchDeleting( pvtDWPList );
}

template<class T>
VOID CLockFreeQueue<T>::BatchDeleting( __in vectDelWaitingPtrList* pvtDWPList )
{
	if( pvtDWPList == NULL )
	{
		char szError[256] = {0,};
		sprintf_s( szError, 256, "[ERROR] Parameter(%#p) is invalid.\n", pvtDWPList );
		OutputDebugStringA( szError );
		return;
	}

	std::vector<UnitNode*> vtTempHPList;
	HazardPointer* pTempHP = NULL;
	vectDelWaitingPtrList::iterator itor, itorStart;
	UnitNode* pUnusefulNode = NULL;

	pTempHP = m_pHPHead;
	while( NULL != pTempHP )
	{
		if( 1 == pTempHP->iIsUsing )
			vtTempHPList.push_back( pTempHP->pHazardP );

		pTempHP = ( pTempHP->pNextP );
	}

	std::sort( vtTempHPList.begin(), vtTempHPList.end() );

	itorStart = pvtDWPList->begin();
	for( itor = itorStart; itor != pvtDWPList->end(); )
	{
		pUnusefulNode = *itor;
		if( false == std::binary_search( vtTempHPList.begin(), vtTempHPList.end(), pUnusefulNode ) )
		{
			itor = pvtDWPList->erase( itor );

			delete pUnusefulNode;
			pUnusefulNode = NULL;
		}
		else
			++itor;
	}
}
