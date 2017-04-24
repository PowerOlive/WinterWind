/**
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

#include "l_string.h"
#include <luaengine.h>
#include <utils/base64.h>
#include <utils/hmac.h>
#include <json/json.h>
#include <sstream>

int LuaString::l_base64_decode(lua_State *L)
{
	std::string to_decode = read<std::string>(L, 1);
	write<std::string>(L, base64_decode(to_decode));
	return 1;
}

int LuaString::l_base64_encode(lua_State *L)
{
	std::string to_encode = read<std::string>(L, 1);
	write<std::string>(L, base64_encode(to_encode));
	return 1;
}

int LuaString::l_hmac_sha1(lua_State *L)
{
	std::string key = read<std::string>(L, 1);
	std::string to_hash = read<std::string>(L, 2);
	write<std::string>(L, hmac_sha1(key, to_hash));
}

int LuaString::l_read_json(lua_State *L)
{
	const char *jsonstr = luaL_checkstring(L, 1);

	// Use passed nullvalue or default to nil
	int nullindex = 2;
	if (lua_isnone(L, nullindex)) {
		lua_pushnil(L);
		// nullindex = lua_gettop(L);
	}

	Json::Value root;
	Json::Reader reader;
	std::istringstream stream(jsonstr);

	if (!reader.parse(stream, root)) {
		lua_pushnil(L);
		return 1;
	}

	if (!write<Json::Value>(L, root)) {
		lua_pushnil(L);
	}
	return 1;

}

int LuaString::l_write_json(lua_State *L)
{
	bool styled = false;
	if (!lua_isnone(L, 2)) {
		styled = read<bool>(L, 2);
		lua_pop(L, 1);
	}

	Json::Value root;
	if (!read<Json::Value>(L, 1, root)) {
		lua_pushnil(L);
		lua_pushstring(L, "failed to parse json");
		return 2;
	}

	std::string out;
	if (styled) {
		Json::StyledWriter writer;
		out = writer.write(root);
	} else {
		Json::FastWriter writer;
		out = writer.write(root);
	}
	write<std::string>(L, out);
	return 1;

}

void LuaString::register_functions(LuaEngine *engine, int top)
{
	engine->REGISTER_LUA_FCT(base64_encode);
	engine->REGISTER_LUA_FCT(base64_decode);
	engine->REGISTER_LUA_FCT(hmac_sha1);
	engine->REGISTER_LUA_FCT(read_json);
	engine->REGISTER_LUA_FCT(write_json);
}
