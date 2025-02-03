#include "GameServer.h"

#include <process.h>

#include "Engine/Engine.h"
#include "EngineGame/Levels/GameLevel.h"

#define TEST 1

unsigned WINAPI AcceptClientsThread(void* arg);

int main(int argc, char* argv[])
{
    CheckMemoryLeak();

#if !TEST
    if (argc != 2) 
    {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }
#endif 

    try
    {
#if TEST
        GameServer* server = new GameServer("9190");
#else
        GameServer* server = new GameServer(argv[1]);
#endif
        HANDLE acceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptClientsThread, server, 0, NULL); 

        Engine* engine = new Engine();

        while (server->IsRunning())
        {
            // 게임 로직
            server->SynchronizeGameState();

            Sleep(100);
        }

        WaitForSingleObject(acceptThread, INFINITE);

        CloseHandle(acceptThread);
        delete engine;
        delete server;
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Error: %s\n", ex.what());
        return -1;
    }


    return 0;
}

unsigned WINAPI AcceptClientsThread(void* arg)
{
    try
    {
        GameServer* server = static_cast<GameServer*>(arg);
        server->AcceptClients();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Exception in AcceptClientsThread: %s\n", ex.what());
    }
    return 0;
}