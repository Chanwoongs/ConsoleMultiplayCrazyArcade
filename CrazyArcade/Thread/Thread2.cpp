#include <iostream>
#include <windows.h>
#include <process.h>
unsigned WINAPI ThreadFunc(void* arg);

int main(int argc, char* argv[])
{
    HANDLE hThread;
    DWORD wr;
    unsigned threadID;
    int param = 5;

    // 쓰레드의 메인 함수로 ThreadFunc를 ThreadFunc의 변수 param의 주소값을 전달 하면서 쓰레드의 생성을 요구
    hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)&param, 0, &threadID);
    if (hThread == 0)
    {
        puts("_beginthreadex() error");
        return -1;
    }

    //  WaitForSingleObject 함수 호출을 통해서 쓰레드의 종료를 대기
    if ((wr = WaitForSingleObject(hThread, INFINITE)) == WAIT_FAILED)
    {
        puts("thread wait error");
        return -1;
    }

    //  WaitForSingleObject 함수의 반환 값을 통해서 반환의 원인을 확인하고 있다.
    printf("wait result: %s \n", (wr == WAIT_OBJECT_0) ? "signaled" : "time-out");
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