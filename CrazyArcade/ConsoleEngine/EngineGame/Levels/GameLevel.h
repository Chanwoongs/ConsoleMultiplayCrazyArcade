#pragma once

#include "Level/Level.h"

class ENGINE_API GameLevel : public Level
{
    RTTI_DECLARATIONS(GameLevel, Level)

public:
    GameLevel();
    ~GameLevel();

    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    void LoadMap();

    void SerializeGameState(char* buffer, size_t bufferSize, size_t& outSize);
    void DeserializeGameState(const char* buffer);

    bool CanPlayerMove(const class Vector2& position);

private:
    bool CheckGameClear();

private:

    class DrawableActor* map;

    std::vector<class Player*> players;

    std::vector<class Vector2*> wallPositions;
     
    bool isGameClear = false;
};