#include "table.h"

Table GEONS_LEDGER_DB_TABLES[] = {
    {
        .name="logs",
        .schema="(\
            id INTEGER PRIMARY KEY AUTOINCREMENT,\
            region TEXT,\
            location TEXT,\
            service TEXT,\
            delay INTEGER,\
            status TEXT CHECK (status IN ('success', 'failed')),\
            device TEXT CHECK (LENGTH(device) <= 128),\
            isp TEXT CHECK (LENGTH(isp) <= 64),\
            internet_type TEXT CHECK (internet_type IN ('phone', 'modem')),\
            timestamp INTEGER\
        )"
    },
    {
        .name="services",
        .schema="(\
            id INTEGER PRIMARY KEY AUTOINCREMENT,\
            service_name TEXT UNIQUE,\
            service_url TEXT,\
            is_local NUMBER\
        )"
    }
};

Table GEONS_LOCAL_DB_TABLES[] = {
    {
        .name="settings",
        .schema="(\
            id INTEGER PRIMARY KEY AUTOINCREMENT,\
            setting_key TEXT UNIQUE,\
            setting_value TEXT\
        )"
    },
    {
        .name="nodes",
        .schema="(\
            id INTEGER PRIMARY KEY AUTOINCREMENT,\
            server TEXT CHECK (LENGTH(server) <= 15),\
            node_gateway SHORT,\
            status TEXT CHECK (status IN ('active', 'inactive'))\
        )"
    }
};

Service GEONS_DEFAULT_SERVICES[] = {
    {
        .name = "Github",
        .url = "https://github.com/",
        .is_local = 0
    },
    {
        .name = "Google",
        .url = "https://www.google.com/",
        .is_local = 0
    },
    {
        .name = "Gmail",
        .url = "https://www.google.com/gmail/about/",
        .is_local = 0
    },
    {
        .name = "Android Developer",
        .url = "https://developer.android.com/",
        .is_local = 0
    },
    {
        .name = "Wikipedia",
        .url = "https://www.wikipedia.org/",
        .is_local = 0
    },
    {
        .name = "PlayStation",
        .url = "https://www.playstation.com/en-ie/",
        .is_local = 0
    },
    {
        .name = "Aparat",
        .url = "https://www.aparat.com/",
        .is_local = 1
    },
    {
        .name = "Filimo",
        .url = "https://www.filimo.com/",
        .is_local = 1
    },
    {
        .name = "Digikala",
        .url = "https://www.digikala.com/",
        .is_local = 1
    }
};


uchar remove_node(Database *db, Node *node) {
    uchar sql_query[MAX_SQL_QUERY_SIZE];
    snprintf(sql_query, sizeof(sql_query),
        "DELETE FROM nodes WHERE server = '%s' AND node_gateway = %d;",
        node->server_addr,
        node->node_gateway
    );

    return db_exec(db, sql_query);
}


char get_all_active_nodes(Database *db, Node **nodes, uchar size_of_nodes) {
    if (db->is_ledger)
        return -1;
    
    uchar sql_query[MAX_SQL_QUERY_SIZE];
    snprintf(sql_query, sizeof(sql_query), 
        "SELECT * FROM nodes WHERE status = 'active' LIMIT %d;",
        size_of_nodes
    );

    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db->sqlite_db, sql_query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->sqlite_db));
        sqlite3_finalize(stmt);
        return -1;
    }
    
    uchar nodes_count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Node *node = (Node *) memalloc(sizeof(Node));
        if (node == NULL) {
            perror("Memory error");
            sqlite3_finalize(stmt);
            return -1;
        }

        uchar id = sqlite3_column_int(stmt, 0);
        uchar *server_addr = (uchar *) sqlite3_column_text(stmt, 1);
        ushort node_gateway = sqlite3_column_int(stmt, 2);
        uchar *status = (uchar *) sqlite3_column_text(stmt, 4);
        strncpy(node->server_addr, server_addr, sizeof(node->server_addr) - 1);
        node->server_addr[strlen(node->server_addr)] = '\0';
        strncpy(node->status, status, sizeof(node->status) - 1);
        node->status[strlen(node->status)] = '\0';
        node->id = id;
        node->node_gateway = node_gateway;
        nodes[nodes_count++] = node;
    }
    sqlite3_finalize(stmt);
    return nodes_count;
}


