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

#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>


#include <twitterclient.h>

#include "unittests_config.h"

static std::string TWITTER_CONSUMER_KEY = "";
static std::string TWITTER_CONSUMER_SECRET = "";
static std::string TWITTER_ACCESS_TOKEN = "";
static std::string TWITTER_ACCESS_TOKEN_SECRET = "";

class WinterWindTest_Twitter: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(WinterWindTest_Twitter);
	CPPUNIT_TEST(twitter_authenticate);
	CPPUNIT_TEST(twitter_user_timeline);
	CPPUNIT_TEST(twitter_home_timeline);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp()
	{
		m_twitter_client = new TwitterClient(TWITTER_CONSUMER_KEY, TWITTER_CONSUMER_SECRET,
			TWITTER_ACCESS_TOKEN, TWITTER_ACCESS_TOKEN_SECRET);
	}

	void tearDown()
	{
		delete m_twitter_client;
	}

protected:
	void twitter_authenticate()
	{
		CPPUNIT_ASSERT(m_twitter_client->authenticate() == TwitterClient::TWITTER_OK);
	}

	void twitter_user_timeline()
	{
		twitter_authenticate();
		Json::Value res;
		CPPUNIT_ASSERT(m_twitter_client->get_user_timeline(res, 10, 0, true) == TwitterClient::TWITTER_OK);
	}

	void twitter_home_timeline()
	{
		twitter_authenticate();
		Json::Value res;
		CPPUNIT_ASSERT(m_twitter_client->get_home_timeline(res, 10) == TwitterClient::TWITTER_OK);
		CPPUNIT_ASSERT(res.isObject() || res.isArray());
	}
private:
	TwitterClient *m_twitter_client = nullptr;
};
