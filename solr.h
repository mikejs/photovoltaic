#ifndef _SOLR_H_
#define _SOLR_H_

#include <libxml/tree.h>
#include <libxml/parser.h>

typedef xmlNodePtr solr_doc;

void solr_init();
void solr_cleanup();

solr_doc solr_doc_new(const char *id);
void solr_doc_free(solr_doc doc);
void solr_doc_add_field(solr_doc doc, const char *name, const char *content);

void solr_add_doc(solr_doc doc);


#endif
