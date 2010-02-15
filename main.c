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
#include "mongo.h"
#include "schema.h"
#include "oplog.h"
#include "solr.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    mongo_connection conn;
    char *host;
    int port, poll_interval;
} pv_conf;

bson_bool_t pv_init_connection(pv_conf *conf) {
    mongo_connection_options opts;

    if (conf->host == NULL || conf->port == 0) {
        return 0;
    }

    strncpy(opts.host, conf->host, 255);
    opts.host[254] = '\0';

    opts.port = conf->port;

    if (!mongo_connect(&conf->conn, &opts)) {
        return 1;
    } else {
        return 0;
    }
}

void pv_print_help(char *cmd) {
    printf("Usage: %s [--host 127.0.0.1] [--port 27017]\n", cmd);
    exit(0);
}

void pv_parse_args(pv_conf *conf, int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0 ||
            strcmp(argv[i], "-h") == 0) {
            i++;
            if (i < argc) {
                conf->host = strdup(argv[i]);
            } else {
                pv_print_help(argv[0]);
            }
        } else if (strcmp(argv[i], "--port") == 0 ||
                   strcmp(argv[i], "-p") == 0) {
            i++;
            if (i < argc) {
                conf->port = atoi(argv[i]);
            } else {
                pv_print_help(argv[0]);
            }
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            pv_print_help(argv[0]);
        }
    }

    if (conf->host == NULL) {
        conf->host = strdup("127.0.0.1");
    }

    if (conf->port == 0){
        conf->port = 27017;
    }

    conf->poll_interval = 2;
}

int main(int argc, char **argv) {
    pv_conf conf;
    pv_schema *schema;
    mongo_cursor *cursor = NULL;
    bson_iterator it;
    bson_timestamp_t last = 0;
    solr_docset docset;

    memset(&conf, 0, sizeof(pv_conf));

    pv_parse_args(&conf, argc, argv);

    if (!pv_init_connection(&conf)) {
        printf("Failed connecting.\n");
        exit(1);
    }

    if ((schema = pv_get_schema(&conf.conn)) == NULL) {
        printf("No schema.\n");
        exit(1);
    }

    printf("Watching namespace: %s\n", schema->watch_ns);
    printf("Polling interval: %d\n", conf.poll_interval);

    solr_init();

    docset = solr_docset_new();

    while (1) {
        printf("Looping.\n");

        if (cursor && cursor->mm) {
            mongo_cursor_get_more(cursor);
            cursor->current.data = NULL;
        }

        if (!cursor || !cursor->mm || cursor->mm->fields.cursorID == 0) {
            /* Need to recreate cursor */
            printf("(Re)creating cursor\n");
            cursor = pv_oplog_since(&conf.conn, last);
        }

        while (mongo_cursor_next(cursor)) {
            const char *op;
            bson o, o2;
            bson_type type;
            solr_doc doc;
            int field_count = 0;

            bson_find(&it, &cursor->current, "ns");
            if (strcmp(bson_iterator_string(&it), schema->watch_ns)) {
                continue;
            }

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
            } else {
                printf("Bad op type: %s", op);
                continue;
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
                printf("Bad ID type: %d\n", type);
                continue;
            }

            bson_iterator_subobject(&it, &o);
            while ((type = bson_iterator_next(&it)) != bson_eoo) {
                const char *key = bson_iterator_key(&it);

                if (schema->fields != NULL) {
                    pv_field *field = schema->fields;
                    int found = 0;

                    while (field != NULL) {
                        if (!strcmp(field->name, key)) {
                            found = 1;
                            break;
                        }
                        field = field->next;
                    }

                    if (!found) {
                        continue;
                    }
                }

                if (type == bson_string) {
                    solr_doc_add_field(doc, key, bson_iterator_string(&it));
                    field_count++;
                } else {
                    continue;
                }
            }

            if (field_count > 0) {
                solr_docset_add_doc(docset, doc);
            } else {
                solr_doc_free(doc);
            }
        }

        solr_add_docset(docset);
        solr_docset_clear(docset);

        sleep(conf.poll_interval);
    }

    solr_docset_free(docset);
    solr_cleanup();

    return 0;
}
