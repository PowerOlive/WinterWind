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

#include <iostream>
#include <curl/curl.h>
#include <cassert>
#include <cstring>
#include "httpclient.h"

std::atomic_bool HTTPClient::m_inited(false);

HTTPClient::HTTPClient(uint32_t max_file_size):
		m_maxfilesize(max_file_size)
{
	if (!HTTPClient::m_inited) {
		curl_global_init(CURL_GLOBAL_ALL);
	}
}

HTTPClient::~HTTPClient()
{
	delete m_json_writer;
	delete m_json_reader;
}

void HTTPClient::deinit()
{
	curl_global_cleanup();
}

Json::Writer *HTTPClient::json_writer()
{
	if (!m_json_writer) {
        m_json_writer = new Json::FastWriter();
	}

	return m_json_writer;
}

Json::Reader *HTTPClient::json_reader()
{
	if (!m_json_reader) {
		m_json_reader = new Json::Reader();
	}

	return m_json_reader;
}

size_t HTTPClient::curl_writer(char *data, size_t size, size_t nmemb,
							   void *read_buffer) {
	size_t realsize = size * nmemb;
	((std::string *)read_buffer)->append((const char *) data, realsize);
	return realsize;
}

void HTTPClient::request(std::string url, std::string &res,
		int32_t flag, HTTPMethod method, const std::string &post_data)
{
	CURL *curl = curl_easy_init();
	m_http_code = 0;

	{
		std::string buf = "";
		bool first_param = true;
		for (const auto &p: m_uri_params) {
			if (first_param) {
				url.append("?");
				first_param = false;
			} else {
				url.append("&");
			}

			http_string_escape(p.first, buf);
			url += buf + "=";
			http_string_escape(p.second, buf);
			url += buf;
		}
	}

	struct curl_slist *chunk = NULL;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, m_maxfilesize); // Limit request size to 20ko
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER , (flag & HTTPCLIENT_REQ_NO_VERIFY_PEER) ? 0 : 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST , 1);

	switch (method) {
		case HTTP_METHOD_DELETE: {
			static const std::string method_propfind = "DELETE";
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_propfind.c_str());
			break;
		}
		case HTTP_METHOD_HEAD: {
			static const std::string method_propfind = "HEAD";
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_propfind.c_str());
			break;
		}
		case HTTP_METHOD_PATCH: {
			static const std::string method_propfind = "PATCH";
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_propfind.c_str());
			break;
		}
		case HTTP_METHOD_PROPFIND: {
			static const std::string method_propfind = "PROPFIND";
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_propfind.c_str());
			break;
		}
		case HTTP_METHOD_PUT: {
			static const std::string method_propfind = "PUT";
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_propfind.c_str());
			break;
		}
		case HTTP_METHOD_POST:
		case HTTP_METHOD_GET:
		default: break;
	}

	if (flag & HTTPCLIENT_REQ_AUTH) {
		std::string auth_str = m_username + ":" + m_password;
		curl_easy_setopt(curl, CURLOPT_USERPWD, auth_str.c_str());
	}

	for (const auto &h: m_http_headers) {
		const std::string header = std::string(h.first + ": " + h.second).c_str();
		chunk = curl_slist_append(chunk, header.c_str());
	}

	if (chunk) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	}

	if (!post_data.empty()) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
	}

	// @TODO add flag to add custom headers
#if defined(__FreeBSD__)
	curl_easy_setopt(curl, CURLOPT_CAINFO, "/usr/local/etc/ssl/cert.pem");
#else
	curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/cert.pem");
#endif

	CURLcode r = curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &m_http_code);

	if (chunk) {
		curl_slist_free_all(chunk);
	}

	if (r != CURLE_OK) {
		std::cerr << "HTTPClient: curl_easy_perform failed to do request! Error was: "
			<< curl_easy_strerror(r) << std::endl;
	}

	curl_easy_cleanup(curl);

	if (!(flag & HTTPCLIENT_REQ_KEEP_HEADER_CACHE_AFTER_REQUEST)) {
		m_http_headers.clear();
	}

	m_uri_params.clear();
}

void HTTPClient::get_html_tag_value(const std::string &url, const std::string &xpath,
		std::vector<std::string> &res, int32_t pflag)
{
	std::string page_res = "";
	assert(!((pflag & XMLPARSER_XML_SIMPLE) && (pflag & XMLPARSER_XML_WITHOUT_TAGS)));
	_get(url, page_res);

	XMLParser parser(XMLPARSER_MODE_HTML);
	parser.parse(page_res, xpath, pflag, res);
}

bool HTTPClient::_get_json(const std::string &url, Json::Value &res, const HTTPHeadersMap &headers)
{
	std::string res_str = "";
	add_http_header("Content-Type", "application/json");
	for (const auto &header: headers) {
		add_http_header(header.first, header.second);
	}

	_get(url, res_str, HTTPCLIENT_REQ_SIMPLE);

	if (!json_reader()->parse(res_str, res)) {
		std::cerr << "Failed to parse query for " << url << std::endl;
		return false;
	}

	return true;
}

bool HTTPClient::_post_json(const std::string &url, const Json::Value &data,
		Json::Value &res)
{
	std::string res_str = "";
	add_http_header("Content-Type", "application/json");
	_post(url, json_writer()->write(data), res_str);

	if (m_http_code == 400) {
		std::cerr << "Bad request for " << url << ", error was: '" << res_str
				<< "'" <<std::endl;
		return false;
	}

	if (res_str.empty() || !json_reader()->parse(res_str, res)) {
		std::cerr << "Failed to parse query for " << url << std::endl;
		return false;
	}

	return true;
}

void HTTPClient::http_string_escape(const std::string &src, std::string &dst)
{
	dst.clear();

	CURL *curl = curl_easy_init();
	if (char *output = curl_easy_escape(curl, src.c_str(), (int) src.length())) {
		dst = std::string(output);
		curl_free(output);
	}

	curl_easy_cleanup(curl);
}
