# Copyright (c) 2016-2017, Loic Blot <loic.blot@unix-experience.fr>
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

option(ENABLE_AMQP "AMQP features" TRUE)
option(ENABLE_POSTGRESQL "PostgreSQL features" TRUE)
option(ENABLE_MYSQL "MySQL features" TRUE)
option(ENABLE_CONSOLE "Enable console features" TRUE)
option(ENABLE_READLINE "Enable readline features (requires ENABLE_CONSOLE)" TRUE)
option(ENABLE_HTTPCLIENT "Enable HTTP client features" TRUE)
option(ENABLE_HTTPSERVER "Enable HTTP server features" TRUE)
option(ENABLE_REDIS "Enable redis client" TRUE)
option(ENABLE_OAUTHCLIENT "Enable OAuth client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_LUA_ENGINE "Enable lua engine" TRUE)
option(ENABLE_JWT "Enable Json WebTokens" TRUE)
option(ENABLE_COVERAGE "Enable code coverage" FALSE)

set(SRC_FILES
	utils/base64.cpp
	utils/hmac.cpp
	utils/log.cpp
	utils/semaphore.cpp
	utils/stringutils.cpp
	utils/threads.cpp
	utils/time.cpp
	utils/uuid.cpp
	xmlparser.cpp
	http/log.cpp)

set(HEADER_FILES
	${INCLUDE_SRC_PATH}/core/utils/base64.h
	${INCLUDE_SRC_PATH}/core/utils/classhelpers.h
	${INCLUDE_SRC_PATH}/core/utils/exception.h
	${INCLUDE_SRC_PATH}/core/utils/log.h
	${INCLUDE_SRC_PATH}/core/utils/macros.h
	${INCLUDE_SRC_PATH}/core/utils/semaphore.h
	${INCLUDE_SRC_PATH}/core/utils/stringutils.h
	${INCLUDE_SRC_PATH}/core/utils/threads.h
	${INCLUDE_SRC_PATH}/core/utils/threadpool.h
	${INCLUDE_SRC_PATH}/core/utils/threadsafequeue.h
	${INCLUDE_SRC_PATH}/core/utils/time.h
	${INCLUDE_SRC_PATH}/core/xmlparser.h
	${INCLUDE_SRC_PATH}/core/http/query.h
	${INCLUDE_SRC_PATH}/core/http/log.h)

set(PROJECT_LIBS
	jsoncpp
	log4cplus
	uuid
	xml2)

set(READLINE 0 PARENT_SCOPE)

if (ENABLE_CONSOLE)
	set(SRC_FILES ${SRC_FILES} console.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/console.h)
	if (ENABLE_READLINE)
		set(PROJECT_LIBS ${PROJECT_LIBS} readline)
		set(READLINE 1 PARENT_SCOPE)
	endif()
endif()

if (ENABLE_AMQP)
	set(ENABLE_AMQP 1 PARENT_SCOPE)
	set(SRC_FILES ${SRC_FILES}
		amqp/channel.cpp
		amqp/connection.cpp
		amqp/envelope.cpp
		amqp/exchange.cpp
		amqp/inoutexchange.cpp
		amqp/log.cpp
		amqp/message.cpp
		amqp/queue.cpp)
	set(HEADER_FILES ${HEADER_FILES}
		${INCLUDE_SRC_PATH}/core/amqp/channel.h
		${INCLUDE_SRC_PATH}/core/amqp/connection.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} rabbitmq)
endif()

if (ENABLE_POSTGRESQL)
	set(ENABLE_POSTGRESQL 1 PARENT_SCOPE)
	set(SRC_FILES ${SRC_FILES} databases/postgresqlclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/databases/postgresqlclient.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} pq)
endif()

if (ENABLE_MYSQL)
	set(SRC_FILES ${SRC_FILES} databases/mysqlclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/databases/mysqlclient.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} mysqlclient)
endif()

if (ENABLE_JWT)
	set(SRC_FILES ${SRC_FILES} utils/jsonwebtokens.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/utils/jsonwebtokens.h)
endif()

if (ENABLE_HTTPCLIENT)
	find_package(OpenSSL REQUIRED)
	set(ENABLE_HTTPCLIENT 1 PARENT_SCOPE)
	set(SRC_FILES ${SRC_FILES} httpclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/httpclient.h ${INCLUDE_SRC_PATH}/core/httpcommon.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} crypto curl ssl)

	if (ENABLE_OAUTHCLIENT)
		set(SRC_FILES ${SRC_FILES} oauthclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/oauthclient.h)
	endif()
endif()

if (ENABLE_HTTPSERVER)
	set(SRC_FILES ${SRC_FILES}
		httpresponse.cpp
		httpserver.cpp
	)
	set(HEADER_FILES ${HEADER_FILES}
		${INCLUDE_SRC_PATH}/core/httpcommon.h
		${INCLUDE_SRC_PATH}/core/httpresponse.h
		${INCLUDE_SRC_PATH}/core/httpserver.h
	)
	set(PROJECT_LIBS ${PROJECT_LIBS} microhttpd)
endif()

if (ENABLE_REDIS)
	set(SRC_FILES ${SRC_FILES} redisclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/redisclient.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} hiredis)
endif()

if (ENABLE_LUA_ENGINE)
	include_directories(/usr/local/include/lua53)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lua)
	set(SRC_FILES ${SRC_FILES} ${LUA_SRC_FILES} luaengine.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/luaengine.h)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
		set(PROJECT_LIBS ${PROJECT_LIBS} lua-5.3)
	else()
		set(PROJECT_LIBS ${PROJECT_LIBS} lua)
	endif()
endif()

include_directories(
	/usr/include/uuid
	${INCLUDE_SRC_PATH}/core
)

if (ENABLE_COVERAGE)
	include(CodeCoverage)
	set(PROJECT_LIBS ${PROJECT_LIBS} gcov)
	set(COVERAGE_EXCLUDES
		"/usr/include/json/*"
		"/usr/include/log4cplus/*"
		"/usr/include/log4cplus/*/*"
		"/usr/include/c++/*"
		"/usr/include/c++/*/*"
		"/usr/include/c++/*/*/*"
		"/usr/include/c++/*/*/*/*")

	setup_target_for_coverage(NAME ${PROJECT_NAME}_coverage DEPENDENCIES winterwind
		EXECUTABLE winterwind_unittests)
	APPEND_COVERAGE_COMPILER_FLAGS()
endif()

add_library(winterwind SHARED ${SRC_FILES})
target_link_libraries(winterwind ${PROJECT_LIBS})

install(FILES ${HEADER_FILES} DESTINATION ${INCLUDEDIR}/core)

install(TARGETS winterwind
	LIBRARY DESTINATION ${LIBDIR}
)
