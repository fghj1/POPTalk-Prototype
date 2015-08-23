#ifndef __ACTORPACKETARGUMENT_H
#define __ACTORPACKETARGUMENT_H

// Chamsol:RemoveVoidPointer
//패킷처리함수들에서 무조건 void *를 리턴하는 것을 막기 위해
//void * 대신 이 스트럭쳐에 넣어서 전달하도록 바꿈
//확인결과 CMonster 객체를 CUser 포인터에 대입하여 사용하는 위험한 상황도 있었음

class GObject;
class PacketContainerInterface;
struct PacketContainer	// TSPacket은 클래스가 아니므로 미리 선언할 수 없음
{						// 여기서는 void*형으로 만들고, TSPacket을 대입하는 인터페이스를 만듦
private :
	void *pc_Packet;
public :
	friend class PacketContainerInterface;
};

struct ActorProcArgument
{
	// 원래 전달하던 인수들
	void *First;
	void *Second;
	void *Thire;

	GObject		   *pGO;
	PacketContainer sPac;

	ActorProcArgument(void)
	{
		pGO = NULL;
	}
	~ActorProcArgument(void)
	{
	}
};

#endif __ACTORPACKETARGUMENT_H
