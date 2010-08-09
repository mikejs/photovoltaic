photovoltaic connects MongoDB to a Solr search service. It uses MongoDB's replication internals to automatically update Solr whenever a change is made on the Mongo instance.

Usage
=====

Note: because photovoltaic makes use of MongoDB's replication internals, it can only connect to a MongoDB instance that is configured as a replication master.

For every collection you want to index, create a document in Mongo's local.fts.schemas collection containing a "ns" field consisting of the collection's namespace (collection name prefixed with its database) and a "fields" list containing the fields you want to index. The _id field will be indexed automatically.

::

    $ mongo local
    > db.fts.schemas.save({"ns": "test.data", "fields": ["name", "description"]})
    > exit
    $ ./pv.py

::

    $ mongo test
    > db.data.save({"name": "A name", "description": "This will be indexed."})
    > db.data.save({"description": "So will this"})
