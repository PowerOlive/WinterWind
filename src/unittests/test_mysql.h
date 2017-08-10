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

#include <core/databases/mysqlclient.h>

namespace winterwind {

namespace unittests {

#define INIT_MYSQL_CLIENT                                                                \
    db::MySQLClient mysql("mysql", "unittests", "un1Ttests", 3306, "unittests_db");

#define MYSQL_TEST_TABLE std::string("ut_table")

class Test_MySQL : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_MySQL);
	CPPUNIT_TEST(mysql_create_table);
	CPPUNIT_TEST(mysql_insert);
	CPPUNIT_TEST(mysql_transaction_insert);
	CPPUNIT_TEST(mysql_drop_table);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() override {}
	void tearDown() override
	{
		INIT_MYSQL_CLIENT;
		mysql.exec("DROP TABLE IF EXISTS " + MYSQL_TEST_TABLE);
	}

protected:
	void mysql_create_table()
	{
		INIT_MYSQL_CLIENT;
		bool create_table_ok = false;
		try {
			mysql.exec("CREATE TABLE " + MYSQL_TEST_TABLE + "(i INT, s VARCHAR(64))");
			create_table_ok = true;
		}
		catch (db::MySQLException &e) {
			std::cerr << e.what() << std::endl;
		}

		CPPUNIT_ASSERT(create_table_ok);
	}

	void mysql_insert()
	{
		mysql_create_table();

		INIT_MYSQL_CLIENT;

		static const std::string query = "INSERT INTO " + MYSQL_TEST_TABLE
			+ "(i,s) VALUES (1, 'test')";

		bool insert_ok = false;
		try {
			mysql.exec(query);
			insert_ok = true;
		}
		catch (db::MySQLException &e) {
			std::cerr << e.what() << std::endl;
		}

		CPPUNIT_ASSERT_MESSAGE("MySQL INSERT Test", insert_ok);
	}

	void mysql_transaction_insert()
	{
		mysql_create_table();

		INIT_MYSQL_CLIENT;

		bool insert_ok = false;
		try {
			mysql.begin();
			for (uint8_t i = 0; i < 100; i++) {
				std::string query = "INSERT INTO " + MYSQL_TEST_TABLE + "(i,s) VALUES ("
					+ std::to_string(i) + ", 'test')";
				mysql.exec(query);
			}

			mysql.commit();
			insert_ok = true;
		}
		catch (db::MySQLException &e) {
			std::cerr << e.what() << std::endl;
		}

		CPPUNIT_ASSERT_MESSAGE("MySQL Transaction INSERT Test", insert_ok);
	}

	void mysql_drop_table()
	{
		mysql_create_table();

		INIT_MYSQL_CLIENT;
		bool drop_table_ok = false;
		try {
			mysql.exec("DROP TABLE " + MYSQL_TEST_TABLE);
			drop_table_ok = true;
		}
		catch (db::MySQLException &e) {
			std::cerr << e.what() << std::endl;
		}

		CPPUNIT_ASSERT(drop_table_ok);
	}
};
}
}
