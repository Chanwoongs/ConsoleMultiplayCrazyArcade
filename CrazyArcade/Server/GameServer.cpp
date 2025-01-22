#include "GameServer.h"

#include "Engine/Engine.h"
#include "EngineGame/Levels/GameLevel.h" 
#include "EngineGame/Actors/Player.h"

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
    delete gameLevel;

    closesocket(hServerSocket);
    WSACleanup();

    WaitForMultipleObjects((DWORD)clientThreads.size(), clientThreads.data(), TRUE, INFINITE);


    CloseHandle(mutex);

    if (serverAddress)
    {
        delete serverAddress;
    }

    for (auto& thread : clientThreads)
    {
        WaitForSingleObject(thread, INFINITE);
    }
    WaitForSingleObject(sendThread, INFINITE);
    
    for (auto& thread : clientThreads)
    {
        CloseHandle(thread);
    }
    CloseHandle(sendThread);
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

        auto error = _heapchk();

        if (error != _HEAPOK)
        {
            DebugBreak();
        }

        ClientHandleData* clientHandleData = new ClientHandleData{ this, clientSocket };
        HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)clientHandleData, 0, NULL);

        WaitForSingleObject(mutex, INFINITE);
        clientThreads.push_back(threadHandle); 
        ReleaseMutex(mutex);

        error = _heapchk();

        if (error != _HEAPOK)
        {
            DebugBreak();
        }

        char clientIP[20] = { 0 };
        if (inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, sizeof(clientIP)))
        {
            printf("Connected client IP: %s \n", clientIP);
        } 
        else
        {
            ErrorHandling("inet_ntop() error");
        }

        error = _heapchk();

        if (error != _HEAPOK)
        {
            DebugBreak();
        }
    }
}

unsigned WINAPI GameServer::HandleClient(void* arg)
{
    ClientHandleData* clientHandleData = static_cast<ClientHandleData*>(arg);
    GameServer* server = clientHandleData->server;
    SOCKET hClientSocket = clientHandleData->hClientSocket;

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

    int strLen = 0;
    char buffer[packetBufferSize] = {};

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

    while ((strLen = recv(hClientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        if (_heapchk() != _HEAPOK)
        {
            DebugBreak();
        }

        char* packet = new char[strLen];
        memset(packet, 0, strLen);

        if (_heapchk() != _HEAPOK)
        {
            DebugBreak();
        }

        DeserializePacket(packet, strLen, buffer);

        if (_heapchk() != _HEAPOK)
        {
            DebugBreak();
        }

        server->ProcessPacket(hClientSocket, packet);

        strLen = 0;
    }

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

    WaitForSingleObject(server->mutex, INFINITE);
    auto it = std::find(server->clientSockets.begin(), server->clientSockets.end(), 
        hClientSocket);
    if (it != server->clientSockets.end())
    {
        server->clientSockets.erase(it);
        --server->clientCount;
    }

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

    ReleaseMutex(server->mutex);
    closesocket(hClientSocket);

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

    delete clientHandleData;

    if (_heapchk() != _HEAPOK)
    {
        DebugBreak();
    }

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
                server->Send(task);
            }
            else if (task->type == SendTask::Type::BROADCAST)
            {
                server->Broadcast(task);
            }
            delete task->packet;
            delete task;
        }
        else 
        {
            ReleaseMutex(server->sendMutex);
            Sleep(10);
        }
    }
    return 0;
}

