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

#include "amqp/envelope.h"
#include "amqp/message.h"

namespace winterwind
{
namespace amqp
{

#define TO_CPP_STR(o) \
	std::string((const char *) o.bytes, o.len)

Envelope::Envelope(amqp_envelope_t *envelope) :
	m_message(std::make_shared<Message>(&envelope->message)),
	m_consumer_tag(TO_CPP_STR(envelope->consumer_tag)),
	m_delivery_tag(envelope->delivery_tag),
	m_exchange(TO_CPP_STR(envelope->exchange)),
	m_redelivered(envelope->redelivered != 0),
	m_routing_key(TO_CPP_STR(envelope->routing_key))
{
}

}
}
