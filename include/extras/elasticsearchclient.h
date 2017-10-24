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

#include <core/httpclient.h>
#include <core/utils/classhelpers.h>
#include <core/utils/exception.h>
#include <chrono>
#include <memory>
#include <queue>
#include <utility>

namespace winterwind
{
namespace extras
{
class ElasticsearchException : public BaseException
{
public:
	explicit ElasticsearchException(const std::string &what) noexcept: BaseException(what)
	{}

	~ElasticsearchException() noexcept override = default;
};

struct ElasticsearchNode
{
public:
	explicit ElasticsearchNode(std::string id) : tech_id(std::move(id))
	{}

	std::string http_addr = "";
	std::string tech_id;
	std::string version = "";
	bool is_master = false;

	std::string to_string() const
	{
		return "id: " + tech_id + " http_addr: " + http_addr + " version: " + version +
			" master: " +
			std::to_string(is_master);
	};
};

enum ElasticsearchBulkActionType
{
	ESBULK_AT_CREATE,
	ESBULK_AT_DELETE,
	ESBULK_AT_INDEX,
	ESBULK_AT_UPDATE,
	ESBULK_AT_MAX,
};

struct ElasticsearchBulkAction
{
public:
	explicit ElasticsearchBulkAction(const ElasticsearchBulkActionType a) : action(a)
	{}

	std::string index = "";
	std::string type = "";
	std::string doc_id = "";
	Json::Value doc;

	void toJson(Json::FastWriter &writer, std::string &res);

private:
	ElasticsearchBulkActionType action;
};

typedef std::shared_ptr<ElasticsearchBulkAction> ElasticsearchBulkActionPtr;

class ElasticsearchClient : public http::HTTPClient
{
public:
	explicit ElasticsearchClient(const std::string &url);

	~ElasticsearchClient();

	void discover_cluster();

	void create_doc(const std::string &index, const std::string &type,
					const Json::Value &doc);

	void insert_doc(const std::string &index, const std::string &type,
		const std::string &doc_id,
		const Json::Value &doc);

	void delete_doc(const std::string &index, const std::string &type,
		const std::string &doc_id);

	bool analyze(const std::string &index, const std::string &analyzer,
		const std::string &str, Json::Value &res);

	void add_bulkaction_to_queue(const ElasticsearchBulkActionPtr &action)
	{
		m_bulk_queue.push(action);
	}

	void process_bulkaction_queue(std::string &res, uint32_t actions_limit = 0);

	const std::string &get_node_addr() const
	{
		static const std::string empty_addr;
		if (m_nodes.empty()) {
			return empty_addr;
		}
		return m_nodes[0].http_addr;
	}
private:
	// This function permits to obtain a fresh node on which perform a query
	const ElasticsearchNode &get_fresh_node();

	std::string m_init_url = "";
	std::chrono::time_point<std::chrono::system_clock> m_last_discovery_time;

	std::vector<ElasticsearchNode> m_nodes; // @TODO use unordered_map with node id as index
	std::queue<ElasticsearchBulkActionPtr> m_bulk_queue;

	CL_HELPER_VAR_GET(std::string, cluster_name, "");
};

namespace elasticsearch
{

/**
 * Analyzer object used on indices
 */
class Analyzer
{
public:
	/**
	 * Create analyzer object
	 * @param name
	 * @param tokenizer
	 * @param filters a list for filters, by names
	 */
	Analyzer(std::string name, std::string tokenizer,
		std::vector<std::string> filters):
		m_name(std::move(name)), m_tokenizer(std::move(tokenizer)), m_filters(
		std::move(filters))
	{
	}

	/**
	 * Analyzer type
	 */
	enum Type: uint8_t {
		CUSTOM,
	};

	/**
	 * @return analyzer name
	 */
	const std::string &get_name() const { return m_name; }

	/**
	 * Converts Analyzer object to json
	 * @return jsoncpp representation of the object
	 */
	Json::Value to_json() const;
private:
	/**
	 * Analyzer name
	 */
	std::string m_name;
	/**
	 * Name of the tokenizer to use
	 */
	std::string m_tokenizer;
	/**
	 * Filter list (names)
	 */
	std::vector<std::string> m_filters;
	/**
	 * Analyzer type
	 */
	Type m_type = CUSTOM;
};

typedef std::shared_ptr<Analyzer> AnalyzerPtr;

class Index
{
public:
	Index(const std::string &name, ElasticsearchClient *es_client);
	~Index() = default;

	/**
	 * Requests Elasticsearch if this index exists
	 * @return
	 */
	bool exists();

	/**
	 * Create index if not exists
	 * @return true if index exists or was created
	 */
	bool create();

	/**
	 * Remove index from Elasticsearch
	 * @return true if index doesn't exist or was removed
	 */
	bool remove();

	/**
	 * Sets the shard count before creation.
	 * It cannot be modified after index creation.
	 * @param count
	 * @return true if shard_count has been set
	 */
	bool set_shard_count(uint16_t count);

	/**
	 * Add an analyzer to this index
	 * @param analyzer
	 */
	inline void add_analyzer(const AnalyzerPtr &analyzer)
	{
		m_analyzers[analyzer->get_name()] = analyzer;
	}

	/**
	 * Get the Elasticsearch index name
	 * @return index name
	 */
	const std::string &get_name() const { return m_name; }

private:
	/**
	 * Index name
	 */
	const std::string m_name = "";

	/**
	 * Index shard count
	 * 0 = don't set it when set index settings
	 */
	uint16_t m_shard_count = 0;

	/**
	 * Analyzers for this index
	 */
	std::unordered_map<std::string, AnalyzerPtr> m_analyzers;

	/**
	 * Pointer to ElasticsearchClient used to do requests
	 */
	ElasticsearchClient *m_es_client;
};

}
}
}
