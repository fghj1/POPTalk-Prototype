
// ------------------------------------------------------------------------
//								DelegateFactory.h
// ------------------------------------------------------------------------

#ifndef __DELEGATE_FACTORY__H__
#define __DELEGATE_FACTORY__H__


#include <map>
using namespace DELEGATE;



// Delegage ���� KeyNote
typedef union tagGateKey
{
	struct tagSta
	{
		U16		nPro;		// �������� 
		U8		sType;		// �۾� Ÿ�� 
		U8		sMain;		// ���� Ÿ�� 
	};

	struct tagSta		sCode;				
	U32					dwKey;	

	union tagGateKey(U8 nM, U8 nT, U16 nP)	
	{ 
		sCode.sMain = nM; 
		sCode.sType = nT; 
		sCode.nPro	= nP; 
	}

	bool operator==(const U32 key)
	{
		if (dwKey == key)		return true;
		else					return false;
	}

	U32& operator = ( union tagGateKey gK )
	{
		return dwKey;
	}

} GateKey;		


typedef FastDelegate0<void>								msgDelegate;
typedef FastDelegate3< void*, void*, void*, int >		tPacketHandler;
//typedef FastDelegate3< void*, void*, void*, int >		workDelegate;


// Chamsol:RemoveVoidPointer : ��Ŷ �μ� Ż void*ȭ
// serverDelegate�� �ٸ� �������� �ý���(?)������ ����ϰ� �ֱ⿡, Ŭ���̾�Ʈ ��Ŷ �κи��� ��� serverDelegate_GS�� ����
// serverDelegate_GS�� �μ��� �ϳ�(ActorProcArgument)�� ���� �Լ��� delegate
#include "../lib_Core/ActorPacketArgument.h"
class TSession;
typedef FastDelegate1<ActorProcArgument, int>				serverDelegate_GS;	// serverDelegate_GameServer
typedef FastDelegate2<TSession *, PacketContainer, int>		serverDelegate_DS;	// serverDelegate_DataServer


template <typename _Key, typename _Base, typename _Predicator = std::less<_Key> >
class DelegateFactory 	:	public TSingleton< DelegateFactory< typename _Key, typename _Base, typename _Predicator > >//, 
							
{

	friend TSingleton<DelegateFactory>;   
	typedef TMap< _Key, _Base, NumberHash<_Key> > _mapFactory;
public:
	DelegateFactory() 
	{
		pProcess = new _mapFactory(0);

	};
	~DelegateFactory() 
	{
		SAFE_DELETE(pProcess); 
	};

	_Key RegisterFunction(_Key idKey, _Base&  classCreator)
	{
		
		m_sec.lock();
		pProcess->Add(idKey, classCreator);
		m_sec.unlock();
		return idKey;
	}

	_Base& GetInstance(_Key idKey, _Base& Base)
	{
		
		m_sec.lock();
		pProcess->Find(idKey, &Base);
		m_sec.unlock();
		return Base;
	}

private:
	TCriticalSection	m_sec;
	_mapFactory*		pProcess;

};

namespace SINGLETON
{
	inline static DelegateFactory< U32, tPacketHandler, int >* G_factory()
	{
		return DelegateFactory< U32, tPacketHandler, int >::Instance();
	}
}
using namespace SINGLETON;



#endif // !defined(AFX_CLASSFACTORY_H__CA6330BC_AF6C_460C_AED0_0E76E9D96D55__INCLUDED_)
