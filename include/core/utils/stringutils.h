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

#include <stdint.h>
#include <string>
#include <vector>

/**
 * Join A elements using T and return result
 * A is generally a std::vector and T a std::string
 *
 * @tparam T result type
 * @tparam A iterator type
 * @param begin
 * @param end
 * @param t joining element to append
 * @return join result
 */
template <class T, class A> T join(const A &begin, const A &end, const T &t) {
	T result;
	for (A it = begin; it != end; it++) {
		if (!result.empty())
			result.append(t);
		result.append(*it);
	}
	return result;
}

/**
 * Split str using delim and return splitted res
 * @param str String to split
 * @param delim Splitting character
 * @param res results
 */
void str_split(const std::string &str, char delim, std::vector<std::string> &res);

/**
 * Remove pattern from src string
 * @param src string to modify
 * @param pattern pattern to remove
 */
void str_remove_substr(std::string &src, const std::string &pattern);

/**
 * Count words in str phrase
 * @param str Phrase to analyze
 * @return word count
 */
uint32_t count_words(const std::string &str);

/**
 * Split string using word delimiters and write result to res
 * @param str String to split
 * @param res Result container
 */
void split_string_to_words(const std::string &str, std::vector<std::string> &res);

/**
 * Encode str string to hex
 * @param str String to encode
 * @param res Result container
 * @param upper_case uppercase the result
 */
void str_to_hex(const std::string &str, std::string &res, bool upper_case = false);
