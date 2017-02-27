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

#include <cassert>
#include "elasticsearchclient.h"

#define ES_URL_CLUSTER_STATE "/_cluster/state"
#define ES_URL_NODES "/_nodes"

ElasticsearchClient::ElasticsearchClient(const std::string &url):
	HTTPClient(100 * 1024), m_init_url(url)
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
	if (!_get_json(m_init_url + ES_URL_CLUSTER_STATE, res) ||
		!res.isMember("cluster_name") || !res["cluster_name"].isString()) {
		throw ElasticsearchException("Unable to parse Elasticsearch cluster state");
	}

	m_cluster_name = res["cluster_name"].asString();

	if (!_get_json(m_init_url + ES_URL_NODES, res) ||
		!res.isMember("nodes") || !res["nodes"].isObject()) {
		throw ElasticsearchException("Unable to parse Elasticsearch nodes");
	}

	Json::Value::Members cluster_members = res["nodes"].getMemberNames();
	for (const auto &member: cluster_members) {
		ElasticsearchNode node(member);
		Json::Value member_obj = res["nodes"][member];
		if (member_obj.isMember("http_address") && member_obj["http_address"].isString()) {
			node.http_addr = "http://" + member_obj["http_address"].asString();
		}

		if (member_obj.isMember("version") && member_obj["version"].isString()) {
			node.version = member_obj["version"].asString();
		}

		if (member_obj.isMember("attributes") && member_obj["attributes"].isObject()) {
			Json::Value member_attrs = member_obj["attributes"];
			// Master attribute is a string, not a bool
			if (member_attrs.isMember("master") && member_attrs["master"].isString()) {
				node.is_master = member_attrs["master"].asString().compare("true") == 0;
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
	request(node.http_addr + "/" + index + "/" + type + "/", res,
			HTTPClient::REQ_SIMPLE, HTTP_METHOD_GET, writer.write(doc));
}

void ElasticsearchClient::insert_doc(const std::string &index, const std::string &type,
	const std::string &doc_id, const Json::Value &doc)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string res;
	Json::FastWriter writer;
	_put(node.http_addr + "/" + index + "/" + type + "/" + doc_id, res,
		 HTTPClient::REQ_SIMPLE, writer.write(doc));
}

void ElasticsearchClient::delete_doc(const std::string &index,
	const std::string &type, const std::string &doc_id)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string res;
	_delete(node.http_addr + "/" + index + "/" + type + "/" + doc_id, res);
}

// related to ElasticsearchBulkActionType
static const std::string bulkaction_str_mapping[ESBULK_AT_MAX] =
{
	"create",
	"delete",
	"index",
	"update"
};

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
	}
	else if (action == ESBULK_AT_UPDATE) {
		Json::Value update;
		update["doc"] = doc;
		res += writer.write(update);
	}
}

void ElasticsearchClient::process_bulkaction_queue(std::string &res, uint32_t actions_limit)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string post_data = "";

	uint32_t processed_actions = 0;
	Json::FastWriter writer;
	while (!m_bulk_queue.empty() && (actions_limit == 0 ||
		processed_actions < actions_limit)) {
		processed_actions++;
		const ElasticsearchBulkActionPtr &action = m_bulk_queue.front();
		action->toJson(writer, post_data);
		m_bulk_queue.pop();
	}

	_post(node.http_addr + "/_bulk", post_data, res);
}
