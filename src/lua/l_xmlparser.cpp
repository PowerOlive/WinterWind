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

#include "l_xmlparser.h"
#include "luaengine.h"
#include "xmlparser.h"
#include <iostream>

int LuaEngine::l_create_xmlparser(lua_State *L)
{
	std::string mode = "xml";
	if (!lua_isnil(L, 1)) {
		mode = read<std::string>(L, 1);
	}

	if (mode != "xml" && mode != "html") {
		return 0;
	}

	XMLParser::Mode p_mode = mode == "html" ? XMLParser::Mode::MODE_HTML :
		XMLParser::Mode::MODE_XML;

	LuaRefXMLParser::create(L, new XMLParser(p_mode));
	int object = lua_gettop(L);
	lua_pushvalue(L, object);
	return 1;
}

const char LuaRefXMLParser::className[] = "LuaRefXMLParser";
const luaL_Reg LuaRefXMLParser::methods[] = {
    luamethod(LuaRefXMLParser, parse), {0, 0},
};

LuaRefXMLParser::LuaRefXMLParser(XMLParser *object) : m_object(object) {}

LuaRefXMLParser::~LuaRefXMLParser() { delete m_object; }

LuaRefXMLParser *LuaRefXMLParser::checkobject(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud) {
		std::cerr << "Object has not type " << className << std::endl;
		return nullptr;
	}

	return *(LuaRefXMLParser **) ud; // unbox pointer
}

void LuaRefXMLParser::create(lua_State *L, XMLParser *object)
{
	LuaRefXMLParser *o = new LuaRefXMLParser(object);
	*(void **) (lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

int LuaRefXMLParser::gc_object(lua_State *L)
{
	LuaRefXMLParser *o = *(LuaRefXMLParser **) (lua_touserdata(L, 1));
	delete o;
	return 0;
}

XMLParser *LuaRefXMLParser::getobject(LuaRefXMLParser *ref)
{
	XMLParser *co = ref->m_object;
	return co;
}

void LuaRefXMLParser::Register(lua_State *L)
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

int LuaRefXMLParser::l_parse(lua_State *L)
{
	LuaRefXMLParser *ref = checkobject(L, 1);
	XMLParser *xml = getobject(ref);
	std::string doc = read<std::string>(L, 2);
	std::string xpath = read<std::string>(L, 3);
	int32_t flags = XMLParser::Flag::FLAG_XML_SIMPLE;
	if (!lua_isnil(L, 4)) {
		flags = read<int32_t>(L, 4);
	}

	if (flags == 0) {
		return 0;
	}

	std::vector<std::string> res = {};
	if (!xml->parse(doc, xpath, flags, res)) {
		return 0;
	}

	write<std::vector<std::string>>(L, res);
	return 1;
}
