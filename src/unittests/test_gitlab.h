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

#include <extras/gitlabapiclient.h>

using namespace winterwind::extras;

namespace winterwind {
namespace unittests {

class Test_Gitlab : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_Gitlab);
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
	void setUp() override;
	void tearDown() override;

protected:
	void create_default_groups();
	void create_group();
	void get_group();
	void remove_group();
	void remove_groups();
	void get_namespaces();
	void get_namespace();
	void create_default_projects();
	void create_project();
	void get_projects();
	void get_project();
	void get_project_ns();
	void remove_project();
	void remove_projects();
	void create_label();
	void get_label();
	void remove_label();

private:
	static std::string RUN_TIMESTAMP;
	std::unique_ptr<GitlabAPIClient> m_gitlab_client = nullptr;
	bool gitlab_has_failed_in_init = false;
	uint32_t m_testing_namespace_id = 0;
	std::string TEST_COLOR = "#005577";
	std::string TEST_GROUP = "ww_testgroup_" + RUN_TIMESTAMP;
	std::string TEST_LABEL = "test_label_1";
};
}
}
