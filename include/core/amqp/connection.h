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

namespace winterwind
{
namespace amqp
{

class Channel;
class Message;

class Connection : non_copyable, public std::enable_shared_from_this<amqp::Connection>
{
	friend class Channel;
public:
	Connection(const std::string &url = "");
	Connection(const std::string &host, uint16_t port,
		const std::string &username = "guest", const std::string &password = "guest",
		const std::string &vhost = "/", int32_t frame_max = 131072);

	~Connection();

	bool login(const std::string &user, const std::string &password,
		const std::string &vhost, int32_t frame_max);

	int32_t get_channel_max();

	std::shared_ptr<Channel> create_channel();
	void destroy_channel(std::shared_ptr<Channel> channel);

protected:
	amqp_connection_state_t m_conn = nullptr;

	/**
	 * Get Channel object associated with ID
	 * @param channel_id
	 * @return Channel shared_ptr if object is associated to connection, else nullptr
	 */
	std::shared_ptr<Channel> find_channel(uint16_t channel_id);

private:
	void connect(const std::string &host = "127.0.0.1", uint16_t port = 5672,
		const std::string &username = "guest", const std::string &password = "guest",
		const std::string &vhost = "/", int32_t frame_max = 131072);

	bool open(const std::string &host, uint16_t port);

	amqp_socket_t *socket = nullptr;

	static const uint32_t HEARTBEAT_INTERVAL = 10;

	amqp_channel_t m_next_channel_id = 0;
	std::unordered_map<uint16_t, std::shared_ptr<Channel>> m_channels;
};

}
}
