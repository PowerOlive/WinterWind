/*
 * Copyright (c) 2017, Loic Blot <loic.blot@unix-experience.fr>
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

#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include "types.h"

namespace winterwind
{
namespace amqp
{

class Channel;
class Exchange;

class Queue
{
public:
	Queue(std::string name, std::shared_ptr<Channel> channel);
	~Queue();

	const std::string &get_name() const { return m_name; }

	bool remove();

	bool bind_exchange(std::shared_ptr<Exchange> exchange, const std::string &routing_key);
	bool bind_exchange(const std::string &exchange_name, const std::string &routing_key);

	bool unbind_exchange(std::shared_ptr<Exchange> exchange, const std::string &routing_key);
	bool unbind_exchange(const std::string &exchange_name, const std::string &routing_key);

	bool purge();

	/**
	 *
	 * @param consumer_tag
	 * @param no_local
	 * @param no_ack Enable it to auto acknowledge messages on RabbitMQ
	 * @param exclusive
	 * @return
	 */
	bool consume(const std::string &consumer_tag, const EnvelopeCallbackFct &callback,
		bool no_local = false, bool no_ack = false, bool exclusive = false);
private:
	std::string m_name;
	std::weak_ptr<Channel> m_channel;
};
}
}
