// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine" and the "irrXML" project.
// For conditions of distribution and use, see copyright notice in irrlicht.h and irrXML.h

#ifndef __BKALLOCATOR_H__
#define __BKALLOCATOR_H__

#include "AEType.h"
#include <new>
#include <memory.h>


#ifdef DEBUG_CLIENTBLOCK
#undef DEBUG_CLIENTBLOCK
#define DEBUG_CLIENTBLOCK new
#endif

//! Very simple allocator implementation, containers using it can be used across dll boundaries
template<typename T>
class TAllocator
{

public:
	//! Destructor
	virtual ~TAllocator() {}

	//! Allocate memory for an array of objects
	T* allocate(size_t cnt)
	{
		return (T*)internal_new(cnt* sizeof(T));
	}

	//! Deallocate memory for an array of objects
	void deallocate(T* ptr)
	{
		internal_delete(ptr);
	}

	//! Construct an element
	void construct(T* ptr, const T&e)
	{
		new ((void*)ptr) T(e);
	}

	//! Destruct an element
	void destruct(T* ptr)
	{
		ptr->~T();
	}

	

protected:
	virtual void* internal_new(size_t cnt)
	{
		return operator new(cnt);
	}

	virtual void internal_delete(void* ptr)
	{
		operator delete(ptr);
	}

};


//! Fast allocator, only to be used in containers inside the same memory heap.
/** Containers using it are NOT able to be used it across dll boundaries. Use this
when using in an internal class or function or when compiled into a static lib */
template<typename T>
class TAllocatorFast
{
public:

	//! Allocate memory for an array of objects
	T* allocate(size_t cnt)
	{
		return (T*)operator new(cnt* sizeof(T));
	}

	//! Deallocate memory for an array of objects
	void deallocate(T* ptr)
	{
		operator delete(ptr);
	}

	//! Construct an element
	void construct(T* ptr, const T&e)
	{
		new ((void*)ptr) T(e);
	}

	//! Destruct an element
	void destruct(T* ptr)
	{
		ptr->~T();
	}
};

// 메로리 캐싱 할당자
template<typename T = ULONG>
class TAllocaterEx 
{
	//ULONG m_nAllocationSize;
	ULONG m_nPoolMax;
	ULONG m_nCount;

	struct CNode 
	{
		CNode* m_pNext;
	};
	CNode* m_pHead;

	CNode* Pop()
	{
		assert(m_pHead && m_nCount);
		CNode* pHead = m_pHead;
		m_pHead = pHead->m_pNext;
		m_nCount--;
		return pHead;
	}
	void Push(CNode* pNode)
	{
		assert(pNode);
		pNode->m_pNext = m_pHead;
		m_pHead = pNode;
		m_nCount++;
	}

public:
	TAllocaterEx() : m_nPoolMax(0), m_nCount(0), m_pHead(NULL)
	{  }

	virtual ~TAllocaterEx()
	{
		Clear();
	}

	bool Init(ULONG nPoolSize)
	{
		// ensure our allocation size is at least size of a pointer
		Clear();
		return EnsurePoolMin(m_nPoolMax = nPoolSize);
	}

	bool EnsurePoolMin(ULONG nPoolSize)
	{
		while (m_nCount < nPoolSize)
		{
			int nSize = (sizeof(T)<sizeof(ULONG_PTR)) ? sizeof(ULONG_PTR):sizeof(T);
			CNode* pNode = (CNode*) new BYTE[nSize];
			if (!pNode)
				return false;
			Push(pNode);
		}
		return true;
	}
	void EnsurePoolMax(ULONG nPoolSize)
	{
		while (m_nCount > nPoolSize)
			delete[] (PBYTE) Pop();
	}

	void Clear()
	{
		EnsurePoolMax(0);
	}

	PVOID Alloc()
	{
		return EnsurePoolMin(1) ? (PVOID) Pop() : NULL;
	}
	void Free(PVOID pPtr)
	{
		Push((CNode*) pPtr);
		EnsurePoolMax(m_nPoolMax);
	}
};



#ifdef DEBUG_CLIENTBLOCK
	#undef DEBUG_CLIENTBLOCK
	#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif




#endif