uchar insert_new_node(Database *db, uchar *server_addr, ushort node_gateway_port) {
    // returns 0 if receives any error
    // returns 1 if successfully adds new node
    // returns 2 if node has been already added
    if (db->is_ledger)
        return 0;
    
    uchar sql_query[MAX_SQL_QUERY_SIZE];
    snprintf(sql_query, sizeof(sql_query), 
        "SELECT * FROM nodes WHERE server = '%s' AND node_gateway = %d;",
        server_addr,
        node_gateway_port
    );

    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db->sqlite_db, sql_query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->sqlite_db));
        sqlite3_finalize(stmt);
        return 0;
    }
    
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 2;
    }

    sqlite3_finalize(stmt);
    snprintf(sql_query, sizeof(sql_query), 
        "INSERT INTO nodes (server, node_gateway, status) VALUES ('%s', %d, 'active')",
        server_addr,
        node_gateway_port
    );

    return db_exec(db, sql_query);
}


uchar is_geons_configured(Database *db) {
    if (db->is_ledger)
        return 1;

    uchar *sql_query = "SELECT * FROM settings WHERE setting_key = 'geons_config_status';";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db->sqlite_db, sql_query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->sqlite_db));
        sqlite3_finalize(stmt);
        db_disconnect(db);
    }
    
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const uchar *value = sqlite3_column_text(stmt, 2);
        sqlite3_finalize(stmt);
        return !strncmp(value, "true", strlen("true"));
    }
    
    sqlite3_finalize(stmt);
    return 0;
}


void set_geons_settings_config_status(Database *db, uchar is_configured) {
    if (db->is_ledger)
        return;

    uchar *settings_table = "settings";
    uchar sql_query[MAX_SQL_QUERY_SIZE];
    sqlite3_stmt *stmt;

    snprintf(sql_query, MAX_SQL_QUERY_SIZE, "SELECT * FROM %s WHERE setting_key = 'geons_config_status';", settings_table);
    
    int rc = sqlite3_prepare_v2(db->sqlite_db, sql_query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->sqlite_db));
        sqlite3_finalize(stmt);
        db_disconnect(db);
    }
    
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const uchar *value = sqlite3_column_text(stmt, 2);
        is_configured = !strncmp(value, "true", strlen("true"));
        sqlite3_finalize(stmt);
    }
    else {
        is_configured = 0;
        sqlite3_finalize(stmt);
    }

    if (!is_configured) {
        snprintf(sql_query, MAX_SQL_QUERY_SIZE, "INSERT INTO %s (setting_key, setting_value) VALUES ('geons_config_status', '%s');",
        settings_table, "true");
        db_exec(db, sql_query);
    }
}


void insert_default_values(Database *db) {
    if (db->is_ledger) {
        uchar size_of_default_services = sizeof(GEONS_DEFAULT_SERVICES) / sizeof(GEONS_DEFAULT_SERVICES[0]);
        uchar sql_query[MAX_SQL_QUERY_SIZE];

        uchar *services_table = "services";

        for (uchar i = 0; i < size_of_default_services; i++) {
            snprintf(sql_query, MAX_SQL_QUERY_SIZE, 
                "INSERT INTO %s (service_name, service_url, is_local) VALUES ('%s', '%s', %d);", services_table, 
                GEONS_DEFAULT_SERVICES[i].name, GEONS_DEFAULT_SERVICES[i].url, GEONS_DEFAULT_SERVICES[i].is_local == 1
            );

            db_exec(db, sql_query);
        }

    }
    
    if (!db->is_ledger)
        set_geons_settings_config_status(db, 1);
}


void create_default_tables(Database *db) {
    uchar size_of_tables;
    if (db->is_ledger)
        size_of_tables = sizeof(GEONS_LEDGER_DB_TABLES) / sizeof(GEONS_LEDGER_DB_TABLES[0]);
    else
        size_of_tables = sizeof(GEONS_LOCAL_DB_TABLES) / sizeof(GEONS_LOCAL_DB_TABLES[0]);

    uchar sql_query[MAX_SQL_QUERY_SIZE];
    

    for (uchar i = 0; i < size_of_tables; i++) {
        snprintf(sql_query, MAX_SQL_QUERY_SIZE, 
            "CREATE TABLE IF NOT EXISTS %s %s;", 
            db->is_ledger ? GEONS_LEDGER_DB_TABLES[i].name : GEONS_LOCAL_DB_TABLES[i].name,
            db->is_ledger ? GEONS_LEDGER_DB_TABLES[i].schema : GEONS_LOCAL_DB_TABLES[i].schema
        );
        db_exec(db, sql_query);
    }

    if (!is_geons_configured(db))
        insert_default_values(db);
}