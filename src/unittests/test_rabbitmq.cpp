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
#include <core/amqp/exception.h>
#include "test_rabbitmq.h"

namespace winterwind {

namespace unittests {

void Test_RabbitMQ::tearDown()
{
}

void Test_RabbitMQ::create_connection()
{
	bool connection_success = false;
	try {
		std::shared_ptr<amqp::Connection> conn = std::make_shared<amqp::Connection>();
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
		std::shared_ptr<amqp::Connection> conn = std::make_shared<amqp::Connection>();
		std::shared_ptr<amqp::Channel> channel = conn->create_channel();
		channel_open = true;
	}
	catch (amqp::exception &e) {
		std::cerr << "AMQP Exception: " <<  e.what() << std::endl;
	}

	CPPUNIT_ASSERT(channel_open);
}

#if 0
try {
		channel->basic_qos(1, false);
		std::shared_ptr<amqp::Exchange> wwExchange =
			channel->declare_exchange("ww_ex_direct", amqp::Channel::DIRECT, false, true);
		std::shared_ptr<amqp::Queue> dq1 = channel->declare_queue("direct_queue_1");
		std::shared_ptr<amqp::Queue> dq2 = channel->declare_queue("direct_queue_2");
		conn->create_channel()->exchange_exists("ww_ex_direct3");
		conn->create_channel()->exchange_exists("ww_ex_direct4");
		conn->create_channel()->exchange_exists("ww_ex_direct");
		std::shared_ptr<amqp::Exchange> wwFanoutExchange =
			channel->declare_exchange("ww_ex_fanout2", amqp::Channel::FANOUT);
		std::shared_ptr<amqp::Exchange> wwTopicExchange =
			channel->declare_exchange("ww_ex_topic2", amqp::Channel::TOPIC);
		std::shared_ptr<amqp::Exchange> wwExchange4 =
			channel->declare_exchange("ww_ex_direct4", amqp::Channel::DIRECT);

		channel->bind_exchange("ww_ex_fanout2", "ww_ex_topic2", "routeme");
		channel->bind_exchange("ww_ex_direct", "ww_ex_fanout2", "routeme2");
		channel->bind_exchange("ww_ex_fanout2", "ww_ex_topic2", "routeme2");
		channel->unbind_exchange("ww_ex_fanout2", "ww_ex_topic2", "routeme");
		channel->bind_exchange(wwFanoutExchange, wwExchange4, "routeme4");
		channel->delete_exchange("ww_ex_direct");
		channel->delete_exchange(wwFanoutExchange);

		std::shared_ptr<amqp::Queue> fq = channel->declare_queue("ww_q_fanout");
		//fq->purge();
		fq->bind_exchange(wwExchange4, "routeme7");
		fq->bind_exchange(wwTopicExchange, "routemetopic");
		fq->unbind_exchange(wwExchange4, "routeme3");

		fq->consume("rabbit_test", nullptr);
		channel->remove_queue(dq1);
		dq2->remove();

		for (int i = 0; i < 50; i++) {

			amqp::Message msg("test_data777");
			wwExchange4->basic_publish("routeme7", msg
				.set_immediate(false)
				.set_mandatory(true)
				.set_app_id("winterwind_unittests")
				.set_timestamp()
				.set_reply_to("nobody")
				.set_priority(7)
				.set_expiration(60000)
			);
		}

		conn->destroy_channel(channel);

		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	catch (amqp::exception &e) {
		std::cerr << "AMQP Exception: " <<  e.what() << std::endl;
	}

	try {
		std::shared_ptr<amqp::Consumer> consumer = std::make_shared<amqp::Consumer>();
		std::shared_ptr<amqp::Channel> channel = consumer->create_channel();
		std::shared_ptr<amqp::Queue> fq = channel->declare_queue("ww_q_fanout");
		fq->consume("consumer-pure", [] (amqp::EnvelopePtr) {
			return true;
		});
		consumer->start_consuming();

	}
	catch (amqp::exception &e) {
		std::cerr << "AMQP Exception: " <<  e.what() << std::endl;
	}
#endif
}
}
