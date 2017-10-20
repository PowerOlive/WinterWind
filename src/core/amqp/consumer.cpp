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

#include "amqp/consumer.h"
#include "amqp/channel.h"
#include "amqp/envelope.h"
#include "amqp/log.h"
#include "amqp/message.h"

namespace winterwind
{
namespace amqp
{

Consumer::Consumer(const std::string &url) :
	Connection(url)
{
}

bool Consumer::start_consuming()
{
	while (!m_stop) {
		if (!consume_one()) {
			return false;
		}
	}

	return true;
}

bool Consumer::consume_one()
{
	amqp_frame_t frame{};
	amqp_envelope_t envelope{};

	amqp_maybe_release_buffers(m_conn);
	amqp_rpc_reply_t ret = amqp_consume_message(m_conn, &envelope, nullptr, 0);

	if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
		if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type &&
			AMQP_STATUS_UNEXPECTED_STATE == ret.library_error) {
			if (AMQP_STATUS_OK != amqp_simple_wait_frame(m_conn, &frame)) {
				log_error(amqp_log, std::string(__FUNCTION__)
					+ ": amqp_simple_wait_frame consuming error")
				return false;
			}

			if (AMQP_FRAME_METHOD == frame.frame_type) {
				switch (frame.payload.method.id) {
					case AMQP_BASIC_ACK_METHOD:
						/*
						 * if we've turned publisher confirms on, and we've published a message
						 * here is a message being confirmed
						 */
						// @TODO callback the ack
						break;
					case AMQP_BASIC_RETURN_METHOD: {
						/*
						 * if a published message couldn't be routed and the mandatory flag was set
						 * this is what would be returned. The message then needs to be read.
						 */
						amqp_message_t raw_message{};
						ret = amqp_read_message(m_conn, frame.channel, &raw_message, 0);
						if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
							return false;
						}

						std::shared_ptr<Channel> channel = find_channel(frame.channel);
						if (channel == nullptr) {
							log_error(amqp_log, std::string(__FUNCTION__)
								+ ": failed to reroute unsent message to channel id "
								+ std::to_string(frame.channel))
							return false;
						}

						// Request channel to handle unsent message
						channel->on_unsent_message(std::make_shared<Message>(&raw_message));

						amqp_destroy_message(&raw_message);
						break;
					}
					case AMQP_CHANNEL_CLOSE_METHOD:
						/*
						 * a channel.close method happens when a channel exception occurs, this
						 * can happen by publishing to an exchange that doesn't exist for example
						 *
						 * In this case you would need to open another channel redeclare any queues
						 * that were declared auto-delete, and restart any consumers that were attached
						 * to the previous channel
						 */
						return false;

					case AMQP_CONNECTION_CLOSE_METHOD:
						/*
						 * a connection.close method happens when a connection exception occurs,
						 * this can happen by trying to use a channel that isn't open for example.
						 *
						 * In this case the whole connection must be restarted.
						 */
						return false;

					default:
						log_error(amqp_log, "An unexpected method was received "
							+ std::to_string(frame.payload.method.id));
						return false;
				}
			}
		}

	} else {
		std::stringstream ss;
		ss << "Message received on channel " << envelope.channel << std::endl;
		log_info(amqp_log, ss.str())

		distribute_envelope(std::make_shared<Envelope>(&envelope), envelope.channel);

		amqp_destroy_envelope(&envelope);
	}

	return true;
}

bool Consumer::distribute_envelope(EnvelopePtr envelope, uint16_t channel_id)
{
	std::shared_ptr<Channel> channel = find_channel(channel_id);
	if (channel == nullptr) {
		log_error(amqp_log, std::string(__FUNCTION__)
			+ ": failed to distribute envelope to channel id "
			+ std::to_string(channel_id))
		return false;
	}

	return channel->on_envelope_received(std::move(envelope));
}

}
}
