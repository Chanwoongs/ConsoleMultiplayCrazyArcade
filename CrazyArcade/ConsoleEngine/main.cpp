#include "PrecompiledHeader.h"

#include <iostream>

#include "Engine/Engine.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Engine engine;

	//////////////////////
	// 로그 시스템 초기화
	//////////////////////

	engine.Run();
}