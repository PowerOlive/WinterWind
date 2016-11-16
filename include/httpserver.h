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
#include "httpcommon.h"

struct MHD_Daemon;
struct MHD_Connection;

struct HTTPQueryParams
{
	std::unordered_map<std::string, std::string> query_params;
};

struct HTTPServerReqHandler
{
	typedef std::function<bool(const HTTPQueryParams &, std::string &)> HTTPServerRequestHandler;
	HTTPServerRequestHandler handler;
	std::vector<std::string> query_parameters;
};

typedef std::unordered_map<std::string, HTTPServerReqHandler> HTTPServerReqHandlerMap;
class HTTPServer
{
public:
	HTTPServer(const uint16_t http_port);
	virtual ~HTTPServer();

	static int request_handler(void *http_server, MHD_Connection *connection,
			const char *url, const char *method, const char *version,
			const char *upload_data, size_t *upload_data_size, void **ptr);

	bool handle_get(MHD_Connection *conn, const std::string &url, std::string &result);
	bool handle_post(MHD_Connection *conn, const std::string &url, std::string &result);
	bool handle_put(MHD_Connection *conn, const std::string &url, std::string &result);
	bool handle_patch(MHD_Connection *conn, const std::string &url, std::string &result);
	bool handle_propfind(MHD_Connection *conn, const std::string &url, std::string &result);
	bool handle_delete(MHD_Connection *conn, const std::string &url, std::string &result);
	bool handle_head(MHD_Connection *conn, const std::string &url, std::string &result);

	void register_handler(HTTPMethod method, const std::string &url,
		const std::function<bool(const HTTPQueryParams &, std::string &)> &hdl, const std::vector<std::string> &qp)
	{
		m_handlers[method][url] = {hdl, qp};
	}
private:
	MHD_Daemon *m_mhd_daemon = nullptr;

	HTTPServerReqHandlerMap m_handlers[HTTP_METHOD_MAX];
	uint16_t m_http_port;
};
