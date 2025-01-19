#include "GameServer.h"

#define TEST 1

int main(int argc, char* argv[])
{
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
        GameServer server("9190");
#else
        GameServer server(argv[1]);
#endif

        server.AcceptClients();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Error: %s\n", ex.what());
        return -1;
    }

    return 0;
}
