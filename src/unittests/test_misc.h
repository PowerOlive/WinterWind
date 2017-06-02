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

#include <extras/openweathermapclient.h>
#include <extras/luaextraengine.h>
#include <core/utils/jsonwebtokens.h>

#include "cmake_config.h"

using namespace winterwind::extras;
using namespace winterwind::web;

namespace winterwind {
namespace unittests {

class Test_Misc : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_Misc);
	CPPUNIT_TEST(weather_to_json);
	CPPUNIT_TEST(lua_winterwind_engine);
	CPPUNIT_TEST(jsonwebtokens_write_payload_reserved_claim);
	CPPUNIT_TEST(jsonwebtokens_write_hs256);
	CPPUNIT_TEST(jsonwebtokens_read_hs256);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

	void tearDown()
	{}

protected:
	void lua_winterwind_engine()
	{
		LuaEngineExtras L;
		LuaReturnCode rc = L.init_winterwind_extra_bindings();
		CPPUNIT_ASSERT(rc == LUA_RC_OK);
		rc = L.load_script(UNITTESTS_LUA_FILE);
		CPPUNIT_ASSERT(rc == LUA_RC_OK);
		CPPUNIT_ASSERT(L.run_unittests());
	}

	void weather_to_json()
	{
		Weather w;
		w.sunset = 150;
		w.sunrise = 188;
		w.humidity = 4;
		w.temperature = 25.0f;
		w.city = "test_city";
		Json::Value res;
		w >> res;
		CPPUNIT_ASSERT(res["sunset"].asUInt() == 150);
		CPPUNIT_ASSERT(res["sunrise"].asUInt() == 188);
		CPPUNIT_ASSERT(res["humidity"].asInt() == 4);
		CPPUNIT_ASSERT(res["temperature"].asFloat() == 25.0f);
		CPPUNIT_ASSERT(res["city"].asString() == "test_city");
	}

	void jsonwebtokens_write_payload_reserved_claim()
	{
		Json::Value payload;
		payload["sub"] = "1234567890";
		payload["name"] = "John Doe";
		payload["admin"] = true;
		JsonWebToken jwt(JsonWebToken::Algorithm::ALG_HS256, payload, "secret");

		std::string res;
		CPPUNIT_ASSERT(jwt.get(res) == JsonWebToken::JWTGenStatus::GENSTATUS_JWT_CLAIM_IN_PAYLOAD);
	}

	void jsonwebtokens_write_hs256()
	{
		Json::Value payload;
		payload["name"] = "John Doe";
		payload["admin"] = true;
		JsonWebToken jwt(JsonWebToken::Algorithm::ALG_HS256, payload, "secret");
		jwt.subject("1234567890");

		std::string res;
		CPPUNIT_ASSERT(jwt.get(res) == JsonWebToken::JWTGenStatus::GENSTATUS_OK);

		static const std::string raw_jwt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9Cg.eyJhZG1pbiI6dHJ1ZSwibmFtZSI6IkpvaG4gRG9lIiwic3ViIjoiMTIzNDU2Nzg5MCJ9Cg.tyCpKsAQHAVmicQEgpaoYO1VGeg6kCmFeheW0oJ+U7A";
		const std::string message = res + " != " + raw_jwt;
		CPPUNIT_ASSERT_MESSAGE(message, res.compare(raw_jwt) == 0);
	}

	void jsonwebtokens_read_hs256()
	{
		static const std::string raw_jwt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
			"eyJzdWIiOiJibGFoIiwibmFtZSI6IkpvaG4gRG9lIiwidW5pdHRlc3QiOnRydWV9."
			"GunUY5CARG1fNOHd9cOneFQQfT-OlF1gWB8SeyaXTAE";

		JsonWebToken jwt("secret");
		JsonWebToken::JWTStatus rc = jwt.read_and_verify(raw_jwt);
		CPPUNIT_ASSERT(rc == JsonWebToken::STATUS_OK);
	}
};
}
}
