#include <iostream>
#include <windows.h>
#include <process.h>
unsigned WINAPI ThreadFunc(void* arg);

int main(int argc, char* argv[])
{
    HANDLE hThread;
    unsigned threadID;
    int param = 5;

    // 쓰레드의 메인 함수로 ThreadFunc를 ThreadFunc의 변수 param의 주소값을 전달 하면서 쓰레드의 생성을 요구
    hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);
    if (hThread == 0)
    {
        puts("_beginthreadex() error");
        return -1;
    }
    Sleep(3000);
    puts("end of main");

    return 0;
}

// WINAPI는 _beginthreadex 함수가 요구하는 호출 규약
unsigned WINAPI ThreadFunc(void* arg)
{
    int i;
    int cnt = *((int*)arg);
    for (int i = 0; i < cnt; ++i)
    {
        Sleep(1000);
        puts("running thread");
    }

    return 0;
}