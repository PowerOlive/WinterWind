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

class Test_RabbitMQ : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_RabbitMQ);

	CPPUNIT_TEST(create_connection);
	CPPUNIT_TEST(create_channel);
	CPPUNIT_TEST(remove_channel);
	CPPUNIT_TEST(basic_qos);
	CPPUNIT_TEST(declare_exchange);
	CPPUNIT_TEST(bind_exchange);
	CPPUNIT_TEST(unbind_exchange);
	CPPUNIT_TEST(delete_exchange);
	CPPUNIT_TEST(declare_queue);
	CPPUNIT_TEST(bind_queue_to_exchange);
	CPPUNIT_TEST(purge_queue);
	CPPUNIT_TEST(remove_queue);
	CPPUNIT_TEST(publish_to_exchange);
	CPPUNIT_TEST(consume_queue);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() override {}

	void tearDown() override;

protected:
	void create_connection();
	void create_channel();
	void remove_channel();
	void basic_qos();
	void declare_exchange();
	void bind_exchange();
	void unbind_exchange();
	void delete_exchange();
	void declare_queue();
	void bind_queue_to_exchange();
	void purge_queue();
	void remove_queue();
	void publish_to_exchange();
	void consume_queue();
};
}
}
