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
#include <libircclient/libircclient.h>

void IRCClient::join_channel(const std::string &channel)
{
	if (irc_cmd_join(m_irc_session, channel.c_str(), NULL) != 0) {
		log_warn(irc_log, "IRC join channel error: "
			<< irc_strerror(irc_errno(m_irc_session)));
	}
}

void IRCClient::leave_channel(const std::string &channel)
{
	if (irc_cmd_part(m_irc_session, channel.c_str()) != 0) {
		log_warn(irc_log, "IRC leave channel error: "
			<< irc_strerror(irc_errno(m_irc_session)));
	}
}

void IRCClient::send_message(const std::string &channel, const std::string &what)
{
	if (!is_connected()) {
		log_error(irc_log, "Not connected to IRC " << __FUNCTION__);
		return;
	}

	irc_cmd_msg(m_irc_session, channel.c_str(), what.c_str());
}

void IRCClient::send_notice(const std::string &channel, const std::string &what)
{
	if (!is_connected()) {
		log_error(irc_log, "Not connected to IRC " << __FUNCTION__);
		return;
	}

	irc_cmd_notice(m_irc_session, channel.c_str(), what.c_str());
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
