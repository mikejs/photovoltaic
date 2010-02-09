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
#include <stdlib.h>

typedef struct {
    mongo_connection conn;
    char *watch_ns, *host;
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
}

bson_bool_t pv_parse_config(pv_conf *conf) {
    bson_buffer bb;
    bson b, out;
    bson_iterator it;

    bson_buffer_init(&bb);
    bson_append_string(&bb, "_id", "conf");
    bson_from_buffer(&b, &bb);

    if (mongo_find_one(&conf->conn, "local.fts", &b, NULL, &out)) {
        if (bson_find(&it, &out, "watch_ns")) {
            conf->watch_ns = strdup(bson_iterator_string(&it));
            if (conf->watch_ns == NULL) {
                return 0;
            }
        }

        if (bson_find(&it, &out, "poll_interval")) {
            conf->poll_interval = bson_iterator_int(&it);
        }
    }

    if (conf->watch_ns == NULL) {
        conf->watch_ns = strdup("test.pv");
        if (conf->watch_ns == NULL) {
            return 0;
        }
    }

    if (conf->poll_interval == 0) {
        conf->poll_interval = 3;
    }

    bson_destroy(&b);
    return 1;
}

int main(int argc, char **argv) {
    pv_conf conf;
    mongo_cursor *cursor;
    bson_iterator it;
    bson_timestamp_t last = 0;

    memset(&conf, 0, sizeof(pv_conf));

    pv_parse_args(&conf, argc, argv);

    if (!pv_init_connection(&conf)) {
        printf("Failed connecting.\n");
        exit(1);
    }

    pv_parse_config(&conf);

    printf("Watching namespace: %s\n", conf.watch_ns);
    printf("Polling interval: %d\n", conf.poll_interval);

    solr_init();

    while (1) {
        printf("Looping.\n");

        cursor = pv_oplog_since(&conf.conn, last);

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

        sleep(conf.poll_interval);
    }

    solr_cleanup();

    return 0;
}
