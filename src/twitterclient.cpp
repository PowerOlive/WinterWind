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
#include <sstream>
#include <ctime>
#include "twitterclient.h"
#include "utils/base64.h"

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
	append_oauth_header();
}

void TwitterClient::append_oauth_header()
{
	std::array<char, 33> oauth_nonce = {};
	std::uniform_int_distribution<uint8_t> normal_dist(0, 255);
	for (uint8_t i = 0; i < oauth_nonce.size() - 1; i++) {
		oauth_nonce[i] = (uint8_t) normal_dist(m_rand_engine);
	}
	oauth_nonce[32] = '\0';

	// std::string(oauth_nonce.data()) => to encode using b64
	std::stringstream header;
	header << std::string("OAuth oauth_consumer_key=\"") << m_consumer_key << "\","
		<< "oauth_nonce=\"" << std::string(oauth_nonce.data()) << "\","
		<< "oauth_signature=\"" << "tnnArxj06cWHq44gCs1OSKk%2FjLY%3D" << "\","
		<< "oauth_signature_method=\"HMAC-SHA1\","
		<< "oauth_timestamp=\"" << std::time(0) << "\","
		<< "oauth_token=\"" << m_access_token << "\","
		<< "oauth_version=\"1.0\"";

	std::cout << header.str() << std::endl;

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
	std::string request = TWITTER_API_URL + TWITTER_USER_TIMELINE_1_1 + "?";

	bool prev_param = false;
	if (count > 0) {
		request += "count=" + std::to_string(count);
		prev_param = true;
	}

	if (since_id > 0) {
		if (prev_param) request += "&";
		request += "since_id=" + std::to_string(since_id);
		prev_param = true;
	}

	if (include_rts) {
		if (prev_param) request += "&";
		request += "include_rts=true";
		prev_param = true;
	}

	if (contributor_details) {
		if (prev_param) request += "&";
		request += "contributor_details=true";
	}

	_get_json(request, res);

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
	append_bearer_header();
	std::string request = TWITTER_API_URL + TWITTER_HOME_TIMELINE_1_1 + "?";

	bool prev_param = false;
	if (count > 0) {
		request += "count=" + std::to_string(count);
		prev_param = true;
	}

	if (since_id > 0) {
		if (prev_param) request += "&";
		request += "since_id=" + std::to_string(since_id);
	}

	_get_json(request, res);

	switch (get_http_code()) {
		case 401: return TWITTER_UNAUTHORIZED;
		case 403: return TWITTER_FORBIDDEN;
		default: break;
	}

	return TWITTER_OK;
}
