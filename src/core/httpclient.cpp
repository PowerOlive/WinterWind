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

#include "httpclient.h"
#include <cassert>
#include <cstring>
#include <curl/curl.h>
#include "cmake_config.h"
#include "http/query.h"

namespace winterwind
{

log4cplus::Logger httpc_log = logger.getInstance(LOG4CPLUS_TEXT("httpc"));

namespace http
{
std::atomic_bool HTTPClient::m_inited(false);

HTTPClient::HTTPClient(uint32_t max_file_size) : m_maxfilesize(max_file_size)
{
	if (!HTTPClient::m_inited) {
		curl_global_init(CURL_GLOBAL_ALL);
	}
}

void HTTPClient::deinit()
{
	curl_global_cleanup();
}

Json::Writer &HTTPClient::json_writer()
{
	if (m_json_writer == nullptr) {
		m_json_writer = std::make_unique<Json::FastWriter>();
	}

	return *m_json_writer;
}

Json::Reader &HTTPClient::json_reader()
{
	if (m_json_reader == nullptr) {
		m_json_reader = std::make_unique<Json::Reader>();
	}

	return *m_json_reader;
}

size_t HTTPClient::curl_writer(char *data, size_t size, size_t nmemb, void *read_buffer)
{
	size_t realsize = size * nmemb;
	((std::string *) read_buffer)->append((const char *) data, realsize);
	return realsize;
}

void HTTPClient::request(const Query &query, std::string &res)
{
	assert(query.get_method() < METHOD_MAX);

	std::string url = query.get_url();

	CURL *curl = curl_easy_init();
	m_http_code = 0;

	{
		std::string buf;
		bool first_param = true;
		for (const auto &p : m_uri_params) {
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
	curl_easy_setopt(curl, CURLOPT_MAXFILESIZE,
		m_maxfilesize); // Limit request size to 20ko
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,
		(query.get_flag() & Query::FLAG_NO_VERIFY_PEER) ? 0 : 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);

	static const char *method_str[METHOD_MAX] = {
		"DELETE",
		"GET",
		"HEAD",
		"PATCH",
		"POST",
		"PROPFIND",
		"PUT",
	};

	switch (query.get_method()) {
		case DELETE:
		case HEAD:
		case PATCH:
		case PROPFIND:
		case PUT:
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_str[query.get_method()]);
			break;
		case POST:
		case GET:
		default:
			break;
	}

	if ((query.get_flag() & Query::FLAG_AUTH) != 0) {
		std::string auth_str = m_username + ":" + m_password;
		curl_easy_setopt(curl, CURLOPT_USERPWD, auth_str.c_str());
	}

	for (const auto &h : m_http_headers) {
		const std::string header = std::string(h.first + ": " + h.second);
		chunk = curl_slist_append(chunk, header.c_str());
	}

	if (chunk != nullptr) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	}

	std::string post_data = query.get_post_data();

	if (!m_form_params.empty()) {
		if (!query.get_post_data().empty()) {
			log_error(httpc_log, "HTTPClient: post_data is not empty while form_params "
				"storage has elements. This will ignore "
				" post_data. (url was: " << url << ").");
		}
		post_data.clear();
		std::string buf;
		bool first_param = true;
		for (const auto &p : m_form_params) {
			if (first_param) {
				first_param = false;
			} else {
				post_data.append("&");
			}

			http_string_escape(p.first, buf);
			post_data += buf + "=";
			http_string_escape(p.second, buf);
			post_data += buf;
		}
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
		log_error(httpc_log, "HTTPClient: curl_easy_perform failed to do request! "
			"Error was: " << curl_easy_strerror(r));
	}

	curl_easy_cleanup(curl);

	if ((query.get_flag() & Query::FLAG_KEEP_HEADER_CACHE_AFTER_REQUEST) == 0) {
		m_http_headers.clear();
	}

	m_uri_params.clear();
	m_form_params.clear();

