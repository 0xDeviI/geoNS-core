#ifndef GEONS_SERVER_H
#define GEONS_SERVER_H 1

#include "../core/core.h"
#include "../socket/socket.h"
#include "../parson/parson.h"
#include "../db/db.h"
#include "../http/specs.h"


typedef struct sGeoNSServer
{
    Database *ledger_db;
    Database *local_db;
    SocketServer *node_gateway_server;
    HTTPServer *http_server;
} GeoNSServer;

typedef struct sNode
{
    uchar id;
    uchar server_addr[MAX_IPV6_LENGTH + 1];
    ushort node_gateway;
    uchar status[16];
} Node;


extern Node INIT_NODES[];


uchar connect_localdb_node_servers();
uchar connect_init_node_servers();
GeoNSServer *create_geons_server();
void kill_geons_server(GeoNSServer *server);

#endif // !GEONS_SERVER_H