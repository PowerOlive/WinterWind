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

#include "amqp/connection.h"
#include <iostream>
#include <cstring>
#include "utils/log.h"
#include "amqp/exception.h"
#include "amqp/channel.h"
#include "amqp/envelope.h"
#include "amqp/log.h"
#include "amqp/message.h"

namespace winterwind
{
namespace amqp
{

Connection::Connection(const std::string &url, uint64_t wait_timeout) :
	m_wait_timeout_ms(wait_timeout)
{
	amqp_connection_info info{};
	amqp_default_connection_info(&info);
	if (!url.empty()) {
		char url_buf[1024];
		memset(url_buf, 0, sizeof(url_buf));
		memcpy(url_buf, url.data(), url.size());

		if (amqp_parse_url(url_buf, &info) != AMQP_STATUS_OK) {
			throw amqp::exception("Unable to parse AMQP URL.");
		}
	}

	connect(info.host, (uint16_t) info.port, info.user, info.password, info.vhost);
}

Connection::Connection(const std::string &host, uint16_t port,
	const std::string &username, const std::string &password, const std::string &vhost,
	int32_t frame_max, uint64_t wait_timeout) :
	m_frame_max(frame_max),
	m_wait_timeout_ms(wait_timeout)
{
	connect(host.c_str(), port, username.c_str(), password.c_str(), vhost.c_str());
}

Connection::~Connection()
{
	// Invalidate channels
	for (auto &channel: m_channels) {
		channel.second->invalidate();
	}

	amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS);
	if (amqp_destroy_connection(m_conn) != AMQP_STATUS_OK) {
		log_error(amqp_log, "Failed to destroy AMQP connection.");
	}
}

void Connection::connect(const char *host, uint16_t port,
	const char *username, const char *password, const char *vhost)
{
	m_conn = amqp_new_connection();
	if (!m_conn) {
		log_error(amqp_log, "Unable to allocate a new AMQP connection.");
		throw amqp::exception("Unable to allocate a new AMQP connection.");
	}

	socket = amqp_tcp_socket_new(m_conn);
	if (!socket) {
		log_error(amqp_log, "Unable to allocate a new AMQP socket.");
		throw amqp::exception("Unable to allocate a new AMQP socket.");
	}

	if (!open(host, port)) {
		throw amqp::exception("Unable to open AMQP connection");
	}

	if (!login(username, password, vhost, m_frame_max)) {
		throw amqp::exception("Unable to login to AMQP connection");
	}
}

bool Connection::open(const char *host, uint16_t port)
{
	int status = amqp_socket_open(socket, host, port);
	if (status != AMQP_STATUS_OK) {
		log_error(amqp_log, "Failed to open AMQP connection (code: " +
			std::to_string(status) + ")");
		return false;
	}

	return true;

}

bool Connection::login(const std::string &user, const std::string &password,
	const std::string &vhost, int32_t frame_max)
{
	amqp_rpc_reply_t result = amqp_login(m_conn, vhost.c_str(), 0, frame_max,
		m_heartbeat_interval, AMQP_SASL_METHOD_PLAIN, user.c_str(), password.c_str());
	if (result.reply_type != AMQP_RESPONSE_NORMAL) {
		std::stringstream ss;
		ss << "login failure (reply_type: " << result.reply_type << ").";
		if (result.reply_type == AMQP_RESPONSE_SERVER_EXCEPTION) {
			auto login_exception = (amqp_channel_close_t *)result.reply.decoded;

			ss << " Reply ID: " << result.reply.id << ", exception was: "
				<< std::string((const char *) login_exception->reply_text.bytes,
					login_exception->reply_text.len) << std::endl;
		}
		log_error(amqp_log, ss.str());
	}

	return result.reply_type == AMQP_RESPONSE_NORMAL;
}

bool Connection::set_heartbeat_interval(uint32_t interval)
{
	if (amqp_tune_connection(m_conn, 0, m_frame_max, interval) != AMQP_STATUS_OK) {
		return false;
	}
	m_heartbeat_interval = interval;
	return true;
}

int32_t Connection::get_channel_max()
{
	return amqp_get_channel_max(m_conn);
}

std::shared_ptr<Channel> Connection::create_channel()
{
	if (m_channels.size() == get_channel_max()) {
		log_error(amqp_log, "Unable to open AMQP channel: max channel reached");
		return nullptr;
	}

	// @TODO handle offsets properly
	++m_next_channel_id;

	std::shared_ptr<Channel> new_channel = std::make_shared<Channel>(m_next_channel_id,
		shared_from_this());

	m_channels[m_next_channel_id] = new_channel;
	return new_channel;
}

void Connection::destroy_channel(std::shared_ptr<Channel> channel)
{
	auto channel_it = m_channels.find(channel->m_id);
	if (channel_it != m_channels.end()) {
		m_channels.erase(channel_it);
	}
	channel->close();
}

std::shared_ptr<Channel> Connection::find_channel(uint16_t channel_id)
{
	auto channel_it = m_channels.find(channel_id);
	if (channel_it == m_channels.end()) {
		return nullptr;
	}

	return channel_it->second;
}

bool Connection::start_consuming()
{
	while (!m_stop) {
		if (!consume_one()) {
			return false;
		}
	}

	return true;
}

bool Connection::consume_one()
{
	amqp_frame_t frame{};
	amqp_envelope_t envelope{};
	amqp_rpc_reply_t ret;

	amqp_maybe_release_buffers(m_conn);


	if (m_wait_timeout_ms) {
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = m_wait_timeout_ms;
		ret = amqp_consume_message(m_conn, &envelope, &tv, 0);
	}
	else {
		ret = amqp_consume_message(m_conn, &envelope, nullptr, 0);
	}


	if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
		if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type &&
			AMQP_STATUS_UNEXPECTED_STATE == ret.library_error) {
			if (AMQP_STATUS_OK != amqp_simple_wait_frame(m_conn, &frame)) {
				log_error(amqp_log, "amqp_simple_wait_frame consuming error")
				return false;
			}

			if (AMQP_FRAME_METHOD == frame.frame_type) {
				switch (frame.payload.method.id) {
					case AMQP_BASIC_ACK_METHOD:
						/*
						 * if we've turned publisher confirms on, and we've published a message
						 * here is a message being confirmed
						 */
						// @TODO callback the ack
						break;
					case AMQP_BASIC_RETURN_METHOD: {
						/*
						 * if a published message couldn't be routed and the mandatory flag was set
						 * this is what would be returned. The message then needs to be read.
						 */
						amqp_message_t raw_message{};
						ret = amqp_read_message(m_conn, frame.channel, &raw_message, 0);
						if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
							return false;
						}

						std::shared_ptr<Channel> channel = find_channel(frame.channel);
						if (channel == nullptr) {
							log_error(amqp_log, "failed to reroute unsent message to "
								"channel id " + std::to_string(frame.channel))
							return false;
						}

						// Request channel to handle unsent message
						channel->on_unsent_message(std::make_shared<Message>(&raw_message));

						amqp_destroy_message(&raw_message);
						break;
					}
					case AMQP_CHANNEL_CLOSE_METHOD:
						/*
						 * a channel.close method happens when a channel exception occurs, this
						 * can happen by publishing to an exchange that doesn't exist for example
						 *
						 * In this case you would need to open another channel redeclare any queues
						 * that were declared auto-delete, and restart any consumers that were attached
						 * to the previous channel
						 */
						return false;

					case AMQP_CONNECTION_CLOSE_METHOD:
						/*
						 * a connection.close method happens when a connection exception occurs,
						 * this can happen by trying to use a channel that isn't open for example.
						 *
						 * In this case the whole connection must be restarted.
						 */
						return false;

					default:
						log_error(amqp_log, "An unexpected method was received "
							+ std::to_string(frame.payload.method.id));
						return false;
				}
			}
		}

	} else {
		std::stringstream ss;
		ss << "Message received on channel " << envelope.channel << std::endl;
		log_info(amqp_log, ss.str())

		distribute_envelope(std::make_shared<Envelope>(&envelope), envelope.channel);

		amqp_destroy_envelope(&envelope);
	}

	return true;
}

bool Connection::distribute_envelope(EnvelopePtr envelope, uint16_t channel_id)
{
	std::shared_ptr<Channel> channel = find_channel(channel_id);
	if (channel == nullptr) {
		log_error(amqp_log, "failed to distribute envelope to channel id "
			+ std::to_string(channel_id))
		return false;
	}

	return channel->on_envelope_received(std::move(envelope));
}


}
}
