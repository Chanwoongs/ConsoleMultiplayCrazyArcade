#include <iostream>

#include "GameClient.h"

#include "Engine/Engine.h"
#include "ClientGame/Game.h"
#include "Levels/MenuLevel.h"
#include "Network/Packets.h"

#define TEST 1

int main(int argc, char* argv[])
{
    CheckMemoryLeak();

#if !TEST
    if (argc != 3)
    {
        printf("Usage: %s <IP> <PORT> \n", argv[0]);
        exit(0);
    }
    GameClient* client = new GameClient(argv[1], argv[2]);
#else
    GameClient* client = new GameClient("127.0.0.1", "9190");
#endif
    Game* game = new Game();
    game->SetOnQuitCallBack([&]() { client->RequestExitGame(); });
    game->RequestLevelChange(new MenuLevel);

    client->RunThreads();

    game->Run();

    delete game;
    delete client;

    return 0;
}