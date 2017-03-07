/**
 * Copyright (c) 2016, Loic Blot <loic.blot@unix-experience.fr>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "xmlparser.h"
#include <algorithm>
#include <cassert>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

bool XMLParser::parse(const std::string &document, const std::string &xpath, int32_t pflag,
		      std::vector<std::string> &res)
{
	xmlDoc *doc = nullptr;
	switch (m_mode) {
		case Mode::MODE_XML:
			doc = xmlReadMemory(document.c_str(), (int) document.size(), NULL, NULL, 0);
			break;
		case Mode::MODE_HTML:
			doc = htmlReadMemory(document.c_str(), (int) document.size(), NULL, NULL,
					     HTML_PARSE_RECOVER | HTML_PARSE_NOERROR |
						 HTML_PARSE_NOWARNING);
			break;
		default:
			assert(false);
	}

	res = {};
	if (!doc) {
		return false;
	}

	bool parse_res = true;
	xmlXPathContext *xpathCtx = xmlXPathNewContext(doc);
	if (!xpathCtx) {
		parse_res = false;
		goto final_cleanup;
	}

	if (m_mode == Mode::MODE_XML) {
		for (const auto &ns : m_custom_xml_ns) {
			xmlXPathRegisterNs(xpathCtx, BAD_CAST ns.prefix.c_str(),
					   BAD_CAST ns.uri.c_str());
		}
	}

	{
		xmlXPathObjectPtr xpathObj =
		    xmlXPathEvalExpression(BAD_CAST xpath.c_str(), xpathCtx);
		xmlXPathFreeContext(xpathCtx);
		if (!xpathObj) {
			parse_res = false;
			goto final_cleanup;
		}

		xmlNodeSetPtr nodeset = xpathObj->nodesetval;
		if (!xmlXPathNodeSetIsEmpty(nodeset)) {
			for (uint32_t i = 0; i < nodeset->nodeNr; ++i) {
				xmlNodePtr node = nodeset->nodeTab[i];
				xmlChar *nres = nullptr;

				if (pflag & FLAG_XML_SIMPLE) {
					nres = xmlNodeListGetString(doc, node->xmlChildrenNode, 0);
				} else if (pflag & FLAG_XML_WITHOUT_TAGS) {
					nres = xmlXPathCastNodeToString(node);
				} else {
					// This should not happen
					assert(false);
				}

				std::string str_nres((const char *) nres);
				if (pflag & FLAG_XML_STRIP_NEWLINE) {
					str_nres.erase(
					    std::remove(str_nres.begin(), str_nres.end(), '\n'),
					    str_nres.end());
				}

				res.push_back(str_nres);
				xmlFree(nres);
			}
		}

		xmlXPathFreeObject(xpathObj);
	}

final_cleanup:
	xmlCleanupParser();
	xmlFreeDoc(doc);
	return parse_res;
}
