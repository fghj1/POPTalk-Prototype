#ifndef __ACTORPACKETARGUMENT_H
#define __ACTORPACKETARGUMENT_H

// Chamsol:RemoveVoidPointer
//��Ŷó���Լ��鿡�� ������ void *�� �����ϴ� ���� ���� ����
//void * ��� �� ��Ʈ���Ŀ� �־ �����ϵ��� �ٲ�
//Ȯ�ΰ�� CMonster ��ü�� CUser �����Ϳ� �����Ͽ� ����ϴ� ������ ��Ȳ�� �־���

class GObject;
class PacketContainerInterface;
struct PacketContainer	// TSPacket�� Ŭ������ �ƴϹǷ� �̸� ������ �� ����
{						// ���⼭�� void*������ �����, TSPacket�� �����ϴ� �������̽��� ����
private :
	void *pc_Packet;
public :
	friend class PacketContainerInterface;
};

struct ActorProcArgument
{
	// ���� �����ϴ� �μ���
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
