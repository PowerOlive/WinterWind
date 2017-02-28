/**
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

#include "tests.h"
#include "test_postgresql.h"
#include "test_string.h"

int main(int argc, const char* argv[])
{
	if (argc < 2) {
		std::cerr << argv[0] << ": Missing gitlab token" << std::endl;
		return -1;
	}

	if (argc < 3) {
		std::cerr << argv[0] << ": Missing Twitter consumer key" << std::endl;
		return -1;
	}

	if (argc < 4) {
		std::cerr << argv[0] << ": Missing Twitter consumer secret" << std::endl;
		return -1;
	}

	if (argc < 5) {
		std::cerr << argv[0] << ": Missing Twitter access token" << std::endl;
		return -1;
	}

	if (argc < 6) {
		std::cerr << argv[0] << ": Missing Twitter access token secret" << std::endl;
		return -1;
	}

	GITLAB_TOKEN = std::string(argv[1]);
	TWITTER_CONSUMER_KEY = std::string(argv[2]);
	TWITTER_CONSUMER_SECRET = std::string(argv[3]);
	TWITTER_ACCESS_TOKEN = std::string(argv[4]);
	TWITTER_ACCESS_TOKEN_SECRET = std::string(argv[5]);

	if (argc >= 7) {
		ES_HOST = std::string(argv[6]);
	}

	CppUnit::TextUi::TestRunner runner;
	runner.addTest(WinterWindTest_PostgreSQL::suite());
	runner.addTest(WinterWindTest_String::suite());
	runner.addTest(WinterWindTests::suite());
	std::cout << "Starting unittests...." << std::endl;
	return runner.run() ? 0 : 1;

}
