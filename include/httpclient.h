/**
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

#pragma once

#include <string>
#include <json/json.h>
#include <unordered_map>
#include "httpcommon.h"
#include "xmlparser.h"

enum HTTPClientReqFlag
{
	HTTPCLIENT_REQ_SIMPLE = 0x01,
	HTTPCLIENT_REQ_AUTH = 0x02,
	HTTPCLIENT_REQ_NO_VERIFY_PEER = 0x04,
	HTTPCLIENT_REQ_KEEP_HEADER_CACHE_AFTER_REQUEST = 0x08,
};

typedef std::unordered_map<std::string, std::string> HTTPHeadersMap;
class HTTPClient
{
public:
	HTTPClient(uint32_t max_file_size = 1024 * 1024);
	virtual ~HTTPClient();

	inline void _post(const std::string &url, const std::string &post_data,
		std::string &res, int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		request(url, res, flag, HTTP_METHOD_POST, post_data);
	}

	inline void _get(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		request(url, res, flag, HTTP_METHOD_GET);
	}

	inline void _delete(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		request(url, res, flag, HTTP_METHOD_DELETE);
	}

	inline void _head(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		request(url, res, flag, HTTP_METHOD_HEAD);
	}

	inline void _propfind(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE, const std::string &post_data = "")
	{
		request(url, res, flag, HTTP_METHOD_PROPFIND, post_data);
	}

	inline void _put(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE, const std::string &post_data = "")
	{
		request(url, res, flag, HTTP_METHOD_PUT, post_data);
	}

	void get_html_tag_value(const std::string &url, const std::string &xpath,
			std::vector<std::string> &res, int32_t pflag = XMLPARSER_XML_SIMPLE);

	bool _get_json(const std::string &url, Json::Value &res,
		const HTTPHeadersMap &headers = {});

	void add_http_header(const std::string &header, const std::string &value)
	{
		m_http_headers[header] = value;
	}

	void add_http_request_param(const std::string &param, const std::string &value)
	{
		m_http_request_params[param] = value;
	}

	bool _post_json(const std::string &url, const Json::Value &data, Json::Value &res);

	long get_http_code() const { return m_http_code; }

	void http_string_escape(const std::string &src, std::string &dst);
protected:
	static size_t curl_writer(char *data, size_t size, size_t nmemb, void *user_data);
	void request(std::string url, std::string &res, int32_t
	flag = HTTPCLIENT_REQ_SIMPLE, HTTPMethod method = HTTP_METHOD_GET,
		const std::string &post_data = "");

	std::string m_username = "";
	std::string m_password = "";
	std::unordered_map<std::string, std::string> m_http_headers = {};
	std::unordered_map<std::string, std::string> m_http_request_params = {};
	long m_http_code = 0;
	Json::Writer *json_writer();
	Json::Reader *json_reader();
private:
	Json::FastWriter *m_json_writer = nullptr;
	Json::Reader *m_json_reader = nullptr;
	uint32_t m_maxfilesize = 0;
};
