#pragma once

#include "Level/Level.h"

#include <queue>

class DrawableActor;
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

    const Vector2& GetRandomEmptyPosition();

	inline void SetClientId(int id) { clientId = id; }

    void MovePlayer(int playerId, enum class Direction direction);
	void RemovePlayer(int playerId);

    std::vector<Vector2> FindPath(const int playerId, const Vector2& end);
    
private:
    bool CheckGameClear();

private:

    DrawableActor* map = nullptr;

    std::vector<class Player*> players;

    std::vector<class Vector2*> wallPositions;

    std::vector<class Vector2*> emptyPositions;
     
    bool isGameClear = false;

    bool isMapInitialized = false;

    DrawableActor* tempMap = nullptr;

    std::vector<class Player*> tempPlayers;

    bool isThreadWriting = false;

    HANDLE mutex = 0;

    int clientId = 0;

    int mapWidth = 0;

    int mapHeight = 0;
};