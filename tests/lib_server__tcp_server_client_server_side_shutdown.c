/*
Scenario: 
    1. Creates a simple socket server, and checks for successfull spawn
    2. Connects from 3 clients to the server, and checks for successfull connection
    3. Sends 3 messages to the server and receives server's reply, and checks for consistency of connections
    4. Kills the connection in a server side manner to ensure both kinds of connection endings
*/


#include "../src/libs/unity/unity.h"
#include "../src/libs/server/server.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5432

#define CLIENT_REQUEST "Hello world!\n"
#define SERVER_RESPONSE "Got your message, it's an echo from server.\n"

SocketServer *server;

void setUp(void) {
}

void tearDown(void) {
}

void test_server_callback(int fd, uchar *request, PeerInfo *peer_info) {
    TEST_ASSERT_EQUAL_STRING(CLIENT_REQUEST, request);
    send(fd, SERVER_RESPONSE, strlen(SERVER_RESPONSE), 0);
}

void test_communication_with_server(void) {
    TEST_ASSERT_NOT_NULL(server);
    TEST_ASSERT_EQUAL_CHAR(1, server->is_alive);
    
    SocketServer *clients[3];
    char clients_size = sizeof(clients) / sizeof(clients[0]);
    for (int i = 0; i < clients_size; i++) {
        clients[i] = connect_to_socket_server(SERVER_ADDR, SERVER_PORT);
        TEST_ASSERT_NOT_NULL(clients[i]);
        sleep(1);
    }

    
    // Sending 3 messages for each of clients, each in 1 second
    char response[strlen(SERVER_RESPONSE) + 1];
    for (int i = 0; i < clients_size; i++)
        for (int j = 0; j < 3; j++) {
            send_message(clients[i]->fd, CLIENT_REQUEST, strlen(CLIENT_REQUEST), 0);
            recv_message(clients[i]->fd, response, sizeof(response), 0);
            TEST_ASSERT_EQUAL_STRING(SERVER_RESPONSE, response);
            sleep(1);
        }
}

void test_create_socket_server(void) {
    server = open_server_socket(SERVER_ADDR, SERVER_PORT);
    TEST_ASSERT_NOT_NULL(server);

    handle_server_socket(server, &test_server_callback);
    test_communication_with_server();

    kill_socket_server(server);
    TEST_ASSERT(server->connections == NULL);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_socket_server);
    UNITY_END();
}