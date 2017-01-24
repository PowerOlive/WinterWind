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

#pragma once

#include <string>
#include <lua.hpp>
#include "cmake_config.h"

enum LuaReturnCode
{
	LUA_RC_OK,
	LUA_RC_LOADFILE_ERROR,
	LUA_RC_LOADFILE_CONTENT_ERROR,
};

class LuaEngine
{
public:
	LuaEngine();
	~LuaEngine();

	// Lua wrappers
	void newtable();
	void setglobal(const std::string &name);
	int getglobal(const std::string &name);
	bool isfunction(int index);
	void pop(int index = 1);

	template<typename T> T read(int index) const;
	template<typename T> static T read(lua_State *l, int index);

	LuaReturnCode init_winterwind_bindings();

	LuaReturnCode load_script(const std::string &file);

	void register_function(const char *name, lua_CFunction f, int top);

#if UNITTESTS
	bool run_unittests();
#endif

	// Handlers
	static int l_get_ratp_schedules(lua_State *L);
protected:
	lua_State *m_lua = nullptr;
};
