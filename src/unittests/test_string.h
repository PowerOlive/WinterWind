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

#include <utils/base64.h>
#include <utils/stringutils.h>
#include <utils/hmac.h>

#include "cmake_config.h"

class WinterWindTest_String : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(WinterWindTest_String);
	CPPUNIT_TEST(split_string);
	CPPUNIT_TEST(remove_substring);
	CPPUNIT_TEST(base64_encode_test);
	CPPUNIT_TEST(base64_encode_test2);
	CPPUNIT_TEST(base64_decode_test);
	CPPUNIT_TEST(hmac_sha1_test);
	CPPUNIT_TEST(str_hex);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}

protected:
	void str_hex()
	{
		std::string orig = "hello hex to convert.";
		std::string dest = "";
		str_to_hex(orig, dest);
		CPPUNIT_ASSERT(dest == "68656c6c6f2068657820746f20636f6e766572742e");
	}

	void split_string()
	{
		std::string orig = "hello, this is winterwind.";
		std::vector<std::string> res;
		str_split(orig, ' ', res);
		CPPUNIT_ASSERT(res.size() == 4 && res[2] == "is");
	}

	void remove_substring()
	{
		std::string orig = "The world is mine, the world is not yours";
		std::string to_alter = orig;
		str_remove_substr(to_alter, "world ");
		CPPUNIT_ASSERT(to_alter == "The is mine, the is not yours");
	}

	void base64_encode_test()
	{
		std::string src = "unittest_b64encode";
		std::string res = base64_encode((const unsigned char *) src.c_str(), src.size());
		CPPUNIT_ASSERT(res.compare("dW5pdHRlc3RfYjY0ZW5jb2Rl") == 0);
	}

	void base64_decode_test()
	{
		std::string src = "dW5pdHRlc3RfYjY0ZGVjb2Rl";
		std::string res = base64_decode(src);
		CPPUNIT_ASSERT(res.compare("unittest_b64decode") == 0);
	}

	void base64_encode_test2()
	{
		std::string src = "unittest_b64encode";
		std::string res = base64_encode(src);
		CPPUNIT_ASSERT(res.compare("dW5pdHRlc3RfYjY0ZW5jb2Rl") == 0);
	}

	void hmac_sha1_test()
	{
		std::string res = hmac_sha1("unittest_key", "hashthatthing");
		CPPUNIT_ASSERT(base64_encode(res) == "rfsumIkQ/lUjuI68D1t0eJe/PgE=");
	}
};
