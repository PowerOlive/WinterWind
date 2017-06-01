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

#include <unordered_map>
#include <core/utils/base64.h>
#include <cassert>
#include <core/httpclient.h>
#include <core/utils/hmac.h>
#include <core/utils/stringutils.h>
#include <iostream>
#include "utils/jsonwebtokens.h"

namespace winterwind {
namespace web {

static const std::string jwt_alg_str[JsonWebToken::Algorithm::JWT_ALG_MAX] {
		"HS256",
};

JsonWebToken::JsonWebToken(Algorithm alg, const Json::Value &payload,
		const std::string &secret):
	m_algorithm(alg),
	m_payload(payload),
	m_secret(secret)
{
}

void JsonWebToken::get(std::string &result) const
{
	assert(m_algorithm < Algorithm::JWT_ALG_MAX);

	Json::FastWriter json_writer;
	std::string tmp_result = "";

	{
		Json::Value header;
		header["alg"] = jwt_alg_str[m_algorithm];
		header["typ"] = "JWT";

		tmp_result = base64_urlencode(json_writer.write(header)) + ".";
	}

	tmp_result += base64_urlencode(json_writer.write(m_payload));

	std::string signature = hmac_sha256(m_secret, tmp_result);
	tmp_result += "." + base64_encode(signature);

	// Strip '=' char from the whole string
	str_remove_substr(tmp_result, "=");

	result = std::move(tmp_result);
}

JsonWebToken::JWTStatus JsonWebToken::decode(std::string raw_token)
{
	// Strip '=' char
	str_remove_substr(raw_token, "=");

	std::vector<std::string> res = {};
	str_split(raw_token, '.', res);
	if (res.size() != 3) {
		return STATUS_INVALID_STRING;
	}

	Json::Reader reader;
	if (!reader.parse(base64_urldecode(res[0]), m_header)) {
		return STATUS_JSON_PARSE_ERROR;
	}

	Json::Value payload;
	if (!reader.parse(base64_urldecode(res[1]), m_payload)) {
		return STATUS_JSON_PARSE_ERROR;
	}

	std::string signature = base64_urlencode(hmac_sha256(m_secret, res[0] + "." + res[1]));
	// Strip '=' char
	str_remove_substr(signature, "=");

	if (signature.compare(res[2]) != 0) {
		return STATUS_INVALID_SIGNATURE;
	}

	return STATUS_OK;
}

}
}
