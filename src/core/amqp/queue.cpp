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

#include "amqp/queue.h"
#include "amqp/channel.h"
#include "amqp/exchange.h"

namespace winterwind
{
namespace amqp
{
Queue::Queue(std::string name, std::shared_ptr<Channel> channel) :
	m_name(std::move(name)), m_channel(channel)
{
}

Queue::~Queue()
{
	std::shared_ptr<Channel> channel = m_channel.lock();
}

bool Queue::remove()
{
	std::shared_ptr<Channel> channel = m_channel.lock();
	return channel->remove_queue(m_name);
}

bool Queue::bind_exchange(std::shared_ptr<Exchange> exchange,
	const std::string &routing_key)
{
	return bind_exchange(exchange->get_name(), routing_key);
}

bool Queue::bind_exchange(const std::string &exchange_name,
	const std::string &routing_key)
{
	std::shared_ptr<Channel> channel = m_channel.lock();
	return channel->bind_queue(this, exchange_name, routing_key);
}

bool Queue::unbind_exchange(std::shared_ptr<Exchange> exchange,
	const std::string &routing_key)
{
	return unbind_exchange(exchange->get_name(), routing_key);
}

bool Queue::unbind_exchange(const std::string &exchange_name,
	const std::string &routing_key)
{
	std::shared_ptr<Channel> channel = m_channel.lock();
	return channel->unbind_queue(this, exchange_name, routing_key);
}

bool Queue::purge()
{
	std::shared_ptr<Channel> channel = m_channel.lock();
	return channel->purge_queue(this);
}

bool Queue::consume(const std::string &consumer_tag, const EnvelopeCallbackFct &callback,
	bool no_local, bool no_ack, bool exclusive)
{
	std::shared_ptr<Channel> channel = m_channel.lock();
	return channel->basic_consume(this, consumer_tag, callback, no_local, no_ack,
		exclusive);
}

}
}
