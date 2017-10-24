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

#include "core/amqp/log.h"

namespace winterwind
{
namespace amqp
{

log4cplus::Logger amqp_log = logger.getInstance(LOG4CPLUS_TEXT("amqp"));

void handle_reply_error(amqp_rpc_reply_t reply)
{
	switch (reply.reply.id) {
		case AMQP_CHANNEL_CLOSE_METHOD: {
			auto *error = reinterpret_cast<amqp_channel_close_t *>(reply.reply.decoded);
			std::stringstream ss;
			ss << std::string((char *) error->reply_text.bytes, error->reply_text.len)
				<< std::endl;
			log_error(amqp_log, ss.str());
			break;
		}
		case AMQP_CONNECTION_CLOSE_METHOD: {
			auto *error =
				reinterpret_cast<amqp_connection_close_t *>(reply.reply.decoded);
			std::stringstream ss;
			ss << std::string((char *) error->reply_text.bytes, error->reply_text.len)
				<< std::endl;
			log_error(amqp_log, ss.str());
			break;
		}
		case 0:
			break;
		default:
			log_warn(amqp_log, "Unhandled error id " + std::to_string(reply.reply.id));
			break;
	}
}
}
}
