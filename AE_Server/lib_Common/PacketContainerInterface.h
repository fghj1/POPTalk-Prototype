#ifndef __PACKETCONTAINER_H
#define __PACKETCONTAINER_H
// Chamsol:RemoveVoidPointer : 캐스팅 곤란한 TSPacket을 위한 인터페이스
#include "../lib_Core/ActorPacketArgument.h"

class PacketContainerInterface
{
private :
	PacketContainer &pci_PckCnt;
public :
	PacketContainerInterface(PacketContainer &pc) : pci_PckCnt(pc)
	{
	}
	~PacketContainerInterface(void)
	{
	}
	inline void operator =(TSPacket *pack)
	{
		pci_PckCnt.pc_Packet = pack;
	}
	inline operator TSPacket *(void)
	{
		return (TSPacket *)pci_PckCnt.pc_Packet;
	}
};

#endif __PACKETCONTAINER_H
