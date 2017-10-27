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

#include <cstdint>
#include <string>
#include <utility>
#include <amqp.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include "core/utils/classhelpers.h"
#include "types.h"

namespace winterwind
{
namespace amqp
{

class Connection;
class Consumer;
class Envelope;
class Exchange;
class Queue;
class Message;

class Channel : non_copyable, public std::enable_shared_from_this<Channel>
{
	friend class amqp::Connection;
	friend class amqp::Consumer;
	friend class amqp::Envelope;
	friend class amqp::Exchange;
	friend class amqp::Queue;
public:
	Channel() = delete;
	Channel(amqp_channel_t id, std::shared_ptr<Connection> conn);

	~Channel();

	enum ExchangeType : uint8_t
	{
		DIRECT,
		FANOUT,
		TOPIC,
	};

	/*
	 * Exchanges
	 */

	/**
	 * Verify if an exchange exists.
	 *
	 * @warning This function should not run on an existing channel because RabbitMQ will
	 * kill the channel if exchange doesn't exists
	 * @param exchange_name
	 * @return exchange existence
	 */
	bool exchange_exists(const std::string &exchange_name);

	/**
	 * Declare exchange with exchange_name with the exchange_type.
	 * @param exchange_name
	 * @param exchange_type
	 * @param durable
	 * @param auto_delete
	 * @return nullptr if declaration failed, else shared_ptr to exchange object
	 */
	std::shared_ptr<Exchange> declare_exchange(const std::string &exchange_name,
		ExchangeType exchange_type = ExchangeType::DIRECT,
		bool durable = false, bool auto_delete = false);
	bool delete_exchange(std::shared_ptr<Exchange> exchange,
		bool only_if_unused = false);
	bool delete_exchange(const std::string &exchange_name,
		bool only_if_unused = false);

	bool bind_exchange(const std::string &destination, const std::string &source,
		const std::string &routing_key);
	bool bind_exchange(std::shared_ptr<Exchange> destination,
		std::shared_ptr<Exchange> source, const std::string &routing_key);
	bool unbind_exchange(const std::string &destination, const std::string &source,
		const std::string &routing_key);
	bool unbind_exchange(std::shared_ptr<Exchange> destination,
		std::shared_ptr<Exchange> source, const std::string &routing_key);

	/*
	 * Queues
	 */

	/**
	 * Declare queue with name
	 * @param name
	 * @param durable Enable to make queue persist when no consumer and producer
	 * @param exclusive
	 * @param auto_delete
	 * @return nullptr if declaration failed, else shared_ptr to the queue object
	 */
	std::shared_ptr<Queue> declare_queue(const std::string &name, bool durable = false,
		bool exclusive = false, bool auto_delete = false);
	bool remove_queue(std::shared_ptr<Queue> queue,
		bool only_if_unused = false, bool only_if_empty = false);
	bool remove_queue(const std::string &name,
		bool only_if_unused = false, bool only_if_empty = false);

	/*
	 * Channel
	 */
	bool basic_qos(uint16_t prefetch_count, bool global);
	typedef std::function<bool(std::shared_ptr<Message>)> MessageHandlerCallback;

	/**
	 * Register a callback function to call when unsent message is returned back
	 * by RabbitMQ client
	 * @param callback
	 */
	void register_error_message_callback(const MessageHandlerCallback &callback)
	{
		m_error_message_handlers.push_back(callback);
	}

	typedef std::vector<MessageHandlerCallback> MessageHandlerCallbacks;
private:
	void open();
	void close();
	void invalidate()
	{
		m_valid = false;
	}

	bool purge_queue(Queue *queue);
	bool bind_queue(Queue *queue, const std::string &exchange,
		const std::string &routing_key);
	bool unbind_queue(Queue *queue, const std::string &exchange,
		const std::string &routing_key);

	/**
	 *
	 * @param exchange
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
	int32_t basic_publish(Exchange *exchange, const std::string &routing_key,
		const Message &message);

	/**
	 *
	 * @param queue
	 * @param consumer_tag
	 * @param no_local
	 * @param no_ack Enable it to auto acknowledge messages on RabbitMQ
	 * @param exclusive
	 * @return
	 */
	bool basic_consume(Queue *queue, const std::string &consumer_tag,
		const EnvelopeCallbackFct &callback, bool no_local = false, bool no_ack = false,
		bool exclusive = false);

	/**
	 * Callback from consumer when message was not sent
	 * @param message
	 * @return true if a callback consume it
	 */
	bool on_unsent_message(std::shared_ptr<Message> message);

	/**
	 * Callback from consumer when envelope was received
	 * If the registered queue callback returned true, message was flagged as ack and function
	 * will return true
	 * @param envelope
	 * @return true if a callback consume it
	 */
	bool on_envelope_received(EnvelopePtr envelope);

	std::weak_ptr<Connection> m_conn;
	amqp_channel_t m_id = 0;
	bool m_valid = true;

	std::unordered_map<std::string, EnvelopeCallbackFct> m_consumers_callbacks;
	MessageHandlerCallbacks m_error_message_handlers;
};
}
}
