#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <WS2tcpip.h>
#include <queue>
#include "Network/Packets.h"

class GameClient
{
    struct PacketData
    {
        GameClient* client = nullptr;
        PacketType type;
        size_t size;
        char* packet;

        PacketData(GameClient* client, PacketType type, size_t size, char* packet)
            : client(client), type(type), size(size)
        {
            this->packet = new char[size];
            memcpy(this->packet, packet, size);
        }
        ~PacketData()
        {
            delete[] this->packet;
        }
    };

public:
    static GameClient& Get() { return *Instance; }

    GameClient(const char* ip, const char* port);
    ~GameClient();

    PacketData* CreatePacketData(PacketType packetType, size_t packetSize, char* packet);

    void ConnectServer();
    void RunThreads();
    void RunSendThread(void* arg);

    static unsigned WINAPI Send(void* arg);
    static unsigned WINAPI Receive(void* arg);

    void EnqueueSend(PacketData* data);
    void ProcessPacket(char* packet, int size);

    inline SOCKET Socket() const { return hSocket; }
    inline bool IsGameover() const { return isGameover; }
    inline std::queue<PacketData*>& GetSendQueue() { return sendQueue; }
    inline int PlayerId() const{ return playerId; }

    void ErrorHandling(const char* message);

private:
    static GameClient* Instance;

    SOCKET hSocket = 0;
    SOCKADDR_IN* serverAddress;

    HANDLE hSendThread = nullptr;
    HANDLE hReceiveThread = nullptr;
    HANDLE hSendMutex = nullptr;

    std::queue<PacketData*> sendQueue;

    int playerId = 0;
    bool isGameover = false;

    static constexpr int maxBufferSize = 1024;
    static constexpr int gameStateBufferSize = 2048;
};