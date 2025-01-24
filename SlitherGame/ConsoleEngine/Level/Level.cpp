#include "PrecompiledHeader.h"

#include "Level.h"
#include "Actor/Actor.h"

Level::Level()
{
}

Level::~Level()
{
	// 메모리 해제
    for (Actor* actor : actors)
    {
        delete actor;
    }
}

void Level::Update(float deltaTime)
{
	// 레벨에 포함된 액터를 순회하면서 Update 함수 호출
    for (Actor* actor : actors)
    {
        if (!actor->isActive || actor->isExpired)
        {
            continue;
        }
        actor->Update(deltaTime);
    }
}

void Level::Draw()
{	
    for (Actor* actor : actors)
    {
        // 액터가 비활성화 상태이거나, 삭제 요청된 경우 건너뛰기.
        if (!actor->isActive || actor->isExpired)
        {
            continue;
        }

        actor->Draw();
    }
}

void Level::AddActor(Actor* newActor)
{
    addRequestedActor.push_back(newActor);
}

void Level::ProcessAddedAndDestroyActor()
{
    // 삭제 요청된 액터 처리
    for (auto it = actors.begin(); it != actors.end(); )
    {
        if ((*it)->isExpired)
        {
            delete *it;
            it = actors.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 추가 요청된 액터 처리
    if (addRequestedActor.size())
    {
        for (auto& actor : addRequestedActor)
        {
            actors.push_back(actor);
        }
        
        addRequestedActor.clear();
    }
}