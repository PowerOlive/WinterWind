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

#include <string>
#include <json/json.h>
#include <unordered_map>
#include "xmlparser.h"

enum HTTPClientMethod {
	HTTPCLIENT_METHOD_DELETE,
	HTTPCLIENT_METHOD_GET,
	HTTPCLIENT_METHOD_HEAD,
	HTTPCLIENT_METHOD_PATCH,
	HTTPCLIENT_METHOD_PROPFIND,
	HTTPCLIENT_METHOD_PUT,
};

enum HTTPClientReqFlag {
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

	void perform_request(const std::string &url, std::string &res, int32_t
		flag = HTTPCLIENT_REQ_SIMPLE, HTTPClientMethod method = HTTPCLIENT_METHOD_GET,
		const std::string &post_data = "");

	inline void perform_post(const std::string &url, const std::string &post_data,
		std::string &res, int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		perform_request(url, res, flag, HTTPCLIENT_METHOD_GET, post_data);
	}

	inline void perform_get(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		perform_request(url, res, flag, HTTPCLIENT_METHOD_GET, "");
	}

	inline void perform_delete(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		perform_request(url, res, flag, HTTPCLIENT_METHOD_DELETE, "");
	}

	inline void perform_head(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE)
	{
		perform_request(url, res, flag, HTTPCLIENT_METHOD_HEAD, "");
	}

	inline void perform_propfind(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE, const std::string &post_data = "")
	{
		perform_request(url, res, flag, HTTPCLIENT_METHOD_PROPFIND, post_data);
	}

	inline void perform_put(const std::string &url, std::string &res,
		int32_t flag = HTTPCLIENT_REQ_SIMPLE, const std::string &post_data = "")
	{
		perform_request(url, res, flag, HTTPCLIENT_METHOD_PUT, post_data);
	}

	void fetch_html_tag_value(const std::string &url, const std::string &xpath,
			std::vector<std::string> &res, int32_t pflag = XMLPARSER_XML_SIMPLE);
	bool fetch_json(const std::string &url, Json::Value &res)
	{
		return fetch_json(url, {}, res);
	}

	bool fetch_json(const std::string &url,
			const HTTPHeadersMap &headers, Json::Value &res);

	void add_http_header(const std::string &header, const std::string &value)
	{
		m_http_headers[header] = value;
	}

	bool post_json(const std::string &url, const std::string &post_data, Json::Value &res);

	long get_http_code() const { return m_http_code; }

	void http_string_escape(const std::string &src, std::string &dst);
protected:
	static size_t curl_writer(char *data, size_t size, size_t nmemb, void *user_data);

	std::string m_username = "";
	std::string m_password = "";
	std::unordered_map<std::string, std::string> m_http_headers;
	long m_http_code = 0;
private:
	uint32_t m_maxfilesize = 0;
};
