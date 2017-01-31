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
#include <httpserver.h>
#include <utils/stringutils.h>
#include <elasticsearchclient.h>
#include <console.h>
#include <openweathermapclient.h>
#include <luaengine.h>
#include <postgresqlclient.h>

#include "unittests_config.h"

#define CPPUNIT_TESTSUITE_CREATE(s) CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite(std::string(s));
#define CPPUNIT_ADDTEST(c, s, f) suiteOfTests->addTest(new CppUnit::TestCaller<c>(s, &c::f));

static std::string GITLAB_TOKEN = "";
static std::string ES_HOST = "localhost";
static std::string RUN_TIMESTAMP = std::to_string(time(NULL));

class WinterWindTests: public CppUnit::TestFixture
{
public:
	WinterWindTests() {}
	virtual ~WinterWindTests()
	{
		delete m_gitlab_client;
		delete m_http_server;
	}

	static CppUnit::Test *suite()
	{
		CPPUNIT_TESTSUITE_CREATE("WinterWind")
		CPPUNIT_ADDTEST(WinterWindTests, "StringUtils - Test1 - Split string", split_string);
		CPPUNIT_ADDTEST(WinterWindTests, "StringUtils - Test2 - Remove substring", remove_substring);

		CPPUNIT_ADDTEST(WinterWindTests, "LuaEngine - Test1 - Load winterwind engine", lua_winterwind_engine);

		CPPUNIT_ADDTEST(WinterWindTests, "Weather - Test1", weather_to_json);

		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test1 - Handle GET", httpserver_handle_get);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test2 - Test headers", httpserver_header);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test3 - Test get params", httpserver_getparam);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test4 - Handle POST (form encoded)", httpserver_handle_post);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test5 - Handle POST (json)", httpserver_handle_post_json);

		CPPUNIT_ADDTEST(WinterWindTests, "ElasticsearchClient - Test1 - ElasticsearchBulkAction Index/Create to JSON", es_bulk_to_json);
		CPPUNIT_ADDTEST(WinterWindTests, "ElasticsearchClient - Test2 - ElasticsearchBulkAction Update to JSON", es_bulk_update_to_json);
		CPPUNIT_ADDTEST(WinterWindTests, "ElasticsearchClient - Test3 - ElasticsearchBulkAction Delete to JSON", es_bulk_delete_to_json);
		CPPUNIT_ADDTEST(WinterWindTests, "ElasticsearchClient - Test4 - ElasticsearchBulkAction Index data", es_bulk_play_index);
		CPPUNIT_ADDTEST(WinterWindTests, "ElasticsearchClient - Test5 - ElasticsearchBulkAction Update data", es_bulk_play_update);
		CPPUNIT_ADDTEST(WinterWindTests, "ElasticsearchClient - Test6 - ElasticsearchBulkAction Delete data", es_bulk_play_delete);

		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test1 - Creation", create_default_groups);
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test2 - Creation (parameters)", create_group);
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test3 - Get", get_group);
		CPPUNIT_ADDTEST(WinterWindTests, "Namespace - Test1 - Get (all)", get_namespaces);
		CPPUNIT_ADDTEST(WinterWindTests, "Namespace - Test2 - Get", get_namespace);
		CPPUNIT_ADDTEST(WinterWindTests, "Project - Test1 - Creation", create_default_projects);
		CPPUNIT_ADDTEST(WinterWindTests, "Project - Test2 - Creation (parameters)", create_project);
		CPPUNIT_ADDTEST(WinterWindTests, "Project - Test3 - Get (multiple)", get_projects);
		CPPUNIT_ADDTEST(WinterWindTests, "Project - Test4 - Get", get_project);
		CPPUNIT_ADDTEST(WinterWindTests, "Project - Test5 - Get (with namespace)", get_project_ns);
		CPPUNIT_ADDTEST(WinterWindTests, "Label - Test1 - Creation", create_label);
		CPPUNIT_ADDTEST(WinterWindTests, "Label - Test2 - Get", get_label);
		CPPUNIT_ADDTEST(WinterWindTests, "Label - Test3 - Removal", remove_label);

		// Should be done at the end
		CPPUNIT_ADDTEST(WinterWindTests, "Project - Test5 - Removal", remove_project);
		CPPUNIT_ADDTEST(WinterWindTests, "Project - Test6 - Removal (multiple)", remove_projects);
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test3 - Removal", remove_group);
		CPPUNIT_ADDTEST(WinterWindTests, "Group - Test3 - Removal (multiple)", remove_groups);

		/*
		 * Test HTTPClient TODO
		 * Gitlab client TODO
		 * - tags
		 * - issues
		 * - merge requests
		 * PostgreSQL client
		 */
		return suiteOfTests;
	}

	/// Setup method
	void setUp()
	{
		m_gitlab_client = new GitlabAPIClient("https://gitlab.com", GITLAB_TOKEN);
		m_http_server = new HTTPServer(58080);
		BIND_HTTPSERVER_HANDLER(m_http_server, GET, "/unittest.html",
			&WinterWindTests::httpserver_testhandler, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, GET, "/unittest2.html",
			&WinterWindTests::httpserver_testhandler2, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, GET, "/unittest3.html",
			&WinterWindTests::httpserver_testhandler3, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, POST, "/unittest4.html",
			&WinterWindTests::httpserver_testhandler4, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, POST, "/unittest5.html",
			&WinterWindTests::httpserver_testhandler5, this)
	}

	/// Teardown method
	void tearDown() {}

