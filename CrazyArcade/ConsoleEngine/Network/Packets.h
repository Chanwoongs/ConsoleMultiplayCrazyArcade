#pragma once

#include <iostream>
#include <Core.h>

#include "EngineGame/Levels/GameLevel.h"
#include <new.h>

enum class ENGINE_API PacketType
{
    INPUT = 1,
    MOVE,
    PLAYER_ENTER_REQUEST,
    PLAYER_ENTER_RESPOND,
    GAME_STATE_SYNCHRONIZE
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
        : playerId(playerId), posY(posY), posX(posX), gameStateBuffer(nullptr), gameStateSize((uint32_t)gameStateSize)
    {
        header.packetType = (uint32_t)PacketType::PLAYER_ENTER_RESPOND;
        header.packetSize = sizeof(PlayerEnterRespondPacket);

        gameStateBuffer = new char[gameStateSize];
        memcpy(gameStateBuffer, gameStateData, gameStateSize);

        header.packetSize += (uint32_t)gameStateSize;
    }

    ~PlayerEnterRespondPacket()
    {
        delete[] gameStateBuffer;
    }

    char* Serialize(size_t& size)
    {
        size_t totalPacketSize = sizeof(PacketHeader) +
            sizeof(playerId) +
            sizeof(posY) +
            sizeof(posX) +
            sizeof(gameStateSize) +
            gameStateSize; 

        char* sendBuffer = new char[totalPacketSize];

        size_t offset = 0;

        memcpy(sendBuffer + offset, &header, sizeof(PacketHeader));
        offset += sizeof(PacketHeader);

        memcpy(sendBuffer + offset, &playerId, sizeof(playerId));
        offset += sizeof(playerId);

        memcpy(sendBuffer + offset, &posY, sizeof(posY));
        offset += sizeof(posY);

        memcpy(sendBuffer + offset, &posX, sizeof(posX));
        offset += sizeof(posX);

        memcpy(sendBuffer + offset, &gameStateSize, sizeof(gameStateSize));
        offset += sizeof(gameStateSize);

        memcpy(sendBuffer + offset, gameStateBuffer, gameStateSize);
        offset += gameStateSize;

        size = offset;

        return sendBuffer;
    }

    void Deserialize(const char* buffer, size_t size)
    {
        size_t offset = 0;

        memcpy(&header, buffer + offset, sizeof(PacketHeader));
        offset += sizeof(PacketHeader);

        memcpy(&playerId, buffer + offset, sizeof(playerId));
        offset += sizeof(playerId);

        memcpy(&posY, buffer + offset, sizeof(posY));
        offset += sizeof(posY);

        memcpy(&posX, buffer + offset, sizeof(posX));
        offset += sizeof(posX);

        memcpy(&gameStateSize, buffer + offset, sizeof(gameStateSize));
        offset += sizeof(gameStateSize);

        delete[] gameStateBuffer; 
        gameStateBuffer = new char[gameStateSize];
        memset(gameStateBuffer, 0, gameStateSize);
        memcpy(gameStateBuffer, buffer + offset, gameStateSize);
    }
};

struct ENGINE_API GameStateSynchronizePacket
{
public:
    PacketHeader header;
    uint32_t gameStateSize;
    char* gameStateBuffer;

    GameStateSynchronizePacket(size_t gameStateSize, char* gameStateData)
        : gameStateBuffer(gameStateData), gameStateSize((uint32_t)gameStateSize)
    {
        header.packetType = (uint32_t)PacketType::GAME_STATE_SYNCHRONIZE;
        header.packetSize = sizeof(GameStateSynchronizePacket);

        gameStateBuffer = new char[gameStateSize];
        memcpy(gameStateBuffer, gameStateData, gameStateSize);

        header.packetSize += (uint32_t)gameStateSize;
    }
    ~GameStateSynchronizePacket()
    {
        delete[] gameStateBuffer;
    }

    char* Serialize(size_t& size)
    {
        size_t totalPacketSize = sizeof(PacketHeader) +
            sizeof(gameStateSize) +
            gameStateSize;

        char* sendBuffer = new char[totalPacketSize];

        size_t offset = 0;

        memcpy(sendBuffer + offset, &header, sizeof(PacketHeader));
        offset += sizeof(PacketHeader);

        memcpy(sendBuffer + offset, &gameStateSize, sizeof(gameStateSize));
        offset += sizeof(gameStateSize);

        memcpy(sendBuffer + offset, gameStateBuffer, gameStateSize);
        offset += gameStateSize;

        size = offset;

        return sendBuffer;
    }

    void Deserialize(const char* buffer, size_t size)
    {
        size_t offset = 0;

        memcpy(&header, buffer + offset, sizeof(PacketHeader));
        offset += sizeof(PacketHeader);

        memcpy(&gameStateSize, buffer + offset, sizeof(gameStateSize));
        offset += sizeof(gameStateSize);

        delete[] gameStateBuffer;
        gameStateBuffer = new char[gameStateSize];
        memset(gameStateBuffer, 0, gameStateSize);
        memcpy(gameStateBuffer, buffer + offset, gameStateSize);
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