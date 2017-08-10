/*
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

#pragma once

#include <string>
#include "core/utils/exception.h"

namespace winterwind {
namespace db {

class DatabaseException : public BaseException
{
public:
	explicit DatabaseException(const std::string &what) : BaseException(what) {}
	DatabaseException() = delete;

	virtual ~DatabaseException() throw() = default;
};

struct DatabaseTableField
{
	std::string column_name = "";
	std::string data_type = "";
	bool is_nullable = false;
	std::string column_default = "";
};

class DatabaseInterface
{
public:
	/**
	 * Start a transaction
	 */
	virtual void begin() = 0;

	/**
	 * Commit a transaction
	 */
	virtual void commit() = 0;

	/**
	 * Rollbacks currently started transaction
	 */
	virtual void rollback() = 0;


	/**
	 * Check if connection is working.
	 * If it's not the case this interface should:
	 * - Fix the connection
	 * - Trigger a DatabaseException.
	 */
	virtual void check_connection() = 0;
protected:

	/**
	 * Database connection method. If connection failed a DatabaseException should be
	 * thrown
	 */
	virtual void connect() = 0;

	/**
	 * Disconnect client from its backend
	 */
	virtual void disconnect() = 0;

	/**
	 * Enable or disable connection check before execute a query
	 *
	 * @param e enable/disable flag
	 */
	void set_check_before_exec(bool e) { m_check_before_exec = e; }

	bool m_check_before_exec = true;
};

}
}
