#include <stdio.h>
#include "libs/server/server.h"
#include "libs/argparse/argparse.h"
#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/http/http.h"

// TODO: code parser

static const char *const usages[] = {
    "geons-core [options] [[--] args]",
    "geons-core [options]",
    NULL,
};

int main(int argc, const char *argv[]) {
    init_io_system(argv[0]);
    init_logger();
    if (!init_config_manager()) {
        printf("Config error: Couldn't read config file properly.\n");
        exit(EXIT_FAILURE);
    }

    char server = 0;
    char client = 0;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Core options"),
        OPT_BOOLEAN('d', "debug", &is_debugging, "enables debugging mode", NULL, 0, 0),
        OPT_GROUP("Server options"),
        OPT_BOOLEAN('s', "server", &server, "starts geoNS server", NULL, 0, 0),
        OPT_GROUP("Client options"),
        OPT_BOOLEAN('c', "client", &client, "starts geoNS client", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\ngeoNS (Geolocational Net Stat) is a decentralized service that monitors internet quality.", "\nThe core service is responsible for handling decentralized operations, log collection and API provision.");
    argc = argparse_parse(&argparse, argc, argv);

    msglog(DEBUG, "geoNS-core started.");

    if (server) {
        //* Running Server
        HTTPServer *server = create_http_server(CONFIG->geons_server_addr, 8000, "./");
        if (server != NULL) {
            handle_server_socket(server->socket_server, &http_server_callback);
            sleep(60);
            kill_http_server(server);
        }


        // GeoNSServer *server = create_geons_server();
        // if (server != NULL) {
        //     sleep(15); //? MemCheck: killing the server after some seconds
        //     kill_geons_server(server);
        // }
    }
    else {
        // TODO:
        //* Running Client
    }
    release_config();

    msglog(DEBUG, "geoNS-core finished.");
    return 0;
}