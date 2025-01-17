#pragma once

#include <Actor/DrawableActor.h>

// 플레이어가 걸어다닐 수 있는 바닥
class Ground : public DrawableActor
{
    RTTI_DECLARATIONS(Ground, DrawableActor)

public:
    Ground(const Vector2& position);
};