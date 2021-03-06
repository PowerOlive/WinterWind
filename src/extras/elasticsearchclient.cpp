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

#include "elasticsearchclient.h"
#include <cassert>
#include <iostream>
#include <core/http/query.h>

namespace winterwind
{
using namespace http;
namespace extras
{
#define ES_URL_CLUSTER_STATE "/_cluster/state"
#define ES_URL_NODES "/_nodes"
#define ES_BULK "/_bulk"
#define ES_ANALYZE "/_analyze"

ElasticsearchClient::ElasticsearchClient(const std::string &url)
	: HTTPClient(100 * 1024), m_init_url(url)
{
	discover_cluster();
}

ElasticsearchClient::~ElasticsearchClient()
{
	while (!m_bulk_queue.empty()) {
		m_bulk_queue.pop();
	}
}

void ElasticsearchClient::discover_cluster()
{
	Json::Value res;
	Query query(m_init_url + ES_URL_CLUSTER_STATE);

	if (!_get_json(query, res) ||
		!res.isMember("cluster_name") ||
		!res["cluster_name"].isString()) {
		throw ElasticsearchException("Unable to parse Elasticsearch cluster state");
	}

	m_cluster_name = res["cluster_name"].asString();

	if (!_get_json(query, res) || !res.isMember("nodes") ||
		!res["nodes"].isObject()) {
		throw ElasticsearchException("Unable to parse Elasticsearch nodes");
	}

	Json::Value::Members cluster_members = res["nodes"].getMemberNames();
	for (const auto &member : cluster_members) {
		ElasticsearchNode node(member);
		const Json::Value &member_obj = res["nodes"][member];
		if (member_obj.isMember("http_address") &&
			member_obj["http_address"].isString()) {
			node.http_addr = "http://" + member_obj["http_address"].asString();
		} else if (member_obj.isMember("http") &&
			member_obj["http"].isObject() &&
			member_obj["http"].isMember("publish_address") &&
			member_obj["http"]["publish_address"].isString()) {
			node.http_addr =
				"http://" + member_obj["http"]["publish_address"].asString();
		}


		// If node HTTP_ADDR is empty, try nodes API
		if (node.http_addr.empty()) {
			Json::Value res_http;
			Query query_http(m_init_url + ES_URL_NODES + "/" + member + "/http");
			if (_get_json(query_http, res_http) &&
				res_http.isMember("cluster_name") &&
				res_http["cluster_name"].isString() &&
				res_http["cluster_name"].asString() == m_cluster_name &&
				res_http.isMember("nodes") &&
				res_http["nodes"].isObject() &&
				res_http["nodes"].isMember(member) &&
				res_http["nodes"][member].isObject() &&
				res_http["nodes"][member].isMember("http") &&
				res_http["nodes"][member]["http"].isObject() &&
				res_http["nodes"][member]["http"].isMember("publish_address") &&
				res_http["nodes"][member]["http"]["publish_address"].isString()) {
				const Json::Value &http_member_obj = res_http["nodes"][member];
				node.http_addr =
					"http://" + http_member_obj["http"]["publish_address"].asString();
			}
		}

		if (member_obj.isMember("version") && member_obj["version"].isString()) {
			node.version = member_obj["version"].asString();
		}

		if (member_obj.isMember("attributes") && member_obj["attributes"].isObject()) {
			Json::Value member_attrs = member_obj["attributes"];
			// Master attribute is a string, not a bool
			if (member_attrs.isMember("master") && member_attrs["master"].isString()) {
				node.is_master =
					member_attrs["master"].asString() == "true";
			}
		}

		m_nodes.push_back(node);
	}

	m_last_discovery_time = std::chrono::system_clock::now();
}

const ElasticsearchNode &ElasticsearchClient::get_fresh_node()
{
	// Rediscover cluster after 1 min
	const auto freshness = std::chrono::system_clock::now() - m_last_discovery_time;
	if (freshness.count() > 60000) {
		discover_cluster();
	}

	return m_nodes[0];
}

void ElasticsearchClient::create_doc(const std::string &index, const std::string &type,
	const Json::Value &doc)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string res;
	Json::FastWriter writer;
	std::string post_data = writer.write(doc);
	Query query(node.http_addr + "/" + index + "/" + type + "/", post_data);
	request(query, res);
}

void ElasticsearchClient::insert_doc(const std::string &index, const std::string &type,
	const std::string &doc_id, const Json::Value &doc)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string res;
	Json::FastWriter writer;

	std::string post_data = writer.write(doc);
	Query query(node.http_addr + "/" + index + "/" + type + "/" + doc_id, post_data, PUT);
	request(query, res);
}

void ElasticsearchClient::delete_doc(const std::string &index, const std::string &type,
	const std::string &doc_id)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string res;
	request(Query(node.http_addr + "/" + index + "/" + type + "/" + doc_id, DELETE), res);
}

