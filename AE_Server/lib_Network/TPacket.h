//----------------------------------------------------------
//						TPack.h
//----------------------------------------------------------

#ifndef __TPACKET___H__
#define __TPACKET___H__

#include "./Define_Network.h"

#define		PACKET_SIZE			8192*3
#define		CLIENT_HEADER_SIZE	12
#define		SERVER_HEADER_SIZE	12

class SERVER_HEADER
{
public:
	U16*		nSize;
	U16*		_nCmd;
	U64*		_uID;

	SERVER_HEADER(){};
	U8 init(U8* pBuf)	
	{	
		nSize		= ( U16* )((pBuf)  );					//  packetSize size		= 2
		_nCmd		= ( U16* )((pBuf)  + 2);				//  protocolID size		= 2
		_uID		= ( U64* )((pBuf)  + 2 + 2);			//  User ID size	    = 8
		return SERVER_HEADER_SIZE;
	}

	void		protocol( U16 pro )	{	(*_nCmd) = pro;	}
	U16			protocol(void)		{	return *_nCmd;	}

	void		userID( U64 id )	{	(*_uID) = id;	}
	U64			userID(void)		{	return (*_uID);	}
	U16			userKey(void)							
	{	
		UID uid( (*_uID) ); 
		return uid.sInf.nMap;	
	}
};

//--------------------------------------
//		Client Packet Header
//--------------------------------------
class CLIENT_HEADER 
{

public:	
	U16*		nSize;
	U16*		_nCmd;
public:
	CLIENT_HEADER()	{	}
	
	U8 init(U8* pBuf)	
	{
		nSize		= ( U16* )((pBuf) + 0 );			//  packetSize size		= 2
		_nCmd		= ( U16* )((pBuf) + 2);				//  protocolID size		= 2
		return CLIENT_HEADER_SIZE;
	}

	void		protocol( U16 pro )	{	(*_nCmd) = pro;	}
	U16			protocol(void)		{	return *_nCmd;	}

	void		userID( U64 id )	{	}
	U64			userID(void)		{	return 0;	}
	U16			userKey(void)		{	return 0;	}		
};

template <class T>
class TPack
{	
	public:
		TPack(void);
		TPack( U8* source, int size );
		TPack( TPack<T>* source );
		virtual ~TPack(){};

		bool			isValidHeader();
		bool			isValidPacket();

		void			protocol( U16 pro );
		U16				protocol(void);

		void			userID( U64 ID );		// User ID
		U64				userID(void);
		U16				userKey(void);

		U16				getDataFieldSize() { return (Length()-headerSize ); };
		U16				getReceivedSize(){ return receivedSize; }

		void			clear( int bufferSize = PACKET_SIZE );
		U8*				getBuffer(){ return packetBuffer; }
		U8*				getData(){ return dataField; }

		void*			GetBuffer(int size)
		{
			if (readPosition + size > endOfDataField)
				return NULL;
			return readPosition;
		}

		// NOTE: C# 스크립트에서 패킷 보내기를 위해 추가한 함수 요주의!!
		void*			GetWritePosition()
		{
			return writePosition;
		}
		// NOTE: C# 스크립트에서 패킷 보내기를 위해 추가한 함수 요주의!!
		void			MoveWritePosition(int iOffset)
		{
			writePosition += iOffset;
			receivedSize += iOffset;
			*packetHeader.nSize += iOffset;
		}
		// NOTE: C# 스크립트에서 패킷 보내기를 위해 추가한 함수 요주의!!
		int				GetAvailableSpace()
		{
			return static_cast<int>(PACKET_SIZE -(writePosition -packetBuffer));
		}

		bool			MoveHead(int size)
		{
			if (readPosition +size > dataField +getDataFieldSize())
				return false;

			readPosition += size;
			return true;
		}

		void			rewind();
		void			reSetBody();

		U16				Length(){	return *packetHeader.nSize;	}
		void			copyToBuffer( U8* buff, int size );
		void			ReadBuffer( unsigned char* buffer );				// Size 포함 
		U8*			WriteBuffer( unsigned char* buffer, U16 size );		// Size 포함 
		template<class VALUE>
		void			Write(const VALUE& data)								// 데이타만 
		{
			//ASSERT( writePosition + size > endOfDataField );
			U16 size = sizeof(VALUE);
			::CopyMemory( writePosition, &data, size );
			writePosition += size;
			receivedSize += size;
			*packetHeader.nSize += size;
		}

		template<class VALUE>												// 데이타 만 
		void			Read( VALUE& data )
		{
			U16 size = sizeof(VALUE);
			::CopyMemory( buffer, readPosition, size );
			readPosition += size;
		}

		//TPack&	operator ;
		TPack<T>&	operator << ( bool arg );
		TPack<T>&	operator << ( char arg );
		TPack<T>&	operator << ( U8 arg );
		TPack<T>&	operator << ( S8 arg );
		TPack<T>&	operator << ( U16 arg );
		TPack<T>&	operator << ( S16 arg );
		TPack<T>&	operator << ( U32 arg );
		TPack<T>&	operator << ( S32 arg );
		TPack<T>&	operator << ( F32 arg );
		TPack<T>&	operator << ( U64 arg );
		TPack<T>&	operator << ( S64 arg );
		TPack<T>&	operator << ( F64 arg );
		TPack<T>&	operator << ( LPTSTR arg );
		TPack<T>&	operator << ( LPCTSTR arg );
		TPack<T>&	operator << ( sCHAR& arg );
		TPack<T>&	operator << ( std::tstring& arg );
		TPack<T>&	operator << ( TPack<T>& arg );
		

		TPack<T>&	operator >> ( bool& arg );
		TPack<T>&	operator >> ( char& arg );
		TPack<T>&	operator >> ( U8& arg );
		TPack<T>&	operator >> ( S8& arg );
		TPack<T>&	operator >> ( U16& arg );
		TPack<T>&	operator >> ( S16& arg );
		TPack<T>&	operator >> ( U32& arg );
		TPack<T>&	operator >> ( S32& arg );
		TPack<T>&	operator >> ( F32& arg );
		TPack<T>&	operator >> ( U64& arg );
		TPack<T>&	operator >> ( S64& arg );
		TPack<T>&	operator >> ( F64& arg );
		TPack<T>&	operator >> ( LPTSTR arg );
		TPack<T>&	operator >> ( LPCTSTR arg );
		TPack<T>&	operator >> ( sCHAR& arg );
		TPack<T>&	operator >> ( std::tstring& arg );
		TPack<T>&	operator >> ( TPack<T>& arg );

		template<class VALUE>
		TPack<T>& operator << ( const std::vector<VALUE>& _arr)
		{
			*this << (U16)_arr.size();
			if (_arr.size() > 0) 
				writeData( (void*)&_arr.front(), (U16)sizeof(VALUE)*_arr.size() );
			return *this;
		}

		template<class VALUE>
		TPack<T>& operator >> (std::vector<VALUE>& _arr)
		{
			U16 size = 0;
			*this >> size;
			if (size > 0)
			{
				_arr.resize(size);
				readData( &_arr.front(), (U16)sizeof(VALUE)*_arr.size() );
			}
			return *this;
		}

		bool		readData( void* buffer, U16 size );
		bool		writeData( void* buffer, U16 size );

		void CorrectIndex(int &index)
		{
			index += headerSize;
			if(index > Length())				index = Length();
		}

		void MoveItems(int destIndex, int srcIndex)
		{
			memmove(packetBuffer + destIndex, packetBuffer + srcIndex, 
				sizeof(char) * (Length() - srcIndex + 1));
		}

		void InsertSpace(int &index, int size)
		{
			CorrectIndex(index);
			MoveItems(index + size, index);
		}

		template<class VALUE> U16 Insert(int index, VALUE c)
		{
			InsertSpace(index, sizeof(VALUE));
			char* buf = (char*)packetBuffer;
			::CopyMemory( &buf[index], &c, sizeof(VALUE) );

			writePosition += sizeof(VALUE);
			receivedSize += sizeof(VALUE);
			*packetHeader.nSize += sizeof(VALUE);

			return U16(*packetHeader.nSize);
		}

		template<class VALUE> U16 Remove(int index, VALUE c)
		{
			CorrectIndex(index);
			MoveItems(index, index + sizeof(VALUE));
			writePosition -= sizeof(VALUE);
			receivedSize -= sizeof(VALUE);
			*packetHeader.nSize -= sizeof(VALUE);
			return U16(*packetHeader.nSize);
		}

	protected:
		T			packetHeader;
		U8			packetBuffer[PACKET_SIZE];
		U8*			dataField;
		U8*			readPosition;
		U8*			writePosition;
		U8*			endOfDataField;
		int			receivedSize;
		U8			headerSize;
};

#include "./TPackFunc.h"


typedef TPack< CLIENT_HEADER >		TCPacket;
typedef TPack< SERVER_HEADER >		TSPacket;

#endif 

