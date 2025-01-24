#include "PrecompiledHeader.h"

#include "Packets.h"

void ENGINE_API SerializePacket(const void* packet, size_t packetSize, char* buffer)
{
    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

    memcpy(buffer, packet, packetSize);

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }
}

void ENGINE_API DeserializePacket(void* packet, size_t packetSize, const char* buffer)
{
    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

    memcpy(packet, buffer, packetSize);

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }
}