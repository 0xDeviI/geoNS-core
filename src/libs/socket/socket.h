#ifndef GEONS_SOCKET_H
#define GEONS_SOCKET_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include "../core/core.h"


typedef struct sPeerInfo
{
    uchar *server_addr;
    ushort server_port;
    uchar *client_addr;
    ushort client_port;
} PeerInfo;

typedef enum eConnectionStatus 
{
    NOT_CONNECTED,
    CONNECTION_ESTABLISHED
} ConnectionStatus;


typedef struct sSocketConnection
{
    int fd;
    struct sSocketConnection *next;
    struct sPeerInfo peer_info;
    enum eConnectionStatus connection_status;
    size_t buffer_size_limit;
    char *buffer;
    llint buffer_size;
} SocketConnection;

typedef ssize_t (ServerCallback)(void *args, ...);


typedef struct sSocketServer
{
    int fd;
    struct sockaddr_in address;
    struct sSocketConnection *connections;
    int addrlen;
    ushort port;
    uchar *server_addr;
    uchar is_alive: 1;
    thread thread;
    ServerCallback *callback;
    size_t buffer_size_per_client;
    void *meta;
} SocketServer;


typedef struct sClientData
{
    SocketConnection **head;
    SocketConnection *current;
    ServerCallback *server_callback;
    void *meta;
} ClientData;


uchar is_my_ip(const uchar *ip);
void add_connection(SocketConnection **head, SocketConnection *connection);
uchar remove_connection(SocketConnection **head, SocketConnection *connection);
SocketServer *open_server_socket(uchar *server, ushort port, void *meta);
void handle_server_socket(SocketServer *server, ServerCallback *server_callback);
void kill_socket(int fd);
void kill_socket_server(SocketServer *server);
void *handle_client(void *arg);
SocketServer *connect_to_socket_server(uchar *server_addr, ushort port);
size_t send_message(int fd, uchar *message, size_t message_length, int flags);
size_t recv_message(int fd, void *buffer, size_t buffer_size, int flags);



#endif // !GEONS_SOCKET_H