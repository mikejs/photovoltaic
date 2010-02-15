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
#include "schema.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

pv_schema* pv_get_schema(mongo_connection *conn) {
    bson_buffer bb;
    bson b, out;
    pv_schema *schema;

    bson_buffer_init(&bb);
    bson_append_string(&bb, "_id", "schema");
    bson_from_buffer(&b, &bb);

    if (!mongo_find_one(conn, "local.fts", &b, NULL, &out)) {
        bson_destroy(&b);
        return NULL;
    }

    schema = pv_parse_schema(&out);

    bson_destroy(&b);
    bson_destroy(&out);

    return schema;
}

pv_schema* pv_parse_schema(bson *b) {
    bson_iterator it, sub;
    bson_type type;
    pv_schema *schema = malloc(sizeof(pv_schema));
    pv_field *field;

    if (schema == NULL) {
        return NULL;
    }

    if (bson_find(&it, b, "watch_ns")) {
        schema->watch_ns = strdup(bson_iterator_string(&it));
    } else {
        schema->watch_ns = strdup("test.pv");
    }
    if (schema->watch_ns == NULL) {
        free(schema);
        return NULL;
    }

    schema->fields = NULL;

    if (bson_find(&it, b, "fields")) {
        bson_iterator_subiterator(&it, &sub);

        while ((type = bson_iterator_next(&sub)) != bson_eoo) {
            field = malloc(sizeof(pv_field));
            if (field == NULL) {
                pv_schema_free(schema);
                return NULL;
            }

            field->name = strdup(bson_iterator_string(&sub));
            if (field->name == NULL) {
                free(field);
                pv_schema_free(schema);
                return NULL;
            }

            field->next = schema->fields;
            schema->fields = field;
        }
    }

    return schema;
}

void pv_schema_free(pv_schema *schema) {
    pv_field *field, *next;

    free(schema->watch_ns);

    field = schema->fields;
    while(field != NULL) {
        next = field->next;
        free(field->name);
        free(field);
        field = next;
    }

    free(schema);
}
