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

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <microhttpd.h>
#include "httpcommon.h"

/*
 * HTTPHandler Param Query
 * This structure contain parameters sent to MHD to retrieve some informations
 */
struct HTTPHandlerPQ
{
	std::vector<std::string> params;
};

struct HTTPQuery
{
	std::unordered_map<std::string, std::string> headers;
	std::unordered_map<std::string, std::string> params;
};

typedef std::function<bool(const HTTPQuery &, std::string &)> HTTPServerRequestHandler;
struct HTTPServerReqHandler
{
	HTTPServerRequestHandler handler;
	HTTPHandlerPQ handler_params_query;
};

#define BIND_HTTPSERVER_HANDLER(s, m, u, hdl, obj, params) \
	s->register_handler(HTTP_METHOD_##m, u, \
		std::bind(hdl, obj, std::placeholders::_1, std::placeholders::_2), params);

typedef std::unordered_map<std::string, HTTPServerReqHandler> HTTPServerReqHandlerMap;
class HTTPServer
{
public:
	HTTPServer(const uint16_t http_port);
	virtual ~HTTPServer();

	static int request_handler(void *http_server, MHD_Connection *connection,
			const char *url, const char *method, const char *version,
			const char *upload_data, size_t *upload_data_size, void **ptr);

	static void request_completed(void *cls, struct MHD_Connection *connection,
					   void **con_cls, MHD_RequestTerminationCode toe);

	bool handle_query(HTTPMethod m, MHD_Connection *conn, const std::string &url, std::string &result);

	void register_handler(HTTPMethod method, const std::string &url,
		const HTTPServerRequestHandler &hdl, const HTTPHandlerPQ &pq)
	{
		m_handlers[method][url] = {hdl, pq};
	}

	uint16_t get_port() const { return m_http_port; }
private:
	static int mhd_iter_headers(void *cls, enum MHD_ValueKind kind, const char *key,
		const char *value);
	MHD_Daemon *m_mhd_daemon = nullptr;

	HTTPServerReqHandlerMap m_handlers[HTTP_METHOD_MAX];
	uint16_t m_http_port;
};
