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

#include <core/amqp/connection.h>
#include <core/amqp/channel.h>
#include <core/amqp/exception.h>
#include <core/amqp/exchange.h>
#include <core/amqp/message.h>
#include <core/amqp/queue.h>
#include "test_rabbitmq.h"

namespace winterwind {

namespace unittests {

void Test_RabbitMQ::tearDown()
{
}

#define RABBITMQ_URL "amqp://rabbitmq/unittests"

void Test_RabbitMQ::create_connection()
{
	bool connection_success = false;
	try {
		std::shared_ptr<amqp::Connection> conn =
			std::make_shared<amqp::Connection>(RABBITMQ_URL);
		connection_success = true;
	}
	catch (amqp::exception &e) {
		std::cerr << "AMQP Exception: " <<  e.what() << std::endl;
	}

	CPPUNIT_ASSERT(connection_success);
}

void Test_RabbitMQ::create_channel()
{
	bool channel_open = false;
	try {
		std::shared_ptr<amqp::Connection> conn =
			std::make_shared<amqp::Connection>(RABBITMQ_URL);
		std::shared_ptr<amqp::Channel> channel = conn->create_channel();
		channel_open = true;
	}
	catch (amqp::exception &e) {
		std::cerr << "AMQP Exception: " <<  e.what() << std::endl;
	}

	CPPUNIT_ASSERT(channel_open);
}

#define RABBITMQ_STARTUP \
	std::shared_ptr<amqp::Connection> conn; \
	std::shared_ptr<amqp::Channel> channel; \
	try { \
		conn = std::make_shared<amqp::Connection>(RABBITMQ_URL); \
		channel = conn->create_channel(); \
	} \
		catch (amqp::exception &e) { \
		std::cerr << "AMQP Exception: " <<  e.what() << std::endl; \
		CPPUNIT_ASSERT(false);\
		return; \
	}


void Test_RabbitMQ::remove_channel()
{
	RABBITMQ_STARTUP
	conn->destroy_channel(channel);
	CPPUNIT_ASSERT(true);
}

void Test_RabbitMQ::basic_qos()
{
	RABBITMQ_STARTUP

	CPPUNIT_ASSERT(channel->basic_qos(1, false));
}

void Test_RabbitMQ::declare_exchange()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Exchange> wwExchange =
		channel->declare_exchange("ww_ex_direct", amqp::Channel::DIRECT, false, true);
	CPPUNIT_ASSERT(wwExchange != nullptr);

	std::shared_ptr<amqp::Exchange> wwFanoutExchange =
		channel->declare_exchange("ww_ex_fanout", amqp::Channel::FANOUT);
	CPPUNIT_ASSERT(wwFanoutExchange != nullptr);

	std::shared_ptr<amqp::Exchange> wwTopicExchange =
		channel->declare_exchange("ww_ex_topic", amqp::Channel::TOPIC);
	CPPUNIT_ASSERT(wwTopicExchange != nullptr);

	std::shared_ptr<amqp::Exchange> wwExchange2 =
		channel->declare_exchange("ww_ex_direct2", amqp::Channel::DIRECT);
	CPPUNIT_ASSERT(wwExchange2 != nullptr);
}

void Test_RabbitMQ::bind_exchange()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Exchange> wwExchange =
		channel->declare_exchange("ww_ex_direct", amqp::Channel::DIRECT, false, true);
	CPPUNIT_ASSERT(wwExchange != nullptr);

	std::shared_ptr<amqp::Exchange> wwFanoutExchange =
		channel->declare_exchange("ww_ex_fanout", amqp::Channel::FANOUT);
	CPPUNIT_ASSERT(wwFanoutExchange != nullptr);

	CPPUNIT_ASSERT(channel->bind_exchange(wwExchange, wwFanoutExchange,
		"route-to-heaven"));

	std::shared_ptr<amqp::Exchange> wwTopicExchange =
		channel->declare_exchange("ww_ex_topic", amqp::Channel::TOPIC);
	CPPUNIT_ASSERT(wwTopicExchange != nullptr);

	CPPUNIT_ASSERT(channel->bind_exchange("ww_ex_topic", "ww_ex_fanout",
		"route-to-hell"));
}

void Test_RabbitMQ::unbind_exchange()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Exchange> wwExchange =
		channel->declare_exchange("ww_ex_direct", amqp::Channel::DIRECT, false, true);
	CPPUNIT_ASSERT(wwExchange != nullptr);

	std::shared_ptr<amqp::Exchange> wwFanoutExchange =
		channel->declare_exchange("ww_ex_fanout", amqp::Channel::FANOUT);
	CPPUNIT_ASSERT(wwFanoutExchange != nullptr);

	CPPUNIT_ASSERT(channel->bind_exchange(wwExchange, wwFanoutExchange,
		"route-to-heaven"));

	CPPUNIT_ASSERT(channel->unbind_exchange(wwExchange, wwFanoutExchange,
		"route-to-heaven"));

	std::shared_ptr<amqp::Exchange> wwTopicExchange =
		channel->declare_exchange("ww_ex_topic", amqp::Channel::TOPIC);
	CPPUNIT_ASSERT(wwTopicExchange != nullptr);

	CPPUNIT_ASSERT(channel->bind_exchange("ww_ex_topic", "ww_ex_fanout",
		"route-to-hell"));

	CPPUNIT_ASSERT(channel->unbind_exchange("ww_ex_topic", "ww_ex_fanout",
		"route-to-hell"));
}

