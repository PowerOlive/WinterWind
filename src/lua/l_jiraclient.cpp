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

#include "l_jiraclient.h"
#include "luaengine.h"
#include <cassert>
#include <iostream>

int LuaEngine::l_create_jiraclient(lua_State *L)
{
	std::string url = read<std::string>(L, 1);
	if (url.empty()) {
		return 0;
	}
	std::string user = read<std::string>(L, 2);
	if (user.empty()) {
		return 0;
	}
	std::string password = read<std::string>(L, 3);
	if (password.empty()) {
		return 0;
	}

	LuaRefJiraClient::create(L, new JiraClient(url, user, password));
	int object = lua_gettop(L);
	lua_pushvalue(L, object);
	return 1;
}

const char LuaRefJiraClient::className[] = "LuaRefJiraClient";
const luaL_Reg LuaRefJiraClient::methods[] = {
    luamethod(LuaRefJiraClient, get_issue),
    {0, 0},
};

LuaRefJiraClient::LuaRefJiraClient(JiraClient *object) : m_object(object) {}

LuaRefJiraClient::~LuaRefJiraClient() { delete m_object; }

LuaRefJiraClient *LuaRefJiraClient::checkobject(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		std::cerr << "Object has not type " << className << std::endl;
	return *(LuaRefJiraClient **) ud; // unbox pointer
}

void LuaRefJiraClient::create(lua_State *L, JiraClient *object)
{
	LuaRefJiraClient *o = new LuaRefJiraClient(object);
	*(void **) (lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

int LuaRefJiraClient::gc_object(lua_State *L)
{
	LuaRefJiraClient *o = *(LuaRefJiraClient **) (lua_touserdata(L, 1));
	delete o;
	return 0;
}

JiraClient *LuaRefJiraClient::getobject(LuaRefJiraClient *ref)
{
	return ref->m_object;
}

void LuaRefJiraClient::Register(lua_State *L)
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

int LuaRefJiraClient::l_get_issue(lua_State *L)
{
	JiraClient *jira = getobject(checkobject(L, 1));
	assert(jira);

	std::string issue = read<std::string>(L, 2);
	Json::Value res;
	if (!jira->get_issue(issue, res)) {
		return 0;
	}

	write<Json::Value>(L, res);
	return 1;
}