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

#include "test_postgresql.h"

#include <core/databases/postgresqlclient.h>

namespace winterwind {

namespace unittests {

#define INIT_PG_CLIENT                                                                   \
    db::PostgreSQLClient pg("host=postgres user=unittests dbname=unittests_db "          \
                "password=un1Ttests");

#define PG_TEST_TABLE std::string("ut_table")

void Test_PostgreSQL::tearDown()
{
	INIT_PG_CLIENT;
	std::string query = "DROP TABLE IF EXISTS " + PG_TEST_TABLE;
	pg.exec(query.c_str());
}

void Test_PostgreSQL::pg_register_embedded_statements()
{
	INIT_PG_CLIENT
	pg.register_embedded_statements();
	CPPUNIT_ASSERT(true);
}

void Test_PostgreSQL::pg_register_custom_statement()
{
	INIT_PG_CLIENT
	CPPUNIT_ASSERT(pg.register_statement("test_stmt", "SELECT * FROM pg_indexes"));
	CPPUNIT_ASSERT(!pg.register_statement("test_stmt", "SELECT * FROM pg_indexes"));
}

void Test_PostgreSQL::pg_add_admin_views()
{
	INIT_PG_CLIENT
	CPPUNIT_ASSERT(pg.add_admin_views("public") == PGRES_COMMAND_OK);
}

void Test_PostgreSQL::pg_drop_schema()
{
	INIT_PG_CLIENT
	pg.register_embedded_statements();
	CPPUNIT_ASSERT(pg.drop_schema("test_schema", true) == PGRES_COMMAND_OK);
}

void Test_PostgreSQL::pg_create_schema()
{
	INIT_PG_CLIENT
	pg.register_embedded_statements();
	CPPUNIT_ASSERT_MESSAGE("Schema creation (success)",
		pg.create_schema("test_schema") == PGRES_COMMAND_OK);

	ExecStatusType pg_status;
	try {
		pg_status = pg.create_schema("test_schema");
	}
	catch (db::PostgreSQLException &e) {
		pg_status = PGRES_FATAL_ERROR;
	}

	CPPUNIT_ASSERT_MESSAGE("Schema creation (failure)",
		pg_status == PGRES_FATAL_ERROR);
	CPPUNIT_ASSERT(pg.drop_schema("test_schema") == PGRES_COMMAND_OK);
}

void Test_PostgreSQL::pg_create_table()
{
	INIT_PG_CLIENT
	pg.register_embedded_statements();

	bool create_table_ok = false;
	try {
		std::string query = "CREATE TABLE " + PG_TEST_TABLE
			+ " (i INTEGER, s VARCHAR)";
		db::PostgreSQLResult result = pg.exec(query.c_str());
		create_table_ok = true;
	}
	catch (db::PostgreSQLException &e) {
		std::cerr << e.what() << std::endl;
	}

	CPPUNIT_ASSERT(create_table_ok);
}

void Test_PostgreSQL::pg_drop_table()
{
	pg_create_table();

	INIT_PG_CLIENT
	std::string query = "DROP TABLE " + PG_TEST_TABLE;

	bool drop_ok = false;
	try {
		pg.exec(query.c_str());
		drop_ok = true;
	}
	catch (db::PostgreSQLException &e) {
		std::cerr << e.what() << std::endl;
	}

	CPPUNIT_ASSERT(drop_ok);
}

void Test_PostgreSQL::pg_insert()
{
	pg_create_table();

	INIT_PG_CLIENT

	std::string query = "INSERT INTO " + PG_TEST_TABLE + "(i,s) VALUES (1, 'test')";

	bool insert_ok = false;
	try {
		pg.exec(query.c_str());
		insert_ok = true;
	}
	catch (db::PostgreSQLException &e) {
		std::cerr << e.what() << std::endl;
	}

	CPPUNIT_ASSERT_MESSAGE("PostgreSQL INSERT Test", insert_ok);
}

void Test_PostgreSQL::pg_transaction_insert()
{
	pg_create_table();

	INIT_PG_CLIENT

	bool insert_ok = false;
	try {
		pg.begin();
		for (uint8_t i = 0; i < 100; i++) {
			std::string query = "INSERT INTO " + PG_TEST_TABLE + "(i,s) VALUES ("
				+ std::to_string(i) + ", 'test')";
			pg.exec(query.c_str());
		}

		pg.commit();
		insert_ok = true;
	}
	catch (db::PostgreSQLException &e) {
		std::cerr << e.what() << std::endl;
	}

	CPPUNIT_ASSERT_MESSAGE("PostgreSQL Transaction INSERT Test", insert_ok);
}

void Test_PostgreSQL::pg_show_tables()
{
	INIT_PG_CLIENT
	pg.register_embedded_statements();
	std::vector<std::string> res;
	CPPUNIT_ASSERT(pg.show_tables("public", res) == PGRES_TUPLES_OK);
}
}
}
