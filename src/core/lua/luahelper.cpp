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

#include "luahelper.h"
#include <cstdint>
#include <string>
#include <vector>
#include <json/json.h>
#include <cmath>

namespace winterwind
{
/*
 * Read helpers
 */
template<>
float LuaHelper::read(lua_State *L, int index)
{
	return (float) lua_tonumber(L, index);
}

template<>
uint8_t LuaHelper::read(lua_State *L, int index)
{
	return (uint8_t) lua_tointeger(L, index);
}

template<>
int8_t LuaHelper::read(lua_State *L, int index)
{
	return (int8_t) lua_tointeger(L, index);
}

template<>
uint16_t LuaHelper::read(lua_State *L, int index)
{
	return (uint16_t) lua_tointeger(L, index);
}

template<>
int16_t LuaHelper::read(lua_State *L, int index)
{
	return (int16_t) lua_tointeger(L, index);
}

template<>
uint32_t LuaHelper::read(lua_State *L, int index)
{
	return (uint32_t) lua_tointeger(L, index);
}

template<>
int32_t LuaHelper::read(lua_State *L, int index)
{
	return (int32_t) lua_tointeger(L, index);
}

template<>
uint64_t LuaHelper::read(lua_State *L, int index)
{
	return (uint64_t) lua_tointeger(L, index);
}

template<>
int64_t LuaHelper::read(lua_State *L, int index)
{
	return (int64_t) lua_tointeger(L, index);
}

template<>
bool LuaHelper::read(lua_State *L, int index)
{ return (bool) lua_toboolean(L, index); }

template<>
std::string LuaHelper::read(lua_State *L, int index)
{
	const char *str = lua_tostring(L, index);
	return std::string(str ? str : "");
}

/*
 * Write helpers
 */
template<>
bool LuaHelper::write(lua_State *L, const float &what)
{
	lua_pushnumber(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const uint8_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const int8_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const uint16_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const int16_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const uint32_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const int32_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const uint64_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const int64_t &what)
{
	lua_pushinteger(L, what);
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const bool &what)
{
	lua_pushboolean(L, static_cast<int>(what));
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const std::string &what)
{
	lua_pushstring(L, what.c_str());
	return true;
}

template<>
bool LuaHelper::write(lua_State *L, const std::vector<std::string> &what)
{
	lua_newtable(L);
	for (uint32_t i = 0; i < what.size(); i++) {
		lua_pushstring(L, what[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}
	return true;
}

/*
 * JSON helpers
 */

static int push_json_value_getdepth(const Json::Value &value)
{
	if (!value.isArray() && !value.isObject())
		return 1;

	int maxdepth = 0;
	for (const auto &v: value) {
		int elemdepth = push_json_value_getdepth(v);
		if (elemdepth > maxdepth)
			maxdepth = elemdepth;
	}
	return maxdepth + 1;
}

/**
 * Recursive function to convert JSON to Lua table
 */
static bool push_json_value_helper(lua_State *L, const Json::Value &value, int nullindex)
{
	switch (value.type()) {
		case Json::intValue:
			lua_pushinteger(L, value.asInt());
			break;
		case Json::uintValue:
			lua_pushinteger(L, value.asUInt());
			break;
		case Json::realValue:
			lua_pushnumber(L, value.asDouble());
			break;
		case Json::stringValue: {
			const char *str = value.asCString();
			lua_pushstring(L, str ? str : "");
			break;
		}
		case Json::booleanValue:
			lua_pushboolean(L, value.asInt());
			break;
		case Json::arrayValue:
			lua_newtable(L);
			for (Json::Value::const_iterator it = value.begin();
				it != value.end(); ++it) {
				push_json_value_helper(L, *it, nullindex);
				lua_rawseti(L, -2, it.index() + 1);
			}
			break;
		case Json::objectValue:
			lua_newtable(L);
			for (Json::Value::const_iterator it = value.begin();
				it != value.end(); ++it) {
				std::string str = it.name();
				lua_pushstring(L, str.c_str());
				push_json_value_helper(L, *it, nullindex);
				lua_rawset(L, -3);
			}
			break;
		case Json::nullValue:
		default:
			lua_pushvalue(L, nullindex);
			break;
	}
	return true;
}

/**
 * JSON -> Lua table
 * @param L
 * @param value
 * @param nullindex
 * @return
 */

bool push_json_value(lua_State *L, const Json::Value &value, int32_t nullindex)
{
	if (nullindex < 0) {
		nullindex = lua_gettop(L) + 1 + nullindex;
	}

	int depth = push_json_value_getdepth(value);

	// The maximum number of Lua stack slots used at each recursion level
	// of push_json_value_helper is 2, so make sure there a depth * 2 slots
	return lua_checkstack(L, depth * 2) != 0 &&
		push_json_value_helper(L, value, nullindex);

}

template<>
bool LuaHelper::write(lua_State *L, const Json::Value &what)
{
	return push_json_value(L, what, lua_gettop(L));
}

/**
 * Converts Lua table --> JSON
 *
 * @param L
 * @param root
 * @param index
 * @param recursion
 * @return
 */
bool read_json_value(lua_State *L, Json::Value &root, int index, uint8_t recursion)
{
	if (recursion > 16) {
		return false;
	}

	int type = lua_type(L, index);
	switch (type) {
		case LUA_TBOOLEAN:
			root = (bool) lua_toboolean(L, index);
			break;
		case LUA_TNUMBER:
			root = lua_tonumber(L, index);
			break;
		case LUA_TSTRING: {
			size_t len;
			const char *str = lua_tolstring(L, index, &len);
			root = std::string(str, len);
			break;
		}
		case LUA_TTABLE: {
			lua_pushnil(L);
			while (lua_next(L, index) != 0) {
				// Key is at -2 and value is at -1
				Json::Value value;
				if (!read_json_value(L, value, lua_gettop(L), recursion + 1)) {
					return false;
				}

				Json::ValueType roottype = root.type();
				int keytype = lua_type(L, -1);
				if (keytype == LUA_TNUMBER) {
					lua_Number key = lua_tonumber(L, -1);
					if (roottype != Json::nullValue && roottype != Json::arrayValue) {
						//throw SerializationError("Can't mix array and object values in JSON");
						return false;
					}

					if (key < 1) {
						//throw SerializationError("Can't use zero-based or negative indexes in JSON");
						return false;
					}

					if (std::floor(key) != key) {
						//throw SerializationError("Can't use indexes with a fractional part in JSON");
						return false;
					}
					root[(Json::ArrayIndex) key - 1] = value;
				} else if (keytype == LUA_TSTRING) {
					if (roottype != Json::nullValue && roottype != Json::objectValue) {
						//throw SerializationError("Can't mix array and object values in JSON");
						return false;
					}
					root[lua_tostring(L, -1)] = value;
				} else {
					//throw SerializationError("Lua key to convert to JSON is not a string or number");
					return false;
				}
			}
			break;
		}
		case LUA_TNIL:
			root = Json::nullValue;
			break;
		default:
			return false;
	}

	lua_pop(L, 1);
	return true;
}

template<>
bool LuaHelper::read(lua_State *L, int index, Json::Value &res)
{
	return read_json_value(L, res, index, 1);
}

template<>
bool LuaHelper::read(lua_State *L, int index, std::vector<std::string> &res)
{
	lua_pushnil(L);
	while (lua_next(L, index) != 0) {
		int cur_index = lua_gettop(L);
		res.push_back(read<std::string>(L, cur_index));
		lua_pop(L, 1);
	}
	return true;
}

/* this permits to read K,V pair
    std::unordered_map<std::string, std::string> fields = {};
	if (lua_istable(L, 5)) {
		lua_pushvalue(L, 5);
		lua_pushnil(L);
		while (lua_next(L, -2)) {
			lua_pushvalue(L, -2);
			fields[read<std::string>(L, -1)] = read<std::string>(L, -2);
			lua_pop(L, 2);
		}
		lua_pop(L, 1);
	}*/

}
