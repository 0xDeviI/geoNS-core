/*
Scenario: 
    1. Creates a simple socket server, and checks for successfull spawn
    2. Connects from a client to the server,  and checks for successfull connection
    3. Sends 5 messages to the server and receives server's reply, and checks for consistency of connection
    4. Kills the server, and checks for successfull shutdown.
*/


#include "../src/libs/unity/unity.h"
#include "../src/libs/server/server.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 4321

#define CLIENT_REQUEST "Hello world!\n"
#define SERVER_RESPONSE "Got your message, it's an echo from server.\n"

SocketServer *server;

void setUp(void) {
}

void tearDown(void) {
}

ssize_t test_server_callback(void *args, ...) {
    SocketConnection *connection = (SocketConnection *) args;
    TEST_ASSERT_EQUAL_STRING(CLIENT_REQUEST, connection->buffer);
    send(connection->fd, SERVER_RESPONSE, strlen(SERVER_RESPONSE), 0);
    return 0;
}

void test_communication_with_server(void) {
    TEST_ASSERT_NOT_NULL(server);
    TEST_ASSERT_EQUAL_CHAR(1, server->is_alive);
    SocketServer *socket_to_server = connect_to_socket_server(SERVER_ADDR, SERVER_PORT);
    TEST_ASSERT_NOT_NULL(socket_to_server);
    
    // Sending 5 messages, each in 1 second
    size_t message_size;
    char response[strlen(SERVER_RESPONSE) + 1];
    for (int i = 0; i < 5; i++) {
        send_message(socket_to_server->fd, CLIENT_REQUEST, strlen(CLIENT_REQUEST), 0);
        message_size = recv_message(socket_to_server->fd, response, sizeof(response), 0);
        response[message_size] = '\0';
        TEST_ASSERT_EQUAL_STRING(SERVER_RESPONSE, response);
        sleep(1);
    }

    kill_socket(socket_to_server->fd);
    free(socket_to_server);
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