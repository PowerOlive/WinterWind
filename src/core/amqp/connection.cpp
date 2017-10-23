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
#include <utility>
#include "utils/log.h"
#include "amqp/exception.h"
#include "amqp/channel.h"
#include "amqp/log.h"

namespace winterwind
{
namespace amqp
{

Connection::Connection(const std::string &url)
{
	amqp_connection_info info{};
	if (url.empty()) {
		amqp_default_connection_info(&info);
	}
	else if (amqp_parse_url((char *) url.c_str(), &info) != AMQP_STATUS_OK) {
		throw amqp::exception("Unable to parse AMQP URL.");
	}

	connect(info.host, (uint16_t) info.port, info.user, info.password, info.vhost);
}

Connection::Connection(const std::string &host, uint16_t port,
	const std::string &username, const std::string &password, const std::string &vhost,
	int32_t frame_max)
{
	connect(host, port, username, password, vhost, frame_max);
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

void Connection::connect(const std::string &host, uint16_t port,
	const std::string &username, const std::string &password, const std::string &vhost,
	int32_t frame_max)
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

	if (!login(username, password, vhost, frame_max)) {
		throw amqp::exception("Unable to login to AMQP connection");
	}
}

bool Connection::open(const std::string &host, uint16_t port)
{
	int status = amqp_socket_open(socket, host.c_str(), port);
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
		HEARTBEAT_INTERVAL, AMQP_SASL_METHOD_PLAIN, user.c_str(), password.c_str());
	return result.reply_type == AMQP_RESPONSE_NORMAL;

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

}
}
