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

#include "test_elasticsearch.h"
#include "test_gitlab.h"
#include "test_http.h"
#include "test_misc.h"
#include "test_postgresql.h"
#include "test_string.h"
#include "test_time.h"
#include "test_twitter.h"
#include "test_threads.h"
#include "test_mysql.h"
#include "test_rabbitmq.h"

#include <thread>
#include <log4cplus/consoleappender.h>

using namespace winterwind;

log4cplus::Logger logger = log4cplus::Logger::getRoot();

int main(int argc, const char *argv[])
{
	log4cplus::helpers::SharedObjectPtr<log4cplus::Appender> amqp_appender(
		new log4cplus::ConsoleAppender());
	amqp_appender->setName(LOG4CPLUS_TEXT("amqp"));
	logger.addAppender(amqp_appender);

	CppUnit::TextUi::TestRunner runner;

	if (!getenv("NO_REMOTE_TESTS")) {
		runner.addTest(winterwind::unittests::Test_Elasticsearch::suite());
		runner.addTest(winterwind::unittests::Test_Gitlab::suite());
		runner.addTest(winterwind::unittests::Test_MySQL::suite());
		runner.addTest(winterwind::unittests::Test_PostgreSQL::suite());
		runner.addTest(winterwind::unittests::Test_RabbitMQ::suite());
		runner.addTest(winterwind::unittests::Test_Twitter::suite());
	}

	runner.addTest(winterwind::unittests::Test_String::suite());
	runner.addTest(winterwind::unittests::Test_Time::suite());
	runner.addTest(winterwind::unittests::Test_Threads::suite());
	runner.addTest(winterwind::unittests::Test_HTTP::suite());
	runner.addTest(winterwind::unittests::Test_Misc::suite());
	std::cout << "Starting unittests...." << std::endl;
	return runner.run() ? 0 : 1;
}
