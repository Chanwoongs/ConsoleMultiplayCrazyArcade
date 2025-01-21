#pragma once

#include <iostream>
#include <Core.h>

#include "EngineGame/Levels/GameLevel.h"

enum class ENGINE_API PacketType
{
    INPUT = 1,
    MOVE,
    PLAYER_ENTER_REQUEST,
    PLAYER_ENTER_RESPOND,
};

struct ENGINE_API PacketHeader 
{
public:
    uint32_t packetType;
    uint32_t packetSize;
};

struct ENGINE_API PlayerEnterRequestPacket
{
public :
    PacketHeader header;
    
public:
    PlayerEnterRequestPacket()
    {
        header.packetType = (uint32_t)PacketType::PLAYER_ENTER_REQUEST;
        header.packetSize = sizeof(PlayerEnterRequestPacket);
    }
};

struct ENGINE_API PlayerEnterRespondPacket
{
public:
    PacketHeader header;
    uint32_t playerId;
    uint32_t posY;
    uint32_t posX;
    uint32_t gameStateSize;
    char* gameStateBuffer;

public:
    PlayerEnterRespondPacket(const uint32_t& playerId,
        const uint32_t& posY, const uint32_t& posX,
        const char* gameStateData, size_t gameStateSize)
        : playerId(playerId), posY(posY), posX(posX), gameStateBuffer(nullptr), gameStateSize(gameStateSize)
    {
        header.packetType = (uint32_t)PacketType::PLAYER_ENTER_RESPOND;
        header.packetSize = sizeof(PlayerEnterRespondPacket);

        gameStateBuffer = new char[gameStateSize + 1];  
        gameStateBuffer[gameStateSize] = '\0';
        memcpy(gameStateBuffer, gameStateData, gameStateSize);

        header.packetSize += gameStateSize;
    }

    ~PlayerEnterRespondPacket()
    {
        delete[] gameStateBuffer;
    }
};

struct ENGINE_API InputPacket
{
public:
    PacketHeader header;
    uint32_t playerId;
    uint8_t keyCode;

public:
    InputPacket(const uint32_t& playerId, const uint8_t& keyCode)
        : playerId(playerId), keyCode(keyCode)
    {
        header.packetType = (uint32_t)PacketType::INPUT;
        header.packetSize = sizeof(InputPacket);        
    }
};

struct ENGINE_API ActorMovePacket
{
public:
    PacketHeader header;
    uint32_t movedActorIndex;
    uint32_t posY;
    uint32_t posX;

public:
    ActorMovePacket(const uint32_t& movedActorIndex, 
        const uint32_t& posX, const uint32_t& posY)
        : movedActorIndex(movedActorIndex), posX(posX), posY(posY)
    {
        header.packetType = (uint32_t)PacketType::MOVE;
        header.packetSize = sizeof(ActorMovePacket);
    }
};

extern "C"
{
    void ENGINE_API SerializePacket(const void* packet, size_t packetSize, char* buffer);
    void ENGINE_API DeserializePacket(void* packet, size_t packetSize, const char* buffer);
}