#pragma once

#include "Actor/DrawableActor.h"

class GameLevel;
class ENGINE_API Player : public DrawableActor
{
    RTTI_DECLARATIONS(Player, DrawableActor)

public:
    Player() = default;
    Player(int id, const Vector2& position, GameLevel* level, bool isSeeker = false);

    virtual void Serialize(char* buffer, size_t& size) override;
    virtual void Deserialize(const char* buffer, size_t& size) override;

    virtual void Update(float deltaTime) override;
    virtual void Draw() override;
    void Initialize(const int clientId);

    inline int Id() const { return id; }
    inline bool IsSeeker() const { return isSeeker; }
    inline void SetToSeeker() { isSeeker = true; }

private:
    // 게임 레벨을 참조하는 변수
    GameLevel* refLevel = nullptr;

    int id = 0;

    bool isSeeker = false;
};