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

#include <core/httpserver.h>

#include "cmake_config.h"

using namespace winterwind::http;

class WinterWindTest_HTTP : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(WinterWindTest_HTTP);
	CPPUNIT_TEST(httpserver_handle_get);
	CPPUNIT_TEST(httpserver_header);
	CPPUNIT_TEST(httpserver_getparam);
	CPPUNIT_TEST(httpserver_handle_post);
	CPPUNIT_TEST(httpserver_handle_post_json);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
		m_http_server = new Server(58080);
		m_http_server->register_handler(winterwind::http::Method::GET, "/unittest.html",
			std::bind(&WinterWindTest_HTTP::httpserver_testhandler, this, std::placeholders::_1));
		m_http_server->register_handler(winterwind::http::Method::GET, "/unittest2.html",
			std::bind(&WinterWindTest_HTTP::httpserver_testhandler2, this, std::placeholders::_1));
		m_http_server->register_handler(winterwind::http::Method::GET, "/unittest3.html",
			std::bind(&WinterWindTest_HTTP::httpserver_testhandler3, this, std::placeholders::_1));
		m_http_server->register_handler(winterwind::http::Method::POST, "/unittest4.html",
			std::bind(&WinterWindTest_HTTP::httpserver_testhandler4, this, std::placeholders::_1));
		m_http_server->register_handler(winterwind::http::Method::POST, "/unittest5.html",
			std::bind(&WinterWindTest_HTTP::httpserver_testhandler5, this, std::placeholders::_1));
	}

	void tearDown() { delete m_http_server; }

protected:
	ResponsePtr httpserver_testhandler(const HTTPQueryPtr q)
	{
		return std::make_shared<Response>(HTTPSERVER_TEST01_STR);
	}

	ResponsePtr httpserver_testhandler2(const HTTPQueryPtr q)
	{
		std::string res = "no";

		const auto it = q->headers.find("UnitTest-Header");
		if (it != q->headers.end() && it->second == "1") {
			res = "yes";
		}
		return std::make_shared<Response>(res);
	}

	ResponsePtr httpserver_testhandler3(const HTTPQueryPtr q)
	{
		std::string res = "no";

		const auto it = q->get_params.find("UnitTestParam");
		if (it != q->get_params.end() && it->second == "thisistestparam") {
			res = "yes";
		}

		return std::make_shared<Response>(res);
	}

	ResponsePtr httpserver_testhandler4(const HTTPQueryPtr q)
	{
		std::string res = "no";
		CPPUNIT_ASSERT(q->get_type() == HTTPQUERY_TYPE_FORM);

		HTTPFormQuery *fq = dynamic_cast<HTTPFormQuery *>(q.get());
		CPPUNIT_ASSERT(fq);

		const auto it = fq->post_data.find("post_param");
		if (it != fq->post_data.end() && it->second == "ilikedogs") {
			res = "yes";
		}

		return std::make_shared<Response>(res);
	}

	ResponsePtr httpserver_testhandler5(const HTTPQueryPtr q)
	{
		Json::Value json_res;
		json_res["status"] = "no";
		CPPUNIT_ASSERT(q->get_type() == HTTPQUERY_TYPE_JSON);

		HTTPJsonQuery *jq = dynamic_cast<HTTPJsonQuery *>(q.get());
		CPPUNIT_ASSERT(jq);

		if (jq->json_query.isMember("json_param") &&
		    jq->json_query["json_param"].asString() == "catsarebeautiful") {
			json_res["status"] = "yes";
		}

		return std::make_shared<JSONResponse>(json_res);
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
		cli._get("http://localhost:58080/unittest3.html?UnitTestParam=thisistestparam",
			 res);
		CPPUNIT_ASSERT(res == "yes");
	}

	void httpserver_handle_post()
	{
		HTTPClient cli;
		std::string res;
		cli.add_http_header("Content-Type", "application/x-www-form-urlencoded");
		cli.add_form_param("post_param", "ilikedogs");
		cli._post("http://localhost:58080/unittest4.html", res);
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

private:
	Server *m_http_server = nullptr;
	std::string HTTPSERVER_TEST01_STR = "<h1>unittest_result</h1>";
};
