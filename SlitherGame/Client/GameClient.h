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
public:
    static GameClient& Get() { return *Instance; }

    GameClient(const char* ip, const char* port);
    ~GameClient();

    ClientPacketData* CreatePacketData(PacketType packetType, size_t packetSize, char* packet);

    void ConnectServer();
    void RunThreads();
    void RunSendThread(void* arg);

    static unsigned WINAPI Send(void* arg);
    static unsigned WINAPI Receive(void* arg);

    void EnqueueSend(ClientPacketData* data);
    void ProcessPacket(char* packet, int size);

    void RequestExitGame();

    inline SOCKET Socket() const { return hSocket; }
    inline bool IsGameover() const { return isGameover; }
    inline std::queue<ClientPacketData*>& GetSendQueue() { return sendQueue; }
    inline int PlayerId() const{ return playerId; }
    inline void EnterGame() { hasEnteredGame = true; }
    inline bool HasEnteredGame() const { return hasEnteredGame; }

    void ErrorHandling(const char* message);

private:
    static GameClient* Instance;

    SOCKET hSocket = 0;
    SOCKADDR_IN* serverAddress;

    HANDLE hSendThread = nullptr;
    HANDLE hReceiveThread = nullptr;
    HANDLE hSendMutex = nullptr;
    HANDLE hReceiveMutex = nullptr;

    std::queue<ClientPacketData*> sendQueue;

    int playerId = 0;
    bool isGameover = false;
    bool hasEnteredGame = false;

    static constexpr int packetBufferSize = 8192;
};