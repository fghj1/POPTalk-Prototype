//------------------------------------------------------
//						TAutoPtr.h
//------------------------------------------------------

#ifndef __TAUTOPTR__H__
#define __TAUTOPTR__H__


#include <vector>
#include <algorithm>
#include <iostream>

#include "./TSingleton.h"
#include "./TCS.h"
#include "./AEUtil.h"




class TAuto_Manager :  public TSingleton<TAuto_Manager>
{
	friend TSingleton<TAuto_Manager>;

private:
	enum { STRUCTS_PER_BLOCK = 2048 };
	typedef unsigned char BYTE;

	TCriticalSection m_CS;

	size_t		m_size;
	size_t		m_block_size;
	BYTE*		m_block;
	int			m_left;
	void*		m_first;
	//CRITICAL_SECTION m_gate_keeper;

	typedef std::vector<BYTE *> MY_VEC;
	MY_VEC m_vec;

public:
	TAuto_Manager(size_t s=sizeof(long)) : m_left(0), m_size(s), m_first(0), m_block_size(STRUCTS_PER_BLOCK * s) 
	{ 
		//InitializeCriticalSection(&m_gate_keeper);
	}

	~TAuto_Manager() 
	{
		SCOPED_LOCK( m_CS )
		{
			std::for_each(m_vec.begin(), m_vec.end(), TAUTO_MEM_RELEASET());
		}
	}

	void add_block() 
	{  
		SCOPED_LOCK( m_CS )
		{
			BYTE *p = (BYTE *) ::malloc(m_block_size);
			if (!p) 
				throw std::bad_alloc();

			m_vec.push_back(p);
			m_block = p;
			m_left = (int)m_block_size;
			m_block_size = (m_block_size * 3) / 2;  //Expand by a 1.5 factor
		}
	}

	void *malloc() 
	{
		void *p = 0;

		SCOPED_LOCK( m_CS )
		{
			if(m_first) 
			{         
				p = m_first; //Extract top-most buffer
				m_first = *(static_cast<void**>(m_first)); // Advance to next element
			}
			else 
			{
				if(!m_left)
					add_block();

				p = m_block;

				m_block += m_size;
				m_left -= (int)m_size;
			}
		}

		return p;
	}

	void free(void *p) 
	{  
		SCOPED_LOCK( m_CS )
		{
			if (!p)		return;

			//Place the buffer (p) on the free list:
			void** temp = static_cast<void**>(p);
			*temp = m_first;  //Place a pointer to the current top
			m_first = temp;   //Update top of list pointer
		}
	}

}; //TAuto_Manager

inline TAuto_Manager* AUTOMANAGER()
{
	return TAuto_Manager::Instance();
}

//extern TAuto_Manager	g_auto_pool;




//TAuto_Ptr Class defintion
template <typename T>
class TAuto_Ptr 
{
private:
	long*	m_ref_p;
	T*		m_inst_p;

public:
	//>x
	TAuto_Ptr(T *inst_p = 0) : m_inst_p(inst_p) 
	{  
		m_ref_p = static_cast<long*>(AUTOMANAGER()->malloc());
		*m_ref_p = 1;
	}

	TAuto_Ptr(const TAuto_Ptr<T> &other) : m_ref_p(other.m_ref_p), m_inst_p(other.m_inst_p) 
	{ 
		InterlockedIncrement(m_ref_p);
	}

	~TAuto_Ptr() 
	{
		if (!InterlockedDecrement(m_ref_p)) 
		{
			delete m_inst_p;
			AUTOMANAGER()->free(m_ref_p);
		}
	}

	TAuto_Ptr &operator=(const TAuto_Ptr<T> &rhs) 
	{  
		TAuto_Ptr<T> temp(rhs);
		swap(temp);

		return *this;
	}

	T &operator*() const {   return *m_inst_p;		}
	T *operator->() const {  return m_inst_p;		}
	operator bool() const {   return m_inst_p != 0;	}
	T *get() const {   		return m_inst_p;		}

	void swap(TAuto_Ptr<T> &other) { 
		std::swap(m_ref_p, other.m_ref_p);
		std::swap(m_inst_p, other.m_inst_p);
	}

};//TAuto_Ptr





#endif 