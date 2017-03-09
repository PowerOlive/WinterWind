/**
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

#include <httpclient.h>

struct RATPSchedule
{
	std::string destination = "";
	std::string hour = "";
};

typedef std::vector<RATPSchedule> RATPScheduleList;

struct RATPStop
{
	std::string train_stop = "";
	time_t entry_time = 0;
	std::vector<RATPSchedule> schedules = {};
};

typedef std::unordered_map<std::string, RATPStop> RATPStopMap;

class RATPClient : private HTTPClient
{
public:
	enum Line
	{
		LINE_RER_A,
		LINE_RER_B,
		RATP_LINE_MAX,
	};

	RATPClient() : HTTPClient(){};
	~RATPClient(){};

	const RATPScheduleList &get_next_trains(const RATPClient::Line line, const std::string &stop,
						const uint8_t direction);

private:
	std::map<RATPClient::Line, RATPStopMap> m_stop_cache;
};