void Test_RabbitMQ::delete_exchange()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Exchange> wwExchange =
		channel->declare_exchange("ww_ex_direct", amqp::Channel::DIRECT, false, true);
	CPPUNIT_ASSERT(wwExchange != nullptr);

	CPPUNIT_ASSERT(channel->delete_exchange(wwExchange));

	std::shared_ptr<amqp::Exchange> wwFanoutExchange =
		channel->declare_exchange("ww_ex_fanout", amqp::Channel::FANOUT);
	CPPUNIT_ASSERT(wwFanoutExchange != nullptr);

	CPPUNIT_ASSERT(channel->delete_exchange("ww_ex_fanout"));
}

void Test_RabbitMQ::declare_queue()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Queue> uq = channel->declare_queue("unittest_queue");
	CPPUNIT_ASSERT(uq != nullptr);

	std::shared_ptr<amqp::Queue> uq2 = channel->declare_queue("unittest_queue2", true);
	CPPUNIT_ASSERT(uq2 != nullptr);

	std::shared_ptr<amqp::Queue> uq3 = channel->declare_queue("unittest_queue3", false, true);
	CPPUNIT_ASSERT(uq3 != nullptr);

	std::shared_ptr<amqp::Queue> uq4 = channel->declare_queue("unittest_queue4", false, false, true);
	CPPUNIT_ASSERT(uq4 != nullptr);
}

void Test_RabbitMQ::bind_queue_to_exchange()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Queue> uq = channel->declare_queue("unittest_queue");
	CPPUNIT_ASSERT(uq != nullptr);

	std::shared_ptr<amqp::Exchange> wwExchange =
		channel->declare_exchange("ww_ex_direct", amqp::Channel::DIRECT, false, true);
	CPPUNIT_ASSERT(wwExchange != nullptr);

	CPPUNIT_ASSERT(uq->bind_exchange(wwExchange, "exchange-to-queue"));
}

void Test_RabbitMQ::purge_queue()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Queue> uq = channel->declare_queue("unittest_queue");
	CPPUNIT_ASSERT(uq != nullptr);

	CPPUNIT_ASSERT(uq->purge());
}

void Test_RabbitMQ::remove_queue()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Queue> uq = channel->declare_queue("unittest_queue");
	CPPUNIT_ASSERT(uq != nullptr);

	CPPUNIT_ASSERT(uq->remove());
}

#define MESSAGES_TO_QUEUE 20

void Test_RabbitMQ::publish_to_exchange()
{
	RABBITMQ_STARTUP

	std::shared_ptr<amqp::Exchange> wwExchange =
		channel->declare_exchange("ww_ex_direct", amqp::Channel::DIRECT, false, true);
	CPPUNIT_ASSERT(wwExchange != nullptr);

	std::shared_ptr<amqp::Queue> uq = channel->declare_queue("unittest_queue");
	CPPUNIT_ASSERT(uq != nullptr);

	CPPUNIT_ASSERT(uq->bind_exchange(wwExchange, "publish-route"));

	for (uint16_t i = 0; i < MESSAGES_TO_QUEUE; i++) {

		amqp::Message msg("unittest-" + std::to_string(i));
		CPPUNIT_ASSERT(wwExchange->basic_publish("publish-route", msg
			.set_immediate(false)
			.set_mandatory(true)
			.set_app_id("winterwind_unittests")
			.set_timestamp()
			.set_reply_to("nobody")
			.set_priority(static_cast<uint8_t>(rand() % 10))
			.set_expiration(static_cast<uint64_t>(rand() % 60000))
		) == AMQP_STATUS_OK);
	}
}

void Test_RabbitMQ::consume_queue()
{
	publish_to_exchange();

	try {
		std::shared_ptr<amqp::Connection> consumer =
			std::make_shared<amqp::Connection>(RABBITMQ_URL);
		std::shared_ptr<amqp::Channel> channel = consumer->create_channel();
		std::shared_ptr<amqp::Queue> fq = channel->declare_queue("unittest_queue");

		uint32_t consumed_messages = 0;
		fq->consume("consumer-pure", [&consumed_messages] (amqp::EnvelopePtr) {
			consumed_messages++;
			return true;
		});

		for (uint16_t i = 0; i < MESSAGES_TO_QUEUE; i++) {
			consumer->consume_one();
		}

		CPPUNIT_ASSERT(consumed_messages == MESSAGES_TO_QUEUE);

	}
	catch (amqp::exception &e) {
		std::cerr << "AMQP Exception: " <<  e.what() << std::endl;
		CPPUNIT_ASSERT(false);
	}
}

}
}
