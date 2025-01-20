#include "GameServer.h"

GameServer::GameServer(const char* port)
    : port(atoi(port)), clientCount(0), isRunning(true)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandling("WSAStartup() error!");
    }

    hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hServerSocket == INVALID_SOCKET)
    {
        ErrorHandling("socket() error");
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

#if TEST
    serverAddress.sin_port = htons(9190);
#else
    serverAddress.sin_port = htons(atoi(port));
#endif

    if (bind(hServerSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        ErrorHandling("bind() error");
    }

    if (listen(hServerSocket, 5) == SOCKET_ERROR)
    {
        ErrorHandling("listen() error");
    }
}

GameServer::~GameServer()
{
    closesocket(hServerSocket);
    WSACleanup();

    for (auto& thread : clientThreads)
    {
        CloseHandle(thread);
    }
    CloseHandle(hMutex);
}

void GameServer::AcceptClients()
{
    while (isRunning)
    {
        SOCKADDR_IN clientAddress;
        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);

        WaitForSingleObject(hMutex, INFINITE);

        if (clientSockets.size() >= 8) 
        {
            closesocket(clientSocket);
            printf("Connection refused: Max clients reached.\n");
            ReleaseMutex(hMutex);
            continue;
        }

        clientSockets.push_back(clientSocket);
        ++clientCount;
        ReleaseMutex(hMutex);

        ClientHandleData* clientHandleData = new ClientHandleData{ this, clientSocket };
        HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)clientHandleData, 0, NULL);

        WaitForSingleObject(hMutex, INFINITE);
        clientThreads.push_back(threadHandle); 
        ReleaseMutex(hMutex);

        char clientIP[20] = { 0 };
        if (inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, sizeof(clientIP)))
        {
            printf("Connected client IP: %s \n", clientIP);
        }
        else
        {
            ErrorHandling("inet_ntop() error");
        }
    }
}

unsigned WINAPI GameServer::HandleClient(void* arg)
{
    ClientHandleData* clientHandleData = static_cast<ClientHandleData*>(arg);
    GameServer* server = clientHandleData->server;
    SOCKET hClientSocket = clientHandleData->hClientSocket;

    int strLen = 0;
    char buffer[MAX_BUFFER_SIZE] = {};

    while ((strLen = recv(hClientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        char* packet = new char[strLen];
        DeserializePacket(packet, strlen(packet), buffer);
        
        server->ProcessPacket(hClientSocket, packet);

        //server->Broadcast(packet);

        strLen = 0;
    }

    WaitForSingleObject(server->hMutex, INFINITE);
    auto it = std::find(server->clientSockets.begin(), server->clientSockets.end(), 
        hClientSocket);
    if (it != server->clientSockets.end())
    {
        server->clientSockets.erase(it);
        --server->clientCount;
    }

    ReleaseMutex(server->hMutex);
    closesocket(hClientSocket);

    delete clientHandleData;
    return 0;
}

void GameServer::ProcessPacket(SOCKET clientSocket, char* packet)
{
    PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(packet);

    switch ((PacketType)packetHeader->packetType)
    {
    case PacketType::MOVE:
        break;
    case PacketType::PLAYER_ENTER_REQUEST:
        //@TODO: 랜덤 위치 생성 로직 
        PlayerEnterRespondPacket* playerEnterRespondPacket =
            new PlayerEnterRespondPacket(++playerCount, 5, 5);
        
        Send(clientSocket, (void*)playerEnterRespondPacket);
        break;
    }
}

void GameServer::Send(SOCKET clientSocket, void* packet)
{
    WaitForSingleObject(hMutex, INFINITE);

    char buffer[MAX_BUFFER_SIZE] = {};
    SerializePacket(packet, sizeof(buffer), buffer);

    send(clientSocket, buffer, sizeof(buffer), 0);

    delete[] packet;
    ReleaseMutex(hMutex);
}

void GameServer::Broadcast(char* packet)
{
    WaitForSingleObject(hMutex, INFINITE);

    InputPacket* inputPacket = reinterpret_cast<InputPacket*>(packet);
    char buffer[MAX_BUFFER_SIZE] = {};
    int sendLen = sprintf_s(buffer, sizeof(buffer), "playerId"); 

    for (auto& client : clientSockets)
    {
        if ((send(client, buffer, sendLen, 0)) == SOCKET_ERROR)
        {
            printf("Failed to send to client: %d\n", WSAGetLastError());
        }
    }
    ReleaseMutex(hMutex);
}

void GameServer::ErrorHandling(const char* message) const
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}