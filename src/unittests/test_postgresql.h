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

#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <core/databases/postgresqlclient.h>

namespace winterwind {

namespace unittests {

#define INIT_PG_CLIENT                                                                   \
    db::PostgreSQLClient pg("host=postgres user=unittests dbname=unittests_db "          \
                "password=un1Ttests");

class Test_PostgreSQL : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_PostgreSQL);
	CPPUNIT_TEST(pg_register_embedded_statements);
	CPPUNIT_TEST(pg_register_custom_statement);
	CPPUNIT_TEST(pg_add_admin_views);
	CPPUNIT_TEST(pg_drop_schema);
	CPPUNIT_TEST(pg_create_schema);
	CPPUNIT_TEST(pg_show_tables);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

	void tearDown()
	{}

protected:
	void pg_register_embedded_statements()
	{
		INIT_PG_CLIENT
		pg.register_embedded_statements();
		CPPUNIT_ASSERT(true);
	}

	void pg_register_custom_statement()
	{
		INIT_PG_CLIENT
		CPPUNIT_ASSERT(pg.register_statement("test_stmt", "SELECT * FROM pg_indexes"));
		CPPUNIT_ASSERT(!pg.register_statement("test_stmt", "SELECT * FROM pg_indexes"));
	}

	void pg_add_admin_views()
	{
		INIT_PG_CLIENT
		CPPUNIT_ASSERT(pg.add_admin_views("public") == PGRES_COMMAND_OK);
	}

	void pg_drop_schema()
	{
		INIT_PG_CLIENT
		pg.register_embedded_statements();
		CPPUNIT_ASSERT(pg.drop_schema("test_schema", true) == PGRES_COMMAND_OK);
	}

	void pg_create_schema()
	{
		INIT_PG_CLIENT
		pg.register_embedded_statements();
		CPPUNIT_ASSERT(pg.create_schema("test_schema") == PGRES_COMMAND_OK);
		CPPUNIT_ASSERT(pg.create_schema("test_schema") != PGRES_COMMAND_OK);
		CPPUNIT_ASSERT(pg.drop_schema("test_schema") == PGRES_COMMAND_OK);
	}

	void pg_show_tables()
	{
		INIT_PG_CLIENT
		pg.register_embedded_statements();
		std::vector<std::string> res;
		CPPUNIT_ASSERT(pg.show_tables("public", res) == PGRES_TUPLES_OK);
	}
};
}
}
