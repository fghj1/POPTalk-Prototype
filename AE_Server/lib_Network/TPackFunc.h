

template<class T>
TPack<T>::TPack(void) : dataField( 0 ), readPosition( 0 ), writePosition( 0 ), receivedSize( 0 )
{
	clear();
}

template<class T>
TPack<T>::TPack( U8* source, int size ) : dataField( 0 ), readPosition( 0 ), writePosition( 0 ), receivedSize( 0 )
{
	
	this->copyToBuffer( source, size );
}

template<class T>
TPack<T>::TPack( TPack<T>* source ) : dataField( 0 ), readPosition( 0 ), writePosition( 0 ), receivedSize( 0 )
{
	this->copyToBuffer( source->getBuffer(), source->Length() );
}

template<class T>
bool TPack<T>::isValidHeader()
{
	if( Length() >= headerSize )
		return true;

	return false;
}

template<class T>
bool TPack<T>::isValidPacket()
{
	if( isValidHeader() == false || receivedSize < headerSize || receivedSize < Length() )
		return false;
	return true;
}

template<class T>
void TPack<T>::clear( int bufferSize )
{
	::ZeroMemory( packetBuffer, bufferSize );
	headerSize = packetHeader.init(packetBuffer);

	dataField = &packetBuffer[headerSize];
	readPosition = writePosition = dataField;
	endOfDataField = &dataField[bufferSize - headerSize - 1];  // ( XXXX <= endOfDataField )비교를 위해 -1

	ASSERT( readPosition <= endOfDataField );

	protocol( 0 );
	receivedSize = 0;
	(*packetHeader.nSize) = headerSize;
	
}

template<class T>
void TPack<T>::rewind()
{
	readPosition = dataField;
	ASSERT( readPosition <= endOfDataField );
}

template<class T>
void TPack<T>::reSetBody()
{
	dataField = &packetBuffer[headerSize];
	readPosition = writePosition = dataField;
	endOfDataField = &dataField[PACKET_SIZE - headerSize - 1];  // ( XXXX <= endOfDataField )비교를 위해 -1


	(*packetHeader.nSize) = headerSize;
	receivedSize = 0;
	ASSERT( writePosition <= endOfDataField );
	ASSERT( readPosition <= endOfDataField );
}

template<class T>
void TPack<T>::protocol( U16 pro )
{
	packetHeader.protocol(pro);
}

template<class T>
U16 TPack<T>::protocol(void)
{
	return packetHeader.protocol();
}

template<class T>
void TPack<T>::userID( U64 id )
{
	packetHeader.userID(id);
}

template<class T>
U64 TPack<T>::userID(void)
{
	return packetHeader.userID();
}

template<class T>
U16 TPack<T>::userKey(void)
{
	return packetHeader.userKey();
}

template<class T>
void TPack<T>::copyToBuffer( U8* buff, int size )
{
	clear();
	::CopyMemory( packetBuffer + receivedSize, buff, size );
	receivedSize += size;
}

template<class T>
void TPack<T>::ReadBuffer( unsigned char* buffer )
{
	//ASSERT( readPosition + size > dataField + getDataFieldSize() || readPosition + size > endOfDataField );

	U16 size = 0;
	::CopyMemory( &size, readPosition, sizeof(U16));
	readPosition += sizeof(U16);

	::CopyMemory( buffer, readPosition, size );
	readPosition += size;
}

template<class T>
U8* TPack<T>::WriteBuffer( unsigned char* buffer, U16 size )
{
	U8* pWrittenPos = NULL;

	::CopyMemory( writePosition, &size, sizeof(U16));
	writePosition += sizeof(U16);
	receivedSize += sizeof(U16);
	*packetHeader.nSize += sizeof(U16);

	::CopyMemory( writePosition, buffer, size );
	pWrittenPos = writePosition;
	writePosition += size;
	receivedSize += size;
	*packetHeader.nSize += size;

	return pWrittenPos;
}


template<class T>
bool TPack<T>::readData( void* buffer, U16 size )
{
	if (readPosition + size > dataField +getDataFieldSize())
		return false;

	::CopyMemory( buffer, readPosition, size );
	readPosition += size;

	return true;
}

