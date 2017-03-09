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

#include "oauthclient.h"

class TwitterClient : private OAuthClient
{
public:
	enum Response
	{
		TWITTER_OK,
		TWITTER_INVALID_RESPONSE,
		TWITTER_UNAUTHORIZED,
		TWITTER_FORBIDDEN,
	};

	TwitterClient(const std::string &consumer_key, const std::string &consumer_secret,
		      const std::string &access_token = "", const std::string &access_token_secret = "");
	TwitterClient::Response authenticate() { return get_oauth2_token(); }
	TwitterClient::Response get_user_timeline(Json::Value &res, const uint16_t count = 0,
						  const uint32_t since_id = 0, bool include_rts = false,
						  bool contributor_details = false);
	TwitterClient::Response get_home_timeline(Json::Value &res, const uint16_t count = 0,
						  const uint32_t since_id = 0);

private:
	void append_auth_header();
	void append_bearer_header();
	TwitterClient::Response get_oauth2_token();

	std::string m_bearer_token = "";
};
