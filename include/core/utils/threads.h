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

#include <atomic>
#include <mutex>
#include <thread>

class Thread
{
public:
	Thread();
	virtual ~Thread();

	/**
	 * Starts the thread and launch the run() function
	 *
	 * @return false if thread is already started
	 */
	const bool start();

	/**
	 * Flag the thread to be stopped
	 */
	virtual void stop() { m_request_stop = true; }

	/**
	 * Kill the current thread. It's a hard kill, state is lost.
	 * @return true if thread has been killed successfuly
	 */
	const bool kill();

	/**
	 * Thread runnable function
	 * This function should be defined in child class and use the m_request_stop flag
	 * to detect a graceful thread stop
	 * @return a raw pointed value if needed
	 */
	virtual void *run() = 0;

	/**
	 * @return thread running status
	 */
	bool is_running() { return m_running; }

	/**
	 * Check is thread is in stopping state
	 * @return stopping state
	 */
	bool is_stopping() const { return m_request_stop; }

	/**
	 * Waits for thread to stop
	 */
	void wait();

	/**
	 * Flag the thread to stop and immediately wait stop.
	 * This is a caller blocking function
	 */
	void stop_and_wait()
	{
		stop();
		wait();
	}

	/**
	 * Disable thread copy constructor
	 */
	Thread(Thread const &) = delete;

	/**
	 * Disable thread copy assignment
	 */
	Thread &operator=(Thread const &) = delete;

protected:
	void ThreadStarted();
	static void set_thread_name(const std::string &name);

private:
	static void *the_thread(void *data);
	std::unique_ptr<std::thread> m_thread = nullptr;
	std::atomic_bool started;
	std::atomic_bool m_running;
	std::atomic_bool m_request_stop;

	std::mutex continuemutex, continuemutex2;
};
