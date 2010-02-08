#include "bson.h"
#include "mongo.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>

int main() {
    bson_buffer bb;
    bson b, q, ts, order;
    mongo_connection conn;
    mongo_connection_options opts;
    mongo_cursor *cursor;
    bson_iterator it;
    bson_timestamp_t last = 0;

    strcpy(opts.host, "127.0.0.01");
    opts.port = 27017;
    mongo_connect(&conn, &opts);

    while (1) {
        printf("Looping.\n");

        bson_buffer_init(&bb);
        bson_append_timestamp(&bb, "$gt", last);
        bson_from_buffer(&ts, &bb);

        bson_buffer_init(&bb);
        bson_append_string(&bb, "ns", "test.pv");
        bson_append_bson(&bb, "ts", &ts);
        bson_from_buffer(&q, &bb);

        bson_buffer_init(&bb);
        bson_append_long(&bb, "$natural", 1);
        bson_from_buffer(&order, &bb);

        bson_buffer_init(&bb);
        bson_append_bson(&bb, "query", &q);
        bson_append_bson(&bb, "orderby", &order);
        bson_from_buffer(&b, &bb);

        cursor = mongo_find(&conn, "local.oplog.$main", &b, NULL, 0, 0, 0);

        while (mongo_cursor_next(cursor)) {
            const char *op;
            bson_find(&it, &cursor->current, "ts");
            last = bson_iterator_timestamp(&it);

            bson_find(&it, &cursor->current, "op");
            op = bson_iterator_string(&it);

            if(op[0] == 'i') {
                printf("Insert %"PRIi64".\n", last);
            } else if(op[0] == 'u') {
                printf("Update %"PRIi64".\n", last);
            }
        }

        mongo_cursor_destroy(cursor);
        bson_destroy(&ts);
        bson_destroy(&q);
        bson_destroy(&order);
        bson_destroy(&b);

        sleep(3);
    }

    return 0;
}
