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

#include "elasticsearchclient.h"

#define ES_URL_CLUSTER_STATE "/_cluster/state"
#define ES_URL_NODES "/_nodes"

ElasticsearchClient::ElasticsearchClient(const std::string &url):
	HTTPClient(100 * 1024), m_init_url(url)
{
	discover_cluster();
}

void ElasticsearchClient::discover_cluster()
{
	Json::Value res;
	if (!fetch_json(m_init_url + ES_URL_CLUSTER_STATE, res) ||
		!res.isMember("cluster_name") || !res["cluster_name"].isString()) {
		throw ElasticsearchException("Unable to parse Elasticsearch cluster state");
	}

	m_cluster_name = res["cluster_name"].asString();

	if (!fetch_json(m_init_url + ES_URL_NODES, res) ||
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
	perform_request(node.http_addr + "/" + index + "/" + type + "/", res,
		HTTPCLIENT_REQ_SIMPLE, HTTP_METHOD_GET, writer.write(doc));
}

void ElasticsearchClient::insert_doc(const std::string &index, const std::string &type,
	const std::string &doc_id, const Json::Value &doc)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string res;
	Json::FastWriter writer;
	perform_put(node.http_addr + "/" + index + "/" + type + "/" + doc_id, res,
		HTTPCLIENT_REQ_SIMPLE, writer.write(doc));
}

void ElasticsearchClient::delete_doc(const std::string &index,
	const std::string &type, const std::string &doc_id)
{
	const ElasticsearchNode &node = get_fresh_node();
	std::string res;
	perform_delete(node.http_addr + "/" + index + "/" + type + "/" + doc_id, res);
}