template<class T>
bool TPack<T>::writeData( void* buffer, U16 size )
{
	if (writePosition +size -packetBuffer >= PACKET_SIZE)
		return false;

	::CopyMemory( writePosition, buffer, size );
	writePosition += size;
	receivedSize += size;

	*packetHeader.nSize += size;

	return true;
}


template<class T>
TPack<T>& TPack<T>::operator << ( LPTSTR arg )
{
	U16 size = TByteSize( arg );
	writeData( &size,  sizeof( U16 ) );
	writeData( arg, size );

	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( LPTSTR arg )
{
	U16 size;
	readData( &size, sizeof( U16 ) );
	readData( arg, size );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( LPCTSTR arg )
{
	U16 size= TByteSize( arg );
	
	writeData( &size,  sizeof( U16 ) );
	writeData( (char*)arg, size );

	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( LPCTSTR arg )
{
	U16 size;
	readData( &size, sizeof( U16 ) );
	readData( (char*)arg, size );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( sCHAR& arg )
{
	U16 size = arg.bytesize();
	writeData( &size,  sizeof( U16 ) );
	writeData( arg.c_str(), size );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( std::tstring& arg )
{
	U16 size = 0;
	#ifdef _UNICODE
		size = (arg.size()<<1);
	#else
		size = arg.size();
	#endif
	
	writeData( &size,  sizeof( U16 ) );
	writeData( (void*)arg.c_str(), size );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( sCHAR& arg )
{
	TCHAR buffer[1024]=_T("");
	U16 size=0;
	readData( &size, sizeof( U16 ) );
	readData( buffer, size);
	arg = buffer;
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( std::tstring& arg )
{
	TCHAR buffer[1024]=_T("");
	U16 size=0;
	readData( &size, sizeof( U16 ) );
	readData( buffer, size);
	arg = buffer;
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( TPack& arg )
{
	CopyMemory(packetBuffer, arg.getBuffer(),  arg.Length());
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( TPack& arg )
{
	CopyMemory(arg.getBuffer(), packetBuffer, Length());
	return *this;
}

//--------------------------------------------------------
template<class T>
TPack<T>& TPack<T>::operator << ( bool arg )
{
	writeData( &arg, sizeof( bool ) );
	return *this;
}
template<class T>
TPack<T>& TPack<T>::operator >> ( bool& arg )
{
	readData( &arg, sizeof( bool ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( char arg )
{
	writeData( &arg, sizeof( char ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( char& arg )
{
	readData( &arg, sizeof( char ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( U8 arg )
{
	writeData( &arg, sizeof( U8 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( U8& arg )
{
	readData( &arg, sizeof( U8 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( S8 arg )
{
	writeData( &arg, sizeof( S8 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( S8& arg )
{
	readData( &arg, sizeof( S8 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( U16 arg )
{
	writeData( &arg, sizeof( U16 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( U16& arg )
{
	readData( &arg, sizeof( U16 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( S16 arg )
{
	writeData( &arg, sizeof( S16 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( S16& arg )
{
	readData( &arg, sizeof( S16 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( U32 arg )
{
	writeData( &arg, sizeof( U32 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( U32& arg )
{
	readData( &arg, sizeof( U32 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( S32 arg )
{
	writeData( &arg, sizeof( S32 ) );

	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( S32& arg )
{
	readData( &arg, sizeof( S32 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( F32 arg )
{
	writeData( &arg, sizeof( F32 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( F32& arg )
{
	readData( &arg, sizeof( F32 ) );

	return* this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( U64 arg )
{
	writeData( &arg, sizeof( U64 ) );

	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( U64& arg )
{
	readData( &arg, sizeof( U64 ) );

	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( S64 arg )
{
	writeData( &arg, sizeof( S64 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( S64& arg )
{
	readData( &arg, sizeof( S64 ) );
	return* this;
}

template<class T>
TPack<T>& TPack<T>::operator << ( F64 arg )
{
	writeData( &arg, sizeof( F64 ) );
	return *this;
}

template<class T>
TPack<T>& TPack<T>::operator >> ( F64& arg )
{
	readData( &arg, sizeof( F64 ) );
	return* this;
}
