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

#include "caldavclient.h"
#include <iostream>
#include <regex>

static const std::string calendar_list_request = "<d:propfind xmlns:d=\"DAV:\" "
						 "xmlns:cs=\"http://calendarserver.org/ns/\" "
						 "xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
						 "  <d:prop>"
						 "     <d:resourcetype></d:resourcetype>"
						 "     <d:displayname />"
						 "     <cs:getctag />"
						 "     <c:supported-calendar-component-set />"
						 "  </d:prop>"
						 "</d:propfind>";

CaldavClient::CaldavClient(const std::string &url, const std::string &username,
			   const std::string &password, bool dont_verify_peer)
    : HTTPClient(10 * 1024 * 1024), m_url(url), m_dont_verify_peer(dont_verify_peer)
{
	m_username = username;
	m_password = password;
	load_calendars();
}

void CaldavClient::load_calendars()
{
	std::string res = "";
	int32_t reqflag = HTTPClient::REQ_SIMPLE | HTTPClient::REQ_AUTH;
	if (m_dont_verify_peer) {
		reqflag |= HTTPClient::REQ_NO_VERIFY_PEER;
	}

	add_http_header("Content-Type", "application/xml; charset=utf-8");
	add_http_header("Depth", "1");
	add_http_header("Prefer", "return-minimal");
	_propfind(m_url + "/calendars/" + m_username + "/", res, reqflag, calendar_list_request);

	reqflag |= XMLParser::Flag::FLAG_XML_SIMPLE | XMLParser::Flag::FLAG_XML_STRIP_NEWLINE;

	XMLParser parser;
	parser.register_ns(XMLParserCustomNs("d", "DAV:"));
	parser.register_ns(XMLParserCustomNs("s", "http://sabredav.org/ns"));
	parser.register_ns(XMLParserCustomNs("cal", "urn:ietf:params:xml:ns:caldav"));
	parser.register_ns(XMLParserCustomNs("cs", "http://calendarserver.org/ns/"));

	m_calendars.clear();

	std::vector<std::string> parse_res;
	parser.parse(res, "//d:response/d:href", reqflag, parse_res);
	for (const auto &s : parse_res) {
		std::smatch rem;
		const std::regex re_calendar_name("/calendars/" + m_username + "/(.+)/$");
		if (std::regex_search(s, rem, re_calendar_name)) {
			m_calendars.push_back(rem.str(1));
		}
	}
}
