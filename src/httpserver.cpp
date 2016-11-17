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

#include "httpserver.h"
#include <cstring>
#include <cassert>
#include <iostream>
#include <stdlib.h>

static const char* NOT_FOUND_PAGE = "<html><head><title>Not found</title></head><body><h1>No resource found at this address.</h1></body></html>";

HTTPServer::HTTPServer(const uint16_t http_port): m_http_port(http_port)
{
	m_mhd_daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, m_http_port, NULL,
			NULL, &HTTPServer::request_handler, this, MHD_OPTION_NOTIFY_COMPLETED,
			&HTTPServer::request_completed, NULL, MHD_OPTION_END);
}

HTTPServer::~HTTPServer()
{
	if (m_mhd_daemon) {
		MHD_stop_daemon(m_mhd_daemon);
	}
}

int HTTPServer::request_handler(void *http_server, struct MHD_Connection *connection,
	const char *url, const char *method, const char *version, const char *upload_data,
	size_t *upload_data_size, void **con_cls)
{
	HTTPServer *httpd = (HTTPServer *) http_server;
	HTTPMethod http_method;
	static int dummy;
	struct MHD_Response *response;
	int ret;

	if (strcmp(method, "GET") == 0) {
		http_method = HTTP_METHOD_GET;
	}
	else if (strcmp(method, "POST") == 0) {
		http_method = HTTP_METHOD_POST;
	}
	else if (strcmp(method, "PUT") == 0) {
		http_method = HTTP_METHOD_PUT;
	}
	else if (strcmp(method, "PATCH") == 0) {
		http_method = HTTP_METHOD_PATCH;
	}
	else if (strcmp(method, "PROPFIND") == 0) {
		http_method = HTTP_METHOD_PROPFIND;
	}
	else if (strcmp(method, "DELETE") == 0) {
		http_method = HTTP_METHOD_DELETE;
	}
	else if (strcmp(method, "HEAD") == 0) {
		http_method = HTTP_METHOD_HEAD;
	}
	else {
		return MHD_NO; /* unexpected method */
	}

	if (&dummy != *con_cls) {
		/* The first time only the headers are valid,
		   do not respond in the first round...*/
		*con_cls = &dummy;
		return MHD_YES;
	}

	if (http_method == HTTP_METHOD_GET && *upload_data_size != 0) {
		return MHD_NO; /* upload data in a GET!? */
	}

	*con_cls = NULL; /* clear context pointer */

	std::string result = "";
	httpd->handle_query(http_method, connection, std::string(url), result);

	response = MHD_create_response_from_buffer(result.length(),
			(void*) result.c_str(), MHD_RESPMEM_MUST_COPY);
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	return ret;
}

void HTTPServer::request_completed(void *cls, struct MHD_Connection *connection,
		void **con_cls, MHD_RequestTerminationCode toe)
{
	// @TODO callback on request complete ?
}

bool HTTPServer::handle_query(HTTPMethod m, MHD_Connection *conn, const std::string &url, std::string &result)
{
	assert(m < HTTP_METHOD_MAX);

	HTTPServerReqHandlerMap::const_iterator url_handler = m_handlers[m].find(url);
	if (url_handler == m_handlers[m].end()) {
		return false;
	}

	// Read which params we want and store them
	HTTPQueryParams rp;
	for (const auto &p: url_handler->second.query_parameters) {
		const char *qp_res = MHD_lookup_connection_value(conn,
				MHD_GET_ARGUMENT_KIND, p.c_str());
		rp.query_params[p] = qp_res != NULL ? std::string(qp_res) : "";
	}

	return url_handler->second.handler(rp, result);
}
