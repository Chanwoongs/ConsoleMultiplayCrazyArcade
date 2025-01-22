#include "Game.h"

#include "GameClient.h"
#include "Levels/MenuLevel.h"
#include "EngineGame/Levels/GameLevel.h"

Game* Game::instance = nullptr;

Game::Game()
    : Engine()
{
    instance = this;

    menuLevel = new MenuLevel();
}

Game::~Game()
{
    if (showMenu)
    {
        delete backLevel;
        backLevel = nullptr;
        mainLevel = nullptr;
    }
    else
    {
        delete mainLevel;
        mainLevel = nullptr;
    }

    delete menuLevel;
    menuLevel = nullptr;
}

void Game::ToggleMenu()
{
    system("cls");

    showMenu = !showMenu;
    if (showMenu)
    {
        backLevel = mainLevel;
        mainLevel = menuLevel;
    }
    else
    {
        mainLevel = backLevel;
    }
}

void Game::RequestEnterGame()
{
    PlayerEnterRequestPacket* playerEnterRequestPacket = new PlayerEnterRequestPacket;

    GameClient::Get().EnqueueSend(
        GameClient::Get().CreatePacketData(
            PacketType(playerEnterRequestPacket->header.packetType),
            sizeof(PlayerEnterRequestPacket),
            (char*)playerEnterRequestPacket)
    );
}

void Game::EnterGame()
{
    system("cls");

    GameClient::Get().EnterGame();
}