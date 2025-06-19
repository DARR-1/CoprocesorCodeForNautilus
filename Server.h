#pragma once

#include <winsock2.h>
#include <string>
#include <cstring>
#include <cstdlib>

class Server
{
public:
    Server(u_short port, std::string hostname);
    Server(u_short port);
    ~Server();
    int initialize();
    int listen();
    int accept();
    int send(const char *buffer, int length);
    int receive(char *buffer, int length);
    int close();

private:
    WSADATA wsaData;
    SOCKET ListenSocket, AcceptSocket;
    u_short port = 27015;
    std::string hostname;
    std::string ip; // ahora es std::string
    sockaddr_in service;
    int error;
};