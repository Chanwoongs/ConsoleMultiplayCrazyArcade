#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <WS2tcpip.h>
#include <queue>

#include "Network/Packets.h"

#pragma comment (lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE 1024
#define BUF_SIZE 100
#define NAME_SIZE 20

struct PacketData;

class GameClient
{
public:
	GameClient(const char* ip, const char* port);
	~GameClient();

	void ConnectServer();
	void RunThreads();
	void RunSendThread(void* arg);

	static unsigned WINAPI Send(void* arg);
	static unsigned WINAPI Receive(void* arg);

    void AddPacketToSendQueue(PacketData* data);

    inline SOCKET Socket() const { return hSocket; }
    inline bool IsGameover() const { return isGameover; }
    inline std::queue<PacketData*> SendQueue() { return sendQueue; }

	void ErrorHandling(const char* message);

private:
	SOCKET hSocket;
	SOCKADDR_IN serverAddress;

	HANDLE hSendThread, hReceiveThread;
    HANDLE hSendMutex;

    std::queue<PacketData*> sendQueue;

    bool isGameover;
};

struct PacketData
{
    GameClient* client;
    PacketType* packetType;
    void* packet;

    PacketData(GameClient* client, PacketType* packetType, void* packet)
        : client(client), packetType(packetType), packet(packet)
    {
    }
};