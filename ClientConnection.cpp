#include "ClientConnection.h"
#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#endif
#include <iostream>

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
    std::cout << "\033[32mEnviando mensaje al cliente " << (ClientConnection::getName()) << ": " << buffer << " (" << length << " bytes)\033[37m\n";

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
    std::cout << "\033[34m[" << (ClientConnection::getName()) << "] " << buffer << " (" << bytesReceived << " bytes)\033[37m\n";

    return bytesReceived;
}

std::string ClientConnection::getName()
{
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    if (getpeername(socket, (sockaddr *)&addr, &addrLen) == 0)
    {
        char ipBuffer[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &addr.sin_addr, ipBuffer, sizeof(ipBuffer)) != nullptr)
        {
            int port = ntohs(addr.sin_port);
            return std::string(ipBuffer) + ":" + std::to_string(port);
        }
    }
    perror("getpeername() or inet_ntop() failed");
    return std::string();
}

void ClientConnection::close()
{
#ifdef _WIN32
    closesocket(socket);
#else
    ::close(socket);
#endif
}
