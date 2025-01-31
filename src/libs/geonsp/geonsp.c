#include "geonsp.h"
#include "../logger/logger.h"


uchar handle_node_info_exchange(Database *db, Node *source_node, Node *destination_node, uchar remove_inactive_nodes) {
    // returns 2 when destination and source IP addresses are the same
    // returns 1 when exchange is successful
    // returns 0 when reaches an error
    if (is_my_ip(destination_node->server_addr))
        return 2;
    else {
        msglog(DEBUG, "Exchanging node info with %s:%d", destination_node->server_addr, destination_node->node_gateway);
        SocketServer *server = is_geons_host_available(
            destination_node->server_addr, 
            destination_node->node_gateway
        );
        if (server != NULL) {
            uchar buffer[MAX_GEONSP_BUFFER_SIZE];
            JSON_Value *request = construct_add_node_request(
                source_node->server_addr,
                source_node->node_gateway,
                source_node->data_gateway
            );
            uchar *request_payload = json_serialize_to_string(request);

            send_message(server->fd, request_payload, strlen(request_payload), 0);
            int message_length = recv_message(server->fd, buffer, sizeof(buffer), 0);
            buffer[message_length] = '\0';
            json_free_serialized_string(request_payload);
            json_value_free(request);

            // Getting list of other nodes from server using GET_NODES request
            request = construct_get_nodes_request();
            request_payload = json_serialize_to_string(request);

            send_message(server->fd, request_payload, strlen(request_payload), 0);
            message_length = recv_message(server->fd, buffer, sizeof(buffer), 0);
            buffer[message_length] = '\0';
            json_free_serialized_string(request_payload);
            json_value_free(request);

            request = json_parse_string(buffer);
            JSON_Array *nodes = json_object_get_array(
                json_value_get_object(request),
                "data"
            );
            uchar size_of_nodes = json_array_get_count(nodes);

            for (uchar i = 0; i < size_of_nodes; i++) {
                JSON_Object *node = json_array_get_object(nodes, i);
                uchar *server_addr = (uchar *)json_object_get_string(node, "server");
                if (!is_my_ip(server_addr)) {
                    ushort node_gateway = json_object_get_number(node, "node");
                    ushort data_gateway = json_object_get_number(node, "data");
                    uchar *status = (uchar *)json_object_get_string(node, "status");
                    uchar result = insert_new_node(db, server_addr, node_gateway, data_gateway);
                    if (!result)
                        msglog(ERROR, "Failed inserting new node %s:%d into localdb.", destination_node->server_addr, destination_node->node_gateway);
                    else if (result == 1)
                        msglog(INFO, "New node %s:%d registered into localdb.", destination_node->server_addr, destination_node->node_gateway);
                }
            }

            json_value_free(request);
            kill_socket_server(server);
            return 1;
        }
        else
        {
            // remove the current node
            if (remove_inactive_nodes) {
                if (remove_node(db, destination_node))
                    msglog(INFO, "Node %s:%d removed from localdb.", destination_node->server_addr, destination_node->node_gateway);
                else
                    msglog(ERROR, "Failed removing node %s:%d from localdb.", destination_node->server_addr, destination_node->node_gateway);
            }
            return 0;
        }
    }
}

JSON_Value *construct_base_geonsp_message(uchar *geonsp_message) {
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "method", geonsp_message);
    return json_value;
}

JSON_Value *construct_add_node_request(uchar *server_addr, ushort node_gateway_port, ushort data_gateway_port) {
    JSON_Value *json_value = construct_base_geonsp_message(GEONSP_MSG_ADD_NODE);
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_dotset_string(json_object, "data.server_addr", server_addr);
    json_object_dotset_number(json_object, "data.node_gateway_port", node_gateway_port);
    json_object_dotset_number(json_object, "data.data_gateway_port", data_gateway_port);
    return json_value;
}


JSON_Value *construct_get_nodes_request(void) {
    return construct_base_geonsp_message(GEONSP_MSG_GET_NODES);
}