void GameServer::ProcessPacket(SOCKET clientSocket, char* packet)
{
    PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(packet);

    if ((PacketType)packetHeader->packetType == PacketType::PLAYER_ENTER_REQUEST)
    {
        if (_heapchk() != _HEAPOK)
        {
            DebugBreak();
        }

        //@TODO: 랜덤 위치 생성 로직 
        char buffer[packetBufferSize] = {};
        size_t gameStateSize = 0;

        if (_heapchk() != _HEAPOK)
        {
            DebugBreak();
        }

        WaitForSingleObject(mutex, INFINITE);

        int y = Engine::Get().GetRandomInt(3, 15);
        int x = Engine::Get().GetRandomInt(10, 60);

        gameLevel->AddActor(new Player(++playerCount, Vector2(x, y), gameLevel));
        gameLevel->SerializeGameState(buffer, packetBufferSize, gameStateSize);

        if (_heapchk() != _HEAPOK)
        {
            DebugBreak();
        }

        PlayerEnterRespondPacket* playerEnterRespondPacket =
            new PlayerEnterRespondPacket(playerCount, y, x, buffer, gameStateSize);

        ReleaseMutex(mutex);

        if (_heapchk() != _HEAPOK)
        {
            DebugBreak();
        }

        size_t serializedSize;
        char* serializedData = playerEnterRespondPacket->Serialize(serializedSize);

        //PlayerEnterRespondPacket* p = new PlayerEnterRespondPacket(0, 0, 0, 0, 0);
        //p->Deserialize(serializedData, serializedSize);

        // EnqueueSend 호출
        EnqueueSend(serializedSize, serializedData, clientSocket);

        delete playerEnterRespondPacket;
    }
    else if ((PacketType)packetHeader->packetType == PacketType::INPUT)
    {
        InputPacket* inputPacket = new InputPacket(0, 0);
        size_t size = 0;
        inputPacket->Deserialize(packet, size);
        
        if (inputPacket->keyCode == VK_UP)
        {    
            gameLevel->MovePlayer(inputPacket->playerId, Direction::UP);
        }
        else if (inputPacket->keyCode == VK_DOWN)
        {
            gameLevel->MovePlayer(inputPacket->playerId, Direction::DOWN);
        }
        else if (inputPacket->keyCode == VK_RIGHT)
        {
            gameLevel->MovePlayer(inputPacket->playerId, Direction::RIGHT);
        }
        else if (inputPacket->keyCode == VK_LEFT)
        {
            gameLevel->MovePlayer(inputPacket->playerId, Direction::LEFT);
        }

        delete inputPacket;
    }
    else if ((PacketType)packetHeader->packetType == PacketType::PLAYER_EXIT_REQUEST)
    {
		PlayerExitRequestPacket* playerExitRequestPacket = (PlayerExitRequestPacket*)packet;

		WaitForSingleObject(mutex, INFINITE);
		gameLevel->RemovePlayer(playerExitRequestPacket->playerId);
		ReleaseMutex(mutex);

		delete playerExitRequestPacket;
    }

    delete[] packet;
}

void GameServer::Send(SendTask* task)
{
    WaitForSingleObject(mutex, INFINITE);

    char buffer[packetBufferSize] = {};
    SerializePacket(task->packet, task->size, buffer);

    send(task->clientSocket, buffer, (int)task->size, 0);

    ReleaseMutex(mutex);
}

void GameServer::Broadcast(SendTask* task)
{
    WaitForSingleObject(mutex, INFINITE);

    char buffer[packetBufferSize] = {};
    SerializePacket(task->packet, task->size, buffer);

    for (SOCKET client : clientSockets)
    {
        send(client, buffer, (int)task->size, 0);
    }

    ReleaseMutex(mutex);
}

void GameServer::EnqueueSend(size_t packetSize, char* packet, SOCKET clientSocket)
{
    WaitForSingleObject(sendMutex, INFINITE);

    SendTask* task = new SendTask{ SendTask::Type::SEND, clientSocket, packetSize, packet};
    sendQueue.push(task);

    ReleaseMutex(sendMutex);
}

void GameServer::EnqueueBroadcast(size_t packetSize, char* packet, SOCKET clientSocket)
{
    WaitForSingleObject(sendMutex, INFINITE);

    SendTask* task = new SendTask{ SendTask::Type::BROADCAST, clientSocket, packetSize, packet };
    sendQueue.push(task);

    ReleaseMutex(sendMutex);
}

void GameServer::SynchronizeGameState()
{
    if (gameLevel == nullptr) return;

    isSynchronizing = true;

    WaitForSingleObject(mutex, INFINITE);

    char buffer[packetBufferSize] = {};
    size_t gameStateSize = 0;

    gameLevel->SerializeGameState(buffer, packetBufferSize, gameStateSize);

    GameLevel* g = new GameLevel();

    g->DeserializeGameState(buffer);

    GameStateSynchronizePacket* gameStatePacket = new GameStateSynchronizePacket(gameStateSize, buffer);

    size_t serializedSize = 0;
    char* serializedData = gameStatePacket->Serialize(serializedSize);

    EnqueueBroadcast(serializedSize, serializedData);

    delete gameStatePacket;

    ReleaseMutex(mutex);
    isSynchronizing = false;
}

void GameServer::ErrorHandling(const char* message) const
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}