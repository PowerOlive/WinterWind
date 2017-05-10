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

#include "luaengine.h"
#include "lua/l_httpclient.h"
#include "lua/l_pgclient.h"
#include "lua/l_string.h"
#include "lua/l_xmlparser.h"
#include <iostream>

namespace winterwind
{
LuaEngine::LuaEngine()
{
	m_lua = luaL_newstate();
	luaL_openlibs(m_lua);
}

LuaEngine::~LuaEngine() { lua_close(m_lua); }

void LuaEngine::newtable() { lua_newtable(m_lua); }

void LuaEngine::setglobal(const std::string &name) { lua_setglobal(m_lua, name.c_str()); }

int LuaEngine::getglobal(const std::string &name) { return lua_getglobal(m_lua, name.c_str()); }

bool LuaEngine::isfunction(int index) { return lua_isfunction(m_lua, index); }

void LuaEngine::pop(int index) { lua_pop(m_lua, index); }

LuaReturnCode LuaEngine::load_script(const std::string &file)
{
	int ret = luaL_loadfile(m_lua, file.c_str());
	if (ret != 0) {
		std::cerr << "Failed to load and run script from " << file << ":" << std::endl
			  << lua_tostring(m_lua, -1) << std::endl;
		lua_pop(m_lua, 2);
		return LUA_RC_LOADFILE_ERROR;
	}

	ret = lua_pcall(m_lua, 0, 0, 0);
	if (ret != 0) {
		std::cerr << "Failed to load and run script from " << file << ":" << std::endl
			  << lua_tostring(m_lua, -1) << std::endl;
		lua_pop(m_lua, 2);
		return LUA_RC_LOADFILE_CONTENT_ERROR;
	}
	lua_pop(m_lua, 1);
	return LUA_RC_OK;
}

void LuaEngine::register_function(const char *name, lua_CFunction f, int top)
{
	lua_pushstring(m_lua, name);
	lua_pushcfunction(m_lua, f);
	lua_settable(m_lua, top);
}

LuaReturnCode LuaEngine::init_winterwind_bindings()
{
	// Initialize global
	newtable();
	setglobal("core");

	getglobal("core");
	int top = lua_gettop(m_lua);

	LuaString::register_functions(this, top);
	LuaRefXMLParser::Register(m_lua);
	REGISTER_LUA_FCT(create_xmlparser);
// HTTP
#ifdef ENABLE_HTTPCLIENT
	LuaRefHTTPClient::Register(m_lua);
	REGISTER_LUA_FCT(create_httpclient);
#endif

// PostgreSQL
#ifdef ENABLE_POSTGRESQL
	LuaRefPostgreSQLClient::Register(m_lua);
	REGISTER_LUA_FCT(create_postgresql_client);
#endif
	pop(1);
	return LUA_RC_OK;
}

#if UNITTESTS
bool LuaEngine::run_unittests()
{
	getglobal("run_unittests");
	if (!isfunction(-1)) {
		pop(1);
		return false;
	}

	write(m_lua, true);
	lua_call(m_lua, 1, 1);

	if (!lua_isboolean(m_lua, -1)) {
		pop(1);
		return false;
	}

	bool ret = read<bool>(m_lua, -1);
	pop(1);
	return ret;
}
#endif
}