JSON_Value *construct_client_hello_request(void) {
    return construct_base_geonsp_message(GEONSP_MSG_CLIENT_HELLO);
}


SocketServer *is_geons_host_available(uchar *server_addr, ushort node_gateway) {
    uchar command[100];
    
    JSON_Value *json_value = construct_client_hello_request();
    uchar *client_hello = json_serialize_to_string(json_value);
    snprintf(command, sizeof(command), "ping -c 1 %s > /dev/null 2>&1", server_addr);
    ushort is_host_available = !system(command);

    if (!is_host_available) {
        msglog(ERROR, "Init node %s is down or can't be observed.", server_addr);
        json_free_serialized_string(client_hello);
        json_value_free(json_value);
        return NULL;
    }

    SocketServer *server = connect_to_socket_server(server_addr, node_gateway);
    if (server == NULL) {
        json_free_serialized_string(client_hello);
        json_value_free(json_value);
        return NULL;
    }
    ushort is_geons_port_open = server != NULL;

    if (!is_host_available || !is_geons_port_open) {
        json_free_serialized_string(client_hello);
        json_value_free(json_value);
        kill_socket_server(server);
        return NULL;
    }

    uchar buffer[MAX_GEONSP_BUFFER_SIZE] = {0};

    send_message(server->fd, client_hello, strlen(client_hello), 0);
    json_free_serialized_string(client_hello);
    json_value_free(json_value);
        
    int message_length = recv_message(server->fd, buffer, sizeof(buffer), 0);
    buffer[message_length] = '\0';
    JSON_Value *response_value = json_parse_string(buffer);
    JSON_Object *response_json = json_value_get_object(response_value);
        
    if (!json_object_has_value(response_json, "status") || !json_object_has_value(response_json, "message")) {
        json_value_free(response_value);
        kill_socket_server(server);
        return NULL;
    }

    uchar *status = (uchar *) json_object_get_string(response_json, "status");
    if (strncmp(status, "success", strlen("success"))) {
        json_value_free(response_value);
        return NULL;
    }

    json_value_free(response_value);
    return server;
}


void server_proto_data_response(SocketConnection *connection, uchar is_success, uchar *message, JSON_Value *data) {
    JSON_Value *response_root = json_value_init_object();
    JSON_Object *response_json = json_value_get_object(response_root);
    json_object_set_string(response_json, "status", is_success ? "success" : "failed");
    json_object_set_string(response_json, "message", message);
    json_object_set_value(response_json, "data", data);
    uchar *response = json_serialize_to_string(response_root);
    msglog(DEBUG, 
        "Response[%s:%d -> %s:%d]:\n- %32s%s", 
        connection->peer_info.client_addr, connection->peer_info.client_port,
        connection->peer_info.server_addr, connection->peer_info.server_port,
        "", response
    );
    send_message(connection->fd, response, strlen(response), 0);
    json_free_serialized_string(response);
    json_value_free(response_root);
}


void server_proto_response(SocketConnection *connection, uchar is_success, uchar *message) {
    JSON_Value *response_root = json_value_init_object();
    JSON_Object *response_json = json_value_get_object(response_root);
    json_object_set_string(response_json, "status", is_success ? "success" : "failed");
    json_object_set_string(response_json, "message", message);
    uchar *response = json_serialize_to_string(response_root);
    msglog(DEBUG, 
        "Response[%s:%d -> %s:%d]:\n- %32s%s", 
        connection->peer_info.client_addr, connection->peer_info.client_port,
        connection->peer_info.server_addr, connection->peer_info.server_port,
        "", response
    );
    send_message(connection->fd, response, strlen(response), 0);
    json_free_serialized_string(response);
    json_value_free(response_root);
}


