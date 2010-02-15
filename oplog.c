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
    bson_append_bson(&bb, "ts", &ts);
    bson_from_buffer(&query, &bb);

    bson_buffer_init(&bb);
    bson_append_long(&bb, "$natural", 1);
    bson_from_buffer(&orderby, &bb);

    bson_buffer_init(&bb);
    bson_append_bson(&bb, "query", &query);
    bson_append_bson(&bb, "orderby", &orderby);
    bson_from_buffer(&b, &bb);

    cursor = mongo_find(conn, "local.oplog.$main", &b, NULL, 0, 0, 2);

    bson_destroy(&ts);
    bson_destroy(&query);
    bson_destroy(&orderby);
    bson_destroy(&b);

    return cursor;
}
