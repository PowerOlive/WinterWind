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

#include "postgresqlclient.h"
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>

namespace winterwind
{
namespace db
{
PostgreSQLResult::PostgreSQLResult(PostgreSQLClient *client, PGresult *result) :
	m_result(result)
{
	m_status = PQresultStatus(result);
	if (m_status != PGRES_COMMAND_OK) {
		client->m_last_error = std::string(PQresultErrorMessage(result));
	}
}

PostgreSQLClient::PostgreSQLClient(const std::string &connect_string,
	int32_t minimum_db_version)
	: m_connect_string(connect_string), m_min_pgversion(minimum_db_version)
{
	connect();
}

PostgreSQLClient::~PostgreSQLClient()
{
	if (m_conn) {
		PQfinish(m_conn);
	}
}

void PostgreSQLClient::connect()
{
	if (m_conn) {
		PQfinish(m_conn);
	}

	m_conn = PQconnectdb(m_connect_string.c_str());

	if (PQstatus(m_conn) != CONNECTION_OK) {
		throw PostgreSQLException(std::string("PostgreSQL database error: ") +
			PQerrorMessage(m_conn));
	}

	m_pgversion = PQserverVersion(m_conn);
	if (m_pgversion < m_min_pgversion) {
		std::stringstream ss;
		ss << "PostgreSQL database error: Server version " << m_pgversion
			<< " < required version " << m_min_pgversion;
		throw PostgreSQLException(ss.str());
	}

	check_db_connection();
}

void PostgreSQLClient::check_db_connection()
{
	if (PQstatus(m_conn) == CONNECTION_OK)
		return;

	if (PQping(m_connect_string.c_str()) != PQPING_OK) {
		throw PostgreSQLException(std::string("PostgreSQL database error: ") +
			PQerrorMessage(m_conn));
	}

	PQresetStart(m_conn);
}

pg_result *PostgreSQLClient::exec(const char *query)
{
	if (m_check_before_exec) {
		check_db_connection();
	}

	return PQexec(m_conn, query);
}

void PostgreSQLClient::begin()
{
	check_results(exec("BEGIN;"));
}

void PostgreSQLClient::commit()
{
	check_results(exec("COMMIT;"));
}

void PostgreSQLClient::set_client_encoding(const std::string &encoding)
{
	static const std::array<std::string, 2> allowed_encoding = {"LATIN1", "UTF8"};
	bool valid = false;
	for (const auto &allowed_value : allowed_encoding) {
		if (allowed_value.compare(encoding) == 0) {
			valid = true;
			break;
		}
	}
	if (!valid) {
		throw PostgreSQLException(
			std::string("PostgreSQLClient: Invalid client_encoding provided"));
	}

	std::string request = "SET client_encoding='";
	request += encoding;
	request += "';";
	check_results(exec(request.c_str()));
}

PGresult *PostgreSQLClient::check_results(PGresult *result, bool clear)
{
	ExecStatusType statusType = PQresultStatus(result);

	switch (statusType) {
		case PGRES_COMMAND_OK:
		case PGRES_TUPLES_OK:
			break;
		case PGRES_FATAL_ERROR:
		default:
			throw PostgreSQLException(std::string("PostgreSQL database error: ") +
				PQresultErrorMessage(result));
	}

	if (clear) {
		PQclear(result);
	}

	return result;
}

void PostgreSQLClient::escape_string(const std::string &param, std::string &res)
{
	char *to = new char[(uint32_t) std::ceil(param.size() * 1.5f)];
	size_t len = PQescapeStringConn(m_conn, to, param.c_str(), param.size(), NULL);
	res = std::string(to, len);
	delete[] to;
}

bool PostgreSQLClient::register_statement(const std::string &stn, const std::string &st)
{
	if (m_statements.find(stn) != m_statements.end()) {
		std::cerr << "WARN: Trying to register statement " << stn
			<< ", but it's already registered, ignoring." << std::endl;
		return false;
	}

	check_results(PQprepare(m_conn, stn.c_str(), st.c_str(), 0, NULL));

	m_statements[stn] = st;
	return true;
}

void PostgreSQLClient::register_embedded_statements()
{
	register_statement("list_tables_into_schema", "SELECT tablename FROM pg_tables "
		"WHERE schemaname=$1");
	register_statement(
		"show_create_table",
		"SELECT column_name, data_type, is_nullable, "
			"column_default FROM information_schema.columns WHERE table_schema = $1 AND "
			"table_name = $2 ORDER BY ordinal_position");
	register_statement("show_create_table_indexes",
		"SELECT indexname, indexdef "
			"FROM pg_indexes WHERE schemaname=$1 AND tablename=$2");
}

#define PGR(q) PostgreSQLResult result(this, q);

ExecStatusType PostgreSQLClient::show_schemas(std::vector<std::string> &res)
{
	PGR(exec("select nspname from pg_catalog.pg_namespace"));

	int32_t nbres = PQntuples(*result);
	for (int32_t i = 0; i < nbres; i++) {
		res.push_back(PQgetvalue(*result, i, 0));
	}

	return result.get_status();
}

ExecStatusType PostgreSQLClient::create_schema(const std::string &name)
{
	std::string name_esc = "";
	escape_string(name, name_esc);
	std::string query = "CREATE SCHEMA " + name_esc;
	PGR(exec(query.c_str()));

	return result.get_status();
}

ExecStatusType PostgreSQLClient::drop_schema(const std::string &name, bool if_exists)
{
	std::string name_esc = "";
	escape_string(name, name_esc);
	std::string query = "DROP SCHEMA ";
	if (if_exists)
		query += "IF EXISTS ";
	query += name_esc;
	PGR(exec(query.c_str()));

	return result.get_status();
}

ExecStatusType PostgreSQLClient::show_tables(const std::string &schema,
	std::vector<std::string> &res)
{
	const char *values[]{schema.c_str()};
	PGR(exec_prepared("list_tables_into_schema", 1, values, NULL, NULL, false, false));
	int32_t nbres = PQntuples(*result);
	for (int32_t i = 0; i < nbres; i++) {
		res.push_back(PQgetvalue(*result, i, 0));
	}

	return result.get_status();
}

ExecStatusType PostgreSQLClient::show_create_table(const std::string &schema,
	const std::string &table,
	PostgreSQLTableDefinition &definition)
{
	const char *values[]{schema.c_str(), table.c_str()};

	{
		PGR(exec_prepared("show_create_table", 2, values, NULL, NULL, false, false));
		int32_t nbres = PQntuples(*result);
		for (int32_t i = 0; i < nbres; i++) {
			PostgreSQLTableField field = {};
			field.column_name = PQgetvalue(*result, i, 0);
			field.data_type = PQgetvalue(*result, i, 1);
			field.is_nullable = strcmp(PQgetvalue(*result, i, 2), "YES") == 0;
			field.column_default = PQgetvalue(*result, i, 3);
			definition.fields.push_back(field);
		}

		if (result.get_status() != PGRES_COMMAND_OK) {
			return result.get_status();
		}
	}

	{
		PGR(exec_prepared("show_create_table_indexes", 2, values, NULL, NULL, false,
			false));
		int32_t nbres = PQntuples(*result);
		for (int32_t i = 0; i < nbres; i++) {
			definition.indexes[pg_to_string(*result, i, 0)] =
				pg_to_string(*result, i, 1);
		}

		if (result.get_status() != PGRES_COMMAND_OK) {
			return result.get_status();
		}
	}

	return PGRES_COMMAND_OK;
}

ExecStatusType PostgreSQLClient::add_admin_views(const std::string &schema)
{
	// @TODO
	// create_schema(schema)
	// use schema
	{
		PGR(exec(
			"CREATE OR REPLACE VIEW view_relations_size AS\n"
				"SELECT\n"
				"    c.relname AS name,\n"
				"    c.reltuples::bigint AS tuples,\n"
				"    pg_relation_size(c.oid) AS table_size,\n"
				"    pg_total_relation_size(c.oid)-pg_relation_size(c.oid) - (CASE WHEN "
				"c.reltoastrelid <> 0 THEN \n"
				"    pg_relation_size(c.reltoastrelid) ELSE 0 END) AS index_size,\n"
				"    CASE WHEN c.reltoastrelid <> 0 THEN "
				"pg_relation_size(c.reltoastrelid) ELSE 0 END AS toast_size,\n"
				"    pg_total_relation_size(c.oid) AS total_size\n"
				"FROM\n"
				"    pg_catalog.pg_class c\n"
				"JOIN\n"
				"    pg_catalog.pg_roles r ON r.oid = c.relowner\n"
				"LEFT JOIN\n"
				"    pg_catalog.pg_namespace n ON n.oid = c.relnamespace\n"
				"WHERE\n"
				"    c.relkind = 'r'\n"
				"AND n.nspname NOT IN ('pg_catalog', 'pg_toast')\n"
				"AND pg_catalog.pg_table_is_visible(c.oid)\n"
				"ORDER BY total_size DESC"));

		if (result.get_status() != PGRES_COMMAND_OK) {
			return result.get_status();
		}
	}

	{
		PGR(exec("CREATE OR REPLACE VIEW view_relations_size_pretty AS\n"
			"SELECT\n"
			"    name,\n"
			"    tuples,\n"
			"    pg_size_pretty(table_size) AS table_size,\n"
			"    pg_size_pretty(index_size) AS index_size,\n"
			"    pg_size_pretty(toast_size) AS toast_size,\n"
			"    pg_size_pretty(total_size) AS total_size\n"
			"FROM view_relations_size"));

		if (result.get_status() != PGRES_COMMAND_OK) {
			return result.get_status();
		}
	}

	{
		PGR(exec(
			"CREATE OR REPLACE VIEW object_privileges AS\n"
				"SELECT  objtype,\n"
				"        schemaname,\n"
				"        objname,\n"
				"        owner,\n"
				"        objuser,\n"
				"        privs,\n"
				"        string_agg(\n"
				"            (case   privs_individual\n"
				"                    when 'arwdDxt' then 'All'\n"
				"                    when '*' then 'Grant'\n"
				"                    when 'r' then 'SELECT'\n"
				"                    when 'w' then 'UPDATE'\n"
				"                    when 'a' then 'INSERT'\n"
				"                    when 'd' then 'DELETE'\n"
				"                    when 'D' then 'TRUNCATE'\n"
				"                    when 'x' then 'REFERENCES'\n"
				"                    when 't' then 'TRIGGER'\n"
				"                    when 'X' then 'EXECUTE'\n"
				"                    when 'U' then 'USAGE'\n"
				"                    when 'C' then 'CREATE'\n"
				"                    when 'c' then 'CONNECT'\n"
				"                    when 'T' then 'TEMPORARY'\n"
				"            else 'Unknown: '||privs end\n"
				"            ), ', ' ORDER BY privs_individual) as privileges_pretty\n"
				"FROM    (SELECT objtype,\n"
				"                schemaname,\n"
				"                objname,\n"
				"                owner,\n"
				"                privileges,\n"
				"                (case when coalesce(objuser,'') is not distinct from\n"
				"'' then 'public' else objuser end)\n"
				"                    || (case when pr2.rolsuper then '*' else '' end)\n"
				"                as objuser,\n"
				"                privs,\n"
				"                (case   when privs in ('*','arwdDxt') then privs\n"
				"                        else regexp_split_to_table(privs,E'\\\\s*')\n"
				"                end) as privs_individual\n"
				"        from    (select distinct\n"
				"                        objtype,\n"
				"                        schemaname,\n"
				"                        objname,\n"
				"                        coalesce(owner,'') || (case when pr.rolsuper\n"
				"then '*' else '' end) as owner,\n"
				"                        regexp_replace(privileges,E'\\/.*','') as "
				"privileges,\n"
				"\n"
				"(regexp_split_to_array(regexp_replace(privileges,E'\\/.*',''),'='))[1]\n"
				"as objuser,\n"
				"\n"
				"(regexp_split_to_array(regexp_replace(privileges,E'\\/.*',''),'='))[2]\n"
				"as privs\n"
				"                from    (SELECT n.nspname as schemaname,\n"
				"                                c.relname as objname,\n"
				"                                CASE c.relkind WHEN 'r' THEN 'table'\n"
				"WHEN 'v' THEN 'view' WHEN 'S' THEN 'sequence' END as objtype,\n"
				"\n"
				"regexp_split_to_table(array_to_string(c.relacl,','),',') as\n"
				"privileges,\n"
				"                                pg_catalog.pg_get_userbyid(c.relowner) as "
				"Owner\n"
				"                        FROM pg_catalog.pg_class c\n"
				"                        LEFT JOIN pg_catalog.pg_namespace n ON n.oid =\n"
				"c.relnamespace\n"
				"                        WHERE c.relkind IN ('r', 'v', 'S', 'f')\n"
				"                        AND n.nspname !~ '(pg_catalog|information_schema)'\n"
				"                        ) as y                                      \n"
				"                left join pg_roles pr on (pr.rolname = y.owner)\n"
				"                ) as p2\n"
				"        left join pg_roles pr2 on (pr2.rolname = p2.objuser)\n"
				"        ) as p3\n"
				"group by objtype, schemaname,objname, owner, objuser, privs\n"
				"order by objtype,schemaname,objname,objuser,privileges_pretty"));

		if (result.get_status() != PGRES_COMMAND_OK) {
			return result.get_status();
		}
	}

	return PGRES_COMMAND_OK;
}

}
}
