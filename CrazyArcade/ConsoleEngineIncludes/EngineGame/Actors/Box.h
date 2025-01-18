#pragma once

#include "Actor/DrawableActor.h"

class ENGINE_API Box : public DrawableActor
{
    RTTI_DECLARATIONS(Box, DrawableActor)

public:
    Box(const Vector2& position);
};