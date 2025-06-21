#pragma once

#include <stdio.h>
#include <string>
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
    std::string getName();
    void close();

    SOCKET getSocket() const { return socket; }
};
