#include "PrecompiledHeader.h"

#include "Packets.h"

void ENGINE_API SerializePacket(const void* packet, size_t packetSize, char* buffer)
{
    memcpy(buffer, packet, packetSize);
}

void ENGINE_API DeserializePacket(void* packet, size_t packetSize, const char* buffer)
{
    memcpy(packet, buffer, packetSize);
}