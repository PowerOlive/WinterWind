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

#pragma once

#include <json/json.h>
#include <string>

enum HTTPResponseType
{
	HTTPRESPONSE_RAW,
	HTTPRESPONSE_JSON,
};

struct HTTPRequestSession;

class HTTPResponse
{
public:
	HTTPResponse(const std::string &r, const uint16_t http_code = 200) : m_response(r), m_http_code(http_code) {}
	virtual ~HTTPResponse(){};

	virtual HTTPResponse &operator<<(const std::string &r);
	virtual HTTPResponse &operator>>(std::string &r);
	virtual HTTPResponse &operator>>(HTTPRequestSession &s);

	virtual const HTTPResponseType get_type() const { return HTTPRESPONSE_RAW; }

protected:
	HTTPResponse(const uint16_t http_code = 200) : m_http_code(http_code) {}
	uint16_t m_http_code = 200;

private:
	std::string m_response = "";
};

class JSONHTTPResponse : public HTTPResponse
{
public:
	JSONHTTPResponse() : HTTPResponse() {}
	JSONHTTPResponse(const Json::Value &r, const uint16_t http_code = 200)
	    : HTTPResponse(http_code), m_json_response(r) {}

	virtual ~JSONHTTPResponse() {}

	JSONHTTPResponse &operator<<(const Json::Value &r);
	virtual HTTPResponse &operator>>(std::string &r);
	JSONHTTPResponse &operator>>(Json::Value &r);
	virtual HTTPResponse &operator>>(HTTPRequestSession &s);

	virtual const HTTPResponseType get_type() const { return HTTPRESPONSE_JSON; }

private:
	Json::Value m_json_response;
};
