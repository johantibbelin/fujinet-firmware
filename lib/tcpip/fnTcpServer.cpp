/* Modified version of ESP-Arduino fnTcpServer.cpp/h */

#include "fnTcpServer.h"

#include <lwip/netdb.h>

#include "../../include/debug.h"


// Configures a listening TCP socket on given port
// Returns 0 for error, 1 for success.
int fnTcpServer::begin(uint16_t port)
{
    if (_listening) {
        Debug_println("TCP Server already listening. Aborting.");
        return 0;
    }

    if (port)
        _port = port;

    // Allocate a socket
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_sockfd < 0)
    {
        Debug_printf("fnTcpServer::begin failed to allocate socket, err %d\r\n", errno);
        return 0;
    }

    // Bind socket to our interface
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(_port);
    if (bind(_sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        Debug_printf("fnTcpServer::begin failed to bind socket, err %d\r\n", errno);
        return 0;
    }

    int enable = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        Debug_printf("fnTcpServer::begin failed to set SO_REUSEADDR, err %d\r\n", errno);
        return 0;
    }

    Debug_printf("Max clients is currently %u\r\n",_max_clients);

    // Now listen in on this socket
    if (listen(_sockfd, _max_clients) < 0)
    {
        Debug_printf("fnTcpServer::begin failed to listen on socket, err %d\r\n", errno);
        return 0;
    }

    // Switch to non-blocking mode
    if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) < 0)
    {
        Debug_printf("fnTcpServer::begin failed to set non-blocking mode. Closing down server. err %d\n", errno);
        stop();
        return 0;
    }

    _listening = true;
    _noDelay = false;
    _accepted_sockfd = -1;
    Debug_printf("TCP Server now listening on port %d.\n", _port);
    return 1;
}

// Returns true if a client has connected to the socket
bool fnTcpServer::hasClient()
{
    if (_accepted_sockfd >= 0)
        return true;

    struct sockaddr_in _client;
    int cs = sizeof(struct sockaddr_in);
    _accepted_sockfd = lwip_accept(_sockfd, (struct sockaddr *)&_client, (socklen_t *)&cs);

    if (_accepted_sockfd >= 0)
    {
        Debug_printf("TcpServer accepted connection from %s\r\n", inet_ntoa(_client.sin_addr.s_addr));
        return true;
    }

    return false;
}

// Returns a new fnTcpClient initialized with the client currently connected to the socket
// or disconnected (uninitialized) fnTcpClient
fnTcpClient fnTcpServer::available()
{
    // Return initialized fnTcpClient if we're not in listening mode
    if (!_listening)
        return fnTcpClient();

    // _accecpted_sockfd is set by hasClient() - use it and reset it's value if it hasn't been used
    int client_sock;
    if (_accepted_sockfd >= 0)
    {
        client_sock = _accepted_sockfd;
        _accepted_sockfd = -1;
    }
    else
    // Otherwise, try to get a new connection
    {
        struct sockaddr_in _client;
        int cs = sizeof(struct sockaddr_in);
        client_sock = lwip_accept(_sockfd, (struct sockaddr *)&_client, (socklen_t *)&cs);
    }

    // If we have a client, turn on SO_KEEPALIVE and TCP_NODELAY and return new fnTcpClient
    if (client_sock >= 0)
    {
        int val = 1;
        if (setsockopt(client_sock, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == ESP_OK)
        {
            val = _noDelay;
            if (setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == ESP_OK)
                return fnTcpClient(client_sock);
        }
    }

    // Return initialized fnTcpClient
    return fnTcpClient();
}

// Set both send and receive timeouts on the TCP socket
int fnTcpServer::setTimeout(uint32_t seconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval)) < 0)
        return -1;
    return setsockopt(_sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
}

// Closes listening socket
void fnTcpServer::stop()
{
    if (_sockfd > 0)
    {
        Debug_printf("fnTcpServer::stop(%d)\r\n", _sockfd);
        lwip_close(_sockfd);
        Debug_printf("close errno %d\r\n",errno);
        _sockfd = -1;
        _listening = false;
    }
}
