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

#pragma once

#include "httpcommon.h"
#include "xmlparser.h"
#include <atomic>
#include <json/json.h>
#include <string>
#include <unordered_map>
#include <core/utils/log.h>

namespace winterwind
{

extern log4cplus::Logger httpc_log;

namespace http
{

typedef std::unordered_map<std::string, std::string> HeadersMap;

class HTTPClient
{
public:
	explicit HTTPClient(uint32_t max_file_size = 1024 * 1024);

	virtual ~HTTPClient() = default;

	enum ReqFlag: uint8_t
	{
		REQ_SIMPLE = 0x01,
		REQ_AUTH = 0x02,
		REQ_NO_VERIFY_PEER = 0x04,
		REQ_KEEP_HEADER_CACHE_AFTER_REQUEST = 0x08,
		REQ_NO_RESPONSE_AWAITED = 0x10,
	};

	/**
	 * This function should be called at the binary lifetime ending
	 */
	static void deinit();

	inline void
	_post(const std::string &url, std::string &res, int32_t flag = HTTPClient::REQ_SIMPLE)
	{
		_post(url, "", res, flag);
	}

	inline void _post(const std::string &url, const std::string &post_data,
		std::string &res, int32_t flag = HTTPClient::REQ_SIMPLE)
	{
		request(url, res, flag, POST, post_data);
	}

	inline void _get(const std::string &url, std::string &res,
		int32_t flag = HTTPClient::REQ_SIMPLE)
	{
		request(url, res, flag, GET);
	}

	bool _delete(const std::string &url, Json::Value &res,
		int32_t flag = HTTPClient::REQ_SIMPLE);

	inline void _delete(const std::string &url, std::string &res,
		int32_t flag = HTTPClient::REQ_SIMPLE)
	{
		request(url, res, flag, DELETE);
	}

	inline void
	_head(const std::string &url, std::string &res, int32_t flag = HTTPClient::REQ_SIMPLE)
	{
		request(url, res, flag, HEAD);
	}

	inline void _propfind(const std::string &url, std::string &res,
		int32_t flag = HTTPClient::REQ_SIMPLE,
		const std::string &post_data = "")
	{
		request(url, res, flag, PROPFIND, post_data);
	}

	inline void
	_put(const std::string &url, std::string &res, const std::string &post_data = "",
		int32_t flag = HTTPClient::REQ_SIMPLE)
	{
		request(url, res, flag, PUT, post_data);
	}

	void get_html_tag_value(const std::string &url, const std::string &xpath,
		std::vector<std::string> &res,
		int32_t pflag = XMLParser::Flag::FLAG_XML_SIMPLE);

	bool _get_json(const std::string &url, const Json::Value &req, Json::Value &res,
		int32_t flag = HTTPClient::REQ_SIMPLE);
	bool _get_json(const std::string &url, Json::Value &res,
		int32_t flag = HTTPClient::REQ_SIMPLE, const std::string &reqdata = "");

	void add_http_header(const std::string &header, const std::string &value)
	{ m_http_headers[header] = value; }

	void
	add_uri_param(const std::string &param, const std::string &value, bool escape = true);

	void add_form_param(const std::string &param, const std::string &value,
		bool escape = true);

	bool _post_json(const std::string &url, const Json::Value &data, Json::Value &res,
		int32_t flags = HTTPClient::REQ_SIMPLE);

	bool _put_json(const std::string &url, const Json::Value &data, Json::Value &res,
		int32_t flags = HTTPClient::REQ_SIMPLE);

	virtual long get_http_code() const { return m_http_code; }

	/**
	 * HTTP Escape src string
	 *
	 * @param src
	 * @param dst
	 */
	void http_string_escape(const std::string &src, std::string &dst);

protected:
	static size_t curl_writer(char *data, size_t size, size_t nmemb, void *user_data);

	void request(std::string url, std::string &res, int32_t flag = HTTPClient::REQ_SIMPLE,
		Method method = GET, std::string post_data = "");

	void prepare_json_query();

	std::string m_username = "";
	std::string m_password = "";
	std::unordered_map<std::string, std::string> m_http_headers = {};
	std::unordered_map<std::string, std::string> m_uri_params = {};
	std::unordered_map<std::string, std::string> m_form_params = {};
	long m_http_code = 0;

	Json::Writer &json_writer();
	Json::Reader &json_reader();

private:
	std::unique_ptr<Json::FastWriter> m_json_writer = nullptr;
	std::unique_ptr<Json::Reader> m_json_reader = nullptr;
	uint32_t m_maxfilesize = 0;

	static std::atomic_bool m_inited;
};
}
}
