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

#include <cstdint>
#include <string>
#include <vector>
#include "luahelper.h"

/*
 * Read helpers
 */
template<>
float LuaHelper::read(lua_State* L, int index) { return (float) lua_tonumber(L, index); }
template<>
uint8_t LuaHelper::read(lua_State* L, int index) { return (uint8_t) lua_tointeger(L, index); }
template<>
int8_t LuaHelper::read(lua_State* L, int index) { return (int8_t) lua_tointeger(L, index); }
template<>
uint16_t LuaHelper::read(lua_State* L, int index) { return (uint16_t) lua_tointeger(L, index); }
template<>
int16_t LuaHelper::read(lua_State* L, int index) { return (int16_t) lua_tointeger(L, index); }
template<>
uint32_t LuaHelper::read(lua_State* L, int index) { return (uint32_t) lua_tointeger(L, index); }
template<>
int32_t LuaHelper::read(lua_State* L, int index) { return (int32_t) lua_tointeger(L, index); }
template<>
uint64_t LuaHelper::read(lua_State* L, int index) { return (uint64_t) lua_tointeger(L, index); }
template<>
int64_t LuaHelper::read(lua_State* L, int index) { return (int64_t) lua_tointeger(L, index); }

template<>
bool LuaHelper::read(lua_State* L, int index) { return (bool) lua_toboolean(L, index); }

template<>
std::string LuaHelper::read(lua_State *L, int index)
{
	const char* str = lua_tostring(L, index);
	return std::string(str ? str : "");
}
/*
 * Write helpers
 */
template<>
void LuaHelper::write(lua_State *L, const float &what) { lua_pushnumber(L, what); }
template<>
void LuaHelper::write(lua_State *L, const uint8_t &what) { lua_pushinteger(L, what); }
template<>
void LuaHelper::write(lua_State *L, const int8_t &what) { lua_pushinteger(L, what); }
template<>
void LuaHelper::write(lua_State *L, const uint16_t &what) { lua_pushinteger(L, what); }
template<>
void LuaHelper::write(lua_State *L, const int16_t &what) { lua_pushinteger(L, what); }
template<>
void LuaHelper::write(lua_State *L, const uint32_t &what) { lua_pushinteger(L, what); }
template<>
void LuaHelper::write(lua_State *L, const int32_t &what) { lua_pushinteger(L, what); }
template<>
void LuaHelper::write(lua_State *L, const uint64_t &what) { lua_pushinteger(L, what); }
template<>
void LuaHelper::write(lua_State *L, const int64_t &what) { lua_pushinteger(L, what); }

template<>
void LuaHelper::write(lua_State *L, const bool &what) { lua_pushboolean(L, what); }

template<>
void LuaHelper::write(lua_State *L, const std::string &what) { lua_pushstring(L, what.c_str()); }

template<>
void LuaHelper::write(lua_State *L, const std::vector<std::string> &what)
{
	lua_newtable(L);
	for (uint32_t i = 0; i < what.size(); i++) {
		lua_pushstring(L, what[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}
}
