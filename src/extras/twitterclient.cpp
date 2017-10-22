/*
 * Copyright (c) 2017, Loic Blot <loic.blot@unix-experience.fr>
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

#include "extras/twitterclient.h"
#include <core/utils/base64.h>
#include <core/http/query.h>

namespace winterwind
{
namespace extras
{
static const std::string TWITTER_API_URL = "https://api.twitter.com";
static const std::string TWITTER_OAUTH2_TOKEN = "/oauth2/token";
static const std::string TWITTER_HOME_TIMELINE_1_1 = "/1.1/statuses/home_timeline.json";
static const std::string TWITTER_USER_TIMELINE_1_1 = "/1.1/statuses/user_timeline.json";

TwitterClient::TwitterClient(const std::string &consumer_key,
	const std::string &consumer_secret,
	const std::string &access_token,
	const std::string &access_token_secret)
	: OAuthClient(consumer_key, consumer_secret, access_token, access_token_secret)
{
}

void TwitterClient::append_auth_header()
{
	std::string tmp_auth = m_consumer_key + ":" + m_consumer_secret;
	std::string authorization =
		"Basic " +
			base64_encode((const unsigned char *) tmp_auth.c_str(), tmp_auth.length());
	add_http_header("Authorization", authorization);
}

void TwitterClient::append_bearer_header()
{
	add_http_header("Authorization", "Bearer " + m_bearer_token);
}

TwitterClient::Response TwitterClient::get_oauth2_token()
{
	append_auth_header();
	add_http_header("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
	add_form_param("grant_type", "client_credentials");

	std::string res;
	request(http::Query(TWITTER_API_URL + TWITTER_OAUTH2_TOKEN, http::POST), res);

	if (get_http_code() == 403) {
		return TWITTER_FORBIDDEN;
	}

	Json::Value json_res;
	if (!json_reader().parse(res, json_res)) {
		return TWITTER_INVALID_RESPONSE;
	}

	if (!json_res.isMember("access_token")) {
		return TWITTER_INVALID_RESPONSE;
	}

	m_bearer_token = json_res["access_token"].asString();
	return TWITTER_OK;
}

TwitterClient::Response
TwitterClient::get_user_timeline(Json::Value &res, const uint16_t count,
	const uint32_t since_id, bool include_rts,
	bool contributor_details)
{
	std::string request = TWITTER_API_URL + TWITTER_USER_TIMELINE_1_1;

	if (count > 0) {
		add_uri_param("count", std::to_string(count));
	}

	if (since_id > 0) {
		add_uri_param("since_id", std::to_string(since_id));
	}

	if (include_rts) {
		add_uri_param("include_rts", "true");
	}

	if (contributor_details) {
		add_uri_param("contributor_details", "true");
	}

	append_oauth_header("GET", request);

	http::Query query(request);
	_get_json(query, res);

	switch (get_http_code()) {
		case 401:
			return TWITTER_UNAUTHORIZED;
		case 403:
			return TWITTER_FORBIDDEN;
		default:
			break;
	}

	return TWITTER_OK;
}

TwitterClient::Response
TwitterClient::get_home_timeline(Json::Value &res, const uint16_t count,
	const uint32_t since_id)
{
	std::string request = TWITTER_API_URL + TWITTER_HOME_TIMELINE_1_1;

	if (count > 0) {
		add_uri_param("count", std::to_string(count));
	}

	if (since_id > 0) {
		add_uri_param("since_id", std::to_string(since_id));
	}

	append_oauth_header("GET", request);

	http::Query query(request);
	_get_json(query, res);

	switch (get_http_code()) {
		case 401:
			return TWITTER_UNAUTHORIZED;
		case 403:
			return TWITTER_FORBIDDEN;
		default:
			break;
	}

	return TWITTER_OK;
}
}
}
