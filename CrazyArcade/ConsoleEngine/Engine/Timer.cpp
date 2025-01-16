#include "PrecompiledHeader.h"
#include "Timer.h"

Timer::Timer(float targetTime)
    : targetTime(targetTime)
{
}

void Timer::Update(float deltaTime)
{
    elapsedTime += deltaTime;
}

void Timer::Reset()
{
    elapsedTime = 0.0f;
}
