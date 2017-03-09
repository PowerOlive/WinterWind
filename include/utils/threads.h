/**
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
	const bool start();
	virtual void stop() { requeststop = true; }
	int kill();
	virtual void *run() = 0;
	bool IsRunning() { return running; }
	bool stopRequested() const { return requeststop; }

	void wait();
	void stop_and_wait() {
		stop();
		wait();
	}

	Thread(Thread const &) = delete;
	Thread &operator=(Thread const &) = delete;

protected:
	void ThreadStarted();
	static void SetThreadName(const std::string &name);

private:
	static void *TheThread(void *data);
	std::thread *m_thread = nullptr;
	std::atomic_bool started;
	void *retval;
	std::atomic_bool running;
	std::atomic_bool requeststop;

	std::mutex continuemutex, continuemutex2;
};
