#pragma once

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#define SOCKET int
#endif

class ClientConnection
{
private:
    SOCKET socket;

public:
    ClientConnection(SOCKET socket);
    int send(const char *buffer, int length);
    int receive(char *buffer, int length);
    void close();

    SOCKET getSocket() const { return socket; }
};
