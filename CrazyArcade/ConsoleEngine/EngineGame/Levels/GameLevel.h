#pragma once

#include "Level/Level.h"
//#include "EngineGame/Actors/Ground.h"
//#include "EngineGame/Actors/Wall.h"
//#include "EngineGame/Actors/Player.h"

class DrawableActor;
class GameLevel : public Level
{
    RTTI_DECLARATIONS(GameLevel, Level)

public:
    GameLevel();

    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    void LoadMap();

    void SerializeGameState(char* buffer, size_t bufferSize, size_t& size);
    void DeserializeGameState(const char* buffer, size_t size);

    bool CanPlayerMove(const class Vector2& position);

private:
    bool CheckGameClear();

private:

    std::vector<DrawableActor*> map;

    std::vector<class Player*> players;

    bool isGameClear = false;
};