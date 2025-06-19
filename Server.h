#pragma once

#include <string>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
    #include <winsock2.h>
    #include <Ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>

    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    typedef int SOCKET;
#endif

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
#ifdef _WIN32
    WSADATA wsaData;
#endif
    SOCKET ListenSocket, AcceptSocket;
    u_short port = 27015;
    std::string hostname;
    std::string ip;
    sockaddr_in service;
    int error;
};