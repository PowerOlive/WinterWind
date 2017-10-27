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

#include "amqp/inoutexchange.h"
#include "amqp/channel.h"
#include "amqp/envelope.h"
#include "amqp/exception.h"
#include "amqp/message.h"
#include "utils/log.h"
#include <cstring>
#include <iostream>

namespace winterwind
{
namespace amqp
{

bool InOutExchange::connect_to_rabbitmq()
{
	try {
		m_rabbitmq_con = std::make_shared<amqp::Connection>(m_rabbitmq_url, 20);
		m_rabbitmq_channel = m_rabbitmq_con->create_channel();

		std::shared_ptr<amqp::Exchange> exchange =
			m_rabbitmq_channel->declare_exchange(m_exchange_name);

		if (!exchange) {
			return false;
		}

		m_command_queue = m_rabbitmq_channel->declare_queue(m_queue_name);
		if (!m_command_queue->bind_exchange(exchange, m_routing_key)) {
			log_error(logger, "Unable to bind queue with exchange, invalidate "
				"connection");
			return false;
		}

		m_command_queue->consume(m_consumer_tag, m_recv_callback);

		return true;
	}
	catch (amqp::exception &e) {
		return false;
	}
}

void* InOutExchange::run()
{
	Thread::set_thread_name("RabbitMQInOut(" + m_exchange_name + ")");

	log_notice(logger, "Starting RabbitMQ In/Out Thread bound on '" + m_exchange_name
		+ "' exchange");

	ThreadStarted();

	while (!is_stopping()) {
		std::shared_ptr<amqp::Exchange> exchange = get_valid_exchange();
		if (!exchange) {
			log_error(logger, "Cannot connect to rabbitmq, retrying in "
				+ std::to_string(m_connection_retry_interval) + "sec...");
			std::this_thread::sleep_for(
				std::chrono::seconds(m_connection_retry_interval));
			continue;
		}

		if (!m_rabbitmq_con->consume_one()) {
			// Consuming failed, maybe a big error happened, restart loop
			continue;
		}

		if (m_response_queue_mtx.try_lock()) {
			if (!m_response_queue.empty()) {
				const std::pair<std::string, amqp::Message> &response =
					m_response_queue.front();
				exchange->basic_publish(response.first, response.second);
				m_response_queue.pop();
			}
			m_response_queue_mtx.unlock();
		}
	}

	return nullptr;
}

std::shared_ptr<amqp::Exchange> InOutExchange::get_valid_exchange()
{
	if (!verify_rabbitmq_connection()) {
		return nullptr;
	}

	std::shared_ptr<amqp::Exchange> exchange =
		m_rabbitmq_channel->declare_exchange(m_exchange_name);

	// Failed to declare exchange, connection should be incorrect, reconnect
	if (!exchange) {
		if (!connect_to_rabbitmq()) {
			// On failure return nullptr
			return nullptr;
		}

		exchange =
			m_rabbitmq_channel->declare_exchange(m_exchange_name);
	}

	m_command_queue->bind_exchange(exchange, m_routing_key);

	return exchange;
}

}
}
