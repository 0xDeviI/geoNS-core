#ifndef GEONS_CONFIG_H
#define GEONS_CONFIG_H 1

#include <stdio.h>
#include "../io/io.h"
#include "../core/core.h"
#include "../parson/parson.h"

typedef enum eValueType {
    TYPE_BOOL,
    TYPE_NULL,
    TYPE_STRING,
    TYPE_NUMBER,
    TYPE_VALUE
} ValueType;


typedef struct sGeoNSConfig
{
    uchar geons_server_addr[MAX_IPV6_LENGTH + 1];
    ushort node_gateway_port;
    ushort data_gateway_port;
} GeoNSConfig;


typedef struct sHTTPConfig
{
    uchar accept_any_method: 1;
    uchar directory_indexing: 1;
    uchar trim_large_headers : 1;
    uchar trim_large_body : 1;
    ushort server_port;
} HTTPConfig;


typedef struct sConfig
{
    GeoNSConfig geons_config;
    HTTPConfig http_config;
} Config;


extern uchar CONFIG_FILE_PATH[MAX_SYS_PATH_LENGTH];
extern Config *CONFIG;

void release_config(void);
uchar load_config(void);
void write_config_data(uchar *config_file_path, uchar *config_data);
JSON_Value *get_default_config(uchar is_template);
uchar is_valid_config(uchar *config_file_path);
void create_default_config(uchar *config_file_path);
uchar init_config_manager(void);
JSON_Value *get_config(uchar *key);
uchar set_config(uchar *key, void *value, ValueType type);

#endif // !GEONS_CONFIG_H