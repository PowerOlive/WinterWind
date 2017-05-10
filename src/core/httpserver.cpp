/*
 * Copyright (c) 2016-2017, Loic Blot <loic.blot@unix-experience.fr>
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

#include "httpserver.h"
#include "utils/stringutils.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

static const char *NOT_FOUND_PAGE = "<html><head><title>Not found</title></head><body><h1>No "
				    "resource found at this address.</h1></body></html>";
static const char *BAD_REQUEST =
    "<html><head><title>Bad request</title></head><body><h1>Bad request</h1></body></html>";

HTTPServer::HTTPServer(const uint16_t http_port) : m_http_port(http_port)
{
	m_mhd_daemon = MHD_start_daemon(
	    MHD_USE_POLL_INTERNALLY, m_http_port, NULL, NULL, &HTTPServer::request_handler, this,
	    MHD_OPTION_NOTIFY_COMPLETED, &HTTPServer::request_completed, NULL, MHD_OPTION_END);
}

HTTPServer::~HTTPServer()
{
	if (m_mhd_daemon) {
		MHD_stop_daemon(m_mhd_daemon);
	}
}

int HTTPServer::request_handler(void *http_server, struct MHD_Connection *connection,
				const char *url, const char *method, const char *version,
				const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	HTTPServer *httpd = (HTTPServer *) http_server;
	HTTPMethod http_method;
	struct MHD_Response *response;
	int ret;

	if (strcmp(method, "GET") == 0) {
		http_method = HTTP_METHOD_GET;
	} else if (strcmp(method, "POST") == 0) {
		http_method = HTTP_METHOD_POST;
	} else if (strcmp(method, "PUT") == 0) {
		http_method = HTTP_METHOD_PUT;
	} else if (strcmp(method, "PATCH") == 0) {
		http_method = HTTP_METHOD_PATCH;
	} else if (strcmp(method, "PROPFIND") == 0) {
		http_method = HTTP_METHOD_PROPFIND;
	} else if (strcmp(method, "DELETE") == 0) {
		http_method = HTTP_METHOD_DELETE;
	} else if (strcmp(method, "HEAD") == 0) {
		http_method = HTTP_METHOD_HEAD;
	} else {
		return MHD_NO; /* unexpected method */
	}

	if (*con_cls == NULL) {
		// The first time only the headers are valid,
		//   do not respond in the first round...
		//   Just init our response
		HTTPRequestSession *session = new HTTPRequestSession();
		*con_cls = session;
		return MHD_YES;
	}

	// Handle request
	HTTPRequestSession *session = (HTTPRequestSession *) *con_cls;
	if (!session->data_handled &&
	    !httpd->handle_query(http_method, connection, std::string(url),
				 std::string(upload_data, *upload_data_size), session)) {
		session->result = std::string(BAD_REQUEST);
		session->http_code = MHD_HTTP_BAD_REQUEST;
	}

	// When post data is available, reinit the data_size because this function
	// is called another time. And mark the current session as handled
	if (*upload_data_size > 0) {
		*upload_data_size = 0;
		session->data_handled = true;
		return MHD_YES;
	}

	response = MHD_create_response_from_buffer(
	    session->result.length(), (void *) session->result.c_str(), MHD_RESPMEM_MUST_COPY);
	ret = MHD_queue_response(connection, session->http_code, response);
	MHD_destroy_response(response);

	// clear context pointer
	delete session;
	*con_cls = NULL;
	return ret;
}

void HTTPServer::request_completed(void *cls, struct MHD_Connection *connection, void **con_cls,
				   MHD_RequestTerminationCode toe)
{
}

bool HTTPServer::handle_query(HTTPMethod m, MHD_Connection *conn, const std::string &url,
			      const std::string &upload_data, HTTPRequestSession *session)
{
	assert(m < HTTP_METHOD_MAX);

	HTTPServerReqHandlerMap::const_iterator url_handler = m_handlers[m].find(url);
	if (url_handler == m_handlers[m].end()) {
		return false;
	}

	HTTPQueryPtr q;

	// Read which params we want and store them
	const char *content_type =
	    MHD_lookup_connection_value(conn, MHD_HEADER_KIND, "Content-Type");
	if (!content_type) {
		q = HTTPQueryPtr(new HTTPQuery());
	} else if (strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
		q = HTTPQueryPtr(new HTTPFormQuery());
		if (!parse_post_data(upload_data, dynamic_cast<HTTPFormQuery *>(q.get()))) {
			return false;
		}
	} else if (strcmp(content_type, "application/json") == 0) {
		HTTPJsonQuery *jq = new HTTPJsonQuery();
		q = HTTPQueryPtr(jq);
		Json::Reader reader;
		if (!reader.parse(upload_data, jq->json_query)) {
			return false;
		}
	} else {
		q = HTTPQueryPtr(new HTTPQuery());
	}

	q->url = url;
	MHD_get_connection_values(conn, MHD_HEADER_KIND, &HTTPServer::mhd_iter_headers, q.get());
	MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND, &HTTPServer::mhd_iter_getargs,
				  q.get());

	HTTPResponsePtr http_response = url_handler->second(q);
	if (!http_response) {
		if (content_type && strcmp(content_type, "application/json") == 0) {
			session->result = "{}";
		}

		return false;
	}

	// Convert response to session response
	*http_response >> *session;
	return true;
}

int HTTPServer::mhd_iter_headers(void *cls, MHD_ValueKind, const char *key, const char *value)
{
	HTTPQuery *q = (HTTPQuery *) cls;
	if (q && key && value) {
		q->headers[std::string(key, strlen(key))] = std::string(value, strlen(value));
	}
	return MHD_YES; // continue iteration
}

int HTTPServer::mhd_iter_getargs(void *cls, MHD_ValueKind, const char *key, const char *value)
{
	HTTPQuery *q = (HTTPQuery *) cls;
	if (q && key && value) {
		q->get_params[std::string(key, strlen(key))] = std::string(value, strlen(value));
	}
	return MHD_YES; // continue iteration
}

bool HTTPServer::parse_post_data(const std::string &data, HTTPFormQuery *qf)
{
	std::vector<std::string> first_split;
	str_split(data, '&', first_split);

	for (const auto &s : first_split) {
		// If this post data is empty, abort
		if (s.empty()) {
			return false;
		}

		std::vector<std::string> kv;
		str_split(s, '=', kv);

		// If the key value pair is invalid, abort
		if (kv.size() != 2 || kv[0].empty() || kv[1].empty()) {
			return false;
		}

		qf->post_data[kv[0]] = kv[1];
	}

	return true;
}
