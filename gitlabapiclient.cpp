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

/*
 * Issues
 */

bool GitlabAPIClient::get_issues(const uint32_t project_id, const std::string &filter,
		Json::Value &result)
{
	Json::Value tmp_result;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	fetch_json(m_server_uri + api_v3_endpoint + "/projects/"
			   + std::to_string(project_id) + "/issues"
			   + (filter.length() ? "?" + filter : ""), tmp_result);

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0) {
		return false;
	}

	result = tmp_result;
	return true;
}

bool GitlabAPIClient::get_issue(const uint32_t project_id, const uint32_t issue_id,
		Json::Value &result)
{
	Json::Value tmp_result;
	if (get_issues(project_id, "iid=" + std::to_string(issue_id), tmp_result)) {
		result = tmp_result[0];
		return true;
	}

	return false;
}

bool GitlabAPIClient::delete_issue(const uint32_t project_id, const uint32_t issue_id)
{
	Json::Value issue_result;
	if (!get_issue(project_id, issue_id, issue_result)) {
		return false;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/issues/" + issue_result["id"].asString(), res);
	return true;
}

/*
 * Merge requests
 */

bool GitlabAPIClient::get_merge_requests(const uint32_t project_id, const std::string &filter,
		Json::Value &result)
{
	Json::Value tmp_result;
	//add_http_header("PRIVATE-TOKEN", m_api_token);
	fetch_json(m_server_uri + api_v3_endpoint + "/projects/"
				+ std::to_string(project_id) + "/merge_requests"
				+ (filter.length() ? "?" + filter : ""), tmp_result);

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0) {
		return false;
	}

	result = tmp_result;
	return true;
}

bool GitlabAPIClient::get_merge_request(const uint32_t project_id, const uint32_t issue_id,
		Json::Value &result)
{
	Json::Value tmp_result;
	if (get_merge_requests(project_id, "iid=" + std::to_string(issue_id), tmp_result)) {
		std::cout << tmp_result.toStyledString() << std::endl;
		result = tmp_result[0];
		return true;
	}
	return false;
}

bool GitlabAPIClient::close_merge_request(const uint32_t project_id, const uint32_t issue_id)
{
	Json::Value mr_result;
	if (!get_merge_request(project_id, issue_id, mr_result)) {
		return false;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_put(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/merge_requests/" + mr_result["id"].asString(),
			res, HTTPCLIENT_REQ_SIMPLE, "state_event=close");
	return true;
}

bool GitlabAPIClient::delete_merge_request(const uint32_t project_id, const uint32_t issue_id)
{
	Json::Value mr_result;
	if (!get_merge_request(project_id, issue_id, mr_result)) {
		return false;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/merge_requests/"
			+ mr_result["id"].asString(), res);
	return true;
}
