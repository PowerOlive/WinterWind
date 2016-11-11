# Copyright (c) 2016, Loic Blot <loic.blot@unix-experience.fr>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


option(ENABLE_POSTGRESQL "PostgreSQL features" TRUE)

option(ENABLE_HTTP "Enable HTTP features" TRUE)
option(ENABLE_CALDAV "Enable caldav client (requires ENABLE_HTTP)" TRUE)
option(ENABLE_ELASTICSEARCH "Enable Elasticsearch client features (requires ENABLE_HTTP)" TRUE)
option(ENABLE_GITLAB "Enable gitlab client (requires ENABLE_HTTP)" TRUE)
option(ENABLE_REDIS "Enable redis client" TRUE)

set(SRC_FILES
	utils/base64.cpp
	utils/semaphore.cpp
	utils/stringutils.cpp
	utils/threads.cpp

	console.cpp
	namegenerator.cpp
	xmlparser.cpp)

if (ENABLE_POSTGRESQL)
	set(SRC_FILES ${SRC_FILES} postgresqlclient.cpp)
endif()

if (ENABLE_HTTP)
	set(SRC_FILES ${SRC_FILES} httpclient.cpp)

	if (ENABLE_CALDAV)
		set(SRC_FILES ${SRC_FILES} caldavclient.cpp)
	endif()

	if (ENABLE_ELASTICSEARCH)
		set(SRC_FILES ${SRC_FILES} elasticsearchclient.cpp)
	endif()

	if (ENABLE_GITLAB)
		set(SRC_FILES ${SRC_FILES} gitlabapiclient.cpp)
	endif()
endif()

if (ENABLE_REDIS)
	set(SRC_FILES ${SRC_FILES} redisclient.cpp)
endif()

set(PROJECT_LIBS
	crypto
	curl
	hiredis
	jsoncpp
	pq
	ssl
	xml2
)

add_library(winterwind SHARED ${SRC_FILES})
target_link_libraries(winterwind ${PROJECT_LIBS})

install(DIRECTORY ../include/ DESTINATION ${INCLUDEDIR}
	FILES_MATCHING PATTERN "*.h")

install(TARGETS winterwind
	LIBRARY DESTINATION ${LIBDIR}

)
