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

#include <string>
#include <core/utils/log.h>

extern log4cplus::Logger irc_log;

typedef struct irc_session_s irc_session_t;

class IRCClient
{
public:
	IRCClient();
	~IRCClient() {};

	/**
	 * @return the connecting status
	 */
	const bool is_connected() const;

	void join_channel(const std::string &channel);
	void leave_channel(const std::string &channel);
	void send_message(const std::string &channel, const std::string &what);
	void send_notice(const std::string &channel, const std::string &what);
	bool send_ctcp_ping(const std::string &who);

	virtual void on_event_connect(const std::string &origin,
		const std::vector<std::string> &params) {}
	virtual void on_event_join(const std::string &origin,
		const std::vector<std::string> &params) {}
	virtual void on_event_part(const std::string &origin,
		const std::vector<std::string> &params) {}

	virtual void on_event_topic(const std::string &origin,
		const std::vector<std::string> &params) {}

	virtual void on_event_notice(const std::string &origin,
		const std::vector<std::string> &params) {}
	virtual void on_event_channel_notice(const std::string &origin,
		const std::vector<std::string> &params) {}

	virtual void on_event_message(const std::string &origin,
		const std::vector<std::string> &params) {}
	virtual void on_event_privmsg(const std::string &origin,
		const std::vector<std::string> &params) {}

	virtual void on_event_kick(const std::string &origin,
		const std::vector<std::string> &params) {}

	virtual void on_event_nick(const std::string &origin,
		const std::vector<std::string> &params) {}
	virtual void on_event_quit(const std::string &origin,
		const std::vector<std::string> &params) {}

	virtual void on_event_numeric(uint32_t event_id, const std::string &origin,
		const std::vector<std::string> &params) {}
protected:
	bool create_session();
	irc_session_t *m_irc_session = nullptr;
};
