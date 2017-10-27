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

#include "amqp/message.h"
#include <cstring>
#include <iostream>

namespace winterwind
{
namespace amqp
{

#define TO_AMQP_STR(s) \
	amqp_bytes_malloc_dup(amqp_cstring_bytes(s.c_str()))

#define TO_CPP_STR(o) \
	std::string((const char *) o.bytes, o.len)

Message::Message(const std::string &data)
{
	m_data.resize(data.size());
	std::memcpy(&m_data[0], &data[0], data.size());
}

Message::Message(amqp_message_t *message)
{
	m_data.resize(message->body.len);
	std::memcpy(&m_data[0], message->body.bytes, message->body.len);

	if (message->properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG)
		set_content_type(TO_CPP_STR(message->properties.content_type));

	if (message->properties._flags & AMQP_BASIC_CONTENT_ENCODING_FLAG)
		set_content_encoding(TO_CPP_STR(message->properties.content_encoding));

	if (message->properties._flags & AMQP_BASIC_DELIVERY_MODE_FLAG)
		set_delivery_mode(message->properties.delivery_mode);

	if (message->properties._flags & AMQP_BASIC_PRIORITY_FLAG)
		set_priority(message->properties.priority);

	if (message->properties._flags & AMQP_BASIC_CORRELATION_ID_FLAG)
		set_correlation_id(TO_CPP_STR(message->properties.correlation_id));

	if (message->properties._flags & AMQP_BASIC_REPLY_TO_FLAG)
		set_reply_to(TO_CPP_STR(message->properties.reply_to));

	if (message->properties._flags & AMQP_BASIC_EXPIRATION_FLAG)
		set_expiration(TO_CPP_STR(message->properties.expiration));

	if (message->properties._flags & AMQP_BASIC_MESSAGE_ID_FLAG)
		set_message_id(TO_CPP_STR(message->properties.message_id));

	if (message->properties._flags & AMQP_BASIC_TIMESTAMP_FLAG)
		set_timestamp(message->properties.timestamp);

	if (message->properties._flags & AMQP_BASIC_TYPE_FLAG)
		set_type(TO_CPP_STR(message->properties.type));

	if (message->properties._flags & AMQP_BASIC_USER_ID_FLAG)
		set_user_id(TO_CPP_STR(message->properties.user_id));

	if (message->properties._flags & AMQP_BASIC_APP_ID_FLAG)
		set_app_id(TO_CPP_STR(message->properties.app_id));

	if (message->properties._flags & AMQP_BASIC_CLUSTER_ID_FLAG)
		set_cluster_id(TO_CPP_STR(message->properties.cluster_id));
}

Message::~Message()
{
	if (m_properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG)
		amqp_bytes_free(m_properties.content_type);

	if (m_properties._flags & AMQP_BASIC_CONTENT_ENCODING_FLAG)
		amqp_bytes_free(m_properties.content_encoding);

	if (m_properties._flags & AMQP_BASIC_CORRELATION_ID_FLAG)
		amqp_bytes_free(m_properties.correlation_id);

	if (m_properties._flags & AMQP_BASIC_REPLY_TO_FLAG)
		amqp_bytes_free(m_properties.reply_to);

	if (m_properties._flags & AMQP_BASIC_EXPIRATION_FLAG)
		amqp_bytes_free(m_properties.expiration);

	if (m_properties._flags & AMQP_BASIC_MESSAGE_ID_FLAG)
		amqp_bytes_free(m_properties.message_id);

	if (m_properties._flags & AMQP_BASIC_TYPE_FLAG)
		amqp_bytes_free(m_properties.type);

	if (m_properties._flags & AMQP_BASIC_USER_ID_FLAG)
		amqp_bytes_free(m_properties.user_id);

	if (m_properties._flags & AMQP_BASIC_APP_ID_FLAG)
		amqp_bytes_free(m_properties.app_id);

	if (m_properties._flags & AMQP_BASIC_CLUSTER_ID_FLAG)
		amqp_bytes_free(m_properties.cluster_id);
}

Message& Message::set_content_type(const std::string &content_type)
{
	m_properties._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;
	m_properties.content_type = TO_AMQP_STR(content_type);
	return *this;
}

std::string Message::get_content_type() const
{
	if (m_properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG)
		return TO_CPP_STR(m_properties.content_type);

	return "";
}

Message& Message::set_content_encoding(const std::string &content_encoding)
{
	m_properties._flags |= AMQP_BASIC_CONTENT_ENCODING_FLAG;
	m_properties.content_encoding = TO_AMQP_STR(content_encoding);
	return *this;
}

std::string Message::get_content_encoding() const
{
	if (m_properties._flags & AMQP_BASIC_CONTENT_ENCODING_FLAG)
		return TO_CPP_STR(m_properties.content_encoding);

	return "";
}

Message& Message::set_delivery_mode(uint8_t delivery_mode)
{
	m_properties._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
	m_properties.delivery_mode = delivery_mode;
	return *this;
}

uint8_t Message::get_delivery_mode() const
{
	if (m_properties._flags & AMQP_BASIC_DELIVERY_MODE_FLAG)
		return m_properties.delivery_mode;

	return UINT8_MAX;
}

Message& Message::set_priority(uint8_t priority)
{
	m_properties._flags |= AMQP_BASIC_PRIORITY_FLAG;
	m_properties.priority = priority;
	return *this;
}

uint8_t Message::get_priority() const
{
	if (m_properties._flags & AMQP_BASIC_PRIORITY_FLAG)
		return m_properties.priority;

	return UINT8_MAX;
}

Message& Message::set_correlation_id(const std::string &correlation_id)
{
	m_properties._flags |= AMQP_BASIC_CORRELATION_ID_FLAG;
	m_properties.correlation_id = TO_AMQP_STR(correlation_id);
	return *this;
}

std::string Message::get_correlation_id() const
{
	if (m_properties._flags & AMQP_BASIC_CORRELATION_ID_FLAG)
		return TO_CPP_STR(m_properties.correlation_id);

	return "";
}

Message& Message::set_reply_to(const std::string &reply_to)
{
	m_properties._flags |= AMQP_BASIC_REPLY_TO_FLAG;
	m_properties.reply_to = TO_AMQP_STR(reply_to);
	return *this;
}

std::string Message::get_reply_to() const
{
	if (m_properties._flags & AMQP_BASIC_REPLY_TO_FLAG)
		return TO_CPP_STR(m_properties.reply_to);

	return "";
}

Message& Message::set_expiration(uint64_t expiration_ms)
{
	set_expiration(std::to_string(expiration_ms));
	return *this;
}

Message& Message::set_expiration(const std::string &expiration)
{
	m_properties._flags |= AMQP_BASIC_EXPIRATION_FLAG;
	m_properties.expiration = TO_AMQP_STR(expiration);
	return *this;
}

int64_t Message::get_expiration() const
{
	if (m_properties._flags & AMQP_BASIC_EXPIRATION_FLAG)
		return std::atoll(TO_CPP_STR(m_properties.expiration).c_str());

	return 0;
}

Message& Message::set_message_id(const std::string &message_id)
{
	m_properties._flags |= AMQP_BASIC_MESSAGE_ID_FLAG;
	m_properties.message_id = TO_AMQP_STR(message_id);
	return *this;
}

std::string Message::get_message_id() const
{
	if (m_properties._flags & AMQP_BASIC_MESSAGE_ID_FLAG)
		return TO_CPP_STR(m_properties.message_id);

	return "";
}

Message& Message::set_timestamp(uint64_t timestamp)
{
	m_properties._flags |= AMQP_BASIC_TIMESTAMP_FLAG;
	m_properties.timestamp = timestamp;
	return *this;
}

std::time_t Message::get_timestamp() const
{
	if (m_properties._flags & AMQP_BASIC_TIMESTAMP_FLAG)
		return m_properties.timestamp;

	return 0;
}

Message& Message::set_type(const std::string &type)
{
	m_properties._flags |= AMQP_BASIC_TYPE_FLAG;
	m_properties.type = TO_AMQP_STR(type);
	return *this;
}

std::string Message::get_type() const
{
	if (m_properties._flags & AMQP_BASIC_TYPE_FLAG)
		return TO_CPP_STR(m_properties.type);

	return "";
}

Message& Message::set_user_id(const std::string &user_id)
{
	m_properties._flags |= AMQP_BASIC_USER_ID_FLAG;
	m_properties.user_id = TO_AMQP_STR(user_id);
	return *this;
}

std::string Message::get_user_id() const
{
	if (m_properties._flags & AMQP_BASIC_USER_ID_FLAG)
		return TO_CPP_STR(m_properties.user_id);

	return "";
}

Message& Message::set_app_id(const std::string &app_id)
{
	m_properties._flags |= AMQP_BASIC_APP_ID_FLAG;
	m_properties.app_id = TO_AMQP_STR(app_id);
	return *this;
}

std::string Message::get_app_id() const
{
	if (m_properties._flags & AMQP_BASIC_APP_ID_FLAG)
		return TO_CPP_STR(m_properties.app_id);

	return "";
}

Message& Message::set_cluster_id(const std::string &cluster_id)
{
	m_properties._flags |= AMQP_BASIC_CLUSTER_ID_FLAG;
	m_properties.cluster_id = TO_AMQP_STR(cluster_id);
	return *this;
}

std::string Message::get_cluster_id() const
{
	if (m_properties._flags & AMQP_BASIC_CLUSTER_ID_FLAG)
		return TO_CPP_STR(m_properties.cluster_id);

	return "";
}
}
}
