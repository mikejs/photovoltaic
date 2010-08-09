#!/usr/bin/env python
import time
import logging

import pysolr
import pymongo


class Schema(object):
    def __init__(self, doc, solr=None):
        self.ns = doc['ns']
        self.fields = doc['fields']

    def match(self, op):
        return op['ns'] == self.ns and op['op'] in ['i', 'u']

    def run(self, op):
        o = op['o']
        obj = {'id': str(o['_id'])}
        for field in self.fields:
            if field in o:
                obj[field] = o[field]
        return obj


def init(conn, solr, schemas):
    for schema in schemas:
        logging.debug("Importing all documents from ns '%s' to solr" %
                      schema.ns)
        coll = conn
        for part in schema.ns.split('.'):
            coll = coll[part]

        docs = [schema.run({'o': obj}) for obj in coll.find()]
        solr.add(docs)


def run(mongo_host='localhost', mongo_port=27017,
        solr_url="http://127.0.0.1:8983/solr/"):
    conn = pymongo.Connection(mongo_host, mongo_port)
    db = conn.local
    solr = pysolr.Solr(solr_url)
    schemas = [Schema(o) for o in db.fts.schemas.find()]
    spec = {}
    cursor = None

    state = db.fts.find_one({'_id': 'state'})
    if state and 'ts' in state:
        first = db.oplog['$main'].find_one()
        if (first['ts'].time > state['ts'].time and
            first['ts'].inc > state['ts'].inc):

            init(conn, solr, schemas)
        else:
            spec['ts'] = {'$gt': state['ts']}
    else:
        init(conn, solr, schemas)

    while True:
        if not cursor or not cursor.alive:
            cursor = db.oplog['$main'].find(spec, tailable=True).sort(
                "$natural", 1)

        solr_docs = []
        for op in cursor:
            if op['op'] in ['i', 'u']:
                for schema in schemas:
                    if schema.match(op):
                        solr_docs.append(schema.run(op))
                spec['ts'] = {'$gt': op['ts']}

        if solr_docs:
            logging.debug('Sending %d docs to solr' % len(solr_docs))
            solr.add(solr_docs)

        db.fts.save({'_id': 'state', 'ts': spec['ts']['$gt']})

        time.sleep(1)


if __name__ == '__main__':
    import argparse

    logging.basicConfig(level=logging.DEBUG)

    parser = argparse.ArgumentParser()
    parser.add_argument('--mongo_host', '-m', dest='mongo_host', type=str,
                        default="localhost",
                        help=("hostname or IP address of the Mongo instance"
                              " to use"))
    parser.add_argument('--mongo_port', '-p', dest='mongo_port', type=int,
                        default=27017,
                        help="port number of the Mongo instance")
    parser.add_argument('--solr_url', '-s', dest='solr_url', type=str,
                        default="http://127.0.0.1:8983/solr/",
                        help="URL of the Solr instance to use")

    args = parser.parse_args()

    run(mongo_host=args.mongo_host, mongo_port=args.mongo_port,
        solr_url=args.solr_url)
