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
#ifndef _PV_SCHEMA_H_
#define _PV_SCHEMA_H_

#include "mongo.h"

typedef struct pv_field_t {
    char *name;
    struct pv_field_t *next;
} pv_field;

typedef struct {
    char *watch_ns;
    pv_field *fields;
} pv_schema;

pv_schema* pv_get_schema(mongo_connection *conn);
pv_schema* pv_parse_schema(bson *b);
void pv_schema_free(pv_schema *schema);

#endif
