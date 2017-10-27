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
#include <amqp_tcp_socket.h>
#include <unordered_map>
#include <memory>
#include "core/utils/classhelpers.h"
#include "types.h"

namespace winterwind
{
namespace amqp
{

class Channel;
class Envelope;
class Message;

class Connection : non_copyable, public std::enable_shared_from_this<amqp::Connection>
{
	friend class Channel;
public:
	explicit Connection(const std::string &url = "", uint64_t wait_timeout = 0);
	Connection(const std::string &host, uint16_t port,
		const std::string &username, const std::string &password,
		const std::string &vhost = "/", int32_t frame_max = 131072,
		uint64_t wait_timeout = 0);

	virtual ~Connection();

	bool login(const std::string &user, const std::string &password,
		const std::string &vhost, int32_t frame_max);

	/**
	 * Sets heartbeat interval on an active amqp connection
	 * @param interval
	 * @return true if new options has been applied
	 */
	bool set_heartbeat_interval(uint32_t interval);

	int32_t get_channel_max();

	std::shared_ptr<Channel> create_channel();
	void destroy_channel(std::shared_ptr<Channel> channel);

	/**
	 * Consume until m_stop has been set to true
	 * @return false if an error occured
	 */
	bool start_consuming();

	/**
	 * Wait for and receive a single envelope to handle
	 * @return false if an error occured
	 */
	bool consume_one();

	void stop()
	{
		m_stop = true;
	}

	void set_wait_timeout(uint64_t wait_timeout)
	{
		m_wait_timeout_ms = wait_timeout;
	}

protected:
	amqp_connection_state_t m_conn = nullptr;

	/**
	 * Get Channel object associated with ID
	 * @param channel_id
	 * @return Channel shared_ptr if object is associated to connection, else nullptr
	 */
	std::shared_ptr<Channel> find_channel(uint16_t channel_id);

private:
	/**
	 * Perform connection to RabbitMQ
	 * @param host
	 * @param port
	 * @param username
	 * @param password
	 * @param vhost
	 */
	void connect(const std::string &host = "127.0.0.1", uint16_t port = 5672,
		const std::string &username = "guest", const std::string &password = "guest",
		const std::string &vhost = "/");

	bool open(const std::string &host, uint16_t port);

	/**
	 * Distribute received envelope to Channel
	 * @param envelope
	 * @param channel_id
	 * @return
	 */
	bool distribute_envelope(EnvelopePtr envelope, uint16_t channel_id);

	amqp_socket_t *socket{nullptr};

	uint32_t m_heartbeat_interval{0};
	int32_t m_frame_max{131072};
	uint64_t m_wait_timeout_ms{0};

	amqp_channel_t m_next_channel_id{0};
	std::unordered_map<uint16_t, std::shared_ptr<Channel>> m_channels;

	bool m_stop{false};
};

}
}
