#ifndef __PACKETCONTAINER_H
#define __PACKETCONTAINER_H
// Chamsol:RemoveVoidPointer : ĳ���� ����� TSPacket�� ���� �������̽�
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
