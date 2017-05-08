/*
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

#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <gitlabapiclient.h>
#include <iomanip>
#include <regex>
#include "utils/time.h"

static std::string GITLAB_TOKEN = "";
static std::string RUN_TIMESTAMP = std::to_string(time(NULL));

#define ONLY_IF_GITLAB_INIT_SUCCEED \
if (gitlab_has_failed_in_init) { \
	return; \
}

#define MARK_GITLAB_FAILURE_ON_INIT_IF \
	if (m_gitlab_client->get_http_code() == 502) { \
	gitlab_has_failed_in_init = true; \
}

class WinterWindTest_Gitlab : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(WinterWindTest_Gitlab);
	CPPUNIT_TEST(create_default_groups);
	CPPUNIT_TEST(create_group);
	CPPUNIT_TEST(get_group);
	CPPUNIT_TEST(get_namespaces);

	CPPUNIT_TEST(create_default_projects);
	CPPUNIT_TEST(create_project);
	CPPUNIT_TEST(get_projects);
	CPPUNIT_TEST(get_project);
	CPPUNIT_TEST(get_project_ns);

	CPPUNIT_TEST(create_label);
	CPPUNIT_TEST(get_label);
	CPPUNIT_TEST(remove_label);

	CPPUNIT_TEST(remove_project);
	CPPUNIT_TEST(remove_projects);

	CPPUNIT_TEST(remove_group);
	CPPUNIT_TEST(remove_groups);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
		m_gitlab_client = new GitlabAPIClient("https://gitlab.com", GITLAB_TOKEN);
	}

	void tearDown()
	{
		// Cleanup very old projects
		Json::Value projects;
		if (m_gitlab_client->get_projects("", projects, GITLAB_PROJECT_SS_OWNED) == GITLAB_RC_OK) {
			for (const auto &p: projects) {
				// Creation date is mandatory
				if (!p.isMember("created_at") || !p["created_at"].isString()) {
					continue;
				}

				std::time_t t;
				if (!str_to_timestamp(p["created_at"].asString(), t)) {
					std::cerr << "Failed to parse date: '" << p["create_at"].asString() << "'" << std::endl;
					continue;
				}

				// Remove projects older than 24 hours
				if (t < std::time(0) - 86400) {
					std::cout << "Project " << p["name"].asString() << " will be deleted" << std::endl;
					m_gitlab_client->delete_project(p["name"].asString());
				}

			}
		}

		Json::Value groups;
		if (m_gitlab_client->get_groups("ww_testgroup_", groups)) {
			std::regex re("ww_testgroup_([0-9]+)");
			for (const auto &g : groups) {
				std::smatch sm;
				const std::string gname = g["name"].asString();
				if (std::regex_match(gname, sm, re) && sm.size() > 0) {
					const std::string timestamp_str = sm.str(1);
					std::time_t t = std::atoi(timestamp_str.c_str());
					if (t < std::time(0) - 86400) {
						m_gitlab_client->delete_group(gname);
					}
				}
			}
		}
		delete m_gitlab_client;
	}

protected:
	void create_default_groups()
	{
		Json::Value res;
		GitlabGroup g("ww_testgroup_default_" + RUN_TIMESTAMP,
			      "ww_testgroup_default_" + RUN_TIMESTAMP);

		bool rc = m_gitlab_client->create_group(g, res);
		std::string error_msg = std::string("Unable to create 1st default group (rc: ");
		error_msg += std::to_string(m_gitlab_client->get_http_code()) + ")";

		MARK_GITLAB_FAILURE_ON_INIT_IF

		CPPUNIT_ASSERT_MESSAGE(error_msg, rc || m_gitlab_client->get_http_code() == 502);

		ONLY_IF_GITLAB_INIT_SUCCEED

		GitlabGroup g2("ww_testgroup2_default_" + RUN_TIMESTAMP,
			       "ww_testgroup2_default_" + RUN_TIMESTAMP);

		rc = m_gitlab_client->create_group(g2, res);
		error_msg = std::string("Unable to create 2nd default group (rc: ");
		error_msg += std::to_string(m_gitlab_client->get_http_code()) + ")";
		CPPUNIT_ASSERT_MESSAGE(error_msg, rc
			|| m_gitlab_client->get_http_code() == 502);

		MARK_GITLAB_FAILURE_ON_INIT_IF
	}

	void create_group()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED

		Json::Value res;
		GitlabGroup g(TEST_GROUP, TEST_GROUP);
		g.description = "test";
		g.visibility = GITLAB_GROUP_PUBLIC;
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g, res)
			|| m_gitlab_client->get_http_code() == 502);

		MARK_GITLAB_FAILURE_ON_INIT_IF
	}

	void get_group()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_group(std::string("ww_testgroup_")
				+ RUN_TIMESTAMP, result) == GITLAB_RC_OK
			|| m_gitlab_client->get_http_code() == 502);
	}

	void remove_group()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		CPPUNIT_ASSERT(m_gitlab_client->delete_group(TEST_GROUP) == GITLAB_RC_OK
			|| m_gitlab_client->get_http_code() == 502);
	}

	void remove_groups()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		CPPUNIT_ASSERT(m_gitlab_client->delete_groups(
				   {"ww_testgroup_default_" + RUN_TIMESTAMP,
				    "ww_testgroup2_default_" + RUN_TIMESTAMP}) == GITLAB_RC_OK
			|| m_gitlab_client->get_http_code() == 502);
	}

	void get_namespaces()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_namespaces("", result) == GITLAB_RC_OK);
	}

	void get_namespace()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value result;
		CPPUNIT_ASSERT(
		    m_gitlab_client->get_namespace(std::string("ww_testgroup_") + RUN_TIMESTAMP,
						   result) == GITLAB_RC_OK);

		m_testing_namespace_id = result["id"].asUInt();
	}

	void create_default_projects()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->create_project(
				   GitlabProject("ww_testproj1_default_" + RUN_TIMESTAMP), res) ==
			       GITLAB_RC_OK || m_gitlab_client->get_http_code() == 502);

		MARK_GITLAB_FAILURE_ON_INIT_IF

		ONLY_IF_GITLAB_INIT_SUCCEED

		CPPUNIT_ASSERT(m_gitlab_client->create_project(
				   GitlabProject("ww_testproj2_default_" + RUN_TIMESTAMP), res) ==
			       GITLAB_RC_OK || m_gitlab_client->get_http_code() == 502);

		MARK_GITLAB_FAILURE_ON_INIT_IF
	}

	void create_project()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		// Required to get the namespace on which create the project
		get_namespace();

		Json::Value res;
		GitlabProject proj("ww_testproj_" + RUN_TIMESTAMP);
		proj.builds_enabled = false;
		proj.visibility_level = GITLAB_PROJECT_INTERNAL;
		proj.merge_requests_enabled = false;
		proj.issues_enabled = true;
		proj.lfs_enabled = true;
		proj.description = "Amazing description";
		proj.only_allow_merge_if_all_discussions_are_resolved = true;
		proj.namespace_id = m_testing_namespace_id;
		CPPUNIT_ASSERT(m_gitlab_client->create_project(proj, res) == GITLAB_RC_OK);
	}

	void get_projects()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_projects("ww_testproj", res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res[0].isMember("http_url_to_repo"));
	}

	void get_project()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_project("ww_testproj1_default_" + RUN_TIMESTAMP,
							    res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res.isMember("name_with_namespace"));
	}

	void get_project_ns()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_project_ns("ww_testproj_" + RUN_TIMESTAMP,
							       TEST_GROUP, res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res.isMember("avatar_url"));
	}

	void remove_project()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		CPPUNIT_ASSERT(m_gitlab_client->delete_project(std::string("ww_testproj_") +
							       RUN_TIMESTAMP) == GITLAB_RC_OK);
	}

	void remove_projects()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		CPPUNIT_ASSERT(m_gitlab_client->delete_projects(
				   {"ww_testproj1_default_" + RUN_TIMESTAMP,
				    "ww_testproj2_default_" + RUN_TIMESTAMP}) == GITLAB_RC_OK);
	}

	void create_label()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value result;
		CPPUNIT_ASSERT(
		    m_gitlab_client->create_label(TEST_GROUP, "ww_testproj_" + RUN_TIMESTAMP,
						  TEST_LABEL, TEST_COLOR, result) == GITLAB_RC_OK);
	}

	void get_label()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_label(TEST_GROUP,
							  "ww_testproj_" + RUN_TIMESTAMP,
							  TEST_LABEL, result) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(result.isMember("color") &&
			       result["color"].asString() == TEST_COLOR);
	}

	void remove_label()
	{
		ONLY_IF_GITLAB_INIT_SUCCEED
		CPPUNIT_ASSERT(m_gitlab_client->delete_label(TEST_GROUP,
							     "ww_testproj_" + RUN_TIMESTAMP,
							     TEST_LABEL) == GITLAB_RC_OK);
	}

private:
	GitlabAPIClient *m_gitlab_client = nullptr;
	bool gitlab_has_failed_in_init = false;
	uint32_t m_testing_namespace_id = 0;
	std::string TEST_COLOR = "#005577";
	std::string TEST_GROUP = "ww_testgroup_" + RUN_TIMESTAMP;
	std::string TEST_LABEL = "test_label_1";
};
