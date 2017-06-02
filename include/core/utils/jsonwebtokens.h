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

#include <json/json.h>
#include <ctime>

namespace winterwind {
namespace web {

class JsonWebToken {
public:
	enum Algorithm: uint8_t {
		ALG_HS256,
		ALG_HS384,
		ALG_HS512,
		JWT_ALG_MAX,
	};

	enum JWTStatus: uint8_t {
		STATUS_OK = 0,
		STATUS_INVALID_STRING,
		STATUS_JSON_PARSE_ERROR,
		STATUS_INVALID_SIGNATURE,
	};

	enum JWTGenStatus: uint8_t {
		GENSTATUS_OK,
		GENSTATUS_JWT_CLAIM_IN_PAYLOAD,
	};

	JsonWebToken(Algorithm alg, const Json::Value &payload, const std::string &secret);
	JsonWebToken(const std::string &secret) :
		m_secret(secret)
	{}

	~JsonWebToken() {}

	JsonWebToken &issuedAt(std::time_t when);
	JsonWebToken &expirationTime(std::time_t when);
	JsonWebToken &issuer(const std::string &issuer);
	JsonWebToken &subject(const std::string &subject);

	/**
	 * Generate JWT from object attributes and store it to result
	 *
	 * @param result
	 * @returns successful generation
	 */
	JWTGenStatus get(std::string &result) const;

	JWTStatus read_and_verify(std::string raw_token);

	Json::Value get_payload() const { return m_payload; }
private:
	void sign(const std::string &payload, std::string &signature) const;

	Algorithm m_algorithm = ALG_HS256;
	Json::Value m_header = {};
	Json::Value m_payload = {};
	std::string m_secret = "";

	std::string m_issuer = "";
	bool m_issuer_set = false;
	std::string m_subject = "";
	bool m_subject_set = false;
	std::time_t m_issued_at = 0;
	bool m_issued_at_set = false;
	std::time_t m_expiration_time = 0;
	bool m_expirationtime_set = false;
};
}
}
