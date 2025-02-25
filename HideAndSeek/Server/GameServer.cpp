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
    mutex = CreateMutex(NULL, FALSE, NULL);
    sendThread = (HANDLE)_beginthreadex(NULL, 0, Send, this, 0, NULL);
    
    synchronizeThread = (HANDLE)_beginthreadex(NULL, 0, Sychronize, this, 0, NULL);

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

        if (clientSocket == INVALID_SOCKET)
        {
            if (isRunning)
                break;
            continue;
        }

        WaitForSingleObject(mutex, INFINITE);

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

        CheckHeapStatus();

        ClientHandleData* clientHandleData = new ClientHandleData{ this, clientSocket };
        HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)clientHandleData, 0, NULL);

        WaitForSingleObject(mutex, INFINITE);
        clientThreads.push_back(threadHandle); 
        ReleaseMutex(mutex);

        CheckHeapStatus();

        char clientIP[20] = { 0 };
        if (inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, sizeof(clientIP)))
        {
            printf("Connected client IP: %s \n", clientIP);
        } 
        else
        {
            ErrorHandling("inet_ntop() error");
        }

        CheckHeapStatus();
    }
}

unsigned WINAPI GameServer::HandleClient(void* arg)
{
    ClientHandleData* clientHandleData = static_cast<ClientHandleData*>(arg);
    GameServer* server = clientHandleData->server;
    SOCKET hClientSocket = clientHandleData->hClientSocket;

    CheckHeapStatus();

    int strLen = 0;
    char buffer[packetBufferSize] = {};

    CheckHeapStatus();

    while ((strLen = recv(hClientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        CheckHeapStatus();

        char* packet = new char[strLen];
        memset(packet, 0, strLen);

        CheckHeapStatus();

        DeserializePacket(packet, strLen, buffer);

        CheckHeapStatus();

        server->ProcessPacket(hClientSocket, packet);

        strLen = 0;
    }

    CheckHeapStatus();

    WaitForSingleObject(server->mutex, INFINITE);
    auto it = std::find(server->clientSockets.begin(), server->clientSockets.end(), 
        hClientSocket);
    if (it != server->clientSockets.end())
    {
        server->clientSockets.erase(it);
        --server->clientCount;
    }

    CheckHeapStatus();

    ReleaseMutex(server->mutex);
    closesocket(hClientSocket);

    CheckHeapStatus();

    delete clientHandleData;

    CheckHeapStatus();

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

unsigned __stdcall GameServer::Sychronize(void* arg)
{
    GameServer* server = static_cast<GameServer*>(arg);

    while (server->isRunning)
    {
        if (server->gameLevel == nullptr)
        {
            continue;
        }

        server->SynchronizeGameState();

        Sleep(1);
    }

    return 0;
}

void GameServer::ProcessPacket(SOCKET clientSocket, char* packet)
{
    PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(packet);

    if ((PacketType)packetHeader->packetType == PacketType::PLAYER_ENTER_REQUEST)
    {
        printf("Received %s\n", ToString((PacketType)packetHeader->packetType));

        PlayerEnterRespondPacket* playerEnterRespondPacket = new PlayerEnterRespondPacket;

        EnqueueSend(
            CreatePacketData(
                PacketType(playerEnterRespondPacket->header.packetType),
                sizeof(playerEnterRespondPacket),
                (char*)playerEnterRespondPacket), 
            SendTask::Type::SEND, 
            clientSocket
        );

        delete playerEnterRespondPacket;
    }
    else if ((PacketType)packetHeader->packetType == PacketType::PLAYER_CREATE_REQUEST)
    {
        printf("Received %s\n", ToString((PacketType)packetHeader->packetType));

        CheckHeapStatus();

        char buffer[packetBufferSize] = {};

        CheckHeapStatus();

        WaitForSingleObject(mutex, INFINITE);

        int y = Engine::Get().GetRandomInt(3, 15);
        int x = Engine::Get().GetRandomInt(10, 60);

        CheckHeapStatus();

        PlayerCreateRespondPacket* playerCreateRespondPacket =
            new PlayerCreateRespondPacket(++playerCount);

        ReleaseMutex(mutex);

        CheckHeapStatus();

        size_t serializedSize;
        char* serializedData = playerCreateRespondPacket->Serialize(serializedSize);

        EnqueueSend(
            CreatePacketData(
                PacketType::PLAYER_CREATE_RESPOND,
                serializedSize,
                serializedData),
            SendTask::Type::SEND,
            clientSocket
        );

        if (gameLevel == nullptr)
        {
            gameLevel = new GameLevel();
            gameLevel->LoadMap();
            gameLevel->SetClientId(99999999);
        }

        if (!isSeekerSet)
        {
            gameLevel->AddActor(new Player(playerCount, Vector2(x, y), gameLevel, true));
            isSeekerSet = true;
        }
        else
        {
            gameLevel->AddActor(new Player(playerCount, Vector2(x, y), gameLevel, false));
        }

        delete playerCreateRespondPacket;
    }
    else if ((PacketType)packetHeader->packetType == PacketType::KEY_INPUT)
    {
        KeyInputPacket* keyInputPacket = new KeyInputPacket(0, 0);
        size_t size = 0;
        keyInputPacket->Deserialize(packet, size);

        printf("Received %s / Player ID: %d\n", ToString((PacketType)packetHeader->packetType), keyInputPacket->playerId);
        
        if (keyInputPacket->keyCode == VK_UP)
        {    
            gameLevel->MovePlayer(keyInputPacket->playerId, Direction::UP);
        }
        else if (keyInputPacket->keyCode == VK_DOWN)
        {
            gameLevel->MovePlayer(keyInputPacket->playerId, Direction::DOWN);
        }
        else if (keyInputPacket->keyCode == VK_RIGHT)
        {
            gameLevel->MovePlayer(keyInputPacket->playerId, Direction::RIGHT);
        }
        else if (keyInputPacket->keyCode == VK_LEFT)
        {
            gameLevel->MovePlayer(keyInputPacket->playerId, Direction::LEFT);
        }

        delete keyInputPacket;
    }
    else if ((PacketType)packetHeader->packetType == PacketType::MOUSE_INPUT)
    {
        MouseInputPacket* mouseInputPacket = new MouseInputPacket(0, 0, 0, 0);
        size_t size = 0;
        mouseInputPacket->Deserialize(packet, size);
        printf("Received %s / Player ID: %d / Clicked Position: (%d, %d)\n", ToString((PacketType)packetHeader->packetType), mouseInputPacket->playerId, mouseInputPacket->posX, mouseInputPacket->posY);

        delete mouseInputPacket;
    }
    else if ((PacketType)packetHeader->packetType == PacketType::PLAYER_EXIT_REQUEST)
    {
		PlayerExitRequestPacket* playerExitRequestPacket = (PlayerExitRequestPacket*)packet;

        printf("Received %s / Player ID: %d\n", ToString((PacketType)packetHeader->packetType), playerExitRequestPacket->playerId);

		WaitForSingleObject(mutex, INFINITE);
		gameLevel->RemovePlayer(playerExitRequestPacket->playerId);
		ReleaseMutex(mutex);

		delete playerExitRequestPacket;
    }

    delete[] packet;
}

ServerPacketData* GameServer::CreatePacketData(const PacketType packetType, const size_t packetSize, const char* packet)
{
    return new ServerPacketData(packetType, packetSize, packet);
}

void GameServer::Send(const SendTask* task)
{
    WaitForSingleObject(mutex, INFINITE);

    char buffer[packetBufferSize] = {};
    SerializePacket(task->packet, task->size, buffer);

    send(task->clientSocket, buffer, (int)task->size, 0);

    ReleaseMutex(mutex);
}

void GameServer::Broadcast(const SendTask* task)
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

void GameServer::EnqueueSend(const ServerPacketData* packetData, const SendTask::Type type, const SOCKET clientSocket)
{
    WaitForSingleObject(sendMutex, INFINITE);

    SendTask* task = new SendTask{ type, packetData->type , clientSocket, packetData->size, packetData->packet };
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

    GameStateSynchronizePacket* gameStatePacket = new GameStateSynchronizePacket(gameStateSize, buffer);

    size_t serializedSize = 0;
    char* serializedData = gameStatePacket->Serialize(serializedSize);

    EnqueueSend(
        CreatePacketData(
            PacketType::GAME_STATE_SYNCHRONIZE,
            serializedSize,
            serializedData),
        SendTask::Type::BROADCAST
    );

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