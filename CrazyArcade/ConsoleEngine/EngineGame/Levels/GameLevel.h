#pragma once

#include "Level/Level.h"

#include <queue>

class ENGINE_API GameLevel : public Level
{
    RTTI_DECLARATIONS(GameLevel, Level)

public:
    GameLevel();
    ~GameLevel();

    virtual void AddActor(Actor* newActor) override;
    virtual void Update(float deltaTime) override;
    virtual void Draw() override;

    void LoadMap();

    void SerializeGameState(char* buffer, size_t bufferSize, size_t& outSize);
    void DeserializeGameState(const char* buffer);

    bool CanPlayerMove(const class Vector2& position);

    void MovePlayer(int playerId, enum class Direction direction);
    
private:
    bool CheckGameClear();

private:

    class DrawableActor* map = nullptr;

    std::vector<class Player*> players;

    std::vector<class Vector2*> wallPositions;
     
    bool isGameClear = false;

    bool isMapInitialized = false;

    class DrawableActor* tempMap = nullptr;

    std::vector<class Player*> tempPlayers;

    bool isThreadWriting = false;

    std::queue<std::pair<int, Direction>> moveQueue;

    HANDLE mutex = 0;
};