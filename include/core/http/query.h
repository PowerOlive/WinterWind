/*
 * Copyright (c) 2016-2017, Vincent Glize (Dumbeldor) <vincent.glize@live.fr>
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

#include <cstdint>
#include <iostream>
#include "../httpcommon.h"

namespace winterwind
{
namespace http
{

class Query
{
public:
	enum Flag: uint8_t
	{
		FLAG_SIMPLE = 0x01,
		FLAG_AUTH = 0x02,
		FLAG_NO_VERIFY_PEER = 0x04,
		FLAG_KEEP_HEADER_CACHE_AFTER_REQUEST = 0x08,
		FLAG_NO_RESPONSE_AWAITED = 0x10,
	};

	Query() = delete;
	Query(const std::string &url, std::string &post_data, const Method method = GET,
		const int32_t flag = FLAG_SIMPLE) :
		m_url(url), m_flag((Flag) flag), m_method(method), m_post_data(post_data)
	{}

	Query(const std::string &url, const Method method = GET, const int32_t flag = FLAG_SIMPLE) :
		m_url(url), m_method(method), m_flag((Flag) flag)
	{}

	virtual ~Query() = default;

	const std::string &get_url() const
	{
		return m_url;
	}

	const Flag get_flag() const
	{
		return m_flag;
	}

	const Method get_method() const
	{
		return m_method;
	}

	const std::string &get_post_data() const
	{
		return m_post_data;
	}

	Query &set_post_data(const std::string &post_data)
	{
		m_post_data = post_data;
		return *this;
	}

private:
	const std::string m_url = "";
	const Flag m_flag = FLAG_SIMPLE;
	const Method m_method = GET;
	std::string m_post_data = "";
};
}
}