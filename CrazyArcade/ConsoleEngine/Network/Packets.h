#pragma once

#include <iostream>
#include <Core.h>

enum class ENGINE_API PacketType
{
    INPUT = 1,
    MOVE,
};

struct ENGINE_API PacketHeader 
{
public:
    uint32_t packetType;
    uint32_t packetSize;
};

struct ENGINE_API InputPacket
{
public:
    PacketHeader header;
    uint32_t playerId;
    uint8_t keyCode;

public:
    InputPacket(const PacketType& packetType, const uint32_t& packetSize,
        const uint32_t& playerId, const uint8_t& keyCode)
        : playerId(playerId), keyCode(keyCode)
    {
        header.packetType = (uint32_t)packetType;
        header.packetSize = packetSize;
    }
};

struct ENGINE_API ActorMovePacket
{
public:
    PacketHeader header;
    uint32_t movedActorIndex;
    uint32_t posX;
    uint32_t posY;

public:
    ActorMovePacket(const PacketType& packetType, const uint32_t& packetSize,
        const uint32_t& movedActorIndex, 
        const uint32_t& posX, const uint32_t& posY)
        : movedActorIndex(movedActorIndex), posX(posX), posY(posY)
    {
        header.packetType = (uint32_t)packetType;
        header.packetSize = packetSize;
    }
};

extern "C"
{
    void ENGINE_API SerializePacket(const void* packet, size_t packetSize, char* buffer);
    void ENGINE_API DeserializePacket(void* packet, size_t packetSize, const char* buffer);
}