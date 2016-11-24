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

#include "mysqlclient.h"

MySQLClient::MySQLClient(const std::string &host, const std::string &user,
	const std::string &password, uint16_t port, const std::string &db):
	m_host(host), m_user(user), m_password(password), m_port(port), m_db(db)
{
	m_conn = mysql_init(NULL);
	if (!m_conn) {
		throw MySQLException("MySQL Exception: unable to init client " +
			std::string(mysql_error(m_conn)));
	}

	connect();
}

MySQLClient::~MySQLClient()
{
	disconnect();
}

void MySQLClient::connect()
{
	if (mysql_real_connect(m_conn, m_host.c_str(), m_user.c_str(), m_password.c_str(),
		(!m_db.empty() ? m_db.c_str() : NULL), m_port, NULL, 0) == NULL) {
		throw MySQLException("MySQL Exception: connection failed " +
			std::string(mysql_error(m_conn)));
	}
}

void MySQLClient::disconnect()
{
	if (m_conn) {
		mysql_close(m_conn);
	}
}

bool MySQLClient::query(const std::string &query)
{
	if (mysql_query(m_conn, query.c_str()) != 0) {
		throw MySQLException("MySQL Exception: query failed " +
			std::string(mysql_error(m_conn)));
	}
}
