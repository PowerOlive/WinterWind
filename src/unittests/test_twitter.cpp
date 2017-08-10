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

#include "test_twitter.h"

#include <extras/twitterclient.h>

using namespace winterwind::extras;

#define PRECHECK_TWITTER_CLIENT \
	CPPUNIT_ASSERT_MESSAGE("Twitter client not inited due to configuration error", \
	m_twitter_client.get() != nullptr);

namespace winterwind {
namespace unittests {

void Test_Twitter::setUp()
{
	if (!getenv("TWITTER_CONSUMER_KEY")) {
		std::cerr << __PRETTY_FUNCTION__ << ": Missing Twitter consumer key"
			<< std::endl;
		return;
	}

	std::string TWITTER_CONSUMER_KEY = std::string(getenv("TWITTER_CONSUMER_KEY"));

	if (!getenv("TWITTER_CONSUMER_SECRET")) {
		std::cerr << __PRETTY_FUNCTION__ << ": Missing Twitter consumer secret"
			<< std::endl;
		return;
	}

	std::string TWITTER_CONSUMER_SECRET = std::string(getenv("TWITTER_CONSUMER_SECRET"));

	if (!getenv("TWITTER_ACCESS_TOKEN")) {
		std::cerr << __PRETTY_FUNCTION__ << ": Missing Twitter access token" << std::endl;
		return;
	}

	std::string TWITTER_ACCESS_TOKEN = std::string(getenv("TWITTER_ACCESS_TOKEN"));

	if (!getenv("TWITTER_ACCESS_TOKEN_SECRET")) {
		std::cerr << __PRETTY_FUNCTION__ << ": Missing Twitter access token secret"
			<< std::endl;
		return;
	}

	std::string TWITTER_ACCESS_TOKEN_SECRET = std::string(getenv("TWITTER_ACCESS_TOKEN_SECRET"));

	m_twitter_client = std::make_unique<TwitterClient>(TWITTER_CONSUMER_KEY,
		TWITTER_CONSUMER_SECRET, TWITTER_ACCESS_TOKEN, TWITTER_ACCESS_TOKEN_SECRET);
}

void Test_Twitter::twitter_authenticate()
{
	PRECHECK_TWITTER_CLIENT;

	CPPUNIT_ASSERT(m_twitter_client->authenticate() == TwitterClient::TWITTER_OK);
}

void Test_Twitter::twitter_user_timeline()
{
	PRECHECK_TWITTER_CLIENT;

	twitter_authenticate();
	Json::Value res;
	CPPUNIT_ASSERT(m_twitter_client->get_user_timeline(res, 10, 0, true) ==
			TwitterClient::TWITTER_OK);
}

void Test_Twitter::twitter_home_timeline()
{
	PRECHECK_TWITTER_CLIENT;

	twitter_authenticate();
	Json::Value res;
	CPPUNIT_ASSERT(m_twitter_client->get_home_timeline(res, 10) ==
			TwitterClient::TWITTER_OK);
	CPPUNIT_ASSERT(res.isObject() || res.isArray());
}

}
}
