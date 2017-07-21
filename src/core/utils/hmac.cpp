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

#include "utils/hmac.h"
#include <openssl/hmac.h>

#define MD5_LEN 16
#define SHA1_LEN 20
#define SHA256_LEN 32
#define SHA384_LEN 48
#define SHA512_LEN 64

inline std::string hmac_generic(const std::string &key, const std::string &data,
		const EVP_MD *(*F)(), uint16_t hash_len)
{
	unsigned char *digest = HMAC(F(), key.c_str(), static_cast<int>(key.length()),
		(unsigned char *)data.c_str(), data.length(), NULL, NULL);

	std::string res = "";
	for (int i = 0; i < hash_len; i++) {
		res += digest[i];
	}

	return res;
};

std::string hmac_md5(const std::string &key, const std::string &data)
{
	return hmac_generic(key, data, EVP_md5, MD5_LEN);
}

std::string hmac_sha1(const std::string &key, const std::string &data)
{
	return hmac_generic(key, data, EVP_sha1, SHA1_LEN);
}

std::string hmac_sha256(const std::string &key, const std::string &data)
{
	return hmac_generic(key, data, EVP_sha256, SHA256_LEN);
}

std::string hmac_sha384(const std::string &key, const std::string &data)
{
	return hmac_generic(key, data, EVP_sha384, SHA384_LEN);
}

std::string hmac_sha512(const std::string &key, const std::string &data)
{
	return hmac_generic(key, data, EVP_sha512, SHA512_LEN);
}
