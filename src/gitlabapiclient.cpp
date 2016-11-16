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

#define ADD_REQ_FIELD_IF_NOT_EMPTY(r, n, f) \
	if (!f.empty()) { r[n] = f; }

#define GITLAB_RC_HTTP_CODE(c) \
	m_http_code == 503 ? GITLAB_RC_SERVICE_UNAVAILABLE : GITLAB_RC_##c

const std::string GitlabAPIClient::api_v3_endpoint = "/api/v3";

/*
 * Issues
 */

const GitlabRetCod GitlabAPIClient::get_issues(const uint32_t project_id, const std::string &filter,
		Json::Value &result)
{
	Json::Value tmp_result;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	fetch_json(m_server_uri + api_v3_endpoint + "/projects/"
			   + std::to_string(project_id) + "/issues"
			   + (filter.length() ? "?" + filter : ""), tmp_result);

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0) {
		return GITLAB_RC_HTTP_CODE(INVALID_RESPONSE);
	}

	result = tmp_result;
	return GITLAB_RC_OK;
}

const GitlabRetCod GitlabAPIClient::get_issue(const uint32_t project_id, const uint32_t issue_id,
		Json::Value &result)
{
	Json::Value tmp_result;
	if (get_issues(project_id, "iid=" + std::to_string(issue_id), tmp_result)) {
		result = tmp_result[0];
		return tmp_result[0].isMember("id") ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE;
	}

	return GITLAB_RC_NOT_FOUND;
}

const GitlabRetCod GitlabAPIClient::get_issue(const std::string &ns,
		const std::string &project, const uint32_t issue_id, Json::Value &result,
		const GitlabProjectSearchScope proj_search_scope)
{
	Json::Value p_result;
	GitlabRetCod rc = get_project_ns(project, ns, p_result, proj_search_scope);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	return get_issue(p_result["id"].asUInt(), issue_id, result);
}

