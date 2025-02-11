#ifndef GEONS_CONSTANTS_H
#define GEONS_CONSTANTS_H 1

// Database constants
#define LEDGER_DB                               "ledger.db"
#define LOCAL_DB                                "local.db"
#define MAX_SQL_QUERY_SIZE                      1024

// System constants
#define MAX_SYS_PATH_LENGTH                     1024
#define MAX_SYS_OUTPUT_LENGTH                   8192
#define MAX_SYS_OUTPUT_CHUNK_SIZE               256
#define MAX_SYS_COMMAND_LENGTH_PER_EXEC         1024

// Socket constants
#define MAX_IPV6_LENGTH                         39
#define BASE_SOCKET_BUFFER_SIZE                 1024

// GeoNS-Protocol constants
#define DEFAULT_GEONS_SERVER_ADDR               "0.0.0.0"
#define DEFAULT_NODE_GATEWAY_PORT               9060
#define DEFAULT_DATA_GATEWAY_PORT               6090
#define MAX_GEONSP_BUFFER_SIZE                  4096
#define MAX_GEONSP_METHOD_NAME                  64

// Decentralization constants
#define MAX_ACTIVE_NODES                        16

// Logger constants
#define MAX_LOG_LENGTH                          1024
#define MAX_LOG_FILE_NAME_LENGTH                32
#define MAX_LOG_PRE_TEXT_LENGTH                 32
#define MAX_LOG_TIME_LENGTH                     22

// Config manager constants
#define CONFIG_FILE_NAME                        "config.json"
#define MAX_CONFIG_FILE_CONTENT                 4096

// HTTP constants
#define MAX_HTTP_URI_LENGTH                     2048
#define MAX_HTTP_METHOD_LENGTH                  16
#define MAX_HTTP_VERSION_LENGTH                 8
#define MAX_HTTP_HEADER_NAME_LENGTH             40
#define MAX_HTTP_HEADER_VALUE_LENGTH            1024
#define MAX_HTTP_REQUEST_SIZE                   1024*1024*1024*2        // 2GB
#define BASE_HTTP_REQUEST_SIZE                  1024*8                  // 8KB
#define BASE_HTTP_HEADER_VALUE_LENGTH           1024*2                  // 2KB
#define MAX_HTTP_HEADER_REGEX_MATCH             10

#endif // !GEONS_CONSTANTS_H