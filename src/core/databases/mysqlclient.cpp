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

#include "databases/mysqlclient.h"
#include <cstring>
#include <iostream>

namespace winterwind
{
namespace db
{
#define MYSQL_ROWLOOP(_res, _row)                                         \
    int num_fields = mysql_num_fields(_res);                              \
    MYSQL_ROW _row;                                                       \
    while (((_row) = mysql_fetch_row(_res)))

#define MYSQLROW_TO_STRING(_row, i) _row[i] ? std::string((_row)[i], strlen((_row)[i])) : "NULL"

MySQLResult::MySQLResult(MYSQL *conn)
{
	m_result = mysql_use_result(conn);
	if (!m_result && mysql_errno(conn) != 0) {
		throw MySQLException("MySQLResult failed (errno: "
			+ std::to_string(mysql_errno(conn))
			+ "): '" + std::string(mysql_error(conn)) + "'");
	}
}

MySQLResult::~MySQLResult()
{
	if (m_result) {
		mysql_free_result(m_result);
	}
}

MySQLResult::MySQLResult(MySQLResult &&other) noexcept:
	m_result(other.m_result)
{
}

/*
 * MySQL Client
 */

MySQLClient::MySQLClient(const std::string &host, const std::string &user,
	const std::string &password, uint16_t port, const std::string &db)
	: m_host(host), m_user(user), m_password(password), m_port(port), m_db(db)
{
	try {
		connect();
	}
	catch (MySQLException &e) {
		disconnect();
		throw e;
	}
}

MySQLClient::~MySQLClient()
{
	disconnect();
}

void MySQLClient::connect()
{
	if (m_conn) {
		disconnect();
	}

	m_conn = mysql_init(NULL);
	if (!m_conn) {
		throw MySQLException("unable to init client " +
			std::string(mysql_error(m_conn)));
	}

	if (mysql_real_connect(m_conn, m_host.c_str(), m_user.c_str(), m_password.c_str(),
		(!m_db.empty() ? m_db.c_str() : NULL), m_port, NULL, 0) == NULL) {
		throw MySQLException("connection failed " + std::string(mysql_error(m_conn)));
	}
}

void MySQLClient::disconnect()
{
	if (m_conn) {
		mysql_close(m_conn);
		m_conn = nullptr;
	}
}

MySQLResult MySQLClient::exec(const std::string &query)
{
	if (m_check_before_exec) {
		check_connection();
	}

	if (mysql_real_query(m_conn, query.c_str(), query.size()) != 0) {
		throw MySQLException("query failed " + std::string(mysql_error(m_conn)));
	}

	 return MySQLResult(m_conn);
}

void MySQLClient::begin()
{
	exec("BEGIN");
}

void MySQLClient::commit()
{
	exec("COMMIT");
}

void MySQLClient::rollback()
{
	exec("ROLLBACK");
}

void MySQLClient::check_connection()
{
	if (!mysql_ping(m_conn)) {
		disconnect();
		connect();
	}
}

void MySQLClient::list_tables(std::vector<std::string> &result)
{
	exec("SHOW TABLES");

	MySQLResult mysql_res(m_conn);
	MYSQL_ROWLOOP(*mysql_res, row) {
		for (int i = 0; i < num_fields; i++) {
			result.emplace_back(row[i] ? row[i] : "NULL");
		}
	}
}

bool MySQLClient::get_table_definition(const std::string &table, std::string &res)
{
	if (table.empty()) {
		return false;
	}

	exec("SHOW CREATE TABLE " + table);

	MySQLResult mysql_res(m_conn);
	MYSQL_ROWLOOP(*mysql_res, row) {
		for (int i = 0; i < num_fields; i++) {
			res = (row[i] ? row[i] : "NULL");
		}
	}

	return true;
}

bool MySQLClient::explain(const std::string &q, std::vector<MySQLExplainEntry> &res)
{
	if (q.empty()) {
		return false;
	}

	exec("EXPLAIN " + q);

	MySQLResult mysql_res(m_conn);
	MYSQL_FIELD *field;
	MYSQL_ROWLOOP(*mysql_res, row) {
		MySQLExplainEntry entry;
		for (int i = 0; i < num_fields; i++) {
			field = mysql_fetch_field(*mysql_res);
			if (strcmp(field->name, "id") == 0) {
				entry.id = (uint16_t) (row[i] ? atoi(row[i]) : 0);
			} else if (strcmp(field->name, "select_type") == 0) {
				entry.select_type = MYSQLROW_TO_STRING(row, i);
			} else if (strcmp(field->name, "table") == 0) {
				entry.table = MYSQLROW_TO_STRING(row, i);
			} else if (strcmp(field->name, "type") == 0) {
				entry.type = MYSQLROW_TO_STRING(row, i);
			} else if (strcmp(field->name, "possible_keys") == 0) {
				entry.possible_keys = MYSQLROW_TO_STRING(row, i);
			} else if (strcmp(field->name, "key") == 0) {
				entry.key = MYSQLROW_TO_STRING(row, i);
			} else if (strcmp(field->name, "key_len") == 0) {
				entry.key_len = (uint32) (row[i] ? atoi(row[i]) : 0);
			} else if (strcmp(field->name, "ref") == 0) {
				entry.ref = MYSQLROW_TO_STRING(row, i);
			} else if (strcmp(field->name, "rows") == 0) {
				entry.rows = (uint64) (row[i] ? atoi(row[i]) : 0);
			} else if (strcmp(field->name, "Extra") == 0) {
				entry.extra = MYSQLROW_TO_STRING(row, i);
			}
		}
		res.push_back(entry);
	}
	return true;
}
}
}
