#include "PrecompiledHeader.h"

#include "Engine.h"

#include <iostream>
#include <Windows.h>
#include <time.h>

#include "Level/Level.h"
#include "Actor/Actor.h"
#include "Render/ScreenBuffer.h"

// 스태틱 변수 초기화
Engine* Engine::Instance = nullptr;

// 콘솔 창 메시지 콜백 함수.
BOOL WINAPI MessageProcessor(DWORD message)
{
    switch (message)
    {
    case CTRL_CLOSE_EVENT:
        Engine::Get().QuitGame();
        return true;

    default:
        return false;
    }
}

Engine::Engine()
    : quit(false), mainLevel(nullptr), screenSize(80, 30), levelChangeRequested(false), pendingLevel(nullptr)
{
    // 윈도우 창 버튼 입력 핸들러 등록.
    SetConsoleCtrlHandler(MessageProcessor, true);

    // 랜덤 시드 설정
    srand((unsigned int)time(nullptr));

    // 싱글톤 객체 설정
    Instance = this;

    // 기본 타겟 프레임 속도 설정
    SetTargetFrameRate(60.0f);

    // 화면 버퍼 초기화.
    // 1. 버퍼 크기 할당.
    imageBuffer = new CHAR_INFO[(screenSize.x + 1) * screenSize.y + 1];


    // 버퍼 초기화.
    ClearImageBuffer();

    // 두 개의 버퍼 생성 (버퍼를 번갈아 사용하기 위해-더블 버퍼링).
    COORD size = { (short)screenSize.x, (short)screenSize.y };
    //renderTargets[0] = new ScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE), size);
    renderTargets[0] = new ScreenBuffer(size);
    renderTargets[1] = new ScreenBuffer(size);

    // 스왑 버퍼.
    Present();
}

Engine::~Engine()
{
	if (mainLevel != nullptr)
	{
		delete mainLevel;
	}

    // 클리어 버퍼 삭제.
    delete[] imageBuffer;

    // 화면 버퍼 삭제.
    delete renderTargets[0];
    delete renderTargets[1];
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

        // 레벨 변경 요청이 있는지 확인
        if (levelChangeRequested && pendingLevel != nullptr)
        {
            // 여기서 안전하게 레벨 변경
            if (mainLevel)
            {
                delete mainLevel;
            }

            mainLevel = pendingLevel;
            pendingLevel = nullptr;
            levelChangeRequested = false;
        }

		// 프레임 확인
        if (deltaTime >= targetOneFrameTime)
        {
            // 입력 처리 (현재 키의 눌림 상태 확인)
            ProcessInput();

            // 업데이트 가능한 상태에서만 프레임 업데이트 처리
            CheckInput();
            Update(deltaTime);
            Draw();

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
		}
	}
}

void Engine::LoadLevel(Level* newLevel)
{
	// 기존 레벨이 있다면 삭제 후 교체

	// 메인 레벨 설정
	mainLevel = newLevel;
}

void Engine::RequestLevelChange(Level* newLevel)
{
    pendingLevel = newLevel;
    levelChangeRequested = true;
}

void Engine::AddActor(Actor* newActor)
{
    // 예외 처리
    if (mainLevel == nullptr)
    {
        return;
    }

    mainLevel->AddActor(newActor);
}

