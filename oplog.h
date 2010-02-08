#ifndef _OPLOG_H_
#define _OPLOG_H_

#include "bson.h"
#include "mongo.h"

mongo_cursor* pv_oplog_since(mongo_connection *conn,
                             const bson_timestamp_t last);

#endif