	log_debug(httpc_log, "request: " << method_str[query.get_method()] << " " << url);
}

void HTTPClient::get_html_tag_value(const std::string &url, const std::string &xpath,
	std::vector<std::string> &res, int32_t pflag)
{
	std::string page_res;
	assert(!((pflag & XMLParser::Flag::FLAG_XML_SIMPLE) &&
		(pflag & XMLParser::Flag::FLAG_XML_WITHOUT_TAGS)));

	Query query(url);
	request(query, page_res);

	XMLParser parser(XMLParser::Mode::MODE_HTML);
	parser.parse(page_res, xpath, pflag, res);
}

void HTTPClient::prepare_json_query()
{
	add_http_header("Content-Type", "application/json");
}

bool HTTPClient::_delete(const Query &query, Json::Value &res)
{
	std::string res_str;
	prepare_json_query();

	request(query, res_str);
	if (!json_reader().parse(res_str, res)) {
		log_error(httpc_log, "Failed to parse query for " << query.get_url()
			<< ". Response was not a JSON");
#if UNITTESTS
		log_debug(httpc_log, "Response was: " << res_str << " http rc: " << m_http_code);
#endif
		return false;
	}

	return true;
}

bool HTTPClient::_get_json(Query &query, const Json::Value &req,
		Json::Value &res)
{
	return _get_json(query, res, json_writer().write(req));
}

bool HTTPClient::_get_json(Query &query, Json::Value &res, const std::string &reqdata)
{
	std::string res_str;
	prepare_json_query();

	if (!reqdata.empty()) {
		query.set_post_data(reqdata);
	}

	request(query, res_str);

	if (!json_reader().parse(res_str, res)) {
		log_error(httpc_log, "Failed to parse query for " << query.get_url()
			<< ". Response was not a JSON");
#if UNITTESTS
		log_debug(httpc_log, "Response was: " << res_str << " http rc: " << m_http_code);
#endif
		return false;
	}

	return true;
}

bool
HTTPClient::_post_json(Query &query, const Json::Value &data, Json::Value &res)
{
	std::string res_str;
	prepare_json_query();

	// @Todo delete set_post_data after
	query.set_post_data(json_writer().write(data));
	request(query, res_str);

	if (m_http_code == 400) {
		log_fatal(httpc_log, "Bad request for " << query.get_url() << ", error was: '"
			<< res_str << "'");
		return false;
	}

	if (((query.get_flag() & Query::FLAG_NO_RESPONSE_AWAITED) != 0) && res_str.empty()) {
		return true;
	}

	if (res_str.empty() || !json_reader().parse(res_str, res)) {
		log_error(httpc_log, "Failed to parse query for " << query.get_url());
#if UNITTESTS
		log_debug(httpc_log, "Response was: " << res_str << " http rc: " << m_http_code);
#endif
		return false;
	}

	return true;
}

bool
HTTPClient::_put_json(Query &query, const Json::Value &data, Json::Value &res)
{
	std::string res_str;
	prepare_json_query();
	query.set_post_data(json_writer().write(data));
	request(query, res_str);

	if (m_http_code == 400) {
		log_fatal(httpc_log, "Bad request for " << query.get_url() << ", error was: '"
			<< res_str << "'");
		return false;
	}

	if (((query.get_flag() & Query::FLAG_NO_RESPONSE_AWAITED) != 0) && res_str.empty()) {
		return true;
	}

	if (res_str.empty() || !json_reader().parse(res_str, res)) {
		log_error(httpc_log, "Failed to parse query for " << query.get_url());
#if UNITTESTS
		log_debug(httpc_log, "Response was: " << res_str << " http rc: " << m_http_code);
#endif
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

void
HTTPClient::add_uri_param(const std::string &param, const std::string &value, bool escape)
{
	if (escape) {
		std::string value_esc;
		http_string_escape(value, value_esc);
		m_uri_params[param] = value_esc;
	} else {
		m_uri_params[param] = value;
	}
}

void HTTPClient::add_form_param(const std::string &param, const std::string &value,
	bool escape)
{
	if (escape) {
		std::string value_esc;
		http_string_escape(value, value_esc);
		m_form_params[param] = value_esc;
	} else {
		m_form_params[param] = value;
	}
}
}
}
