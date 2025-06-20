#pragma once

#include <string>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
typedef int SOCKET;
#endif

#include "ClientConnection.h"

class Server
{
private:
    SOCKET ListenSocket;
    std::string hostname;
    std::string ip;
    u_short port;
    sockaddr_in service;
    int error;

#ifdef _WIN32
    WSADATA wsaData;
#endif

public:
    Server(u_short port, std::string hostname = "localhost");
    ~Server();
    int initialize();
    int listen();
    ClientConnection accept();
};
