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

#include <iostream>
#include "ratpclient.h"
#include "luaengine.h"

int LuaEngine::l_get_ratp_schedules(lua_State *L)
{
	if (!lua_isinteger(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}

	int line_raw = read<int>(L, -1);
	lua_pop(L, 1);

	if (line_raw != RATPClient::LINE_RER_A && line_raw != RATPClient::LINE_RER_B) {
		std::cerr << "Lua: " << __FUNCTION__ << ": Invalid RATP Line ID" << std::endl;
		return 0;
	}

	const auto schedules = RATPClient().get_next_trains(RATPClient::LINE_RER_B, "Palaiseau Villebon", 1);
	lua_createtable(L, (int) schedules.size(), 0);
	int table_idx = lua_gettop(L);
	uint8_t idx = 0;
	for (const auto &sched: schedules) {
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
