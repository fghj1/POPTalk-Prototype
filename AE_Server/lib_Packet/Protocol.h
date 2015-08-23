#pragma once

//typedef TPack< CLIENT_HEADER >		TCPacket;
//typedef TPack< SERVER_HEADER >		TSPacket;

class Protocol
{
public:
	virtual int GetTag() = 0;

	virtual bool Read(TSPacket* stream) { return true; }
	virtual bool Write(TSPacket* stream) { return true; }
	virtual bool Read(TCPacket* stream) { return true; }
	virtual bool Write(TCPacket* stream) { return true; }
};


// 영주 번호 (long,8byte)값, 영지번호 (long, 8byte) 을 가질 수 있는 프로토콜.
class DominionProtocol : public Protocol
{
	byte dataSetFlag;
	lordNo_t actorLordNo;
	villageNo_t opponentVillageNo;

public:
	DominionProtocol() : Protocol()
	{
		dataSetFlag = 0;
		actorLordNo = 0;
		opponentVillageNo = 0;
	}

	bool IsActorLordNoSet()
	{
		return (dataSetFlag & 0x01) != 0;
	}

	bool IsOpponentVillageNoSet()
	{
		return (dataSetFlag & 0x02) != 0;
	}

	lordNo_t GetActorLordNo()
	{
		if (IsActorLordNoSet())
			return actorLordNo;
		return 0;
	}

	void SetActorLordNo(lordNo_t lordNo)
	{
		this->actorLordNo = lordNo;
		dataSetFlag |= 0x01;
	}

	villageNo_t GetOpponentVillageNo()
	{
		if (IsOpponentVillageNoSet())
			return opponentVillageNo;
		return 0;
	}

	void SetOpponentVillageNo(villageNo_t villageNo)
	{
		this->opponentVillageNo = villageNo;
		dataSetFlag |= 0x02;
	}

	virtual bool Read(TSPacket* stream)
	{
		// read dataSetFlag
		if (!stream->readData(&dataSetFlag, sizeof(byte)))
			return false;

		if (IsActorLordNoSet())
		{
			// read actorLordNo
			if (!stream->readData(&actorLordNo, sizeof(lordNo_t)))
				return false;
		}
		if (IsOpponentVillageNoSet())
		{
			// read opponentVillageNo
			if (!stream->readData(&opponentVillageNo, sizeof(villageNo_t)))
				return false;
		}

		return true;
	}

	virtual bool Write(TSPacket* stream)
	{
		// write dataSetFlag
		if (!stream->writeData(&dataSetFlag, sizeof(byte)))
			return false;

		if (IsActorLordNoSet())
		{
			// write actorLordNo
			if (!stream->writeData(&actorLordNo, sizeof(lordNo_t)))
				return false;
		}

		if (IsOpponentVillageNoSet())
		{
			// write opponentVillageNo
			if (!stream->writeData(&opponentVillageNo, sizeof(villageNo_t)))
				return false;
		}

		return true;
	}

	virtual bool Read(TCPacket* stream)
	{
		// read dataSetFlag
		if (!stream->readData(&dataSetFlag, sizeof(byte)))
			return false;

		if (IsActorLordNoSet())
		{
			// read actorLordNo
			if (!stream->readData(&actorLordNo, sizeof(lordNo_t)))
				return false;
		}
		if (IsOpponentVillageNoSet())
		{
			// read opponentVillageNo
			if (!stream->readData(&opponentVillageNo, sizeof(villageNo_t)))
				return false;
		}

		return true;
	}

	virtual bool Write(TCPacket* stream)
	{
		// write dataSetFlag
		if (!stream->writeData(&dataSetFlag, sizeof(byte)))
			return false;

		if (IsActorLordNoSet())
		{
			// write actorLordNo
			if (!stream->writeData(&actorLordNo, sizeof(lordNo_t)))
				return false;
		}

		if (IsOpponentVillageNoSet())
		{
			// write opponentVillageNo
			if (!stream->writeData(&opponentVillageNo, sizeof(villageNo_t)))
				return false;
		}

		return true;
	}
};



class SequentialProtocol : public Protocol
{
public:
	int seqNo;

	virtual bool Read(TSPacket* stream)
	{
		// read seqNo
		if (!stream->readData(&seqNo, sizeof(int)))
			return false;

		return true;
	}
	virtual bool Write(TSPacket* stream)
	{
		// write seqNo
		if (!stream->writeData(&seqNo, sizeof(int)))
			return false;

		return true;
	}

	virtual bool Read(TCPacket* stream)
	{
		// read seqNo
		if (!stream->readData(&seqNo, sizeof(int)))
			return false;

		return true;
	}

	virtual bool Write(TCPacket* stream)
	{
		// write seqNo
		if (!stream->writeData(&seqNo, sizeof(int)))
			return false;

		return true;
	}
};

// Thursday, January 01, 1970 12:00:00 AM  
#define UnixEpoch ((unsigned __int64)(0x089f7ff5f7b58000))

// C# DateTime.Ticks를 전달받아 time_t(unix time)로 변환하거나, millisecond 단위로 변환하는 기능.
class DateTime
{
	__int64 m_ticks;

public:
	static time_t ConvertDateTimeTickToUnixTime(__int64 ticks)
	{
		return (time_t)((((unsigned __int64)ticks) -UnixEpoch)/10000000);
	}

	static __int64 ConvertUnixTimeToDateTimeTick(time_t unixTime)
	{
		return unixTime * 10000000 + UnixEpoch;
	}

	time_t GetAsUnixTime()
	{
		return ConvertDateTimeTickToUnixTime(m_ticks);
	}

	// 소수점 이하 반올림
	time_t GetAsUnixTimeWithRoundOff()
	{
		return ConvertDateTimeTickToUnixTime(m_ticks) +((GetInMillisecond() % 1000) >= 500);
	}

	__int64 GetInMillisecond()
	{
		return ((m_ticks & 0x3FFFFFFFFFFFFFFF) / 10000);
	}

	void SetUnixTime(time_t unixTime)
	{
		m_ticks = ConvertUnixTimeToDateTimeTick(unixTime);
	}

	DateTime()
	{
		m_ticks = 0;
	}

	bool Read(TSPacket* stream)
	{ 
		if (!stream->readData(&m_ticks, sizeof(m_ticks)))
			return false;

		return true; 
	}

	bool Write(TSPacket* stream) 
	{ 
		if (!stream->writeData(&m_ticks, sizeof(m_ticks)))
			return false;

		return true; 
	}

	bool Read(TCPacket* stream) 
	{ 
		if (!stream->readData(&m_ticks, sizeof(m_ticks)))
			return false;

		return true; 
	}

	bool Write(TCPacket* stream) 
	{ 
		if (!stream->writeData(&m_ticks, sizeof(m_ticks)))
			return false;

		return true; 
	}

	bool operator==(const DateTime& opponent) const
	{
		return m_ticks == opponent.m_ticks;
	}

	bool operator!=(const DateTime& opponent) const
	{
		return !(*this == opponent);
	}
};
