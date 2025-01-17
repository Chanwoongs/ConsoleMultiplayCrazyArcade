#pragma once

#include "Actor/DrawableActor.h"

class Box : public DrawableActor
{
    RTTI_DECLARATIONS(Box, DrawableActor)

public:
    Box(const Vector2& position);
};