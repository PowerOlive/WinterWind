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

#include <cstdint>
#include <atomic>
#include <vector>
#include <cassert>
#include "threads.h"
#include "threadsafequeue.h"

template <class T>
class ThreadPool
{
public:
	explicit ThreadPool(const uint32_t thread_number):
		m_thread_number(thread_number)
	{
		assert(m_thread_number > 0);

		for (uint32_t i = 0; i < m_thread_number; i++) {
			m_threads.push_back(std::make_unique<T>());
			// Verify if we are working with Thread classes
			assert(dynamic_cast<Thread *>(m_threads[i].get()));
		}
	}

	virtual ~ThreadPool()
	{
		stop_threads();
	}

	/**
	 * Start thread pool workers
	 */
	void start_threads()
	{
		for (uint32_t i = 0; i < m_thread_number; i++) {
			m_threads[i]->start();
		}
	}

	/**
	 * Stop threadpool and optionnaly wait stopping gracefully
	 * @param wait
	 */
	void stop_threads(bool wait = false)
	{
		for (uint32_t i = 0; i < m_thread_number; i++) {
			if (wait) {
				m_threads[i]->stop_and_wait();
			}
			else {
				m_threads[i]->stop();
			}
		}
	}

	void kill_threads()
	{
		for (uint32_t i = 0; i < m_thread_number; i++) {
			m_threads[i]->kill();
		}
	}

protected:
	uint32_t m_thread_number = 0;
	std::vector<std::unique_ptr<T>> m_threads = {};
};

/**
 * @warning You should cleanup queues on your parent object manually yourself
 *
 * @tparam T Thread class which will be pooled to consume IN events and answer in OUT events
 * @tparam IN input object to consume in thread pool
 * @tparam OUT output object to return
 *
 * Thread class should have set_work_queue method referring to the work queue itself
 */
template <class T, typename IN, typename OUT>
class ThreadPoolWorkQueue: public ThreadPool<T>
{
	friend T;
public:
	ThreadPoolWorkQueue(const uint32_t thread_number):
		ThreadPool<T>(thread_number)
	{
		for (uint8_t i = 0; i < ThreadPool<T>::m_thread_number; i++) {
			ThreadPool<T>::m_threads[i]->set_work_queue(this);
		}
	}

	virtual ~ThreadPoolWorkQueue() {}

	void write_input(IN data)
	{
		m_input_queue.push_back(data);
	}

	size_t input_queue_size()
	{
		return m_input_queue.size();
	}

	bool input_queue_empty()
	{
		return m_input_queue.empty();
	}

	OUT read_output()
	{
		return m_output_queue.pop_front();
	}

	size_t output_queue_size()
	{
		return m_output_queue.size();
	}

	bool output_queue_empty()
	{
		return m_output_queue.empty();
	}

private:
	IN read_input_queue()
	{
		return m_input_queue.pop_front();
	}

	void write_output(OUT data)
	{
		m_output_queue.push_back(data);
	}

	ThreadSafeQueue<IN> m_input_queue;
	ThreadSafeQueue<OUT> m_output_queue;
};
