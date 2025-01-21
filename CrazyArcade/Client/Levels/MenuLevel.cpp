#include "MenuLevel.h"
#include "Engine/Engine.h"
#include "ClientGame/Game.h"

MenuLevel::MenuLevel()
{
	menuItems.push_back(new MenuItem("Start Game", []() 
        { 
            Game::Get().RequestEnterGame();
        }));
	menuItems.push_back(new MenuItem("Quit Game", []()
        { 
            Game::Get().QuitGame(); 
        }));
	length = (int)menuItems.size();
}

MenuLevel::~MenuLevel()
{
	for (auto* item : menuItems)
	{
		delete item;
	}
}

void MenuLevel::Update(float deltaTime)
{
	if (Game::Get().GetKeyDown(VK_UP))
	{
		currentIndex = (currentIndex - 1 + length) % length;
	}
	if (Game::Get().GetKeyDown(VK_DOWN))
	{
		currentIndex = (currentIndex + 1) % length;
	}

	if (Game::Get().GetKeyDown(VK_RETURN))
	{
		menuItems[currentIndex]->onSelected();
	}
}

void MenuLevel::Draw()
{
	Super::Draw();

	Engine::Get().SetCursorPosition(0, 0);
	
	SetColor(unselectedColor);
	Log("Slither Game!\n\n");

	for (int ix = 0; ix < length; ++ix)
	{
		SetColor(ix == currentIndex ? selectedColor : unselectedColor);
		Log("%s\n", menuItems[ix]->menuText);
	}
}