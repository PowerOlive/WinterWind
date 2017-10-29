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
#include <string>

namespace winterwind
{
namespace amqp
{

class Channel;
class Message;

class Exchange
{
public:
	Exchange() = delete;

	Exchange(std::string name, std::shared_ptr<Channel> channel);

	const std::string &get_name() const { return m_name; }
	// @TODO implement (un)bind_to, (un)bind_from

	/**
	 * Publish message to exchange with routing_key
	 * @param routing_key
	 * @param message
	 * @return AMQP_STATUS_OK on success, amqp_status_enum value on failure. Note
	 *         that basic.publish is an async method, the return value from this
	 *         function only indicates that the message data was successfully
	 *         transmitted to the broker. It does not indicate failures that occur
	 *         on the broker, such as publishing to a non-existent exchange.
	 *         Possible error values:
	 *         - AMQP_STATUS_TIMER_FAILURE: system timer facility returned an error
	 *           the message was not sent.
	 *         - AMQP_STATUS_HEARTBEAT_TIMEOUT: connection timed out waiting for a
	 *           heartbeat from the broker. The message was not sent.
	 *         - AMQP_STATUS_NO_MEMORY: memory allocation failed. The message was
	 *           not sent.
	 *         - AMQP_STATUS_TABLE_TOO_BIG: a table in the properties was too large
	 *           to fit in a single frame. Message was not sent.
	 *         - AMQP_STATUS_CONNECTION_CLOSED: the connection was closed.
	 *         - AMQP_STATUS_SSL_ERROR: a SSL error occurred.
	 *         - AMQP_STATUS_TCP_ERROR: a TCP error occurred. errno or
	 *           WSAGetLastError() may provide more information
	 */
	int32_t basic_publish(const std::string &routing_key, const Message &message);
private:
	std::string m_name;
	std::weak_ptr<Channel> m_channel;
};
}
}
