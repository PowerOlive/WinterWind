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

#include "ircclient.h"
extern "C" {
#include <libircclient.h>
}

log4cplus::Logger irc_log = logger.getInstance(LOG4CPLUS_TEXT("irc"));

#define IRC_CB_EVENT(n) \
void on_irc_event_##n(irc_session_t *session, const char *, \
	const char *origin, const char **params, unsigned int count) \
{ \
	auto *client = (IRCClient *) irc_get_ctx(session); \
	std::vector<std::string> vparams; \
	for (uint32_t i = 0; i < count; i++) { \
		vparams.emplace_back(params[i]); \
	} \
	\
	client->on_event_##n(std::string(origin), vparams); \
}

void on_irc_event_numeric(irc_session_t *session, unsigned int event,
	const char *origin, const char **params, unsigned int count)
{
	auto *client = (IRCClient *) irc_get_ctx(session);
	std::vector<std::string> vparams;
	for (uint32_t i = 0; i < count; i++) {
		vparams.emplace_back(params[i]);
	}

	client->on_event_numeric(event, std::string(origin), vparams);
}

IRC_CB_EVENT(connect)
IRC_CB_EVENT(join)
IRC_CB_EVENT(part)
IRC_CB_EVENT(notice)
IRC_CB_EVENT(message)
IRC_CB_EVENT(privmsg)
IRC_CB_EVENT(channel_notice)
IRC_CB_EVENT(kick)
IRC_CB_EVENT(nick)
IRC_CB_EVENT(quit)
IRC_CB_EVENT(topic)

#define DECL_IRC_CB(n) \
	callbacks.event_##n = &on_irc_event_##n;

static bool irc_callbacks_inited = false;
static irc_callbacks_t callbacks = { 0 };

IRCClient::IRCClient()
{
	if (!irc_callbacks_inited) {
		DECL_IRC_CB(connect)
		DECL_IRC_CB(join)
		DECL_IRC_CB(part)
		DECL_IRC_CB(notice)
		callbacks.event_channel = &on_irc_event_message;
		DECL_IRC_CB(privmsg)
		DECL_IRC_CB(channel_notice)
		DECL_IRC_CB(kick)
		DECL_IRC_CB(nick)
		DECL_IRC_CB(quit)
		DECL_IRC_CB(numeric)
		DECL_IRC_CB(topic)
		irc_callbacks_inited = true;
	}
}

IRCClient::~IRCClient()
{
	destroy_session();
}

bool IRCClient::create_session()
{
	m_irc_session = irc_create_session(&callbacks);

	if (!m_irc_session) {
		return false;
	}

	irc_set_ctx(m_irc_session, this);
	return true;
}

bool IRCClient::destroy_session()
{
	if (m_irc_session) {
		irc_destroy_session(m_irc_session);
		return true;
	}

	return false;
}

bool IRCClient::run()
{
	if (!m_irc_session) {
		log_fatal(irc_log, std::string(__FUNCTION__) + " function called without valid "
			"session, aborting");
		return false;
	}

	return irc_run(m_irc_session) != 0;
}

bool IRCClient::connect(const std::string &server, uint16_t port,
	const std::string &nickname, const std::string &server_password,
	const std::string &username, const std::string &password)
{
	if (irc_connect(m_irc_session, server.c_str(), port,
		server_password.empty() ? nullptr : server_password.c_str(),
		nickname.c_str(),
		username.empty() ? nullptr : username.c_str(),
		password.empty() ? nullptr : password.c_str())) {
		std::stringstream ss;
		ss << "Unable to connect to IRC server " << server << ", aborting." << std::endl;
		log_error(irc_log, ss.str());
		return false;
	}

	return true;
}

bool IRCClient::join_channel(const std::string &channel)
{
	if (irc_cmd_join(m_irc_session, channel.c_str(), NULL) != 0) {
		log_warn(irc_log, "IRC join channel error: "
			<< irc_strerror(irc_errno(m_irc_session)));
		return false;
	}

	return true;
}

bool IRCClient::leave_channel(const std::string &channel)
{
	if (irc_cmd_part(m_irc_session, channel.c_str()) != 0) {
		log_warn(irc_log, "IRC leave channel error: "
			<< irc_strerror(irc_errno(m_irc_session)));
		return false;
	}

	return true;
}

bool IRCClient::send_message(const std::string &channel, const std::string &what)
{
	if (!is_connected()) {
		log_error(irc_log, "Not connected to IRC " << __FUNCTION__);
		return false;
	}

	return irc_cmd_msg(m_irc_session, channel.c_str(), what.c_str()) != 0;
}

bool IRCClient::send_notice(const std::string &channel, const std::string &what)
{
	if (!is_connected()) {
		log_error(irc_log, "Not connected to IRC " << __FUNCTION__);
		return false;
	}

	return irc_cmd_notice(m_irc_session, channel.c_str(), what.c_str()) != 0;
}

bool IRCClient::send_ctcp_ping(const std::string &who)
{
	if (!is_connected()) {
		log_error(irc_log, "Not connected to IRC " << __FUNCTION__);
		return false;
	}

	return irc_cmd_ctcp_request(m_irc_session, who.c_str(), "PING") == 0;
}

const bool IRCClient::is_connected() const
{
	return m_irc_session && irc_is_connected(m_irc_session);
}
