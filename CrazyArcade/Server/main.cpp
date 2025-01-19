#include "GameServer.h"

int main(int argc, char* argv[])
{
    if (argc != 2) 
    {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }

    try
    {
        GameServer server(argv[1]);

        server.AcceptClients();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Error: %s\n", ex.what());
        return -1;
    }

    return 0;
}
