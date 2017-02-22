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

#include <iostream>
#include "twitterclient.h"
#include "utils/base64.h"

static const std::string TWITTER_API_URL = "https://api.twitter.com";
static const std::string TWITTER_OAUTH2_TOKEN = "/oauth2/token";
static const std::string TWITTER_HOME_TIMELINE_1_1 = "/1.1/statuses/user_timeline.json";
static const std::string TWITTER_USER_TIMELINE_1_1 = "/1.1/statuses/user_timeline.json";

TwitterClient::TwitterClient(const std::string &consumer_key, const std::string &consumer_secret):
	HTTPClient(),
	m_consumer_key(consumer_key),
	m_consumer_secret(consumer_secret)
{
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
	add_http_header("Authorization", "Bearer " + m_access_token);
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

	m_access_token = json_res["access_token"].asString();
	return TWITTER_OK;
}

TwitterClient::Response TwitterClient::get_user_timeline(Json::Value &res, const uint16_t count)
{
	append_bearer_header();
	_get_json(TWITTER_API_URL + TWITTER_USER_TIMELINE_1_1 + "?count="
		+ std::to_string(count), res);

	if (get_http_code() == 403) {
		return TWITTER_FORBIDDEN;
	}
	return TWITTER_OK;
}

TwitterClient::Response TwitterClient::get_home_timeline(Json::Value &res, const uint16_t count)
{
	append_bearer_header();
	_get_json(TWITTER_API_URL + TWITTER_HOME_TIMELINE_1_1 + "?count="
		+ std::to_string(count), res);

	if (get_http_code() == 403) {
		return TWITTER_FORBIDDEN;
	}
	return TWITTER_OK;
}
