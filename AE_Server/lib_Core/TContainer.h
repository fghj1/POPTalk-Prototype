//--------------------------------------------------------------------
//							TContainer.h
//--------------------------------------------------------------------

#ifndef __TCONTAINER__H__
#define __TCONTAINER__H__

#include <vector>

template < class T>
class TContainer
{
private:
	typedef std::vector<T> vectSlots;

	bool	m_bFull;
	bool	m_bCreated;		//	메모리 세팅 체크 
	TIndex  m_Index;		// Todo 이름바꾸기.
	TIndex	m_SlotLock;		// 슬롯 비활성화
	bool	m_locked;		// 컨테이너 잠금
	vectSlots m_vtSlots;

public:
	TContainer() 
	{
		m_bFull = false;
		m_bCreated = false;
		m_Index.ClearAllNumber();
		m_SlotLock.ClearAllNumber();
		m_locked = false;
	};		

	virtual ~TContainer()
	{
		clear();
	}

	bool create( int size, const T& defaultElement = T())
	{
		m_bCreated = true;
		m_vtSlots = std::vector<T>(size, defaultElement);

		return true;
	}

	void clear()
	{
		if(m_bCreated)
		{
			m_vtSlots.clear();
			m_bCreated = false;
			m_bFull = false;
			m_Index.ClearAllNumber();
			m_SlotLock.ClearAllNumber();
		}
	}

	T*  getItem(int slot)
	{
		if (isSet(slot))
			return &m_vtSlots[slot];
		else
			return nullptr;
	}

	bool set(T* pT)
	{
		if(isFull())
		{
			return false;
		}

		slotNo_t &targetSlot = pT->slotNo;

		if(isSet(targetSlot) || isSlotLocked(targetSlot))
		{
			// 락이 걸린 슬롯은 대상에서 제외한다.
			int nNewSlot = getAvailNumber();
			int size = m_vtSlots.size();
			if((nNewSlot == -1) || (nNewSlot >= size))
			{
				return false;
			}
			
			targetSlot = (slotNo_t)nNewSlot;
		}

		if( m_Index.AddNumber(targetSlot) == false )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to add a specified value(%d)." ), targetSlot );
			return false;
		}

		m_vtSlots[targetSlot] = (*pT);

		int size = m_vtSlots.size();
		if(m_Index.GetCount() >= size)
		{
			m_bFull = true;
		}

		return true;
	}

	void setLock(bool isLock = true)
	{
		m_locked = isLock;
	}
	
	bool isLocked()
	{
		return m_locked;
	}

	void setSlotLock(int slotNo, bool bLock = true)
	{
		if (bLock)
		{
			if( m_SlotLock.AddNumber(slotNo) == false )
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to add a specified value(%d)." ), slotNo );
		}
		else
		{
			if( m_SlotLock.ReleaseNumber(slotNo) == false )
				TLOG( LOG_DEBUG, _T( "[ERROR] Failed to release a specified value(%d)." ), slotNo );
		}
	}

	bool isSlotLocked(int slot)
	{
		return m_SlotLock.IsSet(slot);
	}

	bool isSet(int slot)	{ return m_Index.IsSet(slot); }
	bool isFull()
	{
		return m_bFull;
	}

	bool release(int slot)
	{
		// 락이 걸려있으면 해제도 하지 못한다.
		if (isSlotLocked(slot)) return false;
		if (m_bFull) { m_bFull = false; }

		if( m_Index.ReleaseNumber(slot) == false )
		{
			TLOG( LOG_DEBUG, _T( "[ERROR] Failed to release a specified value(%d)." ), slot );
			return false;
		}

		return true;
	}
	
	int getAvailNumber()
	{
		int start = m_Index.GetAvailNumber();
		int size = m_vtSlots.size();
		for (int i = start; i < size; ++i)
		{
			// 락이 걸린 슬롯은 대상에서 제외한다.
			if (!m_Index.IsSet(i) && !m_SlotLock.IsSet(i))
				return i;
		}
		
		return -1;
	}

	inline int getCount()
	{
		return m_Index.GetCount();
	}
};


#endif  // __TCONTAINER__H__