char node_server_callback(SocketConnection *connection) {
    if (strlen(connection->buffer) != 0) {
        msglog(DEBUG, 
            "Request[%s:%d -> %s:%d]:\n- %32s%s", 
            connection->peer_info.client_addr, connection->peer_info.client_port,
            connection->peer_info.server_addr, connection->peer_info.server_port,
            "", connection->buffer
        );
        JSON_Value *request_value = json_parse_string(connection->buffer);
        JSON_Object *request_json = json_value_get_object(request_value);
        if (json_object_has_value(request_json, "method")) {
            uchar *method = (uchar *) json_object_get_string(request_json, "method");
            if (!strncmp(method, GEONSP_MSG_GET_VERSION, strlen(GEONSP_MSG_GET_VERSION))) {
                uchar version[128];
                snprintf(version, sizeof(version), "GeoNS v%s %s", GEONS_VERSION, COMPILE_TIME);
                server_proto_response(connection, 1, version);
            }
            else if (!strncmp(method, GEONSP_MSG_GET_NODES, strlen(GEONSP_MSG_GET_NODES))) {
                Database *local_db = db_open(LOCAL_DB);
                if (local_db == NULL) {
                    server_proto_response(connection, 0, "Failed fetching nodes from database");
                }
                else {
                    db_connect(local_db);
                    Node *active_nodes[MAX_ACTIVE_NODES];
                    char nodes = get_all_active_nodes(local_db, active_nodes, MAX_ACTIVE_NODES);
                    if (nodes != -1) {
                        JSON_Value *json_value = json_value_init_array();
                        JSON_Array *json_array = json_value_get_array(json_value);
                        for (char i = 0; i < nodes; i++) {
                            Node *node = active_nodes[i];
                            if (node != NULL) {
                                JSON_Value *node_json_value = json_value_init_object();
                                JSON_Object *node_json_object = json_value_get_object(node_json_value);
                                json_object_set_string(node_json_object, "server", node->server_addr);
                                json_object_set_number(node_json_object, "node", node->node_gateway);
                                json_object_set_number(node_json_object, "data", node->data_gateway);
                                json_object_set_string(node_json_object, "status", node->status);
                                json_array_append_value(json_array, node_json_value);
                                free(node);
                            }
                            else
                                break;
                        }
                        db_disconnect(local_db);
                        server_proto_data_response(connection, 1, "Successfully fetched list of nodes.", json_value);
                        // json_value_free(json_value);
                    }
                    else
                        server_proto_response(connection, 0, "Failed fetching nodes from database");   
                }
            }
            else if (!strncmp(method, GEONSP_MSG_ADD_NODE, strlen(GEONSP_MSG_ADD_NODE))) {
                Database *local_db = db_open(LOCAL_DB);
                if (local_db == NULL) {
                    server_proto_response(connection, 0, "Failed fetching nodes from database");   
                }
                else {
                    uchar *server_addr = (uchar *) json_object_dotget_string(request_json, "data.server_addr");
                    ushort node_gateway_port = (ushort) json_object_dotget_number(request_json, "data.node_gateway_port");
                    ushort data_gateway_port = (ushort) json_object_dotget_number(request_json, "data.data_gateway_port");

                    db_connect(local_db);
                    uchar insert_node = insert_new_node(local_db, server_addr, node_gateway_port, data_gateway_port);
                    db_disconnect(local_db);

                    if (!insert_node)
                        server_proto_response(connection, 0, "There was a db error inserting new node.");
                    else if (insert_node == 2)
                        server_proto_response(connection, 0, "This node has already been added.");
                    else {
                        server_proto_response(connection, 1, "New node added into database.");
                        // TODO: Consider sharing new node's info into network
                    }
                }
            }
            else {
                json_value_free(request_value);
                server_proto_response(connection, 0, "GEONSP: Invalid protocol method.");
                return 0;
            }
            
            json_value_free(request_value);
            return 1;
        }
        json_value_free(request_value);
        server_proto_response(connection, 0, "GEONSP: Invalid protocol message.");
        return 0;
    }
    
    server_proto_response(connection, 0, "GEONSP: Empty protocol message.");
    return 0;
}