protected:
	void split_string()
	{
		std::string orig = "hello, this is winterwind.";
		std::vector<std::string> res;
		str_split(orig, ' ', res);
		CPPUNIT_ASSERT(res.size() == 4 && res[2] == "is");
	}

	void remove_substring()
	{
		std::string orig = "The world is mine, the world is not yours";
		std::string to_alter = orig;
		str_remove_substr(to_alter, "world ");
		CPPUNIT_ASSERT(to_alter == "The is mine, the is not yours");
	}

	void lua_winterwind_engine()
	{
		LuaEngine L;
		LuaReturnCode  rc = L.init_winterwind_bindings();
		CPPUNIT_ASSERT(rc == LUA_RC_OK);
		rc = L.load_script(UNITTESTS_LUA_FILE);
		CPPUNIT_ASSERT(rc == LUA_RC_OK);
		CPPUNIT_ASSERT(L.run_unittests());
	}

	void weather_to_json()
	{
		Weather w;
		w.sunset = 150;
		w.sunrise = 188;
		w.humidity = 4;
		w.temperature = 25.0f;
		w.city = "test_city";
		Json::Value res;
		w >> res;
		CPPUNIT_ASSERT(res["sunset"].asUInt() == 150);
		CPPUNIT_ASSERT(res["sunrise"].asUInt() == 188);
		CPPUNIT_ASSERT(res["humidity"].asInt() == 4);
		CPPUNIT_ASSERT(res["temperature"].asFloat() == 25.0f);
		CPPUNIT_ASSERT(res["city"].asString() == "test_city");
	}

	HTTPResponse *httpserver_testhandler(const HTTPQueryPtr q)
	{
		return new HTTPResponse(HTTPSERVER_TEST01_STR);
	}

	HTTPResponse *httpserver_testhandler2(const HTTPQueryPtr q)
	{
		std::string res = "no";

		const auto it = q->headers.find("UnitTest-Header");
		if (it != q->headers.end() && it->second == "1") {
			res = "yes";
		}
		return new HTTPResponse(res);
	}

	HTTPResponse *httpserver_testhandler3(const HTTPQueryPtr q)
	{
		std::string res = "no";

		const auto it = q->get_params.find("UnitTestParam");
		if (it != q->get_params.end() && it->second == "thisistestparam") {
			res = "yes";
		}

		return new HTTPResponse(res);
	}

	HTTPResponse *httpserver_testhandler4(const HTTPQueryPtr q)
	{
		std::string res = "no";
		CPPUNIT_ASSERT(q->get_type() == HTTPQUERY_TYPE_FORM);

		HTTPFormQuery *fq = dynamic_cast<HTTPFormQuery *>(q.get());
		CPPUNIT_ASSERT(fq);

		const auto it = fq->post_data.find("post_param");
		if (it != fq->post_data.end() && it->second == "ilikedogs") {
			res = "yes";
		}

		return new HTTPResponse(res);
	}

	HTTPResponse *httpserver_testhandler5(const HTTPQueryPtr q)
	{
		Json::Value json_res;
		json_res["status"] = "no";
		CPPUNIT_ASSERT(q->get_type() == HTTPQUERY_TYPE_JSON);

		HTTPJsonQuery *jq = dynamic_cast<HTTPJsonQuery *>(q.get());
		CPPUNIT_ASSERT(jq);

		if (jq->json_query.isMember("json_param")
			&& jq->json_query["json_param"].asString() == "catsarebeautiful") {
			json_res["status"] = "yes";
		}

		return new JSONHTTPResponse(json_res);
	}

	void httpserver_handle_get()
	{
		HTTPClient cli;
		std::string res;
		cli._get("http://localhost:58080/unittest.html", res);
		CPPUNIT_ASSERT(res == HTTPSERVER_TEST01_STR);
	}

	void httpserver_header()
	{
		HTTPClient cli;
		std::string res;
		cli.add_http_header("UnitTest-Header", "1");
		cli._get("http://localhost:58080/unittest2.html", res);
		CPPUNIT_ASSERT(res == "yes");
	}

	void httpserver_getparam()
	{
		HTTPClient cli;
		std::string res;
		cli._get("http://localhost:58080/unittest3.html?UnitTestParam=thisistestparam", res);
		CPPUNIT_ASSERT(res == "yes");
	}

	void httpserver_handle_post()
	{
		HTTPClient cli;
		std::string res;
		cli.add_http_header("Content-Type", "application/x-www-form-urlencoded");
		cli._post("http://localhost:58080/unittest4.html", "post_param=ilikedogs", res);
		CPPUNIT_ASSERT(res == "yes");
	}

	void httpserver_handle_post_json()
	{
		HTTPClient cli;
		Json::Value query;
		query["json_param"] = "catsarebeautiful";
		Json::Value res;
		cli._post_json("http://localhost:58080/unittest5.html", query, res);
		CPPUNIT_ASSERT(res.isMember("status") && res["status"] == "yes");
	}

	void es_bulk_to_json()
	{
		ElasticsearchBulkAction action(ESBULK_AT_INDEX);
		action.index = "library";
		action.type = "book";
		action.doc_id = "7";
		action.doc["title"] = "A great history";

		Json::FastWriter writer;
		std::string res = "";
		action.toJson(writer, res);

		CPPUNIT_ASSERT(res == "{\"index\":{\"_id\":\"7\",\"_index\":\"library\",\"_type\":\"book\"}}\n"
			"{\"title\":\"A great history\"}\n");
	}

	void es_bulk_update_to_json()
	{
		ElasticsearchBulkAction action(ESBULK_AT_UPDATE);
		action.index = "car";
		action.type = "truck";
		action.doc_id = "666";
		action.doc["engine"] = "Toyota";

		Json::FastWriter writer;
		std::string res = "";
		action.toJson(writer, res);

		CPPUNIT_ASSERT(res == "{\"update\":{\"_id\":\"666\",\"_index\":\"car\",\"_type\":\"truck\"}}\n"
			"{\"doc\":{\"engine\":\"Toyota\"}}\n");
	}

	void es_bulk_delete_to_json()
	{
		ElasticsearchBulkAction action(ESBULK_AT_DELETE);
		action.index = "food";
		action.type = "meat";
		action.doc_id = "5877";

		Json::FastWriter writer;
		std::string res = "";
		action.toJson(writer, res);

		CPPUNIT_ASSERT(res == "{\"delete\":{\"_id\":\"5877\",\"_index\":\"food\",\"_type\":\"meat\"}}\n");
	}

	void es_bulk_play_index()
	{
		ElasticsearchBulkActionPtr action(new ElasticsearchBulkAction(ESBULK_AT_INDEX));
		action->index = "library";
		action->type = "book";
		action->doc_id = "7";
		action->doc["title"] = "A great history";

		std::string bulk_res = "";
		ElasticsearchClient client("http://" + ES_HOST + ":9200");
		client.add_bulkaction_to_queue(action);
		client.process_bulkaction_queue(bulk_res);

		Json::Reader reader;
		Json::Value es_res;
		CPPUNIT_ASSERT(reader.parse(bulk_res, es_res));
		CPPUNIT_ASSERT(es_res.isMember("errors"));
		CPPUNIT_ASSERT(es_res["errors"].isBool());
		CPPUNIT_ASSERT(!es_res["errors"].asBool());
	}

	void es_bulk_play_update()
	{
		ElasticsearchBulkActionPtr action(new ElasticsearchBulkAction(ESBULK_AT_UPDATE));
		action->index = "library";
		action->type = "book";
		action->doc_id = "7";
		action->doc["title"] = "A great history (version 2)";

		ElasticsearchBulkActionPtr action2(new ElasticsearchBulkAction(ESBULK_AT_UPDATE));
		action2->index = "library";
		action2->type = "book";
		action2->doc_id = "7";
		action2->doc["title"] = "A great history (version 3)";

		std::string bulk_res = "";
		ElasticsearchClient client("http://" + ES_HOST + ":9200");
		client.add_bulkaction_to_queue(action);
		client.add_bulkaction_to_queue(action2);
		client.process_bulkaction_queue(bulk_res);

		Json::Reader reader;
		Json::Value es_res;
		CPPUNIT_ASSERT(reader.parse(bulk_res, es_res));
		CPPUNIT_ASSERT(es_res.isMember("errors"));
		CPPUNIT_ASSERT(es_res["errors"].isBool());
		CPPUNIT_ASSERT(!es_res["errors"].asBool());
	}

	void es_bulk_play_delete()
	{
		ElasticsearchBulkActionPtr action(new ElasticsearchBulkAction(ESBULK_AT_DELETE));
		action->index = "library";
		action->type = "book";
		action->doc_id = "7";

		std::string bulk_res = "";
		ElasticsearchClient client("http://" + ES_HOST + ":9200");
		client.add_bulkaction_to_queue(action);
		client.process_bulkaction_queue(bulk_res);

		Json::Reader reader;
		Json::Value es_res;
		CPPUNIT_ASSERT(reader.parse(bulk_res, es_res));
		CPPUNIT_ASSERT(es_res.isMember("errors"));
		CPPUNIT_ASSERT(es_res["errors"].isBool());
		CPPUNIT_ASSERT(!es_res["errors"].asBool());
	}

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
		GitlabGroup g(TEST_GROUP, TEST_GROUP);
		g.description = "test";
		g.visibility = GITLAB_GROUP_PUBLIC;
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g, res));
	}

	void get_group()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_group(
				std::string("ww_testgroup_") + RUN_TIMESTAMP, result) == GITLAB_RC_OK);
	}

	void remove_group()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_group(TEST_GROUP) == GITLAB_RC_OK);
	}

	void remove_groups()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_groups({
				"ww_testgroup_default_" + RUN_TIMESTAMP,
				"ww_testgroup2_default_" + RUN_TIMESTAMP}) == GITLAB_RC_OK);
	}

	void get_namespaces()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_namespaces("", result) == GITLAB_RC_OK);
	}

	void get_namespace()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_namespace(
				std::string("ww_testgroup_") + RUN_TIMESTAMP, result) == GITLAB_RC_OK);

		m_testing_namespace_id = result["id"].asUInt();
	}

	void create_default_projects()
	{
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->create_project(GitlabProject("ww_testproj1_default_" + RUN_TIMESTAMP), res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(m_gitlab_client->create_project(GitlabProject("ww_testproj2_default_" + RUN_TIMESTAMP), res) == GITLAB_RC_OK);
	}

	void create_project()
	{
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
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_projects("ww_testproj", res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res[0].isMember("http_url_to_repo"));
	}

	void get_project()
	{
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_project("ww_testproj1_default_" + RUN_TIMESTAMP, res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res.isMember("name_with_namespace"));
	}

	void get_project_ns()
	{
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_project_ns(
			"ww_testproj_" + RUN_TIMESTAMP, TEST_GROUP, res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res.isMember("avatar_url"));
	}

	void remove_project()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_project(
				std::string("ww_testproj_") + RUN_TIMESTAMP) == GITLAB_RC_OK);
	}

	void remove_projects()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_projects({
				"ww_testproj1_default_" + RUN_TIMESTAMP,
				"ww_testproj2_default_" + RUN_TIMESTAMP}) == GITLAB_RC_OK);
	}

	void create_label()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->create_label(TEST_GROUP,
				"ww_testproj_" + RUN_TIMESTAMP, TEST_LABEL, TEST_COLOR, result)
				== GITLAB_RC_OK);
	}

	void get_label()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_label(TEST_GROUP,
				"ww_testproj_" + RUN_TIMESTAMP, TEST_LABEL, result) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(result.isMember("color") && result["color"].asString() == TEST_COLOR);
	}

	void remove_label()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_label(TEST_GROUP,
				"ww_testproj_" + RUN_TIMESTAMP, TEST_LABEL) == GITLAB_RC_OK);
	}
private:
	GitlabAPIClient *m_gitlab_client = nullptr;
	HTTPServer *m_http_server = nullptr;
	uint32_t m_testing_namespace_id = 0;
	std::string TEST_COLOR = "#005577";
	std::string TEST_GROUP = "ww_testgroup_" + RUN_TIMESTAMP;
	std::string TEST_LABEL = "test_label_1";
	std::string HTTPSERVER_TEST01_STR = "<h1>unittest_result</h1>";
};

int main(int argc, const char* argv[])
{
	if (argc < 2) {
		std::cerr << argv[0] << ": Missing gitlab token" << std::endl;
		return -1;
	}

	GITLAB_TOKEN = std::string(argv[1]);

	if (argc >= 3) {
		ES_HOST = std::string(argv[2]);
	}

	CppUnit::TextUi::TestRunner runner;
	runner.addTest(WinterWindTests::suite());
	std::cout << "Starting unittests...." << std::endl;
	return runner.run() ? 0 : 1;

}
