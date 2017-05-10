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

#include <core/httpclient.h>

namespace winterwind
{
namespace extras
{
struct Weather
{
	std::string city = "";
	std::string weather_desc = "";
	float temperature = 0.0f;
	uint8_t humidity = 0;
	time_t sunrise = 0;
	time_t sunset = 0;

	Weather &operator>>(Json::Value &res);
};

enum OpenWeatherMapReturnCode
{
	OWMRC_OK,
	OWMRC_FORBIDDEN,
	OWMRC_UNKNOWN,
	OWMRC_INVALID_REQUEST,
	OWMRC_INVALID_RESPONSE,
};

class OpenWeatherMapClient : public HTTPClient
{
public:
	OpenWeatherMapClient(const std::string &api_key) : m_api_key(api_key)
	{}

	~OpenWeatherMapClient()
	{}

	OpenWeatherMapReturnCode get(const std::string &city, Weather &result);

	void set_api_key(const std::string &api_key)
	{ m_api_key = api_key; }

private:
	std::string m_api_key = "";
};
}
}
