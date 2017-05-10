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

#include "httpresponse.h"
#include "httpserver.h"

namespace winterwind
{
namespace http
{
HTTPResponse &HTTPResponse::operator<<(const std::string &r)
{
	m_response += r;
	return *this;
}

HTTPResponse &HTTPResponse::operator>>(std::string &r)
{
	r = m_response;
	return *this;
}

HTTPResponse &HTTPResponse::operator>>(HTTPRequestSession &s)
{
	s.http_code = m_http_code;
	s.result = m_response;
	return *this;
}

JSONHTTPResponse &JSONHTTPResponse::operator<<(const Json::Value &r)
{
	m_json_response = r;
	return *this;
}

HTTPResponse &JSONHTTPResponse::operator>>(std::string &r)
{
	r = Json::FastWriter().write(m_json_response);
	return *this;
}

JSONHTTPResponse &JSONHTTPResponse::operator>>(Json::Value &r)
{
	r = m_json_response;
	return *this;
}

HTTPResponse &JSONHTTPResponse::operator>>(HTTPRequestSession &s)
{
	s.result = Json::FastWriter().write(m_json_response);
	s.http_code = m_http_code;
	return *this;
}
}
}
