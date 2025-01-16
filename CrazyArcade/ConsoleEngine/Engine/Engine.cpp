#include "PrecompiledHeader.h"

#include "Engine.h"

#include <iostream>
#include <Windows.h>

#include "Level/Level.h"
#include "Actor/Actor.h"

// 스태틱 변수 초기화
Engine* Engine::Instance = nullptr;

Engine::Engine()
    : quit(false), mainLevel(nullptr), screenSize(80, 25)
{
    // 랜덤 시드 설정
    srand((unsigned int)time(nullptr));

	// 싱글톤 객체 설정
	Instance = this;

    // 기본 타겟 프레임 속도 설정
    SetTargetFrameRate(60.0f);

    // 화면 지울 때 사용할 버퍼 초기화
    // 1. 버퍼 크기 할당
    emptyStringBuffer = new char[screenSize.y * (screenSize.x + 1) + 1];

    // 버퍼 덮어 쓰기
    memset(emptyStringBuffer, ' ', screenSize.y * (screenSize.x + 1) + 1);

    // 2. 값 할당
    for (int y = 0; y < screenSize.y; ++y)
    {
        for (int x = 0; x < screenSize.x; x++)
        {
            emptyStringBuffer[y * (screenSize.x + 1) + x] = ' ';
        }

        // 각 줄 끝에 개행 문자 추가
        emptyStringBuffer[y * (screenSize.x + 1) + screenSize.x] = '\n';
    }
    
    // 마지막에 널 문자 추가
    emptyStringBuffer[screenSize.y * (screenSize.x + 1)] = '\0';

    // 디버깅
#if _DEBUG
    char buffer[2048];
    strcpy_s(buffer, 2048, emptyStringBuffer);
#endif
}

Engine::~Engine()
{
	if (mainLevel != nullptr)
	{
		delete mainLevel;
	}

    delete[] emptyStringBuffer;
}

void Engine::Run()
{
	// 시작 타임 스탬프
	// timeGetTime 함수는 ms(1/1000초) 단위
	//unsigned long currentTime = timeGetTime();
	//unsigned long previousTIme = 0;

	// CPU 시계 사용
	// 시스템 시계 -> 고해상도 카운터
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency); // CPU가 가지고 있는 hz 반환

	// 시작 시간 및 이전 시간을 위한 변수
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	// long long == __int64 == int64_t
	int64_t currentTime = time.QuadPart; // 64bit 한 번에 저장
	int64_t previousTime = currentTime;

	// Game Loop
	while (true)
	{
		if (quit)
		{
			break;
		}

		// 현재 프레임 시간 저장
		// currentTime = timeGetTime();
		QueryPerformanceCounter(&time);
		currentTime = time.QuadPart;

		// 프레임 시간 계산 (1000만으로 나눠서 초를 구함)
		float deltaTime = static_cast<float>(currentTime - previousTime) / 
			static_cast<float>(frequency.QuadPart);

		// 프레임 확인
        if (deltaTime >= targetOneFrameTime)
        {
            // 입력 처리 (현재 키의 눌림 상태 확인)
            ProcessInput();

            // 업데이트 가능한 상태에서만 프레임 업데이트 처리
            if (shouldUpdate)
            {
                Update(deltaTime);
                Draw();
            }

			// 키 상태 저장
			SavePreviousKeyStates();

			// 이전 프레임 시간 저장
			previousTime = currentTime;

            // 액터 정리 (삭제 요청된 액터들 정리)
            if (mainLevel)
            {
                //mainLevel->DestroyActor();
                mainLevel->ProcessAddedAndDestroyActor();
            }

            // 프레임 활성화
            shouldUpdate = true;
		}
	}
}

void Engine::LoadLevel(Level* newLevel)
{
	// 기존 레벨이 있다면 삭제 후 교체

	// 메인 레벨 설정
	mainLevel = newLevel;
}

void Engine::AddActor(Actor* newActor)
{
    // 예외 처리
    if (mainLevel == nullptr)
    {
        return;
    }

    shouldUpdate = false;
    mainLevel->AddActor(newActor);
}

void Engine::DestroyActor(Actor* targetActor)
{
    if (mainLevel == nullptr)
    {
        return;
    }

    shouldUpdate = false;
    targetActor->Destroy();
}

void Engine::SetCursorType(CursorType cursorType)
{
    // 1. 커서 설정 구조체 설정
    CONSOLE_CURSOR_INFO info = { };

    switch (cursorType)
    {
    case CursorType::NoCursor:
        info.dwSize = 1; // 너비를 0으로 하면 무시한다.
        info.bVisible = FALSE;
        break;

    case CursorType::SolidCursor:
        info.dwSize = 100;
        info.bVisible = TRUE;
        break;

    case CursorType::NormalCursor:
        info.dwSize = 20;
        info.bVisible = TRUE;
        break;
    }

    // 2. 설정
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void Engine::SetCursorPosition(const Vector2& position)
{
    SetCursorPosition(position.x, position.y);
}

void Engine::SetCursorPosition(int x, int y)
{
    static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { static_cast<short>(x), static_cast<short>(y) };
    SetConsoleCursorPosition(handle, coord);
}

void Engine::SetTargetFrameRate(float targetFrameRate)
{
    this->targetframeRate = targetFrameRate;

    targetOneFrameTime = 1.0f / targetFrameRate;
}

bool Engine::GetKey(int key)
{
	return keyState[key].isKeyDown;
}

bool Engine::GetKeyDown(int key)
{
	return keyState[key].isKeyDown && !keyState[key].wasKeyDown;
}

bool Engine::GetKeyUp(int key)
{
	return !keyState[key].isKeyDown && keyState[key].wasKeyDown;
}

void Engine::QuitGame()
{
	quit = true;
}

Engine& Engine::Get()
{
	return *Instance;
}

void Engine::ProcessInput()
{
	for (int i = 0; i < 255; i++)
	{
		// (GetAsyncKeyState(i) & 0x8000) 현재 프레임에 눌렸는지 저장
		keyState[i].isKeyDown = (GetAsyncKeyState(i) & 0x8000) ? true : false;
	}
}

void Engine::Update(float deltaTime)
{
	// 레벨 업데이트
	if (mainLevel != nullptr)
	{
		mainLevel->Update(deltaTime);
	}
}

void Engine::Clear()
{
    // 화면의 (0, 0)으로 이동.
    SetCursorPosition(0, 0);

    //// 화면 지우기.
    //int height = 25;
    //for (int i = 0; i < height; i++)
    //{
    //    Log("                              \n");
    //}
    
    // 화면 지우기
    std::cout << emptyStringBuffer;

    // 화면의 (0, 0)으로 이동.
    SetCursorPosition(0, 0);
}

void Engine::Draw()
{
    // 화면 지우기
    //Clear();

	// 레벨 그리기
	if (mainLevel != nullptr)
	{
		mainLevel->Draw();
	}
}

void Engine::SavePreviousKeyStates()
{
	for (int i = 0; i < 255; i++)
	{
		keyState[i].wasKeyDown = keyState[i].isKeyDown;
	}
}
