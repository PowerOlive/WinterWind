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

#include "oauthclient.h"
#include "utils/base64.h"
#include "utils/hmac.h"
#include <algorithm>
#include <ctime>
#include <sstream>

namespace winterwind
{
OAuthClient::OAuthClient(const std::string &consumer_key,
	const std::string &consumer_secret,
	const std::string &access_token, const std::string &access_token_secret)
	: HTTPClient(),
	m_consumer_key(consumer_key),
	m_consumer_secret(consumer_secret),
	m_access_token(access_token),
	m_access_token_secret(access_token_secret)
{
	m_rand_engine = std::mt19937((unsigned long) std::time(0));
}

#define OAUTH_NONCE_LENGTH 32

void OAuthClient::append_oauth_header(const std::string &method, const std::string &url)
{
	std::string oauth_nonce = "";
	std::uniform_int_distribution<uint8_t> normal_dist(0, 255);
	for (uint8_t i = 0; i < OAUTH_NONCE_LENGTH; i++) {
		oauth_nonce += (uint8_t) normal_dist(m_rand_engine);
	}

	std::string buf = base64_encode(oauth_nonce);
	buf.erase(
		std::remove_if(buf.begin(), buf.end(), [](char c) { return !std::isalnum(c); }),
		buf.end());
	time_t t = std::time(0);

	std::map<std::string, std::string> ordered_params = {};

	ordered_params["oauth_consumer_key"] = m_consumer_key;
	ordered_params["oauth_nonce"] = buf;
	ordered_params["oauth_signature_method"] = "HMAC-SHA1";
	ordered_params["oauth_timestamp"] = std::to_string(t);
	ordered_params["oauth_token"] = m_access_token;
	ordered_params["oauth_version"] = "1.0";

	// Append request params
	for (const auto &p : m_uri_params) {
		ordered_params[p.first] = p.second;
	}

	// Generate parameter string
	std::string parameter_string = "";
	uint32_t p_count = 0, p_max = (uint32_t) ordered_params.size();
	for (const auto &p : ordered_params) {
		http_string_escape(p.first, buf);
		parameter_string += buf + "=";
		http_string_escape(p.second, buf);

		parameter_string += buf;

		p_count++;
		if (p_count != p_max) {
			parameter_string += "&";
		}
	}

	// Create signature
	http_string_escape(url, buf);
	std::string signature = method + "&" + buf + "&";
	http_string_escape(parameter_string, buf);
	signature += buf;

	// Generate signing key
	{
		std::string signing_key = "";
		http_string_escape(m_consumer_secret, buf);
		signing_key += buf + "&";
		http_string_escape(m_access_token_secret, buf);
		signing_key += buf;

		// Add sign key to parameters
		ordered_params["oauth_signature"] =
			base64_encode(hmac_sha1(signing_key, signature));
	}

	// Generate OAuth header
	std::stringstream header;
	header << std::string("OAuth ");

	static const std::string oauth_prefix = "oauth_";

	p_count = 0;
	p_max = 7;
	for (const auto &p : ordered_params) {
		// Only manipulate oauth header
		if (p.first.substr(0, oauth_prefix.size()) != oauth_prefix) {
			continue;
		}

		http_string_escape(p.first, buf);
		header << buf << "=\"";
		http_string_escape(p.second, buf);
		header << buf << "\"";

		p_count++;
		if (p_count != p_max) {
			header << ", ";
		}
	}

	add_http_header("Authorization", header.str());
}
}