void Engine::DestroyActor(Actor* targetActor)
{
    if (mainLevel == nullptr)
    {
        return;
    }

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

Vector2 Engine::MousePosition() const
{
    return mousePosition;
}

void Engine::QuitGame()
{
	quit = true;

    if (onQuitCallback)
    {
        onQuitCallback();
    }
}

void Engine::SetOnQuitCallBack(std::function<void()> onQuitCallback)
{
    this->onQuitCallback = onQuitCallback;
;}

Engine& Engine::Get()
{
	return *Instance;
}

void Engine::ProcessInput()
{
    static HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    static int flag = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS;
    if (!SetConsoleMode(inputHandle, flag))
    {
        std::cerr << "Failed to set console mode" << std::endl;
        return;
    }

    INPUT_RECORD record;
    DWORD events;
    while (PeekConsoleInput(inputHandle, &record, 1, &events) && events > 0)
    {
        if (ReadConsoleInput(inputHandle, &record, 1, &events))
        {
            switch (record.EventType)
            {
            case KEY_EVENT:
            {
                // 키 눌림 상태 업데이트.
                if (record.Event.KeyEvent.bKeyDown)
                {
                    keyState[record.Event.KeyEvent.wVirtualKeyCode].isKeyDown = true;
                }
                // 키 눌림 해제 상태 업데이트.
                else
                {
                    keyState[record.Event.KeyEvent.wVirtualKeyCode].isKeyDown = false;
                }
            }
            break;

            case MOUSE_EVENT:
            {
                // 마우스 커서 위치 업데이트.
                mousePosition.x = record.Event.MouseEvent.dwMousePosition.X;
                mousePosition.y = record.Event.MouseEvent.dwMousePosition.Y;

                // 마우스 왼쪽 버튼 클릭 상태 업데이트.
                keyState[VK_LBUTTON].isKeyDown
                    = (record.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;

                // 마우스 오른쪽 버튼 클릭 상태 업데이트.
                keyState[VK_RBUTTON].isKeyDown
                    = (record.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) != 0;
            }
            break;
            }
        }
    }
}

void Engine::CheckInput()
{
    if (Engine::Get().GetKeyDown(VK_ESCAPE))
    {
        Engine::Get().QuitGame();
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

void Engine::Draw(const Vector2& position, const char* image, Color color)
{
    std::string imageStr(image);
    std::vector<std::string> drawData;
    size_t pos = 0;
    std::string token;
    while ((pos = imageStr.find('\n')) != std::string::npos) 
    {
        token = imageStr.substr(0, pos);
        drawData.push_back(token);
        imageStr.erase(0, pos + 1);
    }
    drawData.push_back(imageStr);

    for (int y = 0; y < (int)drawData.size(); ++y)
    {
        const std::string& drawString = drawData[y];
        for (int x = 0; x < (int)drawString.length(); ++x)
        {
            int index = ((position.y + y) * screenSize.x) + position.x + x;
            imageBuffer[index].Char.AsciiChar = drawString[x];
            imageBuffer[index].Attributes = (unsigned long)color;
        }
    }
}

void Engine::SetTargetFrameRate(float targetFrameRate)
{
    this->targetFrameRate = targetFrameRate;
    targetOneFrameTime = 1.0f / targetFrameRate;
}

void Engine::SavePreviousKeyStates()
{
	for (int i = 0; i < 255; i++)
	{
		keyState[i].wasKeyDown = keyState[i].isKeyDown;
	}
}

void Engine::Clear()
{
    ClearImageBuffer();
    //GetRenderer()->Clear();
}

void Engine::Draw()
{
    // 화면 지우기.
    Clear();

    // 레벨 그리기.
    if (mainLevel != nullptr)
    {
        mainLevel->Draw();
    }

    // 백버퍼에 데이터 쓰기.
    GetRenderer()->Draw(imageBuffer);

    // 프론트<->백 버퍼 교환.
    Present();
}

void Engine::Present()
{
    // Swap Buffer.
    SetConsoleActiveScreenBuffer(GetRenderer()->buffer);
    currentRenderTargetIndex = 1 - currentRenderTargetIndex;
}

void Engine::ClearImageBuffer()
{
    // 버퍼 덮어쓰기.
    for (int y = 0; y < screenSize.y; ++y)
    {
        // 버퍼 덮어쓰기.
        for (int x = 0; x < screenSize.x + 1; ++x)
        {
            auto& buffer = imageBuffer[(y * (screenSize.x + 1)) + x];
            buffer.Char.AsciiChar = ' ';
            buffer.Attributes = 0;
        }

        // 각 줄 끝에 개행 문자 추가.
        auto& buffer = imageBuffer[(y * (screenSize.x + 1)) + screenSize.x + 1];
        buffer.Char.AsciiChar = '\n';
        buffer.Attributes = 0;
    }

    // 마지막에 널 문자 추가.
    auto& buffer = imageBuffer[(screenSize.x + 1) * screenSize.y];
    buffer.Char.AsciiChar = '\0';
    buffer.Attributes = 0;
}