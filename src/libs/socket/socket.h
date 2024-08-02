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
    uchar server_addr[MAX_IPV6_LENGTH];
    ushort server_port;
    uchar client_addr[MAX_IPV6_LENGTH];
    ushort client_port;
} PeerInfo;


typedef void (ServerCallback)(int fd, uchar *request, PeerInfo *peer_info);


typedef enum eConnectionStatus 
{
    NOT_CONNECTED,
    CONNECTION_ESTABLISHED
} ConnectionStatus;


typedef struct sSocketConnection
{
    uchar buffer[SOCKET_MAX_BUFFER_SIZE];
    int fd;
    struct sSocketConnection *next;
    enum eConnectionStatus connection_status;
    struct sPeerInfo peer_info;
} SocketConnection;


typedef struct sSocketServer
{
    int fd;
    struct sockaddr_in address;
    struct sSocketConnection *connections;
    int addrlen;
    ushort port;
    uchar *server_addr;
    uchar is_alive;
    thread thread;
} SocketServer;


typedef struct sClientData
{
    SocketConnection **head;
    SocketConnection *current;
    ServerCallback *callback;
} ClientData;


uchar is_my_ip(const uchar *ip);
void add_connection(SocketConnection **head, SocketConnection *connection);
uchar remove_connection(SocketConnection **head, SocketConnection *connection);
SocketServer *open_server_socket(uchar *server, ushort port);
void handle_server_socket(SocketServer *server);
void kill_socket(int fd);
void kill_socket_server(SocketServer *server);
void *handle_client(void *arg);
SocketServer *connect_to_socket_server(uchar *server_addr, ushort port);
ssize_t send_message(int fd, uchar *message, size_t message_length, int flags);
ssize_t recv_message(int fd, void *buffer, size_t buffer_size, int flags);



#endif // !GEONS_SOCKET_H