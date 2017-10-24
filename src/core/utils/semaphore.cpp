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

#include "utils/semaphore.h"

#include <cassert>
#include <iostream>

#define UNUSED(expr)                                                                     \
	do {                                                                                 \
		(void) (expr);                                                                   \
	} while (0)

#include <sys/time.h>

Semaphore::Semaphore(int val)
{
	int ret = sem_init(&semaphore, 0, (unsigned int) val);
	assert(!ret);
	UNUSED(ret);
}

Semaphore::~Semaphore()
{
	int ret = sem_destroy(&semaphore);
#ifdef __ANDROID__
	// Workaround for broken bionic semaphore implementation!
	assert(!ret || errno == EBUSY);
#else
	assert(!ret);
#endif
}

void Semaphore::post(unsigned int num)
{
	assert(num > 0);
	for (unsigned i = 0; i < num; i++) {
		int ret = sem_post(&semaphore);
		assert(!ret);
		UNUSED(ret);
	}
}

void Semaphore::wait()
{
	int ret = sem_wait(&semaphore);
	assert(!ret);
	UNUSED(ret);
}

bool Semaphore::wait(unsigned int time_ms)
{
	timespec wait_time{};
	timeval now{};

	if (gettimeofday(&now, NULL) == -1) {
		std::cerr << "Semaphore::wait(ms): Unable to get time with gettimeofday!"
			  << std::endl;
		abort();
	}

	wait_time.tv_nsec = ((time_ms % 1000) * 1000 * 1000) + (now.tv_usec * 1000);
	wait_time.tv_sec =
	    (time_ms / 1000) + (wait_time.tv_nsec / (1000 * 1000 * 1000)) + now.tv_sec;
	wait_time.tv_nsec %= 1000 * 1000 * 1000;

	int ret = sem_timedwait(&semaphore, &wait_time);

	assert(!ret || (errno == ETIMEDOUT || errno == EINTR));
	return ret == 0;
}
