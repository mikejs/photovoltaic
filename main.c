#include "bson.h"
#include "mongo.h"
#include "oplog.h"
#include "solr.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>

int main() {
    mongo_connection conn;
    mongo_connection_options opts;
    mongo_cursor *cursor;
    bson_iterator it;
    bson_timestamp_t last = 0;
    solr_doc doc;

    solr_init();
    doc = solr_doc_new("id1");
    solr_doc_add_field(doc, "field1_s", "blah");
    solr_doc_add_field(doc, "field2_s", "blah2");
    solr_add_doc(doc);
    solr_doc_free(doc);
    solr_cleanup();

    strcpy(opts.host, "127.0.0.01");
    opts.port = 27017;
    mongo_connect(&conn, &opts);

    while (1) {
        printf("Looping.\n");

        cursor = pv_oplog_since(&conn, last);

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

        sleep(3);
    }

    return 0;
}
