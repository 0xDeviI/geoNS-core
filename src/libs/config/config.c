#include "config.h"


uchar CONFIG_FILE_PATH[MAX_SYS_PATH_LENGTH] = {0};
Config *CONFIG = NULL;


void release_config(void) {
    if (CONFIG != NULL) {
        free(CONFIG);
        CONFIG = NULL;
    }
}


void load_config(void) {
    CONFIG = (Config *) memalloc(sizeof(Config));

    JSON_Value *config = get_config("");
    JSON_Object *config_json_object = json_value_get_object(config);

    uchar *geons_server_addr = (uchar *) json_object_dotget_string(config_json_object, "server.server_addr");
    ushort node_server_port = json_object_dotget_number(config_json_object, "server.node.server_port");
    ushort data_server_port = json_object_dotget_number(config_json_object, "server.data.server_port");

    CONFIG->geons_server_addr = geons_server_addr;
    CONFIG->node_gateway_port = node_server_port;
    CONFIG->data_gateway_port = data_server_port;

    json_value_free(config);
}


void write_config_data(uchar *config_file_path, uchar *config_data) {
    FILE *json_file = fopen(config_file_path, "w+");
    fwrite(config_data, strlen(config_data), 1, json_file);
    fclose(json_file);
}


JSON_Value *get_default_config(uchar is_template) {
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_dotset_string(json_object, "server.server_addr", !is_template ? DEFAULT_GEONS_SERVER_ADDR : "");
    json_object_dotset_number(json_object, "server.node.server_port", !is_template ? DEFAULT_NODE_GATEWAY_PORT : 0);
    json_object_dotset_number(json_object, "server.data.server_port", !is_template ? DEFAULT_DATA_GATEWAY_PORT : 0);

    return json_value;
}

uchar is_valid_config(uchar *config_file_path) {
    if (!is_file_exist(config_file_path))
        return 0;

    FILE *json_file = fopen(config_file_path, "r");
    uchar *buffer = (uchar *) memalloc(MAX_CONFIG_FILE_CONTENT + 1);
    fread(buffer, MAX_CONFIG_FILE_CONTENT, 1, json_file);
    buffer[strlen(buffer)] = '\0';
    fclose(json_file);

    JSON_Value *template_json_value = get_default_config(1);
    JSON_Value *json_value = json_parse_string(buffer);

    uchar result = json_validate(template_json_value, json_value) == JSONSuccess;
    json_value_free(template_json_value);
    json_value_free(json_value);
    free(buffer);

    return result;
}


void create_default_config(uchar *config_file_path) {
    JSON_Value *json_value = get_default_config(0);
    uchar *json = json_serialize_to_string_pretty(json_value);

    write_config_data(config_file_path, json);

    json_free_serialized_string(json);
    json_value_free(json_value);
}


void init_config_manager(void) {
    get_cwd_path(CONFIG_FILE_PATH, sizeof(CONFIG_FILE_PATH));
    strncat(CONFIG_FILE_PATH, CONFIG_FILE_NAME, sizeof(CONFIG_FILE_PATH) - 1);

    if ((!is_file_exist(CONFIG_FILE_PATH)) || (!is_valid_config(CONFIG_FILE_PATH)))
        create_default_config(CONFIG_FILE_PATH);

    load_config();
}


JSON_Value *get_config(uchar *key) {
    if (!is_file_exist(CONFIG_FILE_PATH))
        return NULL;

    FILE *json_file = fopen(CONFIG_FILE_PATH, "r");
    uchar *buffer = (uchar *) memalloc(MAX_CONFIG_FILE_CONTENT);
    fread(buffer, MAX_CONFIG_FILE_CONTENT - 1, 1, json_file);
    fclose(json_file);

    
    JSON_Value *json_value = json_parse_string(buffer);
    if (strlen(key) > 0) {
        uchar is_nested = strchr(key, '.') != NULL;
        JSON_Object *json_object = json_value_get_object(json_value);
        uchar key_exist = is_nested ? json_object_dothas_value(json_object, key) : json_object_has_value(json_object, key);
        if (!key_exist) {
            json_value_free(json_value);
            free(buffer);
            return NULL;
        }

        JSON_Value *value = json_value_deep_copy(
            is_nested ?
                json_object_dotget_value(json_object, key) : json_object_get_value(json_object, key)
        );
        free(buffer);
        json_value_free(json_value);
        return value;
    }
    
    free(buffer);
    return json_value;
}


uchar set_config(uchar *key, void *value, ValueType type) {
    if (!is_file_exist(CONFIG_FILE_PATH))
        return 0;

    JSON_Value *config = get_config("");
    JSON_Object *config_json_object = json_value_get_object(config);
    
    uchar is_nested = strchr(key, '.') != NULL;

    if (type == TYPE_NUMBER) {
        double *casted_value = (double *) value;
        if (is_nested)
            json_object_dotset_number(config_json_object, key, *casted_value);
        else
            json_object_set_number(config_json_object, key, *casted_value);
    } 
    else if (type == TYPE_STRING) {
        uchar *casted_value = (uchar *) value;
        if (is_nested)
            json_object_dotset_string(config_json_object, key, casted_value);
        else
            json_object_set_string(config_json_object, key, casted_value);
    } 
    else if (type == TYPE_BOOL) {
        int *casted_value = (int *) value;
        if (is_nested)
            json_object_dotset_boolean(config_json_object, key, *casted_value);
        else
            json_object_set_boolean(config_json_object, key, *casted_value);
    } 
    else if (type == TYPE_NULL) {
        if (is_nested)
            json_object_dotset_null(config_json_object, key);
        else
            json_object_set_null(config_json_object, key);
    } 
    else if (type == TYPE_VALUE) {
        JSON_Value *casted_value = (JSON_Value *) value;
        if (is_nested)
            json_object_dotset_value(config_json_object, key, casted_value);
        else
            json_object_set_value(config_json_object, key, casted_value);
    }
    else {
        json_value_free(config);
        return 0;
    }


    uchar *config_data = json_serialize_to_string_pretty(config);
    write_config_data(CONFIG_FILE_PATH, config_data);

    json_free_serialized_string(config_data);
    json_value_free(config);

    return 1;
}