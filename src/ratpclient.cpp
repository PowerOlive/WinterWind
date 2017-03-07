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

#include "ratpclient.h"
#include <algorithm>
#include <cassert>

struct RATPLineDef
{
	std::string url;
	std::string dir_1;
	std::string dir_2;
};

static const RATPLineDef ratp_line_defs[RATPClient::RATP_LINE_MAX] = {
    {"http://www.ratp.fr/horaires/fr/ratp/rer/prochains_passages/RA/", "R", "A"},
    {"http://www.ratp.fr/horaires/fr/ratp/rer/prochains_passages/RB/", "R", "A"},
};

static const std::string xpath_rer = "//div[@id='prochains_passages']/fieldset/table/tbody/"
				     "tr[td[@class='passing_time']/text()!='Train sans arrêt' and "
				     "text()!='Supprimé']/td[position() = 2 or position() = 3]";

static const RATPScheduleList EMPTY_SCHEDULE_LIST = {};
static constexpr uint32_t RATP_CACHE_TIME = 60;

const RATPScheduleList &RATPClient::get_next_trains(const RATPClient::Line line,
						    const std::string &stop,
						    const uint8_t direction)
{
	assert(line < RATP_LINE_MAX);
	assert(direction > 0 && direction < 3);

	if (m_stop_cache.find(line) == m_stop_cache.end()) {
		m_stop_cache[line] = RATPStopMap();
	}

	const auto stop_itr = m_stop_cache[line].find(stop);
	if (stop_itr != m_stop_cache[line].end()) {
		if (time(NULL) - stop_itr->second.entry_time < RATP_CACHE_TIME) {
			return stop_itr->second.schedules;
		}
	}

	std::string html_stop = stop;
	std::replace(html_stop.begin(), html_stop.end(), ' ', '+');
	const std::string url = ratp_line_defs[line].url + html_stop + "/";

	std::vector<std::string> res;
	get_html_tag_value(
	    url + (direction == 1 ? ratp_line_defs[line].dir_1 : ratp_line_defs[line].dir_2),
	    xpath_rer, res, XMLParser::Flag::FLAG_XML_WITHOUT_TAGS);
	if (res.size() == 0) {
		return EMPTY_SCHEDULE_LIST;
	}

	if (stop_itr == m_stop_cache[line].end()) {
		m_stop_cache[line][stop] = RATPStop();
		m_stop_cache[line][stop].train_stop = stop;
	}

	m_stop_cache[line][stop].entry_time = time(NULL);
	m_stop_cache[line][stop].schedules = {};

	RATPSchedule tmp_schedule;
	uint16_t offset = 0;
	for (const auto &s : res) {
		if (offset % 2 == 0) {
			tmp_schedule = {};
			tmp_schedule.destination = s;
		} else {
			tmp_schedule.hour = s;
			m_stop_cache[line][stop].schedules.push_back(tmp_schedule);
		}

		offset++;
	}

	return m_stop_cache[line][stop].schedules;
}
