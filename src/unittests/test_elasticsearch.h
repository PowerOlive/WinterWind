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

#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <elasticsearchclient.h>

#include "unittests_config.h"

static std::string ES_HOST = "localhost";

class WinterWindTest_Elasticsearch: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(WinterWindTest_Elasticsearch);
	CPPUNIT_TEST(es_bulk_to_json);
	CPPUNIT_TEST(es_bulk_update_to_json);
	CPPUNIT_TEST(es_bulk_delete_to_json);
	CPPUNIT_TEST(es_bulk_play_index);
	CPPUNIT_TEST(es_bulk_play_update);
	CPPUNIT_TEST(es_bulk_play_delete);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp() {}
	void tearDown() {}

protected:
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
};
