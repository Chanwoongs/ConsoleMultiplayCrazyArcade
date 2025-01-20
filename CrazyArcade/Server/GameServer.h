#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <WS2tcpip.h>
#include <vector>

#include "Network/Packets.h"

#pragma comment (lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE 1024
#define BUF_SIZE 256

struct ClientHandleData;

class GameServer
{
public:
	GameServer(const char* port);
	~GameServer();

	void AcceptClients();
	static unsigned WINAPI HandleClient(void* arg);
	void Broadcast(char* packet, int len);

    inline void Stop() { isRunning = false; }

	void ErrorHandling(const char* message) const;

private:
	SOCKET hServerSocket;
	std::vector<SOCKET> clientSockets;

	SOCKADDR_IN serverAddress;

	HANDLE hMutex;
	HANDLE hThread;

	int port;
	int clientCount;

    bool isRunning;
};

struct ClientHandleData
{
    GameServer* server;
    SOCKET hClientSocket;
};