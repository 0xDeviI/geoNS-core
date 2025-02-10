/*
Scenario: 
    1. Connects from a client to the server, and checks for successfull connection
    2. Sends a message to the server and receives server's reply
    3. Disconnections from the server

NOTE:
    Because of requirements of GeoNS server mechanism and its need to hanlde database
    side operations, testing GeoNS-server creation fails here. In order to test, make
    sure to run a GeoNS-server in geoNS-core and then run the test.
*/


#include "../src/libs/unity/unity.h"
#include "../src/libs/server/server.h"

#define SERVER_ADDR "127.0.0.1"     // Local IP address. Change if needed
#define SERVER_PORT 9060            // Default GeoNS node gateway port

#define CLIENT_REQUEST "{\"method\": \"GET_VERSION\"}"

void setUp(void) {
}

void tearDown(void) {
}

void test_node_gateway_connection(void) {
    SocketServer *client = connect_to_socket_server(SERVER_ADDR, SERVER_PORT);
    TEST_ASSERT_NOT_NULL(client);

    size_t message_size;
    char response[256];
    for (int j = 0; j < 3; j++) {
        send_message(client->fd, CLIENT_REQUEST, strlen(CLIENT_REQUEST), 0);
        message_size = recv_message(client->fd, response, sizeof(response), 0);
        response[message_size] = '\0';
        printf("Server's response:\n\t%s\n", response);
        sleep(1);
    }

    kill_socket(client->fd);
    free(client);
    client = NULL;
    TEST_ASSERT_NULL(client);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_node_gateway_connection);
    UNITY_END();
}