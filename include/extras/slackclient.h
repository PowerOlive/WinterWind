/*
 * Copyright (c) 2016-2017, Loic Blot <loic.blot@unix-experience.fr>
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

#include <core/httpclient.h>
#include <core/utils/threads.h>
#include <string>

#define ASIO_STANDALONE
#include <websocketpp/client.hpp>
// Not very optimized thing due to OpenSSL 1.1 not supporte by Asio TLS
#ifndef SSL_R_SHORT_READ
#define SSL_R_SHORT_READ 219
#endif
#include <websocketpp/config/asio_client.hpp>

typedef std::function<bool(const Json::Value &req, websocketpp::connection_hdl &hdl)> SlackMessageHandler;

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_tls_client;
class SlackClient : protected Thread, protected winterwind::HTTPClient,
	private ws_tls_client
{
public:
	SlackClient(const std::string &api_token);
	virtual ~SlackClient();

	bool auth_test();

	bool create_channel(const std::string &channel);
	bool join_channel(const std::string &channel);

	bool post_message(const std::string &channel, const std::string &text);

	virtual void *run();

	bool register_callback(const std::string &method, const SlackMessageHandler &hdl);
	void send(websocketpp::connection_hdl hdl, const Json::Value &msg);
	void send_message(websocketpp::connection_hdl hdl, const std::string &channel, const std::string &msg);

	const std::string &get_channel_id_by_name(const std::string &name) const {
		static const std::string noname = "";
		const auto &chan = m_groups_reverse.find("name");
		if (chan != m_groups_reverse.end()) {
			return chan->second;
		}
		return noname;
	}

	void set_retry_on_fail(const bool retry) { m_retry_on_fail = retry; }
	void set_retry_interval(const uint32_t s) { m_retry_interval = s; }

private:
	typedef std::unordered_map<std::string, std::vector<SlackMessageHandler>> SlackMessageHdlMap;

	bool rtm_start(Json::Value &res);
	std::string m_api_token;

	uint32_t m_internal_message_id = 1;

	SlackMessageHdlMap m_callbacks = {};
	// groups, by id
	std::unordered_map<std::string, std::string> m_groups = {};
	// groups, by name
	std::unordered_map<std::string, std::string> m_groups_reverse = {};

	bool m_retry_on_fail = false;
	uint32_t m_retry_interval = 30;
};
