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

#include <iostream>
#include "jiraclient.h"

#define JIRA_API_V1_SESSION "/rest/auth/1/session"
#define JIRA_API_V2_ISSUE "/rest/api/2/issue/"
#define JIRA_API_V2_ISSUE_ASSIGNEE "/assignee"
#define JIRA_API_V2_ISSUE_COMMENT "/comment"
#define JIRA_API_V2_ISSUE_TRANSITION "/transitions"
#define JIRA_API_V2_PROJECT "/rest/api/2/project"

JiraClient::JiraClient(const std::string &instance_url, const std::string &user, const std::string &password):
		HTTPClient(), m_instance_url(instance_url)
{
	m_username = user;
	m_password = password;
}

bool JiraClient::test_connection()
{
	Json::Value res;
	if (!_get_json(m_instance_url + JIRA_API_V1_SESSION, res, ReqFlag::REQ_AUTH)) {
		return false;
	}
	return res.isMember("name") && res.isMember("self") && res.isMember("loginInfo");
}

bool JiraClient::get_issue(const std::string &issue_id, Json::Value &result)
{
	return _get_json(m_instance_url + JIRA_API_V2_ISSUE + issue_id, result, ReqFlag::REQ_AUTH);
}

inline Json::Value create_jira_issue_object(const std::string &description, const std::string &summary)
{
	Json::Value req;
	req["fields"] = Json::Value();
	req["fields"]["project"] = Json::Value();
	req["fields"]["summary"] = summary;
	req["fields"]["description"] = description;
	req["fields"]["issuetype"] = Json::Value();
	return req;
}

bool JiraClient::create_issue(const std::string &project_name, const std::string &issue_type,
							  const std::string &summary, const std::string &description, Json::Value &res)
{
	Json::Value req = create_jira_issue_object(description, summary);
	req["fields"]["project"]["key"] = project_name;
	req["fields"]["issuetype"]["name"] = issue_type;

	return _post_json(m_instance_url + JIRA_API_V2_ISSUE, req, res, ReqFlag::REQ_AUTH);
}

bool JiraClient::create_issue(const uint32_t project_id, const uint32_t issue_type_id,
							  const std::string &summary, const std::string &description, Json::Value &res)
{
	Json::Value req = create_jira_issue_object(description, summary);
	req["fields"]["project"]["id"] = project_id;
	req["fields"]["issuetype"]["id"] = issue_type_id;
	return _post_json(m_instance_url + JIRA_API_V2_ISSUE, req, res, ReqFlag::REQ_AUTH);
}

bool JiraClient::assign_issue(const std::string &issue, const std::string &who, Json::Value &res)
{
	if (issue.empty() || who.empty()) {
		return false;
	}

	{
		Json::Value issue_res;
		if (get_issue(issue, issue_res)) {
			if (issue_res.isMember("fields") && issue_res["fields"].isObject() &&
					issue_res["fields"].isMember("assignee") && issue_res["fields"]["assignee"].isObject() &&
					issue_res["fields"]["assignee"].isMember("name") &&
					issue_res["fields"]["assignee"]["name"].isString() &&
					issue_res["fields"]["assignee"]["name"].asString() == who) {
				m_http_code = 304;
				return true;
			}
		}
	}

	Json::Value req;
	req["name"] = who;

	return _put_json(m_instance_url + JIRA_API_V2_ISSUE + issue + JIRA_API_V2_ISSUE_ASSIGNEE, req,
			res, ReqFlag::REQ_AUTH | ReqFlag::REQ_NO_RESPONSE_AWAITED);
}

bool JiraClient::comment_issue(const std::string &issue, const std::string &body, Json::Value &res)
{
	if (issue.empty() || body.empty()) {
		return false;
	}

	Json::Value req;
	req["body"] = body;

	return _post_json(m_instance_url + JIRA_API_V2_ISSUE + issue + JIRA_API_V2_ISSUE_COMMENT, req,
					 res, ReqFlag::REQ_AUTH);
}

bool JiraClient::issue_transition(const std::string &issue, const std::string &transition_id, Json::Value &res,
		const std::string &comment, const Json::Value &fields)
{
	if (issue.empty() || transition_id.empty()) {
		return false;
	}

	Json::Value req;
	req["transition"] = Json::Value();
	req["transition"]["id"] = transition_id;

	if (!fields.empty()) {
		req["fields"] = fields;
	}

	if (!comment.empty()) {
		req["update"] = Json::Value();
		req["update"]["comment"] = Json::Value();

		Json::Value comment_add;
		comment_add["add"] = Json::Value();
		comment_add["add"]["body"] = comment;
		req["update"]["comment"].append(comment_add);
	}

	return _post_json(m_instance_url + JIRA_API_V2_ISSUE + issue + JIRA_API_V2_ISSUE_TRANSITION, req,
					  res, ReqFlag::REQ_AUTH | ReqFlag::REQ_NO_RESPONSE_AWAITED);
}

bool JiraClient::get_issue_transtions(const std::string &issue, Json::Value &res)
{
	return _get_json(m_instance_url + JIRA_API_V2_ISSUE + issue + JIRA_API_V2_ISSUE_TRANSITION, res, ReqFlag::REQ_AUTH);
}

bool JiraClient::list_projects(Json::Value &res)
{
	return _get_json(m_instance_url + JIRA_API_V2_PROJECT, res, ReqFlag::REQ_AUTH);
}