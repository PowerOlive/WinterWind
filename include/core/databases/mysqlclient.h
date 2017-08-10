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

#include "core/utils/exception.h"
#include "database.h"
#include <my_global.h>
#include <mysql.h>
#include <string>
#include <vector>

namespace winterwind
{
namespace db
{
class MySQLException : public DatabaseException
{
public:
	explicit MySQLException(const std::string &what) :
		DatabaseException("MySQL Exception: " + what)
	{}

	MySQLException() = delete;

	virtual ~MySQLException() throw() = default;
};

class MySQLResult
{
public:
	explicit MySQLResult(MYSQL *conn);
	MySQLResult() = delete;

	~MySQLResult();

	MYSQL_RES *operator*() { return m_result; }

	MySQLResult(MySQLResult &&other) noexcept;
	MySQLResult(MySQLResult &other) = delete;
	MySQLResult operator=(MySQLResult &other) = delete;
private:
	MYSQL_RES *m_result = nullptr;
};

struct MySQLExplainEntry
{
	uint16_t id = 0;
	std::string select_type = "";
	std::string table = "";
	std::string type = "";
	std::string possible_keys = "";
	std::string key = "";
	uint32 key_len = 0;
	std::string ref = "";
	uint64 rows = 0;
	std::string extra = "";
};

class MySQLClient: private DatabaseInterface
{
public:
	/**
	 * Connect to a MySQL server
	 *
	 * @throws MySQLException on connection failure
	 * @param host
	 * @param user
	 * @param password
	 * @param port
	 * @param db
	 */
	MySQLClient(const std::string &host, const std::string &user,
		const std::string &password, uint16_t port = 3306,
		const std::string &db = "");

	MySQLClient() = delete;

	virtual ~MySQLClient();

	/**
	 * Start MySQL transaction
	 */
	void begin();

	/**
	 * Commit started MySQL transaction
	 */
	void commit();

	/**
	 * Rollback current transaction
	 */
	void rollback();

	/**
	 * Verify is MySQL connection is working.
	 * If connection is inactive and database is up, reconnects.
	 *
	 * @throws MySQLException
	 */
	void check_connection();

	/**
	 * Exec raw SQL query
	 *
	 * @throws MySQLException if query failed
	 * @param query
	 * @return MySQLResult
	 *
	 */
	MySQLResult exec(const std::string &query);

	void list_tables(std::vector<std::string> &result);

	bool get_table_definition(const std::string &table, std::string &res);

	bool explain(const std::string &q, std::vector<MySQLExplainEntry> &res);

protected:
	/**
	 * Try to connect to MySQL database. If connection failed a MySQLException is thrown.
	 */
	void connect();

	/**
	 * Disconnect from database if connected
	 */
	void disconnect();
private:
	MYSQL *m_conn = nullptr;
	std::string m_host = "localhost";
	std::string m_user = "";
	std::string m_password = "";
	std::string m_db = "";
	uint16_t m_port = 3306;
};
}
}
