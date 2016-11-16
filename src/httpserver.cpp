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
#include <microhttpd.h>
#include <cstring>
#include <cassert>
#include <iostream>

static const char* NOT_FOUND_PAGE = "<html><head><title>Not found</title></head><h1>No resource found at this address.</h1></html>";

HTTPServer::HTTPServer(const uint16_t http_port): m_http_port(http_port)
{
	m_mhd_daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, m_http_port, NULL,
			NULL, &HTTPServer::request_handler, this, MHD_OPTION_END);

	register_handler(HTTP_METHOD_GET, "/toto.html", nullptr, {"tutu"});
}

HTTPServer::~HTTPServer()
{
	if (m_mhd_daemon) {
		MHD_stop_daemon(m_mhd_daemon);
	}
}

int HTTPServer::request_handler(void *http_server, struct MHD_Connection *connection,
	const char *url, const char *method, const char *version, const char *upload_data,
	size_t *upload_data_size, void **ptr)
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

	if (&dummy != *ptr) {
		/* The first time only the headers are valid,
		   do not respond in the first round... */
		*ptr = &dummy;
		return MHD_YES;
	}
	if (http_method == HTTP_METHOD_GET && *upload_data_size != 0) {
		return MHD_NO; /* upload data in a GET!? */
	}

	*ptr = NULL; /* clear context pointer */

	std::string url_str(url);
	std::string result = "";
	switch (http_method) {
		case HTTP_METHOD_GET: httpd->handle_get(connection, url_str, result); break;
		case HTTP_METHOD_POST: httpd->handle_post(connection, url_str, result); break;
		case HTTP_METHOD_PUT: httpd->handle_put(connection, url_str, result); break;
		case HTTP_METHOD_PATCH: httpd->handle_patch(connection, url_str, result); break;
		case HTTP_METHOD_PROPFIND: httpd->handle_propfind(connection, url_str, result); break;
		case HTTP_METHOD_DELETE: httpd->handle_delete(connection, url_str, result); break;
		case HTTP_METHOD_HEAD: httpd->handle_head(connection, url_str, result); break;
		default: assert(false); // This should not happen
	}

	response = MHD_create_response_from_buffer(result.length(),
			(void*) result.c_str(), MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	return ret;
}

bool HTTPServer::handle_get(MHD_Connection *conn, const std::string &url, std::string &result)
{
	std::cout << url << std::endl;
	HTTPServerReqHandlerMap::const_iterator url_handler = m_handlers[HTTP_METHOD_GET].find(url);
	if (url_handler == m_handlers[HTTP_METHOD_GET].end()) {
		return false;
	}

	// Read which params we want and store them
	HTTPQueryParams rp;
	for (const auto &p: url_handler->second.query_parameters) {
		rp.query_params[p] = std::string(MHD_lookup_connection_value(conn,
				MHD_GET_ARGUMENT_KIND, p.c_str()));
	}

	return url_handler->second.handler(rp, result);
}

bool HTTPServer::handle_delete(MHD_Connection *conn, const std::string &url, std::string &result)
{
	HTTPServerReqHandlerMap &url_handler = m_handlers[HTTP_METHOD_DELETE];
	if (url_handler.find(url) == url_handler.end()) {
		return false;
	}

	return true;
}

bool HTTPServer::handle_head(MHD_Connection *conn, const std::string &url, std::string &result)
{
	HTTPServerReqHandlerMap &url_handler = m_handlers[HTTP_METHOD_HEAD];
	if (url_handler.find(url) == url_handler.end()) {
		return false;
	}

	return true;
}

bool HTTPServer::handle_patch(MHD_Connection *conn, const std::string &url, std::string &result)
{
	HTTPServerReqHandlerMap &url_handler = m_handlers[HTTP_METHOD_PATCH];
	if (url_handler.find(url) == url_handler.end()) {
		return false;
	}

	return true;
}

bool HTTPServer::handle_post(MHD_Connection *conn, const std::string &url, std::string &result)
{
	HTTPServerReqHandlerMap &url_handler = m_handlers[HTTP_METHOD_POST];
	if (url_handler.find(url) == url_handler.end()) {
		return false;
	}

	return true;
}

bool HTTPServer::handle_put(MHD_Connection *conn, const std::string &url, std::string &result)
{
	HTTPServerReqHandlerMap &url_handler = m_handlers[HTTP_METHOD_PUT];
	if (url_handler.find(url) == url_handler.end()) {
		return false;
	}

	return true;
}

bool HTTPServer::handle_propfind(MHD_Connection *conn, const std::string &url, std::string &result)
{
	HTTPServerReqHandlerMap &url_handler = m_handlers[HTTP_METHOD_PROPFIND];
	if (url_handler.find(url) == url_handler.end()) {
		return false;
	}

	return true;
}

