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

#include "luaextraengine.h"
#include "ratpclient.h"
#include <iostream>

namespace winterwind
{
namespace extras
{
int LuaEngineExtras::l_get_ratp_schedules(lua_State *L)
{
	if (lua_isnil(L, 1) || lua_isnil(L, 2)) {
		return 0;
	}

	std::string line = read<std::string>(L, 1);
	std::string stop = read<std::string>(L, 2);
	int direction;
	if (lua_isnil(L, 3)) {
		direction = 1;
	} else {
		direction = read<int>(L, 3);
	}

	RATPClient::Line ratp_line;
	if (line == "RER_A") {
		ratp_line = RATPClient::Line::LINE_RER_A;
	} else if (line == "RER_B") {
		ratp_line = RATPClient::Line::LINE_RER_B;
	} else {
		std::cerr << "Lua: " << __FUNCTION__ << ": Invalid RATP Line" << std::endl;
		return 0;
	}

	const auto schedules = RATPClient().get_next_trains(ratp_line, stop, direction);

	lua_createtable(L, (int) schedules.size(), 0);
	int table_idx = lua_gettop(L);
	uint8_t idx = 0;
	for (const auto &sched : schedules) {
		lua_newtable(L);
		lua_pushstring(L, sched.destination.c_str());
		lua_setfield(L, -2, "destination");

		lua_pushstring(L, sched.hour.c_str());
		lua_setfield(L, -2, "hour");

		lua_rawseti(L, table_idx, idx);
		idx++;
	}
	return 1;
}
}
}
