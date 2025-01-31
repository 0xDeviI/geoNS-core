#include "server.h"
#include "../geonsp/geonsp.h"
#include "../logger/logger.h"
#include "../config/config.h"


Node INIT_NODES[] = {
    {
        .id=0, 
        .server_addr=DEFAULT_GEONS_SERVER_ADDR, 
        .node_gateway=DEFAULT_NODE_GATEWAY_PORT, 
        .data_gateway=DEFAULT_DATA_GATEWAY_PORT, 
        .status="active"
    }
};


uchar connect_localdb_node_servers() {
    Database *db = db_open(LOCAL_DB);
    if (db == NULL) {
        return 0;
    }
    db_connect(db);
    Node *active_nodes[MAX_ACTIVE_NODES];
    char nodes = get_all_active_nodes(db, active_nodes, MAX_ACTIVE_NODES);
    msglog(DEBUG, "Connecting to localdb nodes.");
    if (nodes == -1) {
        msglog(WARNING, "No localdb node available. Skipping.");
        db_disconnect(db);
        return 0;
    }


    Node source_node = {
        .id=0,
        .server_addr="",
        .node_gateway=CONFIG->node_gateway_port,
        .data_gateway=CONFIG->data_gateway_port,
        .status="active"
    };
    strncpy(source_node.server_addr, CONFIG->geons_server_addr, sizeof(source_node.server_addr));

    for (uchar i = 0; i < nodes; i++) {
        Node *destination_node = active_nodes[i];
        handle_node_info_exchange(db, &source_node, destination_node, 1);
        free(destination_node);
    }
    
    msglog(DEBUG, "Communication with localdb nodes success.");
    db_disconnect(db);
    return 1;
}


uchar connect_init_node_servers() {
    uchar size_of_init_nodes = sizeof(INIT_NODES) / sizeof(INIT_NODES[0]);
    if (size_of_init_nodes > 0) {
        msglog(DEBUG, "Connecting to init nodes.");
        Database *db = db_open(LOCAL_DB);
        if (db == NULL) {
            return 0;
        }
        db_connect(db);
        Node source_node = {
            .id=0,
            .server_addr="",
            .node_gateway=CONFIG->node_gateway_port,
            .data_gateway=CONFIG->data_gateway_port,
            .status="active"
        };
        strncpy(source_node.server_addr, CONFIG->geons_server_addr, sizeof(source_node.server_addr) - 1);
        source_node.server_addr[sizeof(source_node.server_addr)] = '\0';

        for (uchar i = 0; i < size_of_init_nodes; i++) {
            Node *destination_node = &INIT_NODES[i];
            handle_node_info_exchange(db, &source_node, destination_node, 0);
        }
        db_disconnect(db);
        msglog(DEBUG, "Communication with init nodes finished.");
    }
    else
        msglog(WARNING, "No init node available. Skipping to localdb nodes.");
    return connect_localdb_node_servers();
}


GeoNSServer *create_geons_server() {
    msglog(DEBUG, "Creating geoNS server.");
    GeoNSServer *server = (GeoNSServer *) memalloc(sizeof(GeoNSServer));
    if (server == NULL) {
        perror("Memory error");
        return NULL;
    }


    msglog(DEBUG, "Starting decentralization communication.");
    // initializing decentralization
    if (!connect_init_node_servers()) {
        msglog(ERROR, "Decentralization communication failed.");
        kill_geons_server(server);
        return NULL;
    }
    msglog(DEBUG, "Decentralization communication finished.");
    
    server->ledger_db = db_open(LEDGER_DB);
    if (server->ledger_db == NULL) {
        kill_geons_server(server);
        return NULL;
    }

    server->local_db = db_open(LOCAL_DB);
    if (server->local_db == NULL) {
        kill_geons_server(server);
        return NULL;
    }

    // creating node socket server
    server->node_gateway_server = open_server_socket(CONFIG->geons_server_addr, CONFIG->node_gateway_port);
    if (server->node_gateway_server == NULL) {
        msglog(ERROR, "Creating node server failed on %s:%d", CONFIG->geons_server_addr, CONFIG->node_gateway_port);
        kill_geons_server(server);
        return NULL;
    }
    // TEMP: should be minitored for removal or edition
    server->node_gateway_server->buffer_size_per_client = MAX_GEONSP_BUFFER_SIZE;
    handle_server_socket(server->node_gateway_server, &node_server_callback);

    // creating data socket server (SHOULD CHECK FOR NULL SERVER SOCKET!)
    // server->data_gateway_server = open_server_socket(CONFIG->geons_server_addr, CONFIG->data_gateway_port);
    // handle_server_socket(server->data_gateway_server);

    // TODO: setting up client API
    // .....

    // connecting databases
    db_connect(server->ledger_db);
    db_connect(server->local_db);

    printf("geoNS-Core is now running.\n");
    return server;
}


void kill_geons_server(GeoNSServer *server) {
    if (server != NULL) {
        msglog(DEBUG, "Killing GeoNSServer.");
        // disconnecting databases
        db_disconnect(server->ledger_db);
        db_disconnect(server->local_db);
        msglog(DEBUG, "Databases disconnected.");

        // killing socket servers
        kill_socket_server(server->node_gateway_server);
        // kill_socket_server(server->data_gateway_server);
        
        free(server);
        server = NULL;
        msglog(DEBUG, "GeoNSServer shut down.");
    }
}