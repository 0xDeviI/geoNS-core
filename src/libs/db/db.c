#include "db.h"
#include "table.h"


Database *db_open(uchar *db_name) {
    Database *db = (Database *) memalloc(sizeof(Database));
    if (db == NULL) {
        perror("Memory error");
        return NULL;
    }

    sqlite3 *sqlite_db;
    db->db_name = db_name;
    db->is_ledger = !strncmp(db_name, LEDGER_DB, strlen(db_name));
    db->sqlite_db = sqlite_db;
    return db;
}


void db_fetch(Database *db, uchar *sql_query, uchar **output) {

}


uchar db_exec(Database *db, uchar *sql_query) {
    uchar sql_error = sqlite3_exec(db->sqlite_db, sql_query, 0, 0, 0);
    if (sql_error != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\nCode: %d\nQuery: %s\n", sqlite3_errmsg(db->sqlite_db), sql_error, sql_query);
        return 0;
    }
    return 1;
}


void db_init(Database *db) {
    create_default_tables(db);
}


void db_connect(Database *db) {
    uchar db_path[MAX_SYS_PATH_LENGTH];
    get_cwd_path(db_path, sizeof(db_path));
    strncat(db_path, db->db_name, sizeof(db_path) - 1);

    int connection_error = sqlite3_open(db_path, &db->sqlite_db);
    if (connection_error) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db->sqlite_db));
        db_disconnect(db);
        exit(connection_error);
    }

    db_init(db);
}


void db_disconnect(Database *db) {
    if (db != NULL) {
        if (db->sqlite_db != NULL)
            sqlite3_close(db->sqlite_db);
        free(db);
    }
}