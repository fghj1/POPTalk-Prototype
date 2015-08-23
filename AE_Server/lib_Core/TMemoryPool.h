//----------------------------------------------------------
//						TMemoryPool.h
//----------------------------------------------------------



#ifndef __IMEMORY_POOL__
#define __IMEMORY_POOL__


//------------------------------------------------------------------
//	A memory pool that will dynamically grow as memory is needed.
//------------------------------------------------------------------

	#include <list>
	using std::list;

	class TFreeMemoryPool
	{
	public:
		TFreeMemoryPool( unsigned objectSize, unsigned blockSize = 4096 );
		~TFreeMemoryPool();

		void* Alloc();
		void Free( void* mem );

	private:
		struct Block;

		struct Chunk
		{
			Chunk* next;
		};

		struct BlockHeader
		{
			Block* nextBlock;	// pointer to the next block
		};

		struct Block
		{
			BlockHeader header;
			Chunk* chunk;		// treated as an array of chunks.
		};

		void NewBlock();

		unsigned chunkSize;
		unsigned blockSize;
		unsigned chunksPerBlock;

		unsigned numBlocks;
		unsigned numChunks;

		Block* rootBlock;
		Chunk* head;
	};

	//------------------------------------------------------------------
	//	A memory pool that has a fixed allocation.
	//	Essentially a cross between a linked list and an array.
	//------------------------------------------------------------------
	template <class T, INT nDefaultAllocation = 500>
	class TFixedMemoryPool
	{
	public:
		TFixedMemoryPool() { CreateElement( nDefaultAllocation ); }
		~TFixedMemoryPool()
		{
			while( false == m_listObjectPool.empty() )
			{
				delete ( m_listObjectPool.front() );
				m_listObjectPool.pop_front();
			}
		}

		T* AllocateElement( VOID )
		{
			T* ptElement = NULL;

			if( m_listObjectPool.empty() )
				CreateElement();

			if( m_listObjectPool.empty() )
				ptElement = NULL;
			else
			{
				do
				{
					ptElement = m_listObjectPool.front();
					m_listObjectPool.pop_front();
				}
				while( ( NULL == ptElement ) && ( false == m_listObjectPool.empty() ) );
			}

			if( ptElement )
				new ( ptElement ) T();

			return ptElement;
		}

		VOID ReturnElement( __in T* ptElement )
		{
			if( ptElement )
			{
				ptElement->~T();
				new ( ptElement ) T();

				m_listObjectPool.push_back( ptElement );
			}
		}

		size_t GetRemainder( VOID )
		{
			return m_listObjectPool.size();
		}

	private:
		std::list<T*> m_listObjectPool;

		VOID CreateElement( __in INT nSize = 1 )
		{
			T* ptElement = NULL;

			while( 0 < nSize )
			{
				ptElement = new T;
				if( ptElement )
					m_listObjectPool.push_back( ptElement );

				--nSize;
			}
		}
	};


	class TLinearMemoryPool
	{
	public:
		/// Construct, and pass in the amount of memory needed, in bytes.
		TLinearMemoryPool( unsigned totalMemory );

		~TLinearMemoryPool();

		/// Allocate a chunk of memory.
		void* Alloc( unsigned allocate );

		// Note this does nothing.
		void Free( void* mem )		{}

		/// Return true of this is out of memory.
		bool OutOfMemory()			{ return current == end; }

	public:
		char*	base;
		char*	current;
		char*	end;
	};

	
	

#endif
