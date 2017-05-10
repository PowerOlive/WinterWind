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

#pragma once

#include <lua.hpp>

namespace winterwind
{
/**
 * This macro permits to bootstrap easily a Lua Referenced object
 */
#define LUAREF_OBJECT(name)                                                                                            \
private:                                                                                                               \
    name *m_object;                                                                                                \
    static const char className[];                                                                                 \
    static const luaL_Reg methods[];                                                                               \
    static int gc_object(lua_State *L);                                                                            \
                                                                                                                       \
public:                                                                                                                \
    LuaRef##name(name *object);                                                                                    \
    virtual ~LuaRef##name();                                                                                       \
    static void Register(lua_State *L);                                                                            \
    static void create(lua_State *L, name *object);                                                                \
    static LuaRef##name *checkobject(lua_State *L, int narg);                                                      \
    static name *getobject(LuaRef##name *ref);

/**
 * Convert Lua States to C++ types
 */
class LuaHelper
{
protected:

	/**
	 * Read a value using a template type T from Lua State L and index
	 *
	 * @tparam T type to read from Lua
	 * @param L Lua state
	 * @param index Lua Index to read
	 * @return read value from Lua
	 */
	template<typename T>
	static T read(lua_State *L, int index);

	/**
	 * Read a value using a template type T from Lua State L and index and write it
	 * to res. It's useful to prevent a memory copy on huge objects (like Json)
	 *
	 * @tparam T type to read from Lua
	 * @param L Lua state
	 * @param index Lua Index to read
	 * @param res reference result object
	 * @return true if conversion succeed
	 */
	template<typename T>
	static bool read(lua_State *L, int index, T &res);

	/**
	 * Write what value with type T to Lua State L
	 * @tparam T type of what
	 * @param L Lua State
	 * @param what value to write
	 * @return true if write succeed
	 */
	template<typename T>
	static bool write(lua_State *L, const T &what);
};
}
