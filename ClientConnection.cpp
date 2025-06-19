#include "ClientConnection.h"
#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <errno.h>
#endif

ClientConnection::ClientConnection(SOCKET socket)
{
    this->socket = socket;
}

int ClientConnection::send(const char *buffer, int length)
{
    int bytesSent = ::send(socket, buffer, length, 0);
    if (bytesSent < 0)
    {
        perror("send() failed");
        return -1;
    }
    return bytesSent;
}

int ClientConnection::receive(char *buffer, int length)
{
    int bytesReceived = ::recv(socket, buffer, length, 0);
    if (bytesReceived < 0)
    {
        perror("recv() failed");
        return -1;
    }
    return bytesReceived;
}

void ClientConnection::close()
{
#ifdef _WIN32
    closesocket(socket);
#else
    ::close(socket);
#endif
}
