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
#include "gitlabapiclient.h"

const std::string GitlabAPIClient::api_v3_endpoint = "/api/v3";

bool GitlabAPIClient::get_issue(const uint32_t project_id, const uint32_t issue_id,
		Json::Value &result)
{
	Json::Value tmp_result;
	std::unordered_map<std::string, std::string> headers;
	headers["PRIVATE-TOKEN"] = m_api_token;

	fetch_json(m_server_uri + api_v3_endpoint + "/projects/"
		+ std::to_string(project_id) + "/issues?iid=" + std::to_string(issue_id),
		headers, tmp_result);

	if (tmp_result.empty() || tmp_result.size() == 0) {
		return false;
	}

	result = tmp_result[0];
	return true;
}

bool GitlabAPIClient::get_merge_request(const uint32_t project_id, const uint32_t issue_id,
		Json::Value &result)
{
	Json::Value tmp_result;
	std::unordered_map<std::string, std::string> headers;
	headers["PRIVATE-TOKEN"] = m_api_token;

	fetch_json(m_server_uri + api_v3_endpoint + "/projects/"
			   + std::to_string(project_id) + "/merge_requests?iid=" + std::to_string(issue_id),
			   headers, tmp_result);

	if (tmp_result.empty() || tmp_result.size() == 0) {
		return false;
	}

	result = tmp_result[0];
	return true;
}
