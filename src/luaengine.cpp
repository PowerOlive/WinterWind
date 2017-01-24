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

#include "luaengine.h"

LuaEngine::LuaEngine()
{
	m_lua = luaL_newstate();
	luaL_openlibs(m_lua);
}

LuaEngine::~LuaEngine()
{
	lua_close(m_lua);
}

void LuaEngine::newtable()
{
	lua_newtable(m_lua);
}

void LuaEngine::setglobal(const std::string &name)
{
	lua_setglobal(m_lua, name.c_str());
}

LuaReturnCode LuaEngine::loadScript(const std::string &file)
{
	int ret = luaL_loadfile(m_lua, file.c_str());
	if (ret != 0) {
		return LUA_RC_LOADFILE_ERROR;
	}

	ret = lua_pcall(m_lua, 0, 0, 0);
	if (ret != 0) {
		return LUA_RC_LOADFILE_CONTENT_ERROR;
	}

	return LUA_RC_OK;
}

LuaReturnCode LuaEngine::init_winterwind_bindings()
{
	newtable();
	setglobal("core");

	return LUA_RC_OK;
}
