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

#include "l_httpclient.h"
#include "luaengine.h"
#include <iostream>

int LuaEngine::l_create_httpclient(lua_State *L)
{
	LuaRefHTTPClient::create(L, new HTTPClient());
	int object = lua_gettop(L);
	lua_pushvalue(L, object);
	return 1;
}

const char LuaRefHTTPClient::className[] = "LuaRefHTTPClient";
const luaL_Reg LuaRefHTTPClient::methods[] = {
    luamethod(LuaRefHTTPClient, delete),
    luamethod(LuaRefHTTPClient, get),
    luamethod(LuaRefHTTPClient, head),
    luamethod(LuaRefHTTPClient, post),
    luamethod(LuaRefHTTPClient, propfind),
    luamethod(LuaRefHTTPClient, put),
    luamethod(LuaRefHTTPClient, add_form_param),
    luamethod(LuaRefHTTPClient, add_uri_param),
    {0, 0},
};

LuaRefHTTPClient::LuaRefHTTPClient(HTTPClient *object) : m_object(object) {}

LuaRefHTTPClient::~LuaRefHTTPClient() { delete m_object; }

LuaRefHTTPClient *LuaRefHTTPClient::checkobject(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud) {
		std::cerr << "Object has not type " << className << std::endl;
		return nullptr;
	}

	return *(LuaRefHTTPClient **) ud; // unbox pointer
}

void LuaRefHTTPClient::create(lua_State *L, HTTPClient *object)
{
	LuaRefHTTPClient *o = new LuaRefHTTPClient(object);
	*(void **) (lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

int LuaRefHTTPClient::gc_object(lua_State *L)
{
	LuaRefHTTPClient *o = *(LuaRefHTTPClient **) (lua_touserdata(L, 1));
	delete o;
	return 0;
}

HTTPClient *LuaRefHTTPClient::getobject(LuaRefHTTPClient *ref)
{
	return ref->m_object;
}

void LuaRefHTTPClient::Register(lua_State *L)
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

#define HTTPCLIENT_PARAM_INIT                                                                      \
	LuaRefHTTPClient *ref = checkobject(L, 1);                                                 \
	HTTPClient *http = getobject(ref);                                                         \
	std::string url = read<std::string>(L, 2);                                                 \
	std::string res = "";

#define HTTPCLIENT_RESP_PUSH                                                                       \
	lua_newtable(L);                                                                           \
	write(L, (uint32_t) http->get_http_code());                                                \
	lua_setfield(L, -2, "code");                                                               \
	write(L, res);                                                                             \
	lua_setfield(L, -2, "content");

int LuaRefHTTPClient::l_get(lua_State *L)
{
	HTTPCLIENT_PARAM_INIT
	http->_get(url, res);
	HTTPCLIENT_RESP_PUSH
	return 1;
}

int LuaRefHTTPClient::l_get_html_tag_value(lua_State *L)
{
	LuaRefHTTPClient *ref = checkobject(L, 1);
	HTTPClient *http = getobject(ref);

	if (lua_isnil(L, 2) || lua_isnil(L, 3)) {
		return 0;
	}

	std::string url = read<std::string>(L, 2);
	std::string xpath = read<std::string>(L, 3);
	std::vector<std::string> res = {};
	int32_t flags = XMLParser::Flag::FLAG_XML_SIMPLE;
	if (!lua_isnil(L, 4)) {
		flags = read<int32_t>(L, 4);
	}

	http->get_html_tag_value(url, xpath, res, flags);
	write<std::vector<std::string> >(L, res);
	return 1;
}

int LuaRefHTTPClient::l_delete(lua_State *L)
{
	HTTPCLIENT_PARAM_INIT
	http->_delete(url, res);
	HTTPCLIENT_RESP_PUSH
	return 1;
}

int LuaRefHTTPClient::l_head(lua_State *L)
{
	HTTPCLIENT_PARAM_INIT
	http->_head(url, res);
	HTTPCLIENT_RESP_PUSH
	return 1;
}

int LuaRefHTTPClient::l_post(lua_State *L)
{
	HTTPCLIENT_PARAM_INIT
	http->_post(url, res);
	HTTPCLIENT_RESP_PUSH
	return 1;
}

int LuaRefHTTPClient::l_propfind(lua_State *L)
{
	HTTPCLIENT_PARAM_INIT
	http->_propfind(url, res);
	HTTPCLIENT_RESP_PUSH
	return 1;
}

int LuaRefHTTPClient::l_put(lua_State *L)
{
	HTTPCLIENT_PARAM_INIT
	http->_put(url, res);
	HTTPCLIENT_RESP_PUSH
	return 1;
}

int LuaRefHTTPClient::l_add_uri_param(lua_State *L)
{
	LuaRefHTTPClient *ref = checkobject(L, 1);
	HTTPClient *http = getobject(ref);

	std::string param = read<std::string>(L, 2);
	std::string value = read<std::string>(L, 3);

	http->add_uri_param(param, value);
	return 1;
}

int LuaRefHTTPClient::l_add_form_param(lua_State *L)
{
	LuaRefHTTPClient *ref = checkobject(L, 1);
	HTTPClient *http = getobject(ref);

	std::string param = read<std::string>(L, 2);
	std::string value = read<std::string>(L, 3);

	http->add_form_param(param, value);
	return 1;
}
