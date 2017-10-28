/**
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

#include "core/utils/threads.h"
#include "core/amqp/connection.h"
#include "core/amqp/exchange.h"
#include "core/amqp/message.h"
#include "core/amqp/queue.h"
#include <queue>
#include <utility>

namespace winterwind
{
namespace amqp
{

/**
 * InOutExchange is a resilient In/Out exchange thread to easily communicate with
 * RabbitMQ.
 *
 * It provides:
 *   - auto reconnect
 *   - listening on a single queue though an exchange
 *   - sending response to the same exchange using a routing_key
 *
 * You can use it to easily listen to events and answer using reply_to field (for example)
 */
class InOutExchange : public Thread
{
public:
	/**
	 *
	 * @param rabbitmq_url
	 * @param exchange_name
	 * @param routing_key
	 * @param queue_name
	 * @param consumer_tag
	 * @param callback callback function called when a new envelope was received
	 */
	InOutExchange(std::string rabbitmq_url, std::string exchange_name,
		std::string routing_key, std::string queue_name, std::string consumer_tag,
		EnvelopeCallbackFct callback) :
		Thread(),
		m_rabbitmq_url(std::move(rabbitmq_url)),
		m_exchange_name(std::move(exchange_name)),
		m_routing_key(std::move(routing_key)),
		m_queue_name(std::move(queue_name)),
		m_consumer_tag(std::move(consumer_tag)),
		m_recv_callback(std::move(callback))
	{
		connect_to_rabbitmq();
	}

	/**
	 * Event loop
	 * @return
	 */
	void *run() override;

	/**
	 * Push response to response queue
	 *
	 * This function is thread-safe
	 * @param response
	 */
	void push_response(const std::pair<std::string, amqp::MessagePtr> &response)
	{
		std::unique_lock<std::mutex> lock(m_response_queue_mtx);
		m_response_queue.push(response);
	}

	void set_connection_retry_interval(uint32_t rti)
	{
		m_connection_retry_interval = rti;
	}
private:
	/**
	 * Try to connect to RabbitMQ, create exchange, queue and bindings.
	 * If succeed starts consuming.
	 *
	 * @return connection success
	 */
	bool connect_to_rabbitmq();

	bool verify_rabbitmq_connection()
	{
		// No channel, try to reconnect
		if (!m_rabbitmq_channel) {
			if (!connect_to_rabbitmq()) {
				// On failure ignore
				return false;
			}
		}

		return true;
	}

	const std::string m_rabbitmq_url;
	const std::string m_exchange_name;
	const std::string m_routing_key;
	const std::string m_queue_name;
	const std::string m_consumer_tag;

	std::shared_ptr<amqp::Exchange> get_valid_exchange();

	std::shared_ptr<amqp::Connection> m_rabbitmq_con;
	std::shared_ptr<amqp::Channel> m_rabbitmq_channel;
	std::shared_ptr<amqp::Queue> m_command_queue;

	EnvelopeCallbackFct m_recv_callback;

	/**
	 * Interval between a connection failure and the next connection
	 */
	uint32_t m_connection_retry_interval = 30;

	std::queue<std::pair<std::string, amqp::MessagePtr>> m_response_queue;
	std::mutex m_response_queue_mtx;
};

}
}

