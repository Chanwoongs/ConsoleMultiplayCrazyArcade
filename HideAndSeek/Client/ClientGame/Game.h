#pragma once

#include "Engine/Engine.h"

class Game : public Engine
{
public:
	Game();
	~Game();

	void ToggleMenu();
    void RequestEnterGame();
    void EnterGame();

    virtual void CheckInput() override;

    inline Level* GetCurrentLevel() { return mainLevel; }

	static Game& Get() { return *instance; }

private:
	bool showMenu = false;

	Level* menuLevel = nullptr;
	Level* backLevel = nullptr;

private:
	static Game* instance;

    static constexpr int packetBufferSize = 8192;
};