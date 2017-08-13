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

#include "database.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <json/json.h>

extern "C" {
#include <libpq-fe.h>
}

namespace winterwind
{
namespace db
{
class PostgreSQLException : public DatabaseException
{
public:
	PostgreSQLException(const std::string &what) : DatabaseException(what) {}
	PostgreSQLException() = delete;
	~PostgreSQLException() throw() {}
};

/**
 * RAII class for PostgreSQL results
 */
class PostgreSQLResult
{

public:
	/**
	 * Standard constructor from PGresult pointer
	 * @param result
	 */
	PostgreSQLResult(PGresult *result);

	/**
	 * This object can only be moved
	 * @param other
	 */
	PostgreSQLResult(PostgreSQLResult &&other) noexcept;

	/**
	 * Default constructor not allowed
	 */
	PostgreSQLResult() = delete;

	/**
	 * Copying this object is not allowed
	 * @param other
	 */
	PostgreSQLResult(const PostgreSQLResult &other) = delete;
	PostgreSQLResult operator=(const PostgreSQLResult &other) = delete;

	~PostgreSQLResult();

	/**
	 * Pointer to PGresult
	 * @return m_result
	 */
	PGresult *operator*() { return m_result; }
	ExecStatusType get_status() const { return m_status; }

	void toJson(Json::Value &res);
private:
	PGresult *m_result = nullptr;
	ExecStatusType m_status = PGRES_COMMAND_OK;
};

/**
 * Contains table fields and indices
 */
struct PostgreSQLTableDefinition
{
	std::vector<DatabaseTableField> fields;
	std::unordered_map<std::string, std::string> indexes;
};

/**
 * PostgreSQL client
 */
class PostgreSQLClient: private DatabaseInterface
{
	friend class PostgreSQLResult;
public:
	/**
	 * Construct PostgreSQL client and connect
	 *
	 * @throws a PostgreSQLException if connection failed
	 *
	 * @param connect_string
	 * @param minimum_db_version
	 */
	PostgreSQLClient(const std::string &connect_string,
		int32_t minimum_db_version = 90500);

	PostgreSQLClient() = delete;

	virtual ~PostgreSQLClient();

	/**
	 * Start a transaction
	 */
	void begin();

	/**
	 * Commit a transaction
	 */
	void commit();

	/**
	 * Rollback currently running transaction
	 */
	void rollback();

	/**
	 * Execute a raw query and return a PostgreSQLResult
	 *
	 * @param query SQL string
	 * @return PostgreSQLResult object
	 */
	PostgreSQLResult exec(const char *query);

	/**
	 * Register a statement with a name
	 *
	 * @param stn statement name
	 * @param st  statement value
	 * @return registering success (false means statement already exists)
	 */
	bool register_statement(const std::string &stn, const std::string &st);

	/**
	 * Register some winterwind statements for managing databases
	 */
	void register_embedded_statements();

	/**
	 * List all schemas in current database
	 *
	 * @param res result vector containing list of schemas
	 * @return request status
	 */
	ExecStatusType show_schemas(std::vector<std::string> &res);

	/**
	 * Create a new schema in current database
	 * @param name schema name
	 * @return request status
	 */
	ExecStatusType create_schema(const std::string &name);

	/**
	 * Drop a schema from current database
	 * @param name schema name
	 * @param if_exists if exists flags (permitting to avoid error if schema doesn't exists.
	 * @return request status
	 */
	ExecStatusType drop_schema(const std::string &name, bool if_exists = false);

	/**
	 * List all tables present in schema
	 * @param schema schema affected by query
	 * @param res result vector containing table list
	 * @return request status
	 */
	ExecStatusType show_tables(const std::string &schema, std::vector<std::string> &res);

	/**
	 * Populate a PostgreSQLTableDefinition object with table definition for schema
	 * @param schema selected schema
	 * @param table selected table
	 * @param definition result object
	 * @return request status
	 */
	ExecStatusType show_create_table(const std::string &schema, const std::string &table,
		PostgreSQLTableDefinition &definition);

	ExecStatusType add_admin_views(const std::string &schema = "admin");

	void escape_string(const std::string &param, std::string &res);

protected:
	/**
	 * Verify is PostgreSQL connection is working.
	 * If connection is inactive and database is up, reconnects.
	 *
	 * @throws PostgreSQLException
	 */
	void check_connection();

	void set_client_encoding(const std::string &encoding);

	/**
	 * Read PostgreSQL field for row and col with type T
	 * @tparam T destination type
	 * @param res
	 * @param row
	 * @param colconst
	 * @return PostgreSQL result for row and col converted to T type
	 */
	template<typename T> T read_field(PostgreSQLResult &res, int row, int col);

	/**
	 * Exec a previously prepared query (stmtName)
	 * paramsNumber, params, paramsLenghts, paramsFormats & nobinary refers to:
	 * https://www.postgresql.org/docs/9.6/static/libpq-exec.html (see PQExecPrepared)
	 *
	 * @param stmtName
	 * @param paramsNumber
	 * @param params
	 * @param paramsLengths
	 * @param paramsFormats
	 * @param nobinary
	 * @return PostgreSQLResult object
	 */
	PostgreSQLResult exec_prepared(const char *stmtName, const int paramsNumber,
		const char **params, const int *paramsLengths = NULL,
		const int *paramsFormats = NULL, bool nobinary = true);

	/**
	 * Connects to database using m_connect_string
	 */
	void connect();

	/**
	 * Disconnects from current connected database
	 */
	void disconnect();

private:
	std::string m_connect_string = "";
	PGconn *m_conn = nullptr;
	int32_t m_pgversion = 0;
	int32_t m_min_pgversion = 0;

	std::unordered_map<std::string, std::string> m_statements;
};
}
}
