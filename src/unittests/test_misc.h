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

#include "cmake_config.h"

using namespace winterwind::extras;

namespace winterwind {
namespace unittests {

class Test_Misc : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_Misc);
	CPPUNIT_TEST(weather_to_json);
	CPPUNIT_TEST(lua_winterwind_engine);
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
};
}
}