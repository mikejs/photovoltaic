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
#include "solr.h"
#include <string.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

static CURL *curl = NULL;

struct solr_docset_t {
    solr_doc docs;
};

void solr_init() {
    curl_global_init(CURL_GLOBAL_NOTHING);
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8983/solr/update");
}

solr_doc solr_doc_new(const char *id) {
    solr_doc doc;
    xmlNodePtr id_field;

    doc = xmlNewNode(NULL, BAD_CAST "doc");
    id_field = xmlNewChild(doc, NULL, BAD_CAST "field", BAD_CAST id);
    xmlNewProp(id_field, BAD_CAST "name", BAD_CAST "id");

    return doc;
}

void solr_doc_add_field(solr_doc doc, const char *name, const char *content) {
    xmlNodePtr field;

    field = xmlNewChild(doc, NULL, BAD_CAST "field", BAD_CAST content);
    xmlNewProp(field, BAD_CAST "name", BAD_CAST name);
}

void solr_doc_free(solr_doc doc) {
    xmlFreeNode(doc);
}

solr_docset solr_docset_new() {
    solr_docset docset = malloc(sizeof(struct solr_docset_t));
    docset->docs = NULL;
    return docset;
}

void solr_docset_add_doc(solr_docset docset, solr_doc doc) {
    if (docset->docs == NULL) {
        docset->docs = doc;
    } else {
        xmlAddSibling(docset->docs, doc);
    }
}

void solr_docset_clear(solr_docset docset) {
    xmlFreeNodeList(docset->docs);
    docset->docs = NULL;
}

void solr_docset_free(solr_docset docset) {
    if(docset != NULL) {
        xmlFreeNodeList(docset->docs);
    }

    free(docset);
}

static char* solr_add_doc_xml(solr_doc doc) {
    xmlDocPtr xml = NULL;
    xmlNodePtr root = NULL;
    xmlChar *xmlbuff;
    int buffersize;

    xml = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "add");
    xmlDocSetRootElement(xml, root);
    xmlAddChildList(root, xmlCopyNodeList(doc));

    xmlDocDumpFormatMemory(xml, &xmlbuff, &buffersize, 1);
    xmlFreeDoc(xml);
    xmlCleanupParser();

    return (char*)xmlbuff;
}

void solr_add_doc(solr_doc doc) {
    struct curl_slist *headers = NULL;
    char* xml = solr_add_doc_xml(doc);

    headers = curl_slist_append(headers, "Content-Type: text/xml");

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, xml);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_perform(curl);

    xmlFree(xml);
    curl_slist_free_all(headers);
}

void solr_add_docset(solr_docset docset) {
    /* Don't add empty docset */
    if (docset->docs != NULL) {
        solr_add_doc(docset->docs);
    }
}

void solr_cleanup() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}
