#include "GameServer.h"

#define TEST 1

int main(int argc, char* argv[])
{
    CheckMemoryLeak();

#if !TEST
    if (argc != 2) 
    {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }
#endif 

    try
    {
#if TEST
        GameServer* server = new GameServer("9190");
#else
        GameServer* server = new GameServer(argv[1]);
#endif

        server->AcceptClients();

        delete server;
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Error: %s\n", ex.what());
        return -1;
    }


    return 0;
}
