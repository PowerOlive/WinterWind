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
#include <vector>
#include <string>
#include <amqp.h>
#include <ctime>

namespace winterwind
{
namespace amqp
{

class Channel;

class Message
{
	friend class Channel;
public:
	explicit Message(std::vector<uint8_t> data) : m_data(std::move(data)) {}
	explicit Message(const std::string &data);
	explicit Message(amqp_message_t *message);
	~Message();

	/**
	 * Flag message as mandatory, indicating to the broker that the message MUST be routed
 	 * to a queue. If the broker cannot do this it should respond with
 	 * a basic.return method.
	 * @param mandatory
	 * @return Reference to this object
	 */
	Message &set_mandatory(bool mandatory)
	{
		m_mandatory = mandatory;
		return *this;
	}

	/**
	 * Flag message as immediate indicating to the broker that the message MUST be
	 * delivered to a consumer immediately. If the broker cannot do this it should
 	 * response with a basic.return method.
	 * @param immediate
	 * @return Reference to this object
	 */
	Message &set_immediate(bool immediate)
	{
		m_immediate = immediate;
		return *this;
	}

	Message &set_content_type(const std::string &content_type);
	std::string get_content_type() const;

	Message &set_content_encoding(const std::string &content_encoding);
	std::string get_content_encoding() const;

	// @TODO Message &set_headers(const std::unordered_map<std::string, std::string> &headers);
	Message &set_delivery_mode(uint8_t delivery_mode);
	/**
	 * @return message delivery mode. If not set returns UINT8_MAX
	 */
	uint8_t get_delivery_mode() const;

	Message &set_priority(uint8_t priority);
	/**
	 * @return message priority. If not set returns UINT8_MAX
	 */
	uint8_t get_priority() const;

	Message &set_correlation_id(const std::string &correlation_id);
	std::string get_correlation_id() const;

	Message &set_reply_to(const std::string &reply_to);
	std::string get_reply_to() const;

	Message &set_expiration(uint64_t expiration_ms);
	int64_t get_expiration() const;

	Message &set_message_id(const std::string &message_id);
	std::string get_message_id() const;

	Message &set_timestamp(int64_t timestamp = std::time(nullptr));
	std::time_t get_timestamp() const;

	Message &set_type(const std::string &type);
	std::string get_type() const;

	Message &set_user_id(const std::string &user_id);
	std::string get_user_id() const;

	Message &set_app_id(const std::string &app_id);
	std::string get_app_id() const;

	Message &set_cluster_id(const std::string &cluster_id);
	std::string get_cluster_id() const;

	const std::vector<uint8_t> &get_data() const
	{
		return m_data;
	}

private:
	// Internal function to set expiration as string
	Message &set_expiration(const std::string &expiration);
	std::vector<uint8_t> m_data;

	bool m_mandatory{true};
	bool m_immediate{false};

	amqp_basic_properties_t m_properties{};
};
}
}
