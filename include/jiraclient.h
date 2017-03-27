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

#include "httpclient.h"

class JiraClient : private HTTPClient
{
public:
	JiraClient(const std::string &instance_url, const std::string &user, const std::string &password);
	~JiraClient() {}

	bool test_connection();
	bool get_issue(const std::string &issue_id, Json::Value &result);
	bool create_issue(const std::string &project_key, const std::string &issue_type, const std::string &summary,
					  const std::string &description, Json::Value &res);
	bool create_issue(const uint32_t project_id, const uint32_t issue_type_id, const std::string &summary,
					  const std::string &description, Json::Value &res);
	bool assign_issue(const std::string &issue, const std::string &who, Json::Value &res);
	bool get_issue_transtions(const std::string &issue, Json::Value &res);

	bool list_projects(Json::Value &res);

	const uint16_t get_http_code() const { return (uint16_t) HTTPClient::get_http_code(); }
private:
	std::string m_instance_url = "";
};