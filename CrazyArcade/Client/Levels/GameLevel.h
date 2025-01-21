#pragma once

#include <Level/Level.h>
#include <Math/Vector2.h>

#include "EngineGame/Actors/Ground.h"
#include "EngineGame/Actors/Wall.h"

#include "ClientGame/Player.h"

// 소코반 게임 레벨
class GameLevel : public Level
{
    RTTI_DECLARATIONS(GameLevel, Level)

public:
    GameLevel();

    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    void LoadMap();

    bool CanPlayerMove(const Vector2& position);

private:
    bool CheckGameClear();

private:

    List<DrawableActor*> map;

    Player* player = nullptr;

    bool isGameClear = false;
};