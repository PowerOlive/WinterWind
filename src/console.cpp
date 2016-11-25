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

#include "console.h"

#include <unistd.h>
#include <cstring>
#include <vector>

#include "cmake_config.h"

#if READLINE
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

char *ConsoleThread::rl_gets()
{
	/* If the buffer has already been allocated, return the memory
	   to the free pool. */
	if (m_line_read)
	{
		free(m_line_read);
		m_line_read = nullptr;
	}

	/* Get a line from the user. */
	m_line_read = readline(m_prompt.c_str());

	/* If the line has any text in it, save it on the history. */
	if (m_line_read && *m_line_read)
		add_history(m_line_read);

	return (m_line_read);
}
#else
char *ConsoleThread::rl_gets() { assert(false); return nullptr; }
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

void* ConsoleThread::run()
{
	Thread::SetThreadName("ConsoleThread");

	ThreadStarted();

#if !READLINE
	std::cout << m_prompt;
	std::cout.flush();
#endif

	while (!stopRequested()) {
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
				CommandToProcessPtr cmd(new CommandToProcess(1,
					std::string(command_str, strlen(command_str))));
				m_console_handler->enqueue(cmd);
			}
#if !READLINE
			std::cout << m_prompt;
			std::cout.flush();
#endif
		}
	}
	return NULL;
}
