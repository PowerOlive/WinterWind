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

#include "twitterclient.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <map>
#include <algorithm>
#include "utils/base64.h"
#include "utils/hmac.h"

static const std::string TWITTER_API_URL = "https://api.twitter.com";
static const std::string TWITTER_OAUTH2_TOKEN = "/oauth2/token";
static const std::string TWITTER_HOME_TIMELINE_1_1 = "/1.1/statuses/home_timeline.json";
static const std::string TWITTER_USER_TIMELINE_1_1 = "/1.1/statuses/user_timeline.json";

TwitterClient::TwitterClient(const std::string &consumer_key, const std::string &consumer_secret,
	const std::string &access_token, const std::string &access_token_secret):
	HTTPClient(),
	m_consumer_key(consumer_key),
	m_consumer_secret(consumer_secret),
	m_access_token(access_token),
	m_access_token_secret(access_token_secret)
{
	m_rand_engine = std::mt19937((unsigned long) std::time(0));
}

void TwitterClient::append_auth_header()
{
	std::string tmp_auth = m_consumer_key + ":" + m_consumer_secret;
	std::string authorization = "Basic " +
		base64_encode((const unsigned char*) tmp_auth.c_str(), tmp_auth.length());
	add_http_header("Authorization", authorization);
}

void TwitterClient::append_bearer_header()
{
	add_http_header("Authorization", "Bearer " + m_bearer_token);
}

#define TWITTER_OAUTH_NONCE_LENGTH 32
void TwitterClient::append_oauth_header(const std::string &method, const std::string &url)
{
	std::string oauth_nonce = "";
	std::uniform_int_distribution<uint8_t> normal_dist(0, 255);
	for (uint8_t i = 0; i < TWITTER_OAUTH_NONCE_LENGTH; i++) {
		oauth_nonce += (uint8_t) normal_dist(m_rand_engine);
	}

	std::string buf = base64_encode(oauth_nonce);
	time_t t = std::time(0);

	std::map<std::string, std::string> ordered_params = {};

	ordered_params["oauth_consumer_key"] = m_consumer_key;
	ordered_params["oauth_nonce"] = buf;
	ordered_params["oauth_signature_method"] = "HMAC-SHA1";
	ordered_params["oauth_timestamp"] = std::to_string(t);
	ordered_params["oauth_token"] = m_access_token;
	ordered_params["oauth_version"] = "1.0";

	// Append request params
	for (const auto &p: m_http_request_params) {
		ordered_params[p.first] = p.second;
	}

	http_string_escape(url, buf);
	std::string signature = method + "&" + buf + "&";

	for (const auto &p: ordered_params) {
		http_string_escape(p.first, buf);
		signature += buf + "=";
		http_string_escape(p.second, buf);
		signature += buf + "&";
	}

	signature[signature.size() - 1] = '\0';

	// Generate signing key
	std::string signing_key = "";
	http_string_escape(m_consumer_secret, buf);
	signing_key += buf + "&";
	http_string_escape(m_access_token_secret, buf);
	signing_key += buf;

	ordered_params["oauth_signature"] = base64_encode(hmac_sha1(signing_key, signature));
	std::stringstream header;
	header << std::string("OAuth ");

	static const std::string oauth_prefix = "oauth_";
	for (const auto &p: ordered_params) {
		// Only manipulate oauth header
		if (p.first.substr(0, oauth_prefix.size()) != oauth_prefix) {
			continue;
		}

		http_string_escape(p.first, buf);
		header << buf << "=\"";
		http_string_escape(p.second, buf);
		header << buf << "\", ";
	}

	std::cout << header.str() << std::endl;
	add_http_header("Authorization", header.str());
}

TwitterClient::Response TwitterClient::get_oauth2_token()
{
	append_auth_header();
	add_http_header("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");

	std::string res = "";
	_post(TWITTER_API_URL + TWITTER_OAUTH2_TOKEN, "grant_type=client_credentials", res);

	if (get_http_code() == 403) {
		return TWITTER_FORBIDDEN;
	}

	Json::Value json_res;
	if (!json_reader()->parse(res, json_res)) {
		return TWITTER_INVALID_RESPONSE;
	}

	if (!json_res.isMember("access_token")) {
		return TWITTER_INVALID_RESPONSE;
	}

	m_bearer_token = json_res["access_token"].asString();
	return TWITTER_OK;
}

TwitterClient::Response TwitterClient::get_user_timeline(Json::Value &res, const uint16_t count,
	const uint32_t since_id, bool include_rts, bool contributor_details)
{
	append_bearer_header();
	std::string request = TWITTER_API_URL + TWITTER_USER_TIMELINE_1_1;

	if (count > 0) {
		add_http_request_param("count", std::to_string(count));
	}

	if (since_id > 0) {
		add_http_request_param("since_id", std::to_string(since_id));
	}

	if (include_rts) {
		add_http_request_param("include_rts", "true");
	}

	if (contributor_details) {
		add_http_request_param("contributor_details", "true");
	}

	append_oauth_header("GET", request);

	_get_json(request, res);
	std::cout << res.toStyledString() << std::endl;

	switch (get_http_code()) {
		case 401: return TWITTER_UNAUTHORIZED;
		case 403: return TWITTER_FORBIDDEN;
		default: break;
	}

	return TWITTER_OK;
}

TwitterClient::Response TwitterClient::get_home_timeline(Json::Value &res, const uint16_t count,
	const uint32_t since_id)
{
	std::string request = TWITTER_API_URL + TWITTER_HOME_TIMELINE_1_1;

	if (count > 0) {
		add_http_request_param("count", std::to_string(count));
	}

	if (since_id > 0) {
		add_http_request_param("since_id=", std::to_string(since_id));
	}

	append_oauth_header("GET", request);

	_get_json(request, res);
	std::cout << res.toStyledString() << std::endl;

	switch (get_http_code()) {
		case 401: return TWITTER_UNAUTHORIZED;
		case 403: return TWITTER_FORBIDDEN;
		default: break;
	}

	return TWITTER_OK;
}
