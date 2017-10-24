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

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include "cmake_config.h"

/**
 * Global logger
 * log4cplus is thread-safe you can use it from multiple threads without problem.
 */
extern log4cplus::Logger logger;

#define log_debug(l, s) \
	LOG4CPLUS_DEBUG(l, s);

#define log_info(l, s) \
	LOG4CPLUS_INFO(l, s);

#define log_notice(l, s) \
	LOG4CPLUS_INFO(l, s);

#define log_warn(l, s) \
	LOG4CPLUS_WARN(l, s);

#define log_error(l, s) \
	LOG4CPLUS_ERROR(l, s);

#define log_fatal(l, s) \
	LOG4CPLUS_FATAL(l, s);

namespace winterwind
{

#if ENABLE_IRCCLIENT
extern log4cplus::Logger irc_log;
#endif
/**
 * The logging wrapper class
 * You should instanciate it only one time.
 * Having a child class in a big project permits to add more loggers to the interface
 */
class Logger
{
public:
	/**
	 * Create project logger and init loggers and configuration.
	 * This object is unique in process lifetime.
	 *
	 * @param config path to log4cplus.properties
	 */
	explicit Logger(const std::string &config);
	Logger() = delete;

	/**
	 * Load configuration from m_config path
	 */
	void load_configuration();

protected:
	/**
	 * Init Logger instance. It's called at object creation and should be overloaded by
	 * child objects
	 */
	virtual void init();

private:
	/**
	 * Configuration path
	 */
	std::string m_config = "log4cplus.properties";
	static bool s_inited;
};

}
