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

#include "namegenerator.h"
#include "utils/macros.h"

static const char *name_prefixes[] = {
    "ak",   "bac",  "bak",  "car", "dam",  "dem", "dom", "did", "dud", "det", "den",
    "dav",  "kar",  "kep",  "kil", "kon",  "kor", "kol", "kav", "kuv", "kuf", "plis",
    "pram", "prom", "tran", "tep", "trin", "vat", "vin", "vop", "war", "wer", "wet",
    "wez",  "wef",  "weg",  "wiz", "wok",  "woj", "wom", "wub",
};

static const char *name_suffixes[] = {
    "ar", "as", "at", "er", "es", "es", "ir", "is", "ix", "or", "ous", "ur", "us",
};

static const char *name_stem[] = {"d", "f", "m", "ss", "st", "tr", "l", "j", "z"};

static const char *vowels[] = {"a", "e", "i", "o", "u", "y", "ou", "ae", "ie", "io"};

NameGenerator::NameGenerator(const uint64_t &seed) : m_seed(seed)
{
	m_random_generator = std::mt19937(seed);
	m_vowel_generator = std::uniform_int_distribution<uint8_t>(0, ARRLEN(vowels) - 1);
	m_name_generators[0] =
	    std::uniform_int_distribution<uint16_t>(0, ARRLEN(name_prefixes) - 1);
	m_name_generators[1] =
	    std::uniform_int_distribution<uint16_t>(0, ARRLEN(name_suffixes) - 1);
	m_name_generators[2] = std::uniform_int_distribution<uint16_t>(0, ARRLEN(name_stem) - 1);
	m_name_generators[3] = std::uniform_int_distribution<uint16_t>(0, 2);
}

std::string NameGenerator::generate_name()
{
	std::string res = name_prefixes[m_name_generators[0](m_random_generator)];
	uint8_t stem_number = (uint8_t)(m_name_generators[3](m_random_generator) % 2);

	for (uint8_t i = 0; i < stem_number; i++) {
		res += vowels[m_vowel_generator(m_random_generator)];
		res += name_stem[m_name_generators[2](m_random_generator)];
	}

	res += name_suffixes[m_name_generators[1](m_random_generator)];
	return res;
}
