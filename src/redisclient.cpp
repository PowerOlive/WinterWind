/**
 * Copyright (c) 2016, Loic Blot <loic.blot@unix-experience.fr>
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

#include "redisclient.h"
#include <iostream>

RedisClient::RedisClient(const std::string &host, const uint16_t port, const uint32_t cb_interval)
{
	m_host = host;
	m_port = port;
	m_circuit_breaker_interval = cb_interval;
	connect();
}

RedisClient::~RedisClient()
{
	if (m_context) {
		redisFree(m_context);
	}
}

void RedisClient::connect()
{
	// Circuit breaker
	if ((time(NULL) - m_last_failed_connection) < m_circuit_breaker_interval) {
		return;
	}

	if (m_context) {
		redisFree(m_context);
	}

	m_context = redisConnect(m_host.c_str(), m_port);
	if (m_context == NULL || m_context->err) {
		// Set last failed connection for circuit breaker
		m_last_failed_connection = time(NULL);
		if (m_context) {
			std::cerr << "Redis " << __FUNCTION__ << " error: " << m_context->errstr
				  << std::endl;
			redisFree(m_context);
			m_context = nullptr;
			return;
		} else {
			std::cerr << "Unable to set redis context." << std::endl;
		}
	}

	redisEnableKeepAlive(m_context);
}

// This function permits to test connect & reconnect or fail
#define REDIS_CONNECT                                                                              \
	if (!m_context) {                                                                          \
		connect();                                                                         \
		if (!m_context) {                                                                  \
			return false;                                                              \
		}                                                                                  \
	}

#define REDIS_REPLY_HANDLER                                                                        \
	if (!reply) {                                                                              \
		std::cerr << "Redis " << __FUNCTION__ << " error: " << m_context->errstr           \
			  << std::endl;                                                            \
		return false;                                                                      \
	}

bool RedisClient::type(const std::string &key, std::string &res)
{
	REDIS_CONNECT;
	redisReply *reply = (redisReply *) redisCommand(m_context, "TYPE %s", key.c_str());
	REDIS_REPLY_HANDLER;
	switch (reply->type) {
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_ERROR:
		case REDIS_REPLY_STATUS:
			res = std::string(reply->str, (unsigned long) reply->len);
			break;
		case REDIS_REPLY_NIL:
			freeReplyObject(reply);
			return false;
		default:
			std::cerr << "Redis " << __FUNCTION__ << " error: unhandled response type "
				  << reply->type << std::endl;
			freeReplyObject(reply);
			return false;
	}

	freeReplyObject(reply);
	return true;
}

bool RedisClient::set(const std::string &key, const std::string &value, const uint32_t expire_value)
{
	REDIS_CONNECT;
	redisReply *reply =
	    (redisReply *) redisCommand(m_context, "SET %s %s", key.c_str(), value.c_str());
	REDIS_REPLY_HANDLER;
	freeReplyObject(reply);
	return expire_value ? expire(key, expire_value) : true;
}

bool RedisClient::get(const std::string &key, std::string &res)
{
	REDIS_CONNECT;
	redisReply *reply = (redisReply *) redisCommand(m_context, "GET %s", key.c_str());
	REDIS_REPLY_HANDLER;
	switch (reply->type) {
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_ERROR:
			res = std::string(reply->str, (unsigned long) reply->len);
			break;
		case REDIS_REPLY_NIL:
			freeReplyObject(reply);
			return false;
		default:
			std::cerr << "Redis " << __FUNCTION__ << " error: unhandled response type "
				  << reply->type << std::endl;
			freeReplyObject(reply);
			return false;
	}

	freeReplyObject(reply);
	return true;
}

bool RedisClient::del(const std::string &key)
{
	REDIS_CONNECT;
	redisReply *reply = (redisReply *) redisCommand(m_context, "DEL %s", key.c_str());
	REDIS_REPLY_HANDLER;
	freeReplyObject(reply);
	return true;
}

bool RedisClient::expire(const std::string &key, const uint32_t value)
{
	REDIS_CONNECT;
	redisReply *reply =
	    (redisReply *) redisCommand(m_context, "EXPIRE %s %d", key.c_str(), value);
	REDIS_REPLY_HANDLER;
	freeReplyObject(reply);
	return true;
}

bool RedisClient::hset(const std::string &key, const std::string &skey, const std::string &value,
		       const uint32_t expire_value)
{
	REDIS_CONNECT;
	// Verify type & del wrong key type
	std::string type_verification = "";
	type(key, type_verification);
	if (type_verification.compare("hash") != 0 && type_verification.compare("none") != 0) {
		del(key);
	}

	redisReply *reply = (redisReply *) redisCommand(m_context, "HSET %s %s %s", key.c_str(),
							skey.c_str(), value.c_str());
	REDIS_REPLY_HANDLER;
	freeReplyObject(reply);
	return expire_value == 0 || expire(key, expire_value);
}

bool RedisClient::hdel(const std::string &key, const std::string &skey)
{
	REDIS_CONNECT;
	redisReply *reply =
	    (redisReply *) redisCommand(m_context, "HDEL %s %s", key.c_str(), skey.c_str());
	REDIS_REPLY_HANDLER;
	freeReplyObject(reply);
	return true;
}

bool RedisClient::hget(const std::string &key, const std::string &skey, std::string &res)
{
	REDIS_CONNECT;
	redisReply *reply =
	    (redisReply *) redisCommand(m_context, "HGET %s %s", key.c_str(), skey.c_str());
	REDIS_REPLY_HANDLER;
	switch (reply->type) {
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_ERROR:
			res = std::string(reply->str, (unsigned long) reply->len);
			break;
		case REDIS_REPLY_NIL:
			freeReplyObject(reply);
			return false;
		default:
			std::cerr << "Redis " << __FUNCTION__ << " error: unhandled response type "
				  << reply->type << std::endl;
			freeReplyObject(reply);
			return false;
	}

	freeReplyObject(reply);
	return true;
}

bool RedisClient::hkeys(const std::string &key, std::vector<std::string> &res)
{
	REDIS_CONNECT;
	redisReply *reply = (redisReply *) redisCommand(m_context, "HKEYS %s", key.c_str());
	REDIS_REPLY_HANDLER;
	switch (reply->type) {
		case REDIS_REPLY_ERROR:
			std::cerr << "Redis " << __FUNCTION__ << " error: "
				  << std::string(reply->str, (unsigned long) reply->len)
				  << std::endl;
			break;
		case REDIS_REPLY_ARRAY:
			for (int i = 0; i < reply->elements; i++) {
				res.push_back(std::string(reply->element[i]->str,
							  (unsigned long) reply->element[i]->len));
			}
			break;
		case REDIS_REPLY_NIL:
			freeReplyObject(reply);
			return false;
		default:
			std::cerr << "Redis " << __FUNCTION__ << " error: unhandled response type "
				  << reply->type << std::endl;
			freeReplyObject(reply);
			return false;
	}

	freeReplyObject(reply);
	return true;
}
