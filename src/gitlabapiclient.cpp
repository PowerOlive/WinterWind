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

bool GitlabAPIClient::create_issue(const uint32_t project_id, const std::string &title,
		const GitlabIssue &issue)
{
	std::string res = "", encoded_title = "", post_datas = "title=";

	http_string_escape(title, encoded_title);

	post_datas += encoded_title;
	if (issue.confidential) {
		post_datas += "&confidential=true";
	}

	if (!issue.description.empty()) {
		std::string encoded_desc = "";
		http_string_escape(issue.description, encoded_desc);
		post_datas += "&description=" + encoded_desc;
	}

	if (!issue.due_date.empty()) {
		std::string encoded_date = "";
		http_string_escape(issue.due_date, encoded_date);
		post_datas += "&due_date=" + encoded_date;
	}

	if (issue.labels.size() > 0) {
		std::string encoded_labels = "";
		for (const auto &l : issue.labels) {
			if (!encoded_labels.empty()) {
				encoded_labels.append(",");
			}

			std::string encoded_label= "";
			http_string_escape(l, encoded_label);
			encoded_labels.append(encoded_label);
		}

		post_datas += "&labels=" + encoded_labels;
	}

	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_post(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/issues", post_datas, res);
	return m_http_code == 201;
}

bool GitlabAPIClient::close_issue(const uint32_t project_id, const uint32_t issue_id)
{
	Json::Value issue_res;
	if (!get_issue(project_id, issue_id, issue_res)) {
		return false;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_put(m_server_uri + api_v3_endpoint + "/projects/"
				+ std::to_string(project_id) + "/issues/" + issue_res["id"].asString(),
				res, HTTPCLIENT_REQ_SIMPLE, "state_event=close");
	return m_http_code == 200;
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
	return m_http_code == 200;
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
	return m_http_code == 200;
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
	return m_http_code == 200;
}

/*
 * Labels
 */

bool GitlabAPIClient::create_label(const uint32_t project_id, const std::string &label,
		const std::string &color_id, Json::Value &res)
{
	Json::Value request;
	request["name"] = label;
	request["color"] = color_id;

	add_http_header("PRIVATE-TOKEN", m_api_token);
	if (!post_json(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/labels",
			request.toStyledString(), res)) {
		return false;
	}

	return m_http_code == 201;
}

bool GitlabAPIClient::delete_label(const uint32_t project_id, const std::string &label)
{
	std::string encoded_label = "";
	http_string_escape(label, encoded_label);

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
				   + std::to_string(project_id) + "/labels?name=" + encoded_label, res);
	return m_http_code == 200;
}

/*
 * Groups
 */

bool GitlabAPIClient::create_group(const GitlabGroup &group, Json::Value &res)
{
	Json::Value request;
	request["name"] = group.name;
	request["path"] = group.path;
	if (!group.description.empty()) {
		request["description"] = group.description;
	}

	request["visibility_level"] = (uint16_t) group.visibility;
	request["lfs_enabled"] = group.enable_lfs;
	request["request_access_enabled"] = group.access_requests;

	add_http_header("PRIVATE-TOKEN", m_api_token);
	if (!post_json(m_server_uri + api_v3_endpoint + "/groups",
			request.toStyledString(), res)) {
		return false;
	}

	return m_http_code != 201;
}
