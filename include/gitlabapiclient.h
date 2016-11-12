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
	GITLAB_RC_UNK_OBJECT,
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

enum GitlabGroupVisibility
{
	GITLAB_GROUP_PRIVATE = 0,
	GITLAB_GROUP_INTERNAL = 10,
	GITLAB_GROUP_PUBLIC = 20,
};

struct GitlabGroup
{
	std::string name = "";
	std::string path = "";
	std::string description = "";
	GitlabGroupVisibility visibility = GITLAB_GROUP_PRIVATE;
	bool enable_lfs = false;
	bool access_requests = true;
};

class GitlabAPIClient: public HTTPClient
{
public:
	GitlabAPIClient(const std::string &server_uri, const std::string &api_token):
		m_server_uri(server_uri),
		m_api_token(api_token)
	{}

	// Issues
	bool get_issue(const uint32_t project_id, const uint32_t issue_id, Json::Value &result);
	bool get_issues(const uint32_t project_id, const std::string &filter, Json::Value &result);
	const GitlabRetCod create_issue(const uint32_t project_id, const GitlabIssue &issue);
	const GitlabRetCod modify_issue(const uint32_t project_id, const uint32_t issue_id,
			const GitlabIssue &issue);
	const GitlabRetCod close_issue(const uint32_t project_id, const uint32_t issue_id);
	const GitlabRetCod delete_issue(const uint32_t project_id, const uint32_t issue_id);

	// Merge requests
	bool get_merge_request(const uint32_t project_id, const uint32_t issue_id, Json::Value &result);
	bool get_merge_requests(const uint32_t project_id, const std::string &filter, Json::Value &result);
	const GitlabRetCod close_merge_request(const uint32_t project_id, const uint32_t issue_id);
	const GitlabRetCod delete_merge_request(const uint32_t project_id, const uint32_t issue_id);

	// Labels
	bool create_label(const uint32_t project_id, const std::string &label,
			const std::string &color_id, Json::Value &res);
	bool delete_label(const uint32_t project_id, const std::string &label);

	// Groups
	bool create_group(const GitlabGroup &group, Json::Value &res);
private:
	void build_issue_data(const GitlabIssue &issue, std::string &post_data);

	std::string m_server_uri = "https://gitlab.com";
	std::string m_api_token = "";
	static const std::string api_v3_endpoint;
};
