/*
  Copyright 2010 Michael Stephens.

  This file is part of photovoltaic.

  photovoltaic is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  photovoltaic is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with photovoltaic.  If not, see <http://www.gnu.org/licenses/>.
*/
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

    solr_init();

    strcpy(opts.host, "127.0.0.01");
    opts.port = 27017;
    mongo_connect(&conn, &opts);

    while (1) {
        printf("Looping.\n");

        cursor = pv_oplog_since(&conn, last);

        while (mongo_cursor_next(cursor)) {
            const char *op;
            bson o, o2;
            bson_type type;
            solr_doc doc;

            bson_find(&it, &cursor->current, "ts");
            last = bson_iterator_timestamp(&it);

            bson_find(&it, &cursor->current, "op");
            op = bson_iterator_string(&it);

            bson_find(&it, &cursor->current, "o");
            bson_iterator_subobject(&it, &o);

            if(op[0] == 'i') {
                printf("Insert %"PRIi64".\n", last);
                o2 = o;
            } else if(op[0] == 'u') {
                printf("Update %"PRIi64".\n", last);
                bson_find(&it, &cursor->current, "o2");
                bson_iterator_subobject(&it, &o2);
            }

            bson_find(&it, &o2, "_id");
            type = bson_iterator_type(&it);

            if (type == bson_string) {
                doc = solr_doc_new(bson_iterator_string(&it));
            } else if (type == bson_oid) {
                char oidhex[25];
                bson_oid_to_string(bson_iterator_oid(&it), oidhex);
                doc = solr_doc_new(oidhex);
            } else {
                /* Can't handle other _id types yet */
                continue;
            }

            bson_iterator_subobject(&it, &o);
            while ((type = bson_iterator_next(&it)) != bson_eoo) {
                const char *key = bson_iterator_key(&it);

                if (type == bson_string) {
                    char *new_key;
                    new_key = malloc(strlen(key) + 2 + 1);
                    strcpy(new_key, key);
                    strcat(new_key, "_s");
                    solr_doc_add_field(doc, new_key, bson_iterator_string(&it));
                    free(new_key);
                } else {
                    continue;
                }

                solr_add_doc(doc);
                solr_doc_free(doc);
            }
        }

        mongo_cursor_destroy(cursor);

        sleep(3);
    }

    solr_cleanup();

    return 0;
}
