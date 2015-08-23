#include "CorePCH.h"
#include "Define_HT.h"

unsigned int HASH_T( unsigned close, int relationship )
{
	static unsigned count = 0;

	if( count==0 )
	{
		// Get the size of the table.
		while ( HASH_TABLE[count] ) 
			++count;
	}

	unsigned low = 0;
	unsigned high = count - 1;
	unsigned pivot = ( high + low ) >> 1;

	while ( high > low )
	{
		if ( close > HASH_TABLE[ pivot ] )
			low = pivot + 1;
		else
			high = pivot - 1;

		pivot = ( high + low ) >> 1;
	}
	//	ASSERT( low == high );

	if ( relationship < 0 )
	{
		while ( low > 0 && HASH_TABLE[ low ] > close )
			--low;
	}
	else if ( relationship > 0 )
	{
		while ( low < count && HASH_TABLE[ low ] < close )
			++low;
	}
	return HASH_TABLE[ low ];
}

