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

option(ENABLE_POSTGRESQL "PostgreSQL features" TRUE)
option(ENABLE_MYSQL "MySQL features" TRUE)
option(ENABLE_CONSOLE "Enable console features" TRUE)
option(ENABLE_READLINE "Enable readline features (requires ENABLE_CONSOLE)" TRUE)
option(ENABLE_HTTPCLIENT "Enable HTTP client features" TRUE)
option(ENABLE_HTTPSERVER "Enable HTTP server features" TRUE)
option(ENABLE_CALDAV "Enable caldav client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_ELASTICSEARCH "Enable Elasticsearch client features (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_GITLAB "Enable gitlab client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_JIRACLIENT "Enable JIRA client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_OWMCLIENT "Enable OpenWeatherMap client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_RATPCLIENT "Enable RATP client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_SLACKCLIENT "Enable Slack client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_REDIS "Enable redis client" TRUE)
option(ENABLE_NAMEGENERATOR "Enable embedded name generator" TRUE)
option(ENABLE_OAUTHCLIENT "Enable OAuth client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_TWITTERCLIENT "Enable Twitter client (required ENABLE_OAUTHCLIENT)" TRUE)
option(ENABLE_LUA_ENGINE "Enable lua engine" TRUE)

set(SRC_FILES
	utils/base64.cpp
	utils/hmac.cpp
	utils/semaphore.cpp
	utils/stringutils.cpp
	utils/threads.cpp
	utils/time.cpp
	xmlparser.cpp)

set(HEADER_FILES
	${INCLUDE_SRC_PATH}/core/utils/base64.h
	${INCLUDE_SRC_PATH}/core/utils/classhelpers.h
	${INCLUDE_SRC_PATH}/core/utils/exception.h
	${INCLUDE_SRC_PATH}/core/utils/macros.h
	${INCLUDE_SRC_PATH}/core/utils/semaphore.h
	${INCLUDE_SRC_PATH}/core/utils/stringutils.h
	${INCLUDE_SRC_PATH}/core/utils/threads.h
	${INCLUDE_SRC_PATH}/core/utils/threadpool.h
	${INCLUDE_SRC_PATH}/core/utils/threadsafequeue.h
	${INCLUDE_SRC_PATH}/core/utils/time.h
	${INCLUDE_SRC_PATH}/core/xmlparser.h)

set(PROJECT_LIBS
	jsoncpp
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

if (ENABLE_POSTGRESQL)
	set(ENABLE_POSTGRESQL 1 PARENT_SCOPE)
	set(SRC_FILES ${SRC_FILES} postgresqlclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/postgresqlclient.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} pq)
endif()

if (ENABLE_MYSQL)
	set(SRC_FILES ${SRC_FILES} mysqlclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/mysqlclient.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} mysqlclient)
endif()

if (ENABLE_HTTPCLIENT)
	find_package(OpenSSL REQUIRED)
	set(ENABLE_HTTPCLIENT 1 PARENT_SCOPE)
	set(SRC_FILES ${SRC_FILES} httpclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/httpclient.h ${INCLUDE_SRC_PATH}/core/httpcommon.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} crypto curl ssl)

	if (ENABLE_CALDAV)
		set(SRC_FILES ${SRC_FILES} caldavclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/caldavclient.h)
	endif()

	if (ENABLE_ELASTICSEARCH)
		set(SRC_FILES ${SRC_FILES} elasticsearchclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/elasticsearchclient.h)
	endif()

	if (ENABLE_GITLAB)
		set(SRC_FILES ${SRC_FILES} gitlabapiclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/gitlabapiclient.h)
	endif()

	if (ENABLE_JIRACLIENT)
		set(ENABLE_JIRACLIENT 1 PARENT_SCOPE)
		set(SRC_FILES ${SRC_FILES} jiraclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/jiraclient.h)
	endif()

	if (ENABLE_OWMCLIENT)
		set(SRC_FILES ${SRC_FILES} openweathermapclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/openweathermapclient.h)
	endif()

	if (ENABLE_RATPCLIENT)
		set(ENABLE_RATPCLIENT 1 PARENT_SCOPE)
		set(SRC_FILES ${SRC_FILES} ratpclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/ratpclient.h)
	endif()

	if (ENABLE_SLACKCLIENT)
		set(SRC_FILES ${SRC_FILES} slackclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/slackclient.h)
	endif()

	if (ENABLE_OAUTHCLIENT)
		set(SRC_FILES ${SRC_FILES} oauthclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/oauthclient.h)

		if (ENABLE_TWITTERCLIENT)
			set(SRC_FILES ${SRC_FILES} twitterclient.cpp)
			set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/twitterclient.h)
		endif()
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

if (ENABLE_NAMEGENERATOR)
	set(SRC_FILES ${SRC_FILES} namegenerator.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/namegenerator.h)
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
	${INCLUDE_SRC_PATH}/core
)

add_library(winterwind SHARED ${SRC_FILES})
target_link_libraries(winterwind ${PROJECT_LIBS})

install(FILES ${HEADER_FILES} DESTINATION ${INCLUDEDIR})

install(TARGETS winterwind
	LIBRARY DESTINATION ${LIBDIR}
)
