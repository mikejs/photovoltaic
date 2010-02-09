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
#ifndef _OPLOG_H_
#define _OPLOG_H_

#include "bson.h"
#include "mongo.h"

mongo_cursor* pv_oplog_since(mongo_connection *conn,
                             const bson_timestamp_t last);

#endif
