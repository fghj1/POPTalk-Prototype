
#include "CorePCH.h"
#include "../lib_Common/CommonPCH.h"
#include ".\TMemoryPool.h"


	TFreeMemoryPool::TFreeMemoryPool( unsigned _objectSize, unsigned _blockSize )
	{
		chunkSize = _objectSize;
		if ( chunkSize < sizeof( Chunk ) )
			chunkSize = sizeof( Chunk );
		chunkSize = ( chunkSize + 3 ) & (~0x03 );

		blockSize = _blockSize;

		chunksPerBlock = ( blockSize - sizeof( BlockHeader ) ) / chunkSize;
		ASSERT( chunksPerBlock > 0 );

	#ifdef DEBUG
		numBlocks = 0;
		numChunks = 0;
	#endif

		rootBlock = 0;
		head = 0;
	}


	TFreeMemoryPool::~TFreeMemoryPool()
	{
		ASSERT( numChunks == 0 );

		Block* node = 0;

		while( rootBlock )
		{
			node = rootBlock->header.nextBlock;
			free( rootBlock );
			rootBlock = node;
		}
	}


	void* TFreeMemoryPool::Alloc()
	{
		void* ret = 0;

		if ( !head )
		{
			NewBlock();
			ASSERT( head );
		}

		ret = head;
		head = head->next;

	#ifdef DEBUG
		++numChunks;
		memset( ret, 0xaa, chunkSize );
	#endif

		return ret;
	}


	void TFreeMemoryPool::Free( void* mem )
	{
		Chunk* chunk = (Chunk*) mem;

	#ifdef DEBUG
		--numChunks;
		memset( mem, 0xbb, chunkSize );
	#endif

		chunk->next = 0;
		head = chunk;
	}


	void TFreeMemoryPool::NewBlock()
	{
		ASSERT( head == 0 );

		Block* block = (Block*) malloc( blockSize );
		ASSERT( block );

		block->header.nextBlock = rootBlock;
		rootBlock = block;

	#ifdef DEBUG
		memset( block, 0xcc, blockSize );
		++numChunks;
	#endif

		U32 i;
		for( i=0; i<chunksPerBlock-1; i++ )
		{
			block->chunk[i].next = &block->chunk[i+1];
		}
		block->chunk[ chunksPerBlock-1 ].next = 0;

		head = &block->chunk[0];
	}


	TLinearMemoryPool::TLinearMemoryPool( U32 totalMemory )
	{
		base = (char* ) malloc( totalMemory );
		ASSERT( base );

		current = base;
		end = base + totalMemory;

	#ifdef DEBUG
		memset( base, 0xaa, totalMemory );
	#endif
	}


	TLinearMemoryPool::~TLinearMemoryPool()
	{
		free( base );
	}


	void* TLinearMemoryPool::Alloc( U32 allocate )
	{
		if ( current < end )
		{
			char* ret = current;
			current += allocate;

			// Out of memory check.
			ASSERT( current <= end );
			if ( current <= end )
				return ret;
			else
				return 0;
		}
		else
		{
			// Out of memory!
			ASSERT( 0 );
			return 0;
		}
	}
