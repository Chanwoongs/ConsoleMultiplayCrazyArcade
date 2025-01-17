#pragma once

#include "Core.h"
#include "RTTI.h"
#include "Container/List.h"

// 전방 선언
class Actor;

class ENGINE_API Level : public RTTI
{
    // RTTI 정의
    RTTI_DECLARATIONS(Level, RTTI)

public:
	Level();
	virtual ~Level();

	virtual void Update(float deltaTime);
	virtual void Draw();

	// 액터 추가 함수
	void AddActor(Actor* newActor);

    // 삭제 요청이 된 액터를 정리하는 함수
    //void DestroyActor();
    void ProcessAddedAndDestroyActor();

protected:
    // 게임 공간에 배치되는 물체(Actor) 배열.
    List<Actor*> actors;

    // 추가 요청된 액터
    // @TODO: 현재 하나의 액터만 추가 가능, 동적 배열로 만들어 생성할 개수 제한
    Actor* addRequestedActor = nullptr;
};