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

#include <hiredis/hiredis.h>
#include <string>
#include <vector>

namespace winterwind
{
class RedisClient
{
public:
	RedisClient(const std::string &host, const uint16_t port,
		const uint32_t cb_interval = 0);

	~RedisClient();

	bool type(const std::string &key, std::string &res);

	bool set(const std::string &key, const std::string &value,
		const uint32_t expire_value = 0);

	bool get(const std::string &key, std::string &res);

	bool del(const std::string &key);

	bool hset(const std::string &key, const std::string &skey, const std::string &value,
		const uint32_t expire_value = 0);

	bool hget(const std::string &key, const std::string &skey, std::string &res);

	bool hdel(const std::string &key, const std::string &skey);

	bool hkeys(const std::string &key, std::vector<std::string> &res);

	bool expire(const std::string &key, const uint32_t value);

private:
	void connect();

	std::string m_host = "";
	uint16_t m_port = 6379;
	uint32_t m_circuit_breaker_interval = 60;
	time_t m_last_failed_connection = 0;
	redisContext *m_context = nullptr;
};
}
