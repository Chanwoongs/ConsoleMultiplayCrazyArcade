﻿#pragma once

#include <iostream>
#include <new.h>
#include <string>

#include <Core.h>

#include "EngineGame/Levels/GameLevel.h"

#define ENUM_TO_STRING_CASE(value) case value: return #value;

enum class ENGINE_API PacketType
{
    INPUT = 1,
    MOVE,
    PLAYER_ENTER_REQUEST,
    PLAYER_ENTER_RESPOND,
    PLAYER_CREATE_REQUEST,
    PLAYER_CREATE_RESPOND,
    PLAYER_EXIT_REQUEST,
    GAME_STATE_SYNCHRONIZE
};

ENGINE_API const char* ToString(PacketType packetType);

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
    uint32_t keyCode;

public:
    InputPacket(const uint32_t& playerId, const uint8_t& keyCode)
        : playerId(playerId), keyCode(keyCode)
    {
        header.packetType = (uint32_t)PacketType::INPUT;
        header.packetSize = sizeof(InputPacket);
    }

    char* Serialize(size_t& size)
    {
        size_t totalPacketSize = sizeof(PacketHeader) +
            sizeof(playerId) +
            sizeof(keyCode);

        char* sendBuffer = new char[totalPacketSize];

        size_t offset = 0;

        memcpy(sendBuffer + offset, &header, sizeof(PacketHeader));
        offset += sizeof(PacketHeader);

        memcpy(sendBuffer + offset, &playerId, sizeof(playerId));
        offset += sizeof(playerId);

        memcpy(sendBuffer + offset, &keyCode, sizeof(keyCode));
        offset += sizeof(keyCode);

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

        memcpy(&keyCode, buffer + offset, sizeof(keyCode));
        offset += sizeof(keyCode);
    }
};

struct ENGINE_API MovePacket
{
public:
    PacketHeader header;
    uint32_t movedActorIndex;
    uint32_t posY;
    uint32_t posX;

public:
    MovePacket(const uint32_t& movedActorIndex,
        const uint32_t& posX, const uint32_t& posY)
        : movedActorIndex(movedActorIndex), posX(posX), posY(posY)
    {
        header.packetType = (uint32_t)PacketType::MOVE;
        header.packetSize = sizeof(MovePacket);
    }
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

public:
    PlayerEnterRespondPacket()
    {
        header.packetType = (uint32_t)PacketType::PLAYER_ENTER_RESPOND;
        header.packetSize = sizeof(PlayerEnterRespondPacket);
    }
};

struct ENGINE_API PlayerCreateRequestPacket
{
public:
    PacketHeader header;

public:
    PlayerCreateRequestPacket()
    {
        header.packetType = (uint32_t)PacketType::PLAYER_CREATE_REQUEST;
        header.packetSize = sizeof(PlayerCreateRequestPacket);
    }
};

struct ENGINE_API PlayerCreateRespondPacket
{
public:
    PacketHeader header;
    uint32_t playerId;

public:
    PlayerCreateRespondPacket(const uint32_t& playerId)
        : playerId(playerId)
    {
        header.packetType = (uint32_t)PacketType::PLAYER_CREATE_RESPOND;
        header.packetSize = sizeof(PlayerCreateRespondPacket);
    }

    ~PlayerCreateRespondPacket()
    {
    }

    char* Serialize(size_t& size)
    {
        size_t totalPacketSize = sizeof(PacketHeader) +
            sizeof(playerId);

        char* sendBuffer = new char[totalPacketSize];

        size_t offset = 0;

        memcpy(sendBuffer + offset, &header, sizeof(PacketHeader));
        offset += sizeof(PacketHeader);

        memcpy(sendBuffer + offset, &playerId, sizeof(playerId));
        offset += sizeof(playerId);

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
    }
};

struct ENGINE_API PlayerExitRequestPacket
{
public:
    PacketHeader header;
	uint32_t playerId;

public:
    PlayerExitRequestPacket(uint32_t playerId)
		: playerId(playerId)
    {
        header.packetType = (uint32_t)PacketType::PLAYER_EXIT_REQUEST;
        header.packetSize = sizeof(PlayerExitRequestPacket);
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

struct ENGINE_API ClientPacketData
{
    class GameClient* client = nullptr;
    PacketType type;
    size_t size;
    char* packet;

    ClientPacketData(GameClient* client, PacketType type, size_t size, char* packet)
        : client(client), type(type), size(size)
    {
        this->packet = new char[size];
        memcpy(this->packet, packet, size);
    }
    ~ClientPacketData()
    {
        delete[] this->packet;
    }
};

struct ENGINE_API ServerPacketData
{
    PacketType type;
    size_t size;
    char* packet;

    ServerPacketData(const PacketType& type, const size_t size, const char* packet)
        : type(type), size(size)
    {
        this->packet = new char[size];
        memcpy(this->packet, packet, size);
    }
    ~ServerPacketData()
    {
        delete[] this->packet;
    }
};

extern "C"
{
    void ENGINE_API SerializePacket(const void* packet, size_t packetSize, char* buffer);
    void ENGINE_API DeserializePacket(void* packet, size_t packetSize, const char* buffer);
}