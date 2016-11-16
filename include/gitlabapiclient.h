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

#pragma once

#include "httpclient.h"

enum GitlabRetCod
{
	GITLAB_RC_OK = 0,
	GITLAB_RC_INVALID_PARAMS,
	GITLAB_RC_INVALID_RESPONSE,
	GITLAB_RC_SERVICE_UNAVAILABLE,
	GITLAB_RC_NOT_FOUND,
};

struct GitlabIssue
{
public:
	GitlabIssue(const std::string &t): title(t) {}
	GitlabIssue(const std::string &t, const std::string &desc, bool c = false, const std::string &dd = "",
		const std::vector<std::string> &l = {}):
		title(t),
		description(desc),
		confidential(c),
		due_date(dd),
		labels(l) {}

	std::string title = "";
	std::string description = "";
	bool confidential = false;
	std::string due_date = "";
	std::vector<std::string> labels = {};
};

struct GitlabTag
{
public:
	GitlabTag(const std::string &n, const std::string &r):
		name(n),
		ref(r) {}

	GitlabTag(const std::string &n, const std::string &r, const std::string &m,
			const std::string &rd):
		name(n),
		ref(r),
		message(m),
		release_description(rd) {}

	std::string name = "";
	std::string ref = "";
	std::string message = "";
	std::string release_description = "";
};

enum GitlabGroupVisibility
{
	GITLAB_GROUP_PRIVATE = 0,
	GITLAB_GROUP_INTERNAL = 10,
	GITLAB_GROUP_PUBLIC = 20,
};

enum GitlabProjectVisibility
{
	GITLAB_PROJECT_PRIVATE = 0,
	GITLAB_PROJECT_INTERNAL = 10,
	GITLAB_PROJECT_PUBLIC = 20,
};

struct GitlabGroup
{
public:
	GitlabGroup(const std::string &n, const std::string &p):
		name(n),
		path(p) {}

	std::string name = "";
	std::string path = "";
	std::string description = "";
	GitlabGroupVisibility visibility = GITLAB_GROUP_PRIVATE;
	bool enable_lfs = false;
	bool access_requests = true;
};

struct GitlabProject
{
public:
	GitlabProject(const std::string &n):
		name(n) {}

	std::string name = "";
	std::string path = "";
	uint32_t namespace_id = 0;
	std::string description = "";
	bool issues_enabled = true;
	bool merge_requests_enabled = true;
	bool builds_enabled = true;
	bool wiki_enabled = true;
	bool snippets_enabled = true;
	bool container_registry_enabled = false;
	bool shared_runners_enabled = false;
	GitlabProjectVisibility visibility_level = GITLAB_PROJECT_PUBLIC;
	bool public_builds = true;
	bool only_allow_merge_if_build_succeeds = false;
	bool only_allow_merge_if_all_discussions_are_resolved = false;
	bool lfs_enabled = false;
	bool request_access_enabled = true;
};

enum GitlabProjectSearchScope
{
	GITLAB_PROJECT_SS_STANDARD,
	GITLAB_PROJECT_SS_ALL,
	GITLAB_PROJECT_SS_VISIBLE,
	GITLAB_PROJECT_SS_OWNED,
	GITLAB_PROJECT_SS_STARRED,
};

class GitlabAPIClient: public HTTPClient
{
public:
	GitlabAPIClient(const std::string &server_uri, const std::string &api_token):
		m_server_uri(server_uri),
		m_api_token(api_token)
	{}

	// Issues
	const GitlabRetCod get_issue(const uint32_t project_id, const uint32_t issue_id, Json::Value &result);
	const GitlabRetCod get_issues(const uint32_t project_id, const std::string &filter, Json::Value &result);
	const GitlabRetCod get_issue(const std::string &ns, const std::string &project,
			const uint32_t issue_id, Json::Value &result,
			GitlabProjectSearchScope search_scope = GITLAB_PROJECT_SS_STANDARD);
	const GitlabRetCod create_issue(const uint32_t project_id, const GitlabIssue &issue);
	const GitlabRetCod modify_issue(const uint32_t project_id, const uint32_t issue_id,
			const GitlabIssue &issue);
	const GitlabRetCod close_issue(const uint32_t project_id, const uint32_t issue_id);
	const GitlabRetCod delete_issue(const uint32_t project_id, const uint32_t issue_id);

	// Merge requests
	const GitlabRetCod get_merge_request(const uint32_t project_id, const uint32_t issue_id, Json::Value &result);
	const GitlabRetCod get_merge_requests(const uint32_t project_id, const std::string &filter, Json::Value &result);
	const GitlabRetCod close_merge_request(const uint32_t project_id, const uint32_t issue_id);
	const GitlabRetCod delete_merge_request(const uint32_t project_id, const uint32_t issue_id);

	// Labels
	const GitlabRetCod get_labels(const uint32_t project_id, Json::Value &result);
	const GitlabRetCod get_label(const uint32_t project_id, const std::string &name,
			Json::Value &result);
	const GitlabRetCod get_label(const std::string &ns, const std::string &project,
			const std::string &name, Json::Value &result);
	const GitlabRetCod create_label(const uint32_t project_id, const std::string &label,
			const std::string &color_id, Json::Value &res);
	const GitlabRetCod create_label(const std::string &ns, const std::string &project,
			const std::string &label, const std::string &color_id, Json::Value &res);
	const GitlabRetCod delete_label(const uint32_t project_id, const std::string &label);
	const GitlabRetCod delete_label(const std::string &ns, const std::string &project,
			const std::string &label);

	// Tags
	const GitlabRetCod create_tag(const uint32_t project_id, const GitlabTag &tag);
	const GitlabRetCod delete_tag(const uint32_t project_id, const std::string &tag_name);

	// Namespaces
	const GitlabRetCod get_namespaces(const std::string &name, Json::Value &result);
	const GitlabRetCod get_namespace(const std::string &name, Json::Value &result);

	// Groups
	const GitlabRetCod get_groups(const std::string &filter, Json::Value &result);
	const GitlabRetCod get_group(const std::string &name, Json::Value &result);
	bool create_group(const GitlabGroup &group, Json::Value &res);
	const GitlabRetCod delete_group(const std::string &name);
	const GitlabRetCod delete_groups(const std::vector<std::string> &groups);

	// Projects
	const GitlabRetCod create_project(const GitlabProject &project, Json::Value &res);
	const GitlabRetCod get_projects(const std::string &name, Json::Value &result,
			GitlabProjectSearchScope search_scope = GITLAB_PROJECT_SS_STANDARD);
	const GitlabRetCod get_project(const std::string &name, Json::Value &result,
			GitlabProjectSearchScope search_scope = GITLAB_PROJECT_SS_STANDARD);
	const GitlabRetCod get_project_ns(const std::string &name, const std::string &ns,
			Json::Value &result,
			GitlabProjectSearchScope search_scope = GITLAB_PROJECT_SS_STANDARD);
	const GitlabRetCod delete_project(const std::string &name);
	const GitlabRetCod delete_projects(const std::vector<std::string> &projects);
private:
	void build_issue_data(const GitlabIssue &issue, std::string &post_data);

	std::string m_server_uri = "https://gitlab.com";
	std::string m_api_token = "";
	static const std::string api_v3_endpoint;
};
