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

#include "amqp/channel.h"
#include "utils/log.h"
#include <iostream>
#include "amqp/exception.h"
#include "amqp/exchange.h"
#include "amqp/connection.h"
#include "amqp/envelope.h"
#include "amqp/log.h"
#include "amqp/message.h"
#include "amqp/queue.h"

namespace winterwind
{
namespace amqp
{

#define VERIFY_CHANNEL(R) \
	if (!m_valid) { \
		log_fatal(amqp_log, "channel is invalid."); \
		return R; \
	}

#define CONN_PTR \
	std::shared_ptr<amqp::Connection> conn(m_conn.lock());

#define TO_AMQP_STR(s) \
	amqp_cstring_bytes(s.c_str())

Channel::Channel(amqp_channel_t id, std::shared_ptr<Connection> conn) :
	m_id(id),
	m_conn(conn)
{
	open();
}

Channel::~Channel()
{
	close();
}

void Channel::open()
{
	CONN_PTR

	amqp_channel_open(conn->m_conn, m_id);

	amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn->m_conn);
	switch (reply.reply_type) {
		case AMQP_RESPONSE_NORMAL:
			return;
		case AMQP_RESPONSE_LIBRARY_EXCEPTION:
			log_error(amqp_log, "Unable to open AMQP channel: " +
				std::string(amqp_error_string2(reply.library_error)));
			throw amqp::exception("Unable to open AMQP channel");
		case AMQP_RESPONSE_SERVER_EXCEPTION:
			handle_reply_error(reply);
			throw amqp::exception("Unable to open AMQP channel");
		default:
			assert(false);
	}
}

void Channel::close()
{
	if (!m_valid) {
		return;
	}

	CONN_PTR
	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);
	amqp_channel_close(conn->m_conn, m_id, AMQP_REPLY_SUCCESS);
	m_valid = false;
}

static const char *exchange_type_str[] {
	"direct",
	"fanout",
	"topic",
};

bool Channel::exchange_exists(const std::string &exchange_name)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	amqp_exchange_declare(conn->m_conn, m_id,
		TO_AMQP_STR(exchange_name),
		amqp_cstring_bytes(""),
		true, false, false, 0, amqp_empty_table);
	amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn->m_conn);

	bool result = true;
	switch (reply.reply_type) {
		case AMQP_RESPONSE_NORMAL:
			break;
		case AMQP_RESPONSE_LIBRARY_EXCEPTION: {
			std::stringstream ss;
			ss << "Failed to check AMQP exchange '" << exchange_name << "' (rt: "
				<< reply.reply_type << "): "
				<< amqp_error_string2(reply.library_error) << std::endl;
			log_error(amqp_log, ss.str());
			result = false;
			break;
		}
		case AMQP_RESPONSE_SERVER_EXCEPTION:
			handle_reply_error(reply);
			result = false;
			break;
		default:
			assert(false);
	}

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);
	return result;
}

std::shared_ptr<Exchange> Channel::declare_exchange(const std::string &exchange_name,
	ExchangeType exchange_type, bool durable, bool auto_delete)
{
	VERIFY_CHANNEL(nullptr)
	CONN_PTR

	amqp_exchange_declare(conn->m_conn, m_id,
		TO_AMQP_STR(exchange_name),
		amqp_cstring_bytes(exchange_type_str[exchange_type]),
		false, durable, auto_delete, 0, amqp_empty_table);
	amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn->m_conn);

	switch (reply.reply_type) {
		case AMQP_RESPONSE_NORMAL:
			break;
		case AMQP_RESPONSE_LIBRARY_EXCEPTION: {
			std::stringstream ss;
			ss << "Failed to create AMQP exchange '" << exchange_name << "' (rt: "
				<< reply.reply_type << "): "
				<< amqp_error_string2(reply.library_error) << std::endl;
			log_error(amqp_log, ss.str());
			amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);
			return nullptr;
		}
		case AMQP_RESPONSE_SERVER_EXCEPTION:
			handle_reply_error(reply);
			amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);
			return nullptr;
		default:
			assert(false);
	}

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);
	return std::make_shared<Exchange>(exchange_name, shared_from_this());
}

bool Channel::delete_exchange(const std::string &exchange_name, bool only_if_unused)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	bool status = amqp_exchange_delete(conn->m_conn, m_id,
		TO_AMQP_STR(exchange_name), only_if_unused) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);

	return status;
}

bool Channel::delete_exchange(std::shared_ptr<Exchange> exchange,
	bool only_if_unused)
{
	return delete_exchange(exchange->get_name(), only_if_unused);
}

bool Channel::bind_exchange(const std::string &destination, const std::string &source,
	const std::string &routing_key)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	bool status = amqp_exchange_bind(conn->m_conn, m_id,
		TO_AMQP_STR(destination), TO_AMQP_STR(source), TO_AMQP_STR(routing_key),
		amqp_empty_table) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);
	m_valid = status;
	return status;
}

bool Channel::bind_exchange(std::shared_ptr<Exchange> destination,
	std::shared_ptr<Exchange> source, const std::string &routing_key)
{
	return bind_exchange(destination->get_name(), source->get_name(), routing_key);
}

bool Channel::unbind_exchange(const std::string &destination, const std::string &source,
	const std::string &routing_key)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	bool status = amqp_exchange_unbind(conn->m_conn, m_id,
		TO_AMQP_STR(destination), TO_AMQP_STR(source), TO_AMQP_STR(routing_key),
		amqp_empty_table) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);
	m_valid = status;
	return status;
}

