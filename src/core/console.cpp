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

#include "console.h"

#include <cstring>
#include <cassert>

#include "cmake_config.h"

#if READLINE
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>

namespace winterwind
{
static ConsoleThread *g_console_thread = nullptr;

char *ConsoleThread::rl_gets()
{
	/* If the buffer has already been allocated, return the memory
	   to the free pool. */
	if (m_line_read) {
		free(m_line_read);
		m_line_read = nullptr;
	}

	/* Get a line from the user. */
	m_line_read = readline(m_prompt.c_str());
	rl_bind_key('\t', rl_complete);

	/* If the line has any text in it, save it on the history. */
	if (m_line_read && *m_line_read)
		add_history(m_line_read);

	return (m_line_read);
}

inline static char *dupstr(const std::string &s)
{
	char *r = (char *) malloc(s.length() + 1);
	strcpy(r, s.c_str());
	return r;
}

static char *console_rl_generator(const char *text, int state)
{
	// The console thread should be pointed
	assert(g_console_thread);
	static uint32_t list_index, len;
	std::string name = "";

	if (!state) {
		list_index = 0;
		len = (int) strlen(text);
	}

	name = g_console_thread->get_completion(list_index);
	while (!name.empty()) {
		list_index++;
		if (strncmp(name.c_str(), text, len) == 0)
			return dupstr(name);
		name = g_console_thread->get_completion(list_index);
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

static char **rl_completion(const char *text, int start, int end)
{
	char **matches;

	matches = NULL;

	if (start == 0)
		matches = rl_completion_matches((char *) text, &console_rl_generator);
	else
		rl_bind_key('\t', rl_abort);

	return (matches);
}

#else
char *ConsoleThread::rl_gets()
{
	assert(false);
	return nullptr;
}
static int kb_hit_return()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}
#endif

std::string ConsoleThread::get_completion(uint32_t index)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return index < m_completions.size() ? m_completions[index] : "";
}

void ConsoleThread::add_completion(const std::string &completion)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	for (const auto &c : m_completions) {
		if (c == completion) {
			return;
		}
	}
	m_completions.push_back(completion);
}

void *ConsoleThread::run()
{
	Thread::set_thread_name(m_thread_name);

	ThreadStarted();

#if READLINE
	g_console_thread = this;
	rl_attempted_completion_function = rl_completion;
#else
	std::cout << m_prompt;
	std::cout.flush();
#endif

	while (!is_stopping()) {
#if READLINE
		char *command_str = rl_gets();
#else
		while (!kb_hit_return()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		char commandbuf[256] = {0};
		char *command_str = fgets(commandbuf, sizeof(commandbuf), stdin);
#endif
		if (command_str != NULL) {
			for (int x = 0; command_str[x]; ++x) {
				if (command_str[x] == '\r' || command_str[x] == '\n') {
					command_str[x] = 0;
					break;
				}
			}

			if (command_str[0] != 0) {
				CommandToProcessPtr cmd(new CommandToProcess(
					1, std::string(command_str, strlen(command_str))));
				m_console_handler->enqueue(cmd);
			}
#if !READLINE
			std::cout << m_prompt;
			std::cout.flush();
#endif
		}
	}

#if READLINE
	g_console_thread = nullptr;
#endif
	return nullptr;
}
}
