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
