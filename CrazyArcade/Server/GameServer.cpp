#include "GameServer.h"

#include "EngineGame/Levels/GameLevel.h"

GameServer::GameServer(const char* port)
{
    serverAddress = new SOCKADDR_IN();

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

    memset(serverAddress, 0, sizeof(SOCKADDR_IN));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_addr.s_addr = htonl(INADDR_ANY);

#if TEST
    serverAddress->sin_port = htons(9190);
#else
    serverAddress->sin_port = htons(atoi(port));
#endif

    if (bind(hServerSocket, (SOCKADDR*)serverAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        ErrorHandling("bind() error");
    }

    if (listen(hServerSocket, 5) == SOCKET_ERROR)
    {
        ErrorHandling("listen() error");
    }

    sendMutex = CreateMutex(NULL, FALSE, NULL); 
    sendThread = (HANDLE)_beginthreadex(NULL, 0, Send, this, 0, NULL);

    isRunning = true;
}

GameServer::~GameServer()
{
    closesocket(hServerSocket);
    WSACleanup();

    WaitForMultipleObjects((DWORD)clientThreads.size(), clientThreads.data(), TRUE, INFINITE);

    for (auto& thread : clientThreads)
    {
        CloseHandle(thread);
    }
    CloseHandle(sendThread);
    CloseHandle(mutex);

    if (serverAddress)
    {
        delete serverAddress;
    }
}

void GameServer::AcceptClients()
{
    while (isRunning)
    {
        SOCKADDR_IN clientAddress;
        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);

        WaitForSingleObject(mutex, INFINITE);

        if (gameLevel == nullptr)
        {
            gameLevel = new GameLevel();
            gameLevel->LoadMap();
        }

        if (clientSockets.size() >= 8) 
        {
            closesocket(clientSocket);
            printf("Connection refused: Max clients reached.\n");
            ReleaseMutex(mutex);
            continue;
        }

        clientSockets.push_back(clientSocket);
        ++clientCount;
        ReleaseMutex(mutex);

        ClientHandleData* clientHandleData = new ClientHandleData{ this, clientSocket };
        HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)clientHandleData, 0, NULL);

        WaitForSingleObject(mutex, INFINITE);
        clientThreads.push_back(threadHandle); 
        ReleaseMutex(mutex);

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
    char buffer[packetBufferSize] = {};

    while ((strLen = recv(hClientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        char* packet = new char[strLen];
        DeserializePacket(packet, strlen(packet), buffer);
        
        server->ProcessPacket(hClientSocket, packet);

        strLen = 0;
    }

    WaitForSingleObject(server->mutex, INFINITE);
    auto it = std::find(server->clientSockets.begin(), server->clientSockets.end(), 
        hClientSocket);
    if (it != server->clientSockets.end())
    {
        server->clientSockets.erase(it);
        --server->clientCount;
    }

    ReleaseMutex(server->mutex);
    closesocket(hClientSocket);

    delete clientHandleData;
    return 0;
}

unsigned WINAPI GameServer::Send(void* arg)
{
    GameServer* server = static_cast<GameServer*>(arg);

    while (server->isRunning) 
    {
        WaitForSingleObject(server->sendMutex, INFINITE);

        if (!server->sendQueue.empty()) 
        {
            SendTask* task = server->sendQueue.front();
            server->sendQueue.pop();

            ReleaseMutex(server->sendMutex);

            if (task->type == SendTask::Type::SEND)
            {
                server->Send(task->clientSocket, task->packet);
            }
            else if (task->type == SendTask::Type::BROADCAST)
            {
                server->Broadcast(task->packet);
                WaitForSingleObject(server->mutex, INFINITE);

                char buffer[packetBufferSize] = {};
                SerializePacket(task->packet, sizeof(buffer), buffer);

                for (SOCKET client : server->clientSockets)
                {
                    send(client, buffer, sizeof(buffer), 0);
                }

                ReleaseMutex(server->mutex);
            }
            delete task->packet;
            delete task;
        }
        else 
        {
            ReleaseMutex(server->sendMutex);
        }
    }
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
        char buffer[gameStateBufferSize] = {};
        size_t gameStateSize = 0;
        gameLevel->SerializeGameState(buffer, gameStateBufferSize, gameStateSize);

        PlayerEnterRespondPacket* playerEnterRespondPacket =
            new PlayerEnterRespondPacket(++playerCount, 5, 5, buffer, gameStateSize);

        // EnqueueSend 호출
        EnqueueSend(clientSocket, (void*)playerEnterRespondPacket);

        break;
    }
}


void GameServer::Send(SOCKET clientSocket, void* packet)
{
    WaitForSingleObject(mutex, INFINITE);

    char buffer[packetBufferSize] = {};
    SerializePacket(packet, sizeof(buffer), buffer);

    send(clientSocket, buffer, sizeof(buffer), 0);

    ReleaseMutex(mutex);
}

void GameServer::Broadcast(void* packet)
{
    WaitForSingleObject(mutex, INFINITE);

    char buffer[packetBufferSize] = {};
    SerializePacket(packet, sizeof(buffer), buffer);

    for (SOCKET client : clientSockets)
    {
        send(client, buffer, sizeof(buffer), 0);
    }

    ReleaseMutex(mutex);
}

void GameServer::EnqueueSend(SOCKET clientSocket, void* packet)
{
    WaitForSingleObject(sendMutex, INFINITE);

    SendTask* task = new SendTask{ SendTask::Type::SEND, clientSocket, packet };
    sendQueue.push(task);

    ReleaseMutex(sendMutex);
}

void GameServer::EnqueueBroadcast(SOCKET clientSocket, void* packet)
{
    WaitForSingleObject(sendMutex, INFINITE);

    SendTask* task = new SendTask{ SendTask::Type::BROADCAST, clientSocket, packet };
    sendQueue.push(task);

    ReleaseMutex(sendMutex);
}

void GameServer::ErrorHandling(const char* message) const
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}