#pragma once

#include <iostream>
#include <Core.h>

enum class ENGINE_API PacketType
{
    INPUT = 1,
};

struct ENGINE_API PacketHeader 
{
    uint16_t packetType;
    uint16_t packetSize;
};

struct ENGINE_API InputPacket
{
    PacketHeader header;
    uint32_t playerID;
    uint8_t keyCode;
    uint8_t action;
    uint32_t timestamp;
};