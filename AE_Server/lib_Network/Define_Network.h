//------------------------------------------------------------------
//						Define_TSock.h
//------------------------------------------------------------------


#ifndef __DEFINE_TSOCK__H__
#define __DEFINE_TSOCK__H__


enum IO_TYPE
{
	IO_ACCEPT,
	IO_READ,
	IO_WRITE,
	IO_WORK
};

typedef struct _OVERLAPPED_EX
{
	OVERLAPPED Overlapped;
	IO_TYPE IOType;
	VOID* pParameter;
} OVERLAPPED_EX;

struct NetAddress
{
	union IPAddr
	{
		struct IPBytes {
			U8 b1,b2,b3,b4;
		} Bytes;
		unsigned int num;

		// Constructors
		IPAddr() {SetLocalhost();};
		IPAddr(U8 b1, U8 b2, U8 b3, U8 b4) {Bytes.b1=b1;Bytes.b2=b2;Bytes.b3=b3;Bytes.b4=b4;};
		IPAddr(unsigned int l) {num=l;};

		// Returns true is this ip-address refers to localhost (127.0.0.1)
		inline bool IsLocalhost() {return Bytes.b1==127&&Bytes.b2==0&&Bytes.b3==0&&Bytes.b4==1;};

		inline bool IsLoopback() {return Bytes.b1==127; }

		// Returns true is this ip-address refers to broadcasting address
		// (255.255.255.255)
		inline bool IsBroadcasting() {return Bytes.b1==255&&Bytes.b2==255&&Bytes.b3==255&&Bytes.b4==255;};
		inline void SetLocalhost() {Bytes.b1=127;Bytes.b2=0;Bytes.b3=0;Bytes.b4=1;};
		inline void SetBroadcast() {Bytes.b1=255;Bytes.b2=255;Bytes.b3=255;Bytes.b4=255;};

		// Operators
		bool operator ==(unsigned int ip)			{return num==ip;};
		bool operator ==(IPAddr ip)					{return num==ip.num;};
		bool operator !=(unsigned int ip)			{return num!=ip;};
		bool operator !=(IPAddr ip)					{return num!=ip.num;};
		IPAddr& operator =(const unsigned int ip)	{num=ip; return *this;};
		IPAddr& operator =(const IPAddr ip)			{num=ip.num; return *this;};
	};

	IPAddr				ip;
	unsigned short		port;
	char				str[20];

	inline void SetBinaryAddress(unsigned int binaryIP) { ip = binaryIP; };

	// Returns via s and return value dotted string representation of ip-address
	inline char* ToString(bool writePort=false)
	{
		char* s = str;
		s = AE_COMMON::UIntToStringSpec(ip.Bytes.b1, (char*)s);
		*s++ = '.';
		s = AE_COMMON::UIntToStringSpec(ip.Bytes.b2, (char*)s);
		*s++ = '.';
		s = AE_COMMON::UIntToStringSpec(ip.Bytes.b3, (char*)s);
		*s++ = '.';
		s = AE_COMMON::UIntToStringSpec(ip.Bytes.b4, (char*)s);

		if (writePort)
		{
			*s++ = ':';
			_itoa_s(-1, s+strlen(s), 10, port);
		}

		return str;
	};

	inline bool SetStringAddress(char* s)
	{
		ip.Bytes.b1=(unsigned char)atoi(s);
		s=strchr(s,'.');
		if (!s) {ip.SetLocalhost(); return false;}
		s++;
		ip.Bytes.b2=(unsigned char)atoi(s);
		s=strchr(s,'.');
		if (!s) {ip.SetLocalhost(); return false;}
		s++;
		ip.Bytes.b3=(unsigned char)atoi(s);
		s=strchr(s,'.');
		if (!s) {ip.SetLocalhost(); return false;}
		s++;
		ip.Bytes.b4=(unsigned char)atoi(s);

		return true;
	}

	NetAddress& operator = ( const NetAddress& input )
	{
		ip = input.ip;
		port = input.port;
		return *this;
	}


	bool operator==( const NetAddress& right ) const
	{
		return ip.num == right.ip.num && port == right.port;
	}

	bool operator!=( const NetAddress& right ) const
	{
		return ip.num != right.ip.num || port != right.port;
	}

	bool operator>( const NetAddress& right ) const
	{
		return ( (ip.num > right.ip.num ) || ( ( ip.num == right.ip.num ) && ( port > right.port ) ) );
	}

	bool operator<( const NetAddress& right ) const
	{
		return ( ( ip.num < right.ip.num ) || ( ( ip.num == right.ip.num ) && ( port < right.port ) ) );
	}

};


//typedef TUP::tuple< bool, SOCKET, NetAddress, int, sCHAR >		PERHANDLEDATA;

/*
typedef struct _PERHANDLEDATA
{
	SOCKET			mSocket;
	SOCKADDR_IN		mAddr;
} PERHANDLEDATA;
*/

#ifndef PACKET_BUFFER_SIZE
	#define PACKET_BUFFER_SIZE	(8192 * 3)
#endif 

#ifdef _SERVER_SIDE
#define MAX_QUEUE_LENGTH	50
#else
#define MAX_QUEUE_LENGTH	500
#endif


#endif 