#include <iostream>
#include <windows.h>
#include <process.h>

#define NUM_THREAD 50
unsigned WINAPI Read(void* arg);
unsigned WINAPI Accu(void* arg);

static HANDLE semOne;
static HANDLE semTwo;
static int num;

int main(int argc, char* argv[])
{
    HANDLE hThread1, hThread2;
    semOne = CreateSemaphore(NULL, 0, 1, NULL);
    semTwo = CreateSemaphore(NULL, 1, 1, NULL);

    hThread1 = (HANDLE)_beginthreadex(NULL, 0, Read, NULL, 0, NULL);
    hThread2 = (HANDLE)_beginthreadex(NULL, 0, Accu, NULL, 0, NULL);

    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);

    CloseHandle(hThread1);
    CloseHandle(hThread2);

    return 0;
}

unsigned __stdcall Read(void* arg)
{
    for (int i = 0; i < 5; i++)
    {
        fputs("Input num: ", stdout);
        WaitForSingleObject(semTwo, INFINITE);
        scanf_s("%d", &num);
        ReleaseSemaphore(semOne, 1, NULL);
    }
    return 0;
}

unsigned __stdcall Accu(void* arg)
{
    int sum = 0;
    for (int i = 0; i < 5; i++)
    {
        WaitForSingleObject(semOne, INFINITE);
        sum += num;
        ReleaseSemaphore(semTwo, 1, NULL);
    }
    printf("Result: %d \n", num);
    return 0;
}
