#include <iostream>
#include "GameClient.h"

#define TEST 1

int main(int argc, char* argv[])
{
#if !TEST
    if (argc != 4)
    {
        printf("Usage: %s <IP> <PORT> <USERNAME>\n", argv[0]);
        exit(0);
    }
    GameClient client(argv[2], argv[3]);
#else
    GameClient client("127.0.0.1", "9190");
#endif

    client.RunThreads();

    return 0;
}