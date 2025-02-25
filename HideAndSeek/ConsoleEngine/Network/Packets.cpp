#include "PrecompiledHeader.h"

#include "Packets.h"

const char* ToString(PacketType packetType)
{
    switch (packetType)
    {
        ENUM_TO_STRING_CASE(PacketType::KEY_INPUT);
        ENUM_TO_STRING_CASE(PacketType::MOVE);
        ENUM_TO_STRING_CASE(PacketType::PLAYER_ENTER_REQUEST);
        ENUM_TO_STRING_CASE(PacketType::PLAYER_ENTER_RESPOND);
        ENUM_TO_STRING_CASE(PacketType::PLAYER_CREATE_REQUEST);
        ENUM_TO_STRING_CASE(PacketType::PLAYER_CREATE_RESPOND);
        ENUM_TO_STRING_CASE(PacketType::PLAYER_EXIT_REQUEST);
        ENUM_TO_STRING_CASE(PacketType::GAME_STATE_SYNCHRONIZE);
    default: return "Unknown";
    }
}

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