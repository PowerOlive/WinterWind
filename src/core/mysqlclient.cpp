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

#include "mysqlclient.h"
#include <cstring>
#include <iostream>
#include <stdlib.h>

namespace winterwind
{
namespace db
{
#define MYSQL_STORE_RES(_res)                                                                      \
    MYSQL_RES *_res = mysql_store_result(m_conn);                                              \
    if (_res == NULL) {                                                                        \
        throw MySQLException("MySQL Exception: mysql_store_result failed " +               \
                     std::string(mysql_error(m_conn)));                            \
    }

#define MYSQL_ROWLOOP(_res, _row)                                                                  \
    int num_fields = mysql_num_fields(_res);                                                   \
    MYSQL_ROW _row;                                                                            \
    while ((_row = mysql_fetch_row(_res)))

#define MYSQLROW_TO_STRING(_row, i) _row[i] ? std::string(_row[i], strlen(_row[i])) : "NULL"

MySQLClient::MySQLClient(const std::string &host, const std::string &user,
	const std::string &password, uint16_t port, const std::string &db)
	: m_host(host), m_user(user), m_password(password), m_port(port), m_db(db)
{
	m_conn = mysql_init(NULL);
	if (!m_conn) {
		throw MySQLException("MySQL Exception: unable to init client " +
			std::string(mysql_error(m_conn)));
	}

	connect();
}

MySQLClient::~MySQLClient()
{ disconnect(); }

void MySQLClient::connect()
{
	if (mysql_real_connect(m_conn, m_host.c_str(), m_user.c_str(), m_password.c_str(),
		(!m_db.empty() ? m_db.c_str() : NULL), m_port, NULL, 0) == NULL) {
		throw MySQLException("MySQL Exception: connection failed " +
			std::string(mysql_error(m_conn)));
	}
}

void MySQLClient::disconnect()
{
	if (m_conn) {
		mysql_close(m_conn);
	}
}

void MySQLClient::query(const std::string &query)
{
	if (mysql_query(m_conn, query.c_str()) != 0) {
		throw MySQLException("MySQL Exception: query failed " +
			std::string(mysql_error(m_conn)));
	}
}

void MySQLClient::list_tables(std::vector<std::string> &result)
{
	query("SHOW TABLES");

	MYSQL_STORE_RES(mysql_res)
	MYSQL_ROWLOOP(mysql_res, row) {
		for (int i = 0; i < num_fields; i++) {
			result.push_back(row[i] ? row[i] : "NULL");
		}
	}

	mysql_free_result(mysql_res);
}

bool MySQLClient::get_table_definition(const std::string &table, std::string &res)
{
	if (table.empty()) {
		return false;
	}

	query("SHOW CREATE TABLE " + table);

	MYSQL_STORE_RES(mysql_res)
	MYSQL_ROWLOOP(mysql_res, row) {
		for (int i = 0; i < num_fields; i++) {
			res = (row[i] ? row[i] : "NULL");
		}
	}

	mysql_free_result(mysql_res);
	return true;
}

bool MySQLClient::explain(const std::string &q, std::vector<MySQLExplainEntry> &res)
{
	if (q.empty()) {
		return false;
	}

	query("EXPLAIN " + q);

	MYSQL_STORE_RES(mysql_res)
	MYSQL_FIELD *field;
	MYSQL_ROWLOOP(mysql_res, row) {
		MySQLExplainEntry entry;
		for (int i = 0; i < num_fields; i++) {
			field = mysql_fetch_field(mysql_res);
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

	mysql_free_result(mysql_res);
	return true;
}
}
}
