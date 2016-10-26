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

#pragma once

#include <string>
#include <libpq-fe.h>
#include <unordered_map>
#include <stdlib.h>
#include "utils/exception.h"

class PostgreSQLException: public BaseException
{
public:
	PostgreSQLException(const std::string &what): BaseException(what) {}
	~PostgreSQLException() throw() {}
};

class PostgreSQLClient
{
public:
	PostgreSQLClient(const std::string &connect_string,
		int32_t minimum_db_version = 90500);
	virtual ~PostgreSQLClient();

	void begin();
	void commit();
	bool register_statement(const std::string &stn, const std::string &st);
	PGresult *exec_prepared(const char *stmtName, const int paramsNumber,
			const char **params, const int *paramsLengths = NULL,
			const int *paramsFormats = NULL,
			bool clear = true, bool nobinary = true)
	{
		return check_results(PQexecPrepared(m_conn, stmtName, paramsNumber,
				(const char* const*) params, paramsLengths, paramsFormats,
				nobinary ? 1 : 0), clear);
	}

	inline int pg_to_int(PGresult *res, int row, int col)
	{
		return atoi(PQgetvalue(res, row, col));
	}

	inline const uint32_t pg_to_uint(PGresult *res, int row, int col) {
		return (const uint32_t) atoi(PQgetvalue(res, row, col));
	}

	inline const uint64_t pg_to_uint64(PGresult *res, int row, int col) {
		return (const uint64_t) atoll(PQgetvalue(res, row, col));
	}

	inline const int64_t pg_to_int64(PGresult *res, int row, int col) {
		unsigned char* data = (unsigned char*)PQgetvalue(res, row, col);
		return (const int64_t) atoll((char*) data);
	}

protected:
	void check_db_connection();
	void set_client_encoding(const std::string &encoding);
	PGresult *check_results(PGresult *result, bool clear = true);

private:
	void connect();

	std::string m_connect_string = "";
	PGconn *m_conn = nullptr;
	int32_t m_pgversion = 0;
	int32_t m_min_pgversion = 0;

	std::unordered_map<std::string, std::string> m_statements;
};
