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

#include "slackclient.h"

SlackClient::SlackClient(const std::string &api_token) : Thread(), m_api_token(api_token)
{
	init_asio();
}

SlackClient::~SlackClient() {}

bool SlackClient::auth_test()
{
	Json::Value res;
	_get_json("https://slack.com/api/auth.test?token=" + m_api_token, res);
	return res.isMember("ok") && res["ok"].asBool();
}

bool SlackClient::create_channel(const std::string &channel)
{
	Json::Value res;
	_get_json("https://slack.com/api/channels.create?token=" + m_api_token + "&name=" + channel,
		  res);
	return res.isMember("ok") && res["ok"].asBool();
}

bool SlackClient::join_channel(const std::string &channel)
{
	Json::Value res;
	_get_json("https://slack.com/api/channels.join?token=" + m_api_token + "&channel=" +
		      channel,
		  res);
	return res.isMember("ok") && res["ok"].asBool();
}

bool SlackClient::post_message(const std::string &channel, const std::string &message)
{
	Json::Value res;
	_get_json("https://slack.com/api/channels.join?token=" + m_api_token + "&channel=" +
		      channel + "&text=" + message + "&as_user=true&username=Icecrown",
		  res);
	return res.isMember("ok") && res["ok"].asBool();
}

bool SlackClient::rtm_start(Json::Value &res)
{
	_get_json("https://slack.com/api/rtm.start?token=" + m_api_token, res);
	return res.isMember("ok") && res["ok"].asBool();
}

void *SlackClient::run()
{
	Thread::set_thread_name("SlackClient");
	ThreadStarted();

	uint32_t failure_number = 0;

	while (!stopRequested()) {
		Json::Value res;
		if (!rtm_start(res)) {
			failure_number++;
			std::cerr << "Failed to start RTM with slack, retrying in "
					<< (m_retry_interval * failure_number) << "sec..." << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(m_retry_interval * failure_number));
			continue;
		}

		if (!res.isMember("url")) {
			failure_number++;
			std::cerr << "WSS URL not present in slack response, retrying in " << (m_retry_interval * failure_number)
					<< "sec..." << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(m_retry_interval * failure_number));
			continue;
		}

		if (res.isMember("groups")) {
			for (const auto &group : res["groups"]) {
				if (group.isMember("id") && group.isMember("name")) {
					m_groups[group["id"].asString()] = group["name"].asString();
					m_groups_reverse[group["name"].asString()] =
					    group["id"].asString();
				}
			}
		}

		try {
			set_tls_init_handler([this](websocketpp::connection_hdl) {
				SSL_library_init();
				return websocketpp::lib::make_shared<asio::ssl::context>(
						asio::ssl::context::tlsv12_client);
			});

			set_access_channels(websocketpp::log::alevel::connect |
					    websocketpp::log::alevel::disconnect |
					    websocketpp::log::alevel::fail);

			set_message_handler(
			    [this](websocketpp::connection_hdl hdl,
				   websocketpp::config::asio_client::message_type::ptr msg) {
				    Json::Value recv_msg;

				    if (!this->json_reader()->parse(msg->get_payload(), recv_msg)) {
					    std::cerr << "Invalid slack payload, ignoring."
						      << std::endl;
					    return;
				    }

				    // Ignore some payload not containing type
				    if (!recv_msg.isMember("type")) {
					    return;
				    }

				    const std::string &type = recv_msg["type"].asString();
				    if (type == "reconnect_url") {
					    // This should be handled when slack will use it
					    return;
				    }

				    // Send callbacks
				    const auto &callback_list = m_callbacks.find(type);
				    if (callback_list != m_callbacks.end()) {
					    for (const auto &cb : callback_list->second) {
						    cb(recv_msg, hdl);
					    }
				    }
			    });

			websocketpp::lib::error_code ec;
			ws_tls_client::connection_ptr con =
			    get_connection(res["url"].asString(), ec);
			if (ec) {
				std::cout << "could not create connection because: " << ec.message()
					  << std::endl;
				return nullptr;
			}

			connect(con);
			failure_number = 0;
			ws_tls_client::run();
		} catch (websocketpp::exception const &e) {
			std::cerr << "ERROR handler exception" << std::endl
				  << e.what() << std::endl;
		}

		failure_number++;
		std::cerr << "SlackClient failure, retrying in " << (m_retry_interval * failure_number) << "sec..."
			  << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(m_retry_interval * failure_number));
	}

	return nullptr;
}

bool SlackClient::register_callback(const std::string &method, const SlackMessageHandler &hdl)
{
	if (m_callbacks.find(method) == m_callbacks.end()) {
		m_callbacks[method] = {};
	}

	m_callbacks[method].push_back(hdl);
	return true;
}

void SlackClient::send(websocketpp::connection_hdl hdl, const Json::Value &msg)
{
	websocketpp::lib::error_code ec;
	ws_tls_client::send(hdl, json_writer()->write(msg), websocketpp::frame::opcode::text, ec);
}

void SlackClient::send_message(websocketpp::connection_hdl hdl, const std::string &channel,
			       const std::string &msg)
{
	Json::Value json_msg;
	json_msg["id"] = m_internal_message_id;
	json_msg["type"] = "message";
	json_msg["text"] = msg;
	json_msg["channel"] = channel;

	send(hdl, json_msg);
	m_internal_message_id++;
}
