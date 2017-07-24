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

#include <core/utils/log.h>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>

log4cplus::Logger logger = log4cplus::Logger::getRoot();

namespace winterwind
{

bool Logger::s_inited = false;

Logger::Logger(const std::string &config): m_config(config)
{
	assert(!Logger::s_inited);

	init();
	load_configuration();
}

void Logger::init()
{
	log4cplus::initialize();

	// Create console appender
	log4cplus::helpers::SharedObjectPtr<log4cplus::Appender> console_appender(
		new log4cplus::ConsoleAppender());
	console_appender->setName(LOG4CPLUS_TEXT("console"));
	logger.addAppender(console_appender);

	// Set log levels to INFO
	logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
}

void Logger::load_configuration()
{
	log4cplus::PropertyConfigurator::doConfigure(m_config);
}

}