const GitlabRetCod GitlabAPIClient::create_issue(const uint32_t project_id,
		const GitlabIssue &issue)
{
	if (issue.title.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	std::string res = "", post_data = "";
	build_issue_data(issue, post_data);

	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_post(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/issues", post_data, res);
	return (m_http_code == 201 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::modify_issue(const uint32_t project_id,
		const uint32_t issue_id, const GitlabIssue &issue)
{
	if (issue.title.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	Json::Value issue_res;
	if (!get_issue(project_id, issue_id, issue_res)) {
		return GITLAB_RC_NOT_FOUND;
	}

	std::string res = "", post_data = "";
	build_issue_data(issue, post_data);

	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_put(m_server_uri + api_v3_endpoint + "/projects/"
				 + std::to_string(project_id) + "/issues/" + issue_res.asString(), res,
				HTTPCLIENT_REQ_SIMPLE, post_data);
	return (m_http_code == 201 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

void GitlabAPIClient::build_issue_data(const GitlabIssue &issue, std::string &post_data)
{
	std::string encoded_title = "";

	http_string_escape(issue.title, encoded_title);

	post_data += "title=" + encoded_title;
	if (issue.confidential) {
		post_data += "&confidential=true";
	}

	if (!issue.description.empty()) {
		std::string encoded_desc = "";
		http_string_escape(issue.description, encoded_desc);
		post_data += "&description=" + encoded_desc;
	}

	if (!issue.due_date.empty()) {
		std::string encoded_date = "";
		http_string_escape(issue.due_date, encoded_date);
		post_data += "&due_date=" + encoded_date;
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

		post_data += "&labels=" + encoded_labels;
	}
}

const GitlabRetCod GitlabAPIClient::close_issue(const uint32_t project_id,
		const uint32_t issue_id)
{
	Json::Value issue_res;
	if (!get_issue(project_id, issue_id, issue_res)) {
		return GITLAB_RC_NOT_FOUND;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_put(m_server_uri + api_v3_endpoint + "/projects/"
				+ std::to_string(project_id) + "/issues/" + issue_res["id"].asString(),
				res, HTTPCLIENT_REQ_SIMPLE, "state_event=close");
	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::delete_issue(const uint32_t project_id, const uint32_t issue_id)
{
	Json::Value issue_result;
	if (!get_issue(project_id, issue_id, issue_result)) {
		return GITLAB_RC_NOT_FOUND;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/issues/" + issue_result["id"].asString(), res);
	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

/*
 * Merge requests
 */

const GitlabRetCod GitlabAPIClient::get_merge_requests(const uint32_t project_id, const std::string &filter,
		Json::Value &result)
{
	Json::Value tmp_result;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	fetch_json(m_server_uri + api_v3_endpoint + "/projects/"
				+ std::to_string(project_id) + "/merge_requests"
				+ (filter.length() ? "?" + filter : ""), tmp_result);

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0) {
		return GITLAB_RC_HTTP_CODE(INVALID_RESPONSE);
	}

	result = tmp_result;
	return GITLAB_RC_OK;
}

const GitlabRetCod GitlabAPIClient::get_merge_request(const uint32_t project_id, const uint32_t issue_id,
		Json::Value &result)
{
	Json::Value tmp_result;
	if (get_merge_requests(project_id, "iid=" + std::to_string(issue_id), tmp_result)) {
		result = tmp_result[0];
		return GITLAB_RC_OK;
	}
	return GITLAB_RC_NOT_FOUND;
}

const GitlabRetCod GitlabAPIClient::close_merge_request(const uint32_t project_id, const uint32_t issue_id)
{
	Json::Value mr_result;
	if (!get_merge_request(project_id, issue_id, mr_result)) {
		return GITLAB_RC_NOT_FOUND;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_put(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/merge_requests/" + mr_result["id"].asString(),
			res, HTTPCLIENT_REQ_SIMPLE, "state_event=close");
	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::delete_merge_request(const uint32_t project_id, const uint32_t issue_id)
{
	Json::Value mr_result;
	if (!get_merge_request(project_id, issue_id, mr_result)) {
		return GITLAB_RC_NOT_FOUND;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/merge_requests/"
			+ mr_result["id"].asString(), res);
	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

/*
 * Labels
 */

const GitlabRetCod GitlabAPIClient::get_labels(const uint32_t project_id,
		Json::Value &result)
{
	Json::Value tmp_result;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	fetch_json(m_server_uri + api_v3_endpoint + "/projects/"
			   + std::to_string(project_id) + "/labels", tmp_result);

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0
		|| !tmp_result.isArray()) {
		return GITLAB_RC_HTTP_CODE(INVALID_RESPONSE);;
	}

	for (const auto &l: tmp_result) {
		if (!l.isMember("name") || !l.isMember("color")) {
			return GITLAB_RC_INVALID_RESPONSE;
		}
	}

	result = tmp_result;
	return GITLAB_RC_OK;
}

const GitlabRetCod GitlabAPIClient::get_label(const uint32_t project_id,
		const std::string &name, Json::Value &result)
{
	if (name.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	Json::Value tmp_result;
	const GitlabRetCod rc = get_labels(project_id, tmp_result);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	for (const auto &l: tmp_result) {
		if (l["name"].asString() == name) {
			result = l;
			return GITLAB_RC_OK;
		}
	}

	return GITLAB_RC_NOT_FOUND;
}

const GitlabRetCod GitlabAPIClient::create_label(const uint32_t project_id,
		const std::string &label, const std::string &color_id, Json::Value &res)
{
	Json::Value request;
	request["name"] = label;
	request["color"] = color_id;

	add_http_header("PRIVATE-TOKEN", m_api_token);
	if (!post_json(m_server_uri + api_v3_endpoint + "/projects/"
			+ std::to_string(project_id) + "/labels",
			request.toStyledString(), res)) {
		return GITLAB_RC_INVALID_RESPONSE;
	}

	return (m_http_code == 201 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::create_label(const std::string &ns,
		const std::string &name, const std::string &label, const std::string &color_id,
		Json::Value &res)
{
	Json::Value p_result;
	GitlabRetCod rc = get_project_ns(name, ns, p_result);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	return create_label(p_result["id"].asUInt(), label, color_id, res);
}

const GitlabRetCod GitlabAPIClient::get_label(const std::string &ns,
		const std::string &project, const std::string &name, Json::Value &result,
		const GitlabProjectSearchScope proj_search_scope)
{
	Json::Value p_result;
	GitlabRetCod rc = get_project_ns(project, ns, p_result, proj_search_scope);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	return get_label(p_result["id"].asUInt(), name, result);
}

const GitlabRetCod GitlabAPIClient::delete_label(const uint32_t project_id, const std::string &label)
{
	if (label.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}
	std::string encoded_label = "";
	http_string_escape(label, encoded_label);

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
				   + std::to_string(project_id) + "/labels?name=" + encoded_label, res);
	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::delete_label(const std::string &ns,
		const std::string &name, const std::string &label)
{
	Json::Value p_result;
	GitlabRetCod rc = get_project_ns(name, ns, p_result);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	return delete_label(p_result["id"].asUInt(), label);
}

/*
 * Tags
 */

const GitlabRetCod GitlabAPIClient::create_tag(const uint32_t project_id,
		const GitlabTag &tag)
{
	Json::Value request, res;
	request["tag_name"] = tag.name;
	request["ref"] = tag.ref;
	request["message"] = tag.message;
	request["release_description"] = tag.release_description;

	add_http_header("PRIVATE-TOKEN", m_api_token);
	if (!post_json(m_server_uri + api_v3_endpoint + "/projects/"
				   + std::to_string(project_id) + "/labels",
				   request.toStyledString(), res)) {
		return GITLAB_RC_INVALID_RESPONSE;
	}

	return (m_http_code == 201 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::delete_tag(const uint32_t project_id,
		const std::string &tag_name)
{
	std::string encoded_tag = "";
	http_string_escape(tag_name, encoded_tag);

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
				   + std::to_string(project_id) + "/tags/" + encoded_tag, res);
	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_NOT_FOUND);
}

/*
 * Namespaces
 */

const GitlabRetCod GitlabAPIClient::get_namespaces(const std::string &name,
		Json::Value &result)
{
	Json::Value tmp_result;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	fetch_json(m_server_uri + api_v3_endpoint + "/namespaces"
			   + (name.length() ? "?search=" + name : ""), tmp_result);

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0
			|| !tmp_result.isArray()) {
		return GITLAB_RC_HTTP_CODE(INVALID_RESPONSE);;
	}

	for (const auto &n: tmp_result) {
		if (!n.isMember("id") || !n.isMember("path") || !n.isMember("kind")) {
			return GITLAB_RC_INVALID_RESPONSE;
		}
	}

	result = tmp_result;
	return GITLAB_RC_OK;
}

const GitlabRetCod GitlabAPIClient::get_namespace(const std::string &name,
		Json::Value &result)
{
	if (name.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	Json::Value tmp_result;
	const GitlabRetCod rc = get_namespaces(name, tmp_result);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	for (const auto &n: tmp_result) {
		if (n["path"].asString() == name) {
			result = n;
			return GITLAB_RC_OK;
		}
	}

	return GITLAB_RC_NOT_FOUND;
}

/*
 * Groups
 */

const GitlabRetCod GitlabAPIClient::get_groups(const std::string &filter, Json::Value &result)
{
	Json::Value tmp_result;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	fetch_json(m_server_uri + api_v3_endpoint + "/groups"
			   + (filter.length() ? "?" + filter : ""), tmp_result);

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0
			|| !tmp_result.isArray()) {
		return GITLAB_RC_HTTP_CODE(INVALID_RESPONSE);
	}

	for (const auto &g: tmp_result) {
		if (!g.isMember("id") || !g.isMember("name")) {
			return GITLAB_RC_INVALID_RESPONSE;
		}
	}

	result = tmp_result;
	return GITLAB_RC_OK;
}

const GitlabRetCod GitlabAPIClient::get_group(const std::string &name, Json::Value &result)
{
	if (name.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	Json::Value tmp_result;
	const GitlabRetCod rc = get_groups("search=" + name, tmp_result);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	for (const auto &g: tmp_result) {
		if (g["name"].asString() == name) {
			result = g;
			return GITLAB_RC_OK;
		}
	}

	return GITLAB_RC_NOT_FOUND;
}

bool GitlabAPIClient::create_group(const GitlabGroup &group, Json::Value &res)
{
	Json::Value request;
	request["name"] = group.name;
	request["path"] = group.path;
	ADD_REQ_FIELD_IF_NOT_EMPTY(request, "description", group.description)

	request["visibility_level"] = (uint16_t) group.visibility;
	request["lfs_enabled"] = group.enable_lfs;
	request["request_access_enabled"] = group.access_requests;

	add_http_header("PRIVATE-TOKEN", m_api_token);
	if (!post_json(m_server_uri + api_v3_endpoint + "/groups",
			request.toStyledString(), res)) {
		return false;
	}

	return m_http_code == 201;
}

const GitlabRetCod GitlabAPIClient::delete_group(const std::string &name)
{
	Json::Value result;
	GitlabRetCod rc = get_group(name, result);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/groups/"
			+ result["id"].asString(), res);

	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::delete_groups(const std::vector<std::string> &groups)
{
	for (const auto &g: groups) {
		const GitlabRetCod rc = delete_group(g);
		if (rc != GITLAB_RC_OK) {
			return rc;
		}
	}

	return GITLAB_RC_OK;
}

const GitlabRetCod GitlabAPIClient::create_project(const GitlabProject &project,
		Json::Value &res)
{
	if (project.name.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	Json::Value request;
	request["name"] = project.name;
	request["path"] = project.path.empty() ? project.name : project.path;
	ADD_REQ_FIELD_IF_NOT_EMPTY(request, "description", project.description)
	if (project.namespace_id > 0) {
		request["namespace_id"] = project.namespace_id;
	}
	request["issues_enabled"] = project.issues_enabled;
	request["merge_requests_enabled"] = project.merge_requests_enabled;
	request["builds_enabled"] = project.builds_enabled;
	request["wiki_enabled"] = project.wiki_enabled;
	request["snippets_enabled"] = project.snippets_enabled;
	request["container_registry_enabled"] = project.container_registry_enabled;
	request["shared_runners_enabled"] = project.shared_runners_enabled;
	request["visibility_level"] = (uint32_t) project.visibility_level;
	request["public_builds"] = project.public_builds;
	request["only_allow_merge_if_build_succeeds"] = project.only_allow_merge_if_build_succeeds;
	request["only_allow_merge_if_all_discussions_are_resolved"] = project.only_allow_merge_if_all_discussions_are_resolved;
	request["lfs_enabled"] = project.lfs_enabled;
	request["request_access_enabled"] = project.request_access_enabled;

	add_http_header("PRIVATE-TOKEN", m_api_token);
	if (!post_json(m_server_uri + api_v3_endpoint + "/projects",
				   request.toStyledString(), res)) {
		return GITLAB_RC_INVALID_RESPONSE;
	}

	return m_http_code == 201 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE;
}

const GitlabRetCod GitlabAPIClient::get_projects(const std::string &name,
		Json::Value &result, const GitlabProjectSearchScope search_scope)
{
	Json::Value tmp_result;
	std::string endpoint_suffix = "";
	switch (search_scope) {
		case GITLAB_PROJECT_SS_OWNED: endpoint_suffix += "/owned"; break;
		case GITLAB_PROJECT_SS_ALL: endpoint_suffix += "/all"; break;
		case GITLAB_PROJECT_SS_VISIBLE: endpoint_suffix += "/visible"; break;
		case GITLAB_PROJECT_SS_STARRED: endpoint_suffix += "starred"; break;
		default: break;
	}

	if (!name.empty()) {
		std::string encoded_proj_name = "";
		http_string_escape(name, encoded_proj_name);
		endpoint_suffix += "?search=" + encoded_proj_name;
	}

	add_http_header("PRIVATE-TOKEN", m_api_token);
	if (!fetch_json(m_server_uri + api_v3_endpoint + "/projects" + endpoint_suffix,
			tmp_result)) {
		return GITLAB_RC_HTTP_CODE(INVALID_RESPONSE);;
	}

	if (m_http_code != 200 || tmp_result.empty() || tmp_result.size() == 0
			|| !tmp_result.isArray()) {
		return GITLAB_RC_INVALID_RESPONSE;
	}

	for (const auto &p: tmp_result) {
		if (!p.isMember("id") || !p.isMember("name") || !p.isMember("http_url_to_repo")
			|| !p.isMember("namespace") || !p["namespace"].isMember("id")
			|| !p["namespace"].isMember("name")) {
			return GITLAB_RC_INVALID_RESPONSE;
		}
	}

	result = tmp_result;
	return GITLAB_RC_OK;
}

const GitlabRetCod GitlabAPIClient::get_project(const std::string &name,
		Json::Value &result, const GitlabProjectSearchScope search_scope)
{
	if (name.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	Json::Value tmp_result;
	const GitlabRetCod rc = get_projects(name, tmp_result, search_scope);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	for (const auto &p: tmp_result) {
		if (p["name"].asString() == name) {
			result = p;
			return GITLAB_RC_OK;
		}
	}

	return GITLAB_RC_NOT_FOUND;
}

const GitlabRetCod GitlabAPIClient::get_project_ns(const std::string &name,
		const std::string &ns, Json::Value &result,
		const GitlabProjectSearchScope search_scope)
{
	if (name.empty()) {
		return GITLAB_RC_INVALID_PARAMS;
	}

	Json::Value tmp_result;
	const GitlabRetCod rc = get_projects(name, tmp_result, search_scope);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	for (const auto &p: tmp_result) {
		if (p["name"].asString() == name && p["namespace"]["name"].asString() == ns) {
			result = p;
			return GITLAB_RC_OK;
		}
	}

	return GITLAB_RC_NOT_FOUND;
}

const GitlabRetCod GitlabAPIClient::delete_project(const std::string &name)
{
	Json::Value result;
	GitlabRetCod rc = get_project(name, result);
	if (rc != GITLAB_RC_OK) {
		return rc;
	}

	std::string res;
	add_http_header("PRIVATE-TOKEN", m_api_token);
	perform_delete(m_server_uri + api_v3_endpoint + "/projects/"
				   + result["id"].asString(), res);

	return (m_http_code == 200 ? GITLAB_RC_OK : GITLAB_RC_INVALID_RESPONSE);
}

const GitlabRetCod GitlabAPIClient::delete_projects(
		const std::vector<std::string> &projects)
{
	for (const auto &p: projects) {
		const GitlabRetCod rc = delete_project(p);
		if (rc != GITLAB_RC_OK) {
			return rc;
		}
	}

	return GITLAB_RC_OK;
}
