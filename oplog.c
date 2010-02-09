#include "oplog.h"

mongo_cursor* pv_oplog_since(mongo_connection *conn,
                             const bson_timestamp_t last) {
    bson_buffer bb;
    bson ts, query, orderby, b;
    mongo_cursor *cursor;

    bson_buffer_init(&bb);
    bson_append_timestamp(&bb, "$gt", last);
    bson_from_buffer(&ts, &bb);

    bson_buffer_init(&bb);
    bson_append_string(&bb, "ns", "test.pv");
    bson_append_bson(&bb, "ts", &ts);
    bson_from_buffer(&query, &bb);

    bson_buffer_init(&bb);
    bson_append_long(&bb, "$natural", 1);
    bson_from_buffer(&orderby, &bb);

    bson_buffer_init(&bb);
    bson_append_bson(&bb, "query", &query);
    bson_append_bson(&bb, "orderby", &orderby);
    bson_from_buffer(&b, &bb);

    cursor = mongo_find(conn, "local.oplog.$main", &b, NULL, 0, 0, 0);

    bson_destroy(&ts);
    bson_destroy(&query);
    bson_destroy(&orderby);
    bson_destroy(&b);

    return cursor;
}
