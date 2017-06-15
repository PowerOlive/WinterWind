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

#include "utils/stringutils.h"
#include <sstream>
#include <cstring>
#include <iomanip>
#include <string>

#define IF_WORD_SEPARATOR                                                                          \
	if (c == ' ' || c == '\'' || c == '\n' || c == '\t' || c == ';' || c == '\r' ||            \
	    c == '!' || c == '?' || c == ',' || c == '.' || c == '/' || c == ':')


void str_split(const std::string &str, char delim, std::vector<std::string> &res)
{
	std::stringstream ss;
	ss.str(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
		res.push_back(item);
	}
}

void str_remove_substr(std::string &src, const std::string &pattern)
{
	for (std::string::size_type i = src.find(pattern); i != std::string::npos;
	     i = src.find(pattern)) {
		src.erase(i, pattern.length());
	}
}

uint32_t count_words(const std::string &str)
{
	bool prev_was_word_separator = false;
	uint32_t word_count = 1;
	for (const auto &c : str) {
		IF_WORD_SEPARATOR
		{
			if (!prev_was_word_separator) {
				word_count++;
			}
			prev_was_word_separator = true;
		}
		else { prev_was_word_separator = false; }
	}

	return word_count;
}

void split_string_to_words(const std::string &str, std::vector<std::string> &res)
{
	std::string buf = "";
	bool prev_was_word_separator = false;
	for (const auto &c : str) {
		IF_WORD_SEPARATOR
		{
			if (!prev_was_word_separator) {
				res.push_back(buf);
				buf.clear();
			}
			prev_was_word_separator = true;
		}
		else
		{
			buf += c;
			prev_was_word_separator = false;
		}
	}

	if (!prev_was_word_separator) {
		res.push_back(buf);
	}
}

void str_to_hex(const std::string &str, std::string &res, bool upper_case)
{
	std::ostringstream ret;

	for (std::string::size_type i = 0; i < str.length(); ++i)
		ret << std::hex << std::setfill('0') << std::setw(2)
			<< (upper_case ? std::uppercase : std::nouppercase) << (int)str[i];

	res = ret.str();
}

void trim(std::string &str)
{
	char *p = new char[str.length() + 1];
	char *p_orig = p;
	memcpy(p, str.c_str(), str.length() + 1);

	// Trim leading space
	while (isspace((unsigned char)*p)) p++;

	if (*p == 0) {
		str = "";
		return;
	}

	// Trim trailing space
	char *end = p + strlen(p) - 1;
	while (end > p && isspace((unsigned char)*end)) end--;

	// Write new null terminator
	*(end+1) = '\0';

	str = std::string(p);
	delete [] p_orig;
}

bool replace(std::string &str, const std::string &from, const std::string &to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
