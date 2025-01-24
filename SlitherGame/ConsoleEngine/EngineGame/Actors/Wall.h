#pragma once

#include "Actor/DrawableActor.h"

class ENGINE_API Wall : public DrawableActor
{
    RTTI_DECLARATIONS(Wall, DrawableActor)

public:
    Wall(const Vector2& position);
};