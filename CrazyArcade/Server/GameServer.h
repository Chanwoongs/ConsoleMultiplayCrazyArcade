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

#pragma comment (lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE 1024
#define BUF_SIZE 256

struct ClientHandleData;
struct PacketData;
struct SendTask;
enum class SendTaskType;

class GameServer
{
public:
	GameServer(const char* port);
	~GameServer();

	void AcceptClients();
	static unsigned WINAPI HandleClient(void* arg);
    static unsigned WINAPI Send(void* arg);
    void ProcessPacket(SOCKET clientSocket, char* packet);
	void Send(SOCKET clientSocket, void* packet);
	void Broadcast(void* packet);

    void EnqueueSend(SOCKET clientSocket, void* packet);
    void EnqueueBroadcast(SOCKET clientSocket, void* packet);

    inline void Stop() { isRunning = false; }

	void ErrorHandling(const char* message) const;

private:
	SOCKET hServerSocket;
	std::vector<SOCKET> clientSockets;

	SOCKADDR_IN serverAddress;

	HANDLE mutex;
    HANDLE sendMutex;
	std::vector<HANDLE> clientThreads;
    HANDLE sendThread;

    std::queue<SendTask*> sendQueue;

    int playerCount = 0;
	int port;
	int clientCount;

    bool isRunning;
};

struct ClientHandleData
{
    GameServer* server;
    SOCKET hClientSocket;
};

struct PacketData
{
    PacketType packetType;
    char* packet;

    PacketData(const PacketType& packetType, char* packet)
        : packetType(packetType), packet(packet)
    {
    }
};

struct SendTask
{
    SendTaskType type;
    SOCKET clientSocket;
    void* packet;
};

enum class SendTaskType
{
    SEND = 1,
    BROADCAST
};