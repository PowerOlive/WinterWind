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

#include "l_pgclient.h"
#include "luaengine.h"
#include <iostream>
#include <postgresqlclient.h>

int LuaEngine::l_create_postgresql_client(lua_State *L)
{
	std::string connect_string = luaL_checkstring(L, 1);

	PostgreSQLClient *pg_client = nullptr;
	try {
		pg_client = new PostgreSQLClient(connect_string);
	} catch (PostgreSQLException &e) {
		std::cerr << e.what() << std::endl;
		delete pg_client;
		return 0;
	}
	LuaRefPostgreSQLClient::create(L, pg_client);
	int object = lua_gettop(L);
	lua_pushvalue(L, object);
	return 1;
}

const char LuaRefPostgreSQLClient::className[] = "LuaRefPostgreSQLClient";
const luaL_Reg LuaRefPostgreSQLClient::methods[] = {
    luamethod(LuaRefPostgreSQLClient, register_statement), {0, 0},
};

LuaRefPostgreSQLClient::LuaRefPostgreSQLClient(PostgreSQLClient *object) : m_object(object) {}

LuaRefPostgreSQLClient::~LuaRefPostgreSQLClient() { delete m_object; }

LuaRefPostgreSQLClient *LuaRefPostgreSQLClient::checkobject(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		std::cerr << "Object has not type " << className << std::endl;
	return *(LuaRefPostgreSQLClient **) ud; // unbox pointer
}

void LuaRefPostgreSQLClient::create(lua_State *L, PostgreSQLClient *object)
{
	LuaRefPostgreSQLClient *o = new LuaRefPostgreSQLClient(object);
	*(void **) (lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

int LuaRefPostgreSQLClient::gc_object(lua_State *L)
{
	LuaRefPostgreSQLClient *o = *(LuaRefPostgreSQLClient **) (lua_touserdata(L, 1));
	delete o;
	return 0;
}

PostgreSQLClient *LuaRefPostgreSQLClient::getobject(LuaRefPostgreSQLClient *ref)
{
	PostgreSQLClient *co = ref->m_object;
	return co;
}

void LuaRefPostgreSQLClient::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);
	luaL_newmetatable(L, className);
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable); // hide metatable from Lua getmetatable()

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, gc_object);
	lua_settable(L, metatable);

	lua_pop(L, 1); // drop metatable

	luaL_setfuncs(L, methods, 0);
	lua_pop(L, 1); // drop methodtable
}

int LuaRefPostgreSQLClient::l_register_statement(lua_State *L)
{
	LuaRefPostgreSQLClient *ref = checkobject(L, 1);
	PostgreSQLClient *pg = getobject(ref);

	std::string name = read<std::string>(L, 2);
	std::string statement = read<std::string>(L, 3);

	write<bool>(L, pg->register_statement(name, statement));
	return 1;
}
