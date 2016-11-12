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

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>


#include <gitlabapiclient.h>

#define CPPUNIT_TESTSUITE_CREATE(s) CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite(std::string(s));
#define CPPUNIT_ADDTEST(c, s, f) suiteOfTests->addTest(new CppUnit::TestCaller<c>(s, &c::f));

static std::string GITLAB_TOKEN = "";
static std::string RUN_TIMESTAMP = std::to_string(time(NULL));

class WinterWindTests: public CppUnit::TestFixture
{
public:
	WinterWindTests() {}
	virtual ~WinterWindTests()
	{
		delete m_gitlab_client;
	}

	static CppUnit::Test *suite()
	{
		CPPUNIT_TESTSUITE_CREATE("WinterWind")
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test1 - Creation", create_default_groups);
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test2 - Creation (parameters)", create_group);

		// Should be done at the end
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test3 - Removal", remove_group);
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test3 - Removal (multiple)", remove_groups);
		return suiteOfTests;
	}

	/// Setup method
	void setUp()
	{
		m_gitlab_client = new GitlabAPIClient("https://gitlab.com", GITLAB_TOKEN);
	}

	/// Teardown method
	void tearDown() {}

protected:
	void create_default_groups()
	{
		Json::Value res;
		GitlabGroup g("ww_testgroup_default_" + RUN_TIMESTAMP, "ww_testgroup_default_" + RUN_TIMESTAMP);
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g, res));

		GitlabGroup g2("ww_testgroup2_default_" + RUN_TIMESTAMP, "ww_testgroup2_default_" + RUN_TIMESTAMP);
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g2, res));
	}

	void create_group()
	{
		Json::Value res;
		GitlabGroup g("ww_testgroup_" + RUN_TIMESTAMP, "ww_testgroup_" + RUN_TIMESTAMP);
		g.description = "test";
		g.visibility = GITLAB_GROUP_PUBLIC;
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g, res));
	}

	void remove_group()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_group("ww_testgroup_" + RUN_TIMESTAMP) == GITLAB_RC_OK);
	}

	void remove_groups()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_groups({
				"ww_testgroup_default_" + RUN_TIMESTAMP,
				"ww_testgroup2_default_" + RUN_TIMESTAMP }) == GITLAB_RC_OK);
	}
private:
	GitlabAPIClient *m_gitlab_client = nullptr;
};

int main(int argc, const char* argv[])
{
	if (argc < 2) {
		std::cerr << argv[0] << ": Missing gitlab token" << std::endl;
		return -1;
	}

	GITLAB_TOKEN = std::string(argv[1]);

	CppUnit::TextUi::TestRunner runner;
	runner.addTest(WinterWindTests::suite());
	std::cout << "Starting unittests...." << std::endl;
	return runner.run() ? 0 : 1;

}
