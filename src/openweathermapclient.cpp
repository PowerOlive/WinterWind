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

#include <cmath>
#include <iostream>
#include "openweathermapclient.h"

Weather &Weather::operator>>(Json::Value &res)
{
	res["city"] = city;
	res["description"] = weather_desc;
	res["temperature"] = temperature;
	res["humidity"] = (int) humidity;
	res["sunrise"] = (int) sunrise;
	res["sunset"] = (int) sunset;
	return *this;
}
OpenWeatherMapReturnCode OpenWeatherMapClient::get(const std::string &city, Weather &result)
{
	// @TODO: add a cache
	Json::Value res;
	std::string city_encoded = "";
	http_string_escape(city, city_encoded);
	if (!_get_json("http://api.openweathermap.org/data/2.5/"
			"weather?mode=json&APPID=" + m_api_key +
			"&units=metric&q=" + city_encoded, res)) {
		return OWMRC_INVALID_RESPONSE;
	}

	try {
		if (!res.isMember("cod")) {
			return OWMRC_INVALID_REQUEST;
		}

		switch (res["cod"].asUInt64()) {
			case 200: break;
			case 401:
				return OWMRC_FORBIDDEN;
			default:
				return OWMRC_UNKNOWN;
		}

		if (!res.isMember("main") || !res["main"].isMember("temp")) {
			return OWMRC_INVALID_RESPONSE;
		}

		result.temperature = std::round(res["main"]["temp"].asFloat() * 100) / 100;

		if (res.isMember("weather") && res["weather"].isArray() &&
			res["weather"].isValidIndex(0) && res["weather"][0].isMember("main") &&
			res["weather"][0]["main"].isString()) {
			result.weather_desc = res["weather"][0]["main"].asString();
		}

		if (res["main"].isMember("humidity")) {
			result.humidity = (uint8_t) res["main"]["humidity"].asUInt();
		}

		if (res.isMember("sys") && res["sys"].isMember("sunrise") &&
			res["sys"].isMember("sunset")) {
			result.sunrise = res["sys"]["sunrise"].asInt();
			result.sunset = res["sys"]["sunset"].asInt();
		}

		result.city = city;
		return OWMRC_OK;
	}
	catch (Json::LogicError &e) {
		return OWMRC_INVALID_RESPONSE;
	}
}
