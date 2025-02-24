#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <WS2tcpip.h>
#include <vector>
#include <queue>

#include "Network/Packets.h"

class GameServer
{
    struct ClientHandleData
    {
        GameServer* server;
        SOCKET hClientSocket;
    };

public:
    struct SendTask
    {
        enum class Type
        {
            SEND = 1,
            BROADCAST
        };

        Type type;
        SOCKET clientSocket;
        size_t size;
        char* packet;
    };

public:
	GameServer(const char* port);
	~GameServer();

	void AcceptClients();

    void ProcessPacket(SOCKET clientSocket, char* packet);
    ServerPacketData* CreatePacketData(PacketType packetType, size_t packetSize, char* packet);

    void SynchronizeGameState();

    inline class GameLevel* GetGameLevel() { return gameLevel; }
    inline bool IsRunning() const { return isRunning; }
    inline void Stop() { isRunning = false; }

private:
    static unsigned WINAPI HandleClient(void* arg);
    static unsigned WINAPI Send(void* arg);
    static unsigned WINAPI Sychronize(void* arg);

    void Send(SendTask* task);
    void Broadcast(SendTask* task);

    void EnqueueSend(ServerPacketData* packetData, SendTask::Type type, SOCKET clientSocket = 0);
    void EnqueueBroadcast(const ServerPacketData& packetData);

    void ErrorHandling(const char* message) const;

private:
	SOCKET hServerSocket = 0;
	std::vector<SOCKET> clientSockets;

	SOCKADDR_IN* serverAddress;

    HANDLE mutex = nullptr;
    HANDLE sendMutex = nullptr;

	std::vector<HANDLE> clientThreads;
    HANDLE sendThread = nullptr;
    HANDLE synchronizeThread = nullptr;

    std::queue<SendTask*> sendQueue;

    int playerCount = 0;
	int port = 0;
	int clientCount = 0;

    bool isRunning = false;
    bool isMapInitialzed = false;
    bool isSynchronizing;

    class GameLevel* gameLevel = nullptr;

    static constexpr int packetBufferSize = 8192;
};