bool Channel::unbind_exchange(std::shared_ptr<Exchange> destination,
	std::shared_ptr<Exchange> source, const std::string &routing_key)
{
	return unbind_exchange(destination->get_name(), source->get_name(), routing_key);
}


std::shared_ptr<Queue> Channel::declare_queue(const std::string &name, bool durable,
		bool exclusive, bool auto_delete)
{
	VERIFY_CHANNEL(nullptr)
	CONN_PTR

	bool status = amqp_queue_declare(conn->m_conn, m_id, TO_AMQP_STR(name), false,
		durable, exclusive, auto_delete, amqp_empty_table) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);

	if (!status) {
		return nullptr;
	}

	return std::make_shared<Queue>(name, shared_from_this());
}

bool Channel::remove_queue(const std::string &name, bool only_if_unused,
	bool only_if_empty)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	bool status = amqp_queue_delete(conn->m_conn, m_id, TO_AMQP_STR(name), only_if_unused,
		only_if_empty) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);

	return status;
}

bool Channel::remove_queue(std::shared_ptr<Queue> queue, bool only_if_unused,
	bool only_if_empty)
{
	return remove_queue(queue->get_name(), only_if_unused, only_if_empty);
}

bool Channel::purge_queue(Queue *queue)
{
	VERIFY_CHANNEL(false)
	CONN_PTR
	bool status = amqp_queue_purge(conn->m_conn, m_id,
		TO_AMQP_STR(queue->get_name())) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);

	return status;
}

bool Channel::bind_queue(Queue *queue, const std::string &exchange,
	const std::string &routing_key)
{
	VERIFY_CHANNEL(false)
	CONN_PTR
	bool status = amqp_queue_bind(conn->m_conn, m_id,
		TO_AMQP_STR(queue->get_name()), TO_AMQP_STR(exchange),  TO_AMQP_STR(routing_key),
		amqp_empty_table) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);

	// if bind failed, exchange should not exist and Rabbit terminate channel
	if (!status) {
		invalidate();
	}

	return status;
}

bool Channel::unbind_queue(Queue *queue, const std::string &exchange,
	const std::string &routing_key)
{
	VERIFY_CHANNEL(false)
	CONN_PTR
	bool status = amqp_queue_unbind(conn->m_conn, m_id,
		TO_AMQP_STR(queue->get_name()), TO_AMQP_STR(exchange),  TO_AMQP_STR(routing_key),
		amqp_empty_table) != nullptr;

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);

	// if unbind failed, exchange should not exist and Rabbit terminate channel
	if (!status) {
		invalidate();
	}

	return status;
}

int32_t Channel::basic_publish(Exchange *exchange, const std::string &routing_key,
	const Message &message)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	amqp_bytes_t data{message.m_data.size(), (void *)message.m_data.data()};
	int32_t status = amqp_basic_publish(conn->m_conn, m_id,
		TO_AMQP_STR(exchange->get_name()), TO_AMQP_STR(routing_key), message.m_mandatory,
		message.m_immediate, &message.m_properties, data);

	amqp_maybe_release_buffers_on_channel(conn->m_conn, m_id);

	return status;
}

bool Channel::basic_consume(Queue *queue, const std::string &consumer_tag,
	const EnvelopeCallbackFct &callback, bool no_local,	bool no_ack, bool exclusive)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	if (m_consumers_callbacks.find(consumer_tag) != m_consumers_callbacks.end()) {
		log_error(amqp_log, "Forbid to perform basic consume on already registered "
			"consumer_tag for channel ID " + std::to_string(m_id))
		return false;
	}

	bool status = amqp_basic_consume(conn->m_conn, m_id, TO_AMQP_STR(queue->get_name()),
		TO_AMQP_STR(consumer_tag), no_local, no_ack, exclusive,
		amqp_empty_table) != nullptr;

	// if basic_consume failed, queue should not exist and Rabbit terminate channel
	if (!status) {
		invalidate();
	}
	else {
		m_consumers_callbacks[consumer_tag] = callback;
	}

	return status;
}

bool Channel::basic_qos(uint16_t prefetch_count, bool global)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	// rabbitmq 3.6.12 doesn't implement prefetch_size
	// Error was: operation basic.qos caused a connection exception not_implemented:
	// "prefetch_size!=0 (1)"
	static const uint32_t prefetch_size = 0;
	bool status = amqp_basic_qos(conn->m_conn, m_id, prefetch_size, prefetch_count,
		global) != nullptr;

	// if qos failed, queue should not exist and Rabbit terminate channel
	if (!status) {
		invalidate();
	}

	return status;
}

bool Channel::on_unsent_message(std::shared_ptr<Message> message)
{
	log_error(amqp_log, "received unsent message.")

	for (const auto &hdl : m_error_message_handlers) {
		hdl(message);
	}

	return true;
}

bool Channel::on_envelope_received(EnvelopePtr envelope)
{
	VERIFY_CHANNEL(false)
	CONN_PTR

	log_debug(amqp_log, "received envelope for consumer tag "
		+ envelope->get_consumer_tag() + " with delivery tag "
		+ std::to_string(envelope->get_delivery_tag()))

	auto queue_it = m_consumers_callbacks.find(envelope->get_consumer_tag());
	if (queue_it == m_consumers_callbacks.end()) {
		log_error(amqp_log, std::string(__FUNCTION__)
			+ ": No linked queue found for consumer tag " + envelope->get_consumer_tag()
			+ ", message not acked.")
		return false;
	}

	// if message is consumed, ack
	if (queue_it->second && queue_it->second(envelope)) {
		amqp_basic_ack(conn->m_conn, m_id, envelope->get_delivery_tag(), false);
		return true;
	}

	return false;
}

}
}
