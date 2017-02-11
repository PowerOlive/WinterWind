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

#pragma once

#include "utils/threads.h"
#include <iostream>
#include <memory>
#include <vector>

class ConsoleHandler;

struct CommandToProcess
{
public:
	CommandToProcess(uint16_t ch_id, const std::string &cmd):
		channel_id(ch_id), command(cmd) {}

	CommandToProcess(uint16_t ch_id, void* extras,
			const char* w, const std::string &cmd):
		channel_id(ch_id),
		extra_info(extras),
		who(std::string(w)),
		command(cmd) {}

	CommandToProcess(const CommandToProcess &c):
		channel_id(c.channel_id),
		extra_info(c.extra_info),
		who(c.who),
		command(c.command) {}

	uint16_t channel_id = 0;
	void *extra_info = nullptr;
	std::string who = "";
	std::string command = "";
};

typedef std::shared_ptr<CommandToProcess> CommandToProcessPtr;

enum ChatCommandFlag
{
	CHATCMD_FLAG_NONE = 0x0000,
	CHATCMD_FLAG_LOCAL = 0x0001,
	CHATCMD_FLAG_REMOTE = 0x0002,
	CHATCMD_FLAG_LOCAL_REMOTE = CHATCMD_FLAG_LOCAL | CHATCMD_FLAG_REMOTE,
	CHATCMD_FLAG_ARG_NONE = 0x0010,
	CHATCMD_FLAG_ARG_DYNAMIC = 0x0020,
	CHATCMD_FLAG_ARG_STATIC = 0x0040,
	CHATCMD_FLAG_ARG_DYNAMIC_STATIC = CHATCMD_FLAG_ARG_DYNAMIC | CHATCMD_FLAG_ARG_STATIC,
	CHATCMD_FLAG_ARG_ALLMODES = CHATCMD_FLAG_ARG_DYNAMIC_STATIC | CHATCMD_FLAG_ARG_NONE,
};

struct ChatCommandHandlerArg
{
public:
	ChatCommandHandlerArg(const std::string &w, const std::string &c, const std::string &a):
		who(w), channel(c), args(a) {}
	std::string who = "";
	std::string channel = "";
	std::string args = "";
};

typedef std::function<bool(const ChatCommandHandlerArg&, std::stringstream &)> ChatCommandRequestHandler;

struct ChatCommand
{
	const char* name;
	ChatCommandRequestHandler handler;
	ChatCommand* childCommand;
	int flags = 0x0000;
	const std::string help = "";
	const bool show_help = true;
};

enum ChatCommandSearchResult
{
	CHAT_COMMAND_OK,                    // found accessible command by command string
	CHAT_COMMAND_UNKNOWN,               // first level command not found
	CHAT_COMMAND_UNKNOWN_SUBCOMMAND,    // command found but some level subcommand not find in subcommand list
};

class ConsoleHandler
{
public:
	ConsoleHandler() {}
	virtual ~ConsoleHandler() {}
	virtual void enqueue(CommandToProcessPtr cmd) = 0;
	virtual void process_queue() = 0;
protected:
	virtual bool handle_command(const CommandToProcessPtr cmd, std::stringstream &ss) = 0;
	virtual bool handle_command_help(const ChatCommandHandlerArg &args, std::stringstream &ss) = 0;
};

class ConsoleThread: public Thread
{
public:
	ConsoleThread(ConsoleHandler *hdl, const std::string &&prompt,
		const std::string &thread_name = "ConsoleThread"):
		Thread(), m_thread_name(thread_name), m_console_handler(hdl), m_prompt(prompt) {}

	~ConsoleThread() {}

	void * run();

	// These functions only have effect is ENABLE_READLINE is true
	std::string get_completion(uint32_t index);
	void add_completion(const std::string &completion);
private:
	char *rl_gets();

	std::string m_thread_name = "ConsoleThread";
	ConsoleHandler* m_console_handler = nullptr;
	std::string m_prompt = "";
	std::vector<std::string> m_completions = {};
	std::mutex m_mutex;

	char *m_line_read = nullptr;
};