// related to ElasticsearchBulkActionType
static const std::string bulkaction_str_mapping[ESBULK_AT_MAX] = {
	"create",
	"delete",
	"index",
	"update"};

void ElasticsearchBulkAction::toJson(Json::FastWriter &writer, std::string &res)
{
	// This should not happen
	assert(action < ESBULK_AT_MAX);

	Json::Value action_res;
	std::string action_type = bulkaction_str_mapping[action];
	action_res[action_type] = Json::Value();
	if (!index.empty()) {
		action_res[action_type]["_index"] = index;
	}

	if (!type.empty()) {
		action_res[action_type]["_type"] = type;
	}

	if (!doc_id.empty()) {
		action_res[action_type]["_id"] = doc_id;
	}

	res += writer.write(action_res);

	if (action == ESBULK_AT_CREATE || action == ESBULK_AT_INDEX) {
		res += writer.write(doc);
	} else if (action == ESBULK_AT_UPDATE) {
		Json::Value update;
		update["doc"] = doc;
		res += writer.write(update);
	}
}

void ElasticsearchClient::process_bulkaction_queue(std::string &res,
	uint32_t actions_limit)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string post_data;

	uint32_t processed_actions = 0;
	Json::FastWriter writer;
	while (!m_bulk_queue.empty() &&
		(actions_limit == 0 || processed_actions < actions_limit)) {
		processed_actions++;
		const ElasticsearchBulkActionPtr &action = m_bulk_queue.front();
		action->toJson(writer, post_data);
		m_bulk_queue.pop();
	}

	Query query(node.http_addr + ES_BULK, post_data, POST);
	request(query, res);
}

bool ElasticsearchClient::analyze(const std::string &index, const std::string &analyzer,
		const std::string &str, Json::Value &res)
{
	const ElasticsearchNode &node = get_fresh_node();
	Json::Value request;
	request["analyzer"] = analyzer;
	request["text"] = str;

	Query query(node.http_addr + "/" + index + ES_ANALYZE);
	return _get_json(query, request, res);
}

namespace elasticsearch {

Index::Index(const std::string &name, ElasticsearchClient *es_client):
	m_name(name), m_es_client(es_client)
{

}

bool Index::exists()
{
	Json::Value res;
	Query query(m_es_client->get_node_addr() + "/" + m_name);
	if (!m_es_client->_get_json(query, res)) {
		return false;
	}

	return !res.isMember("error") && res.isMember(m_name) ||
		!(res.isMember("status") && res["status"].isInt() &&
			res["status"].asInt() == 404);
}

bool Index::create()
{
	if (exists()) {
		return true;
	}

	Json::Value req, res;
	req["settings"] = Json::objectValue;
	if (m_shard_count > 0) {
		req["settings"]["number_of_shards"] = m_shard_count;
	}

	if (!m_analyzers.empty()) {
		req["settings"]["analysis"] = Json::objectValue;
		req["settings"]["analysis"]["analyzer"] = Json::objectValue;
		for (const auto &analyzer: m_analyzers) {
			req["settings"]["analysis"]["analyzer"][analyzer.first] =
				analyzer.second->to_json();
		}
	}

	Query query(m_es_client->get_node_addr() + "/" + m_name, PUT);
	if (!m_es_client->_put_json(query, req, res)) {
		return false;
	}

	if (res.isMember("error")) {
		if (res["error"].isObject() && res["error"].isMember("reason")) {
			std::cerr << "Elasticsearch index removal error: "
				<< res["error"]["reason"] << std::endl;
		}
		return false;
	}

	return res.isMember("acknowledged") && res["acknowledged"].isBool()
		&& res["acknowledged"].asBool();
}

bool Index::remove()
{
	if (!exists()) {
		return true;
	}

	Json::Value res;
	if (!m_es_client->_delete(Query(m_es_client->get_node_addr() + "/" + m_name, DELETE),
			res)) {
		return false;
	}

	if (res.isMember("error")) {
		if (res["error"].isObject() && res["error"].isMember("reason")) {
			std::cerr << "Elasticsearch index removal error: "
				<< res["error"]["reason"] << std::endl;
		}
		return false;
	}

	return res.isMember("acknowledged") && res["acknowledged"].asBool();

}

bool Index::set_shard_count(uint16_t count)
{
	if (exists()) {
		std::cerr << "Unable to set shard count on an existing index" << std::endl;
		return false;
	}

	m_shard_count = count;
	return true;
}

Json::Value Analyzer::to_json() const
{
	Json::Value result;
	switch (m_type) {
		case CUSTOM: result["type"] = "custom"; break;
		default: assert(false);
	}

	result["tokenizer"] = m_tokenizer;
	result["filter"] = Json::arrayValue;
	for (const auto &filter: m_filters) {
		result["filter"].append(filter);
	}

	return result;
}
}
}
}
