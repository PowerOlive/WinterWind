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

namespace winterwind {
namespace unittests {

class Test_String : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_String);
	CPPUNIT_TEST(count_words_test);
	CPPUNIT_TEST(split_string);
	CPPUNIT_TEST(remove_substring);
	CPPUNIT_TEST(base64_encode_test);
	CPPUNIT_TEST(base64_encode_test2);
	CPPUNIT_TEST(base64_urlencode_test);
	CPPUNIT_TEST(base64_urlencode_test2);
	CPPUNIT_TEST(base64_decode_test);
	CPPUNIT_TEST(base64_urldecode_test);
	CPPUNIT_TEST(hmac_md5_test);
	CPPUNIT_TEST(hmac_sha1_test);
	CPPUNIT_TEST(hmac_sha256_test);
	CPPUNIT_TEST(hmac_sha384_test);
	CPPUNIT_TEST(hmac_sha512_test);
	CPPUNIT_TEST(replace_str);
	CPPUNIT_TEST(str_hex);
	CPPUNIT_TEST(trim_test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() override {}
	void tearDown() override {}

protected:
	void str_hex();
	void split_string();
	void remove_substring();
	void base64_encode_test();
	void base64_decode_test();
	void base64_encode_test2();
	void base64_urlencode_test();
	void base64_urldecode_test();
	void base64_urlencode_test2();
	void hmac_md5_test();
	void hmac_sha1_test();
	void hmac_sha256_test();
	void hmac_sha384_test();
	void hmac_sha512_test();
	void count_words_test();
	void replace_str();
	void trim_test();
};
}
}
