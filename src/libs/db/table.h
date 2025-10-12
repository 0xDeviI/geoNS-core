#ifndef GEONS_SQL_TABLE_H
#define GEONS_SQL_TABLE_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "db.h"
#include "../core/constants.h"
#include "../core/types.h"
#include "../server/server.h"

typedef struct sTable
{
    uchar *name;
    uchar *schema;
} Table;

typedef struct sService 
{
    uchar *name;
    uchar *url;
    uchar is_local;
} Service;

extern Table GEONS_LEDGER_DB_TABLES[];
extern Table GEONS_LOCAL_DB_TABLES[];
extern Service GEONS_DEFAULT_SERVICES[];

uchar remove_node(Database *db, Node *node);
char get_all_active_nodes(Database *db, Node **nodes, uchar size_of_nodes);
uchar insert_new_node(Database *db, uchar *server_addr, 
    ushort node_gateway_port);
uchar is_geons_configured(Database *db);
void insert_default_values(Database *db);
void set_geons_settings_config_status(Database *db, uchar is_configured);
void create_default_tables(Database *db);

#endif // !GEONS_SQL_TABLE_H