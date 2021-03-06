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

option(ENABLE_CALDAV "Enable caldav client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_ELASTICSEARCH "Enable Elasticsearch client features (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_GITLAB "Enable gitlab client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_IRCCLIENT "Enable IRC client" TRUE)
option(ENABLE_JIRACLIENT "Enable JIRA client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_NAMEGENERATOR "Enable embedded name generator" TRUE)
option(ENABLE_OWMCLIENT "Enable OpenWeatherMap client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_RATPCLIENT "Enable RATP client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_SLACKCLIENT "Enable Slack client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_TWITTERCLIENT "Enable Twitter client (required ENABLE_OAUTHCLIENT)" TRUE)

set(SRC_FILES "")
set(HEADER_FILES "")
set(PROJECT_LIBS "")
set(EXTRAS_INCLUDE_PATH "${INCLUDE_SRC_PATH}/extras")

if (ENABLE_HTTPCLIENT)
	find_package(OpenSSL REQUIRED)

	if (ENABLE_CALDAV)
		set(SRC_FILES ${SRC_FILES} caldavclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/caldavclient.h)
	endif()

	if (ENABLE_ELASTICSEARCH)
		set(SRC_FILES ${SRC_FILES} elasticsearchclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/elasticsearchclient.h)
	endif()

	if (ENABLE_GITLAB)
		set(SRC_FILES ${SRC_FILES} gitlabapiclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/gitlabapiclient.h)
	endif()

	if (ENABLE_JIRACLIENT)
		set(ENABLE_JIRACLIENT 1 PARENT_SCOPE)
		set(SRC_FILES ${SRC_FILES} jiraclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/jiraclient.h)
	endif()

	if (ENABLE_OWMCLIENT)
		set(SRC_FILES ${SRC_FILES} openweathermapclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/openweathermapclient.h)
	endif()

	if (ENABLE_RATPCLIENT)
		set(ENABLE_RATPCLIENT 1 PARENT_SCOPE)
		set(SRC_FILES ${SRC_FILES} ratpclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/ratpclient.h)
	endif()

	if (ENABLE_SLACKCLIENT)
		set(SRC_FILES ${SRC_FILES} slackclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/slackclient.h)
	endif()

	if (ENABLE_OAUTHCLIENT)
		if (ENABLE_TWITTERCLIENT)
			set(SRC_FILES ${SRC_FILES} twitterclient.cpp)
			set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/twitterclient.h)
		endif()
	endif()
endif()

if (ENABLE_LUA_ENGINE)
	include_directories(/usr/local/include/lua53)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lua)
	set(SRC_FILES ${SRC_FILES} ${LUA_SRC_FILES} luaextraengine.cpp)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
		set(PROJECT_LIBS ${PROJECT_LIBS} lua-5.3)
	else()
		set(PROJECT_LIBS ${PROJECT_LIBS} lua)
	endif()
endif()

if (ENABLE_NAMEGENERATOR)
	set(SRC_FILES ${SRC_FILES} namegenerator.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/namegenerator.h)
endif()

if (ENABLE_IRCCLIENT)
	include_directories(/usr/include/libircclient)
	find_library(LIBIRCCLIENT_LIB NAMES libircclient.so)
	set(SRC_FILES ${SRC_FILES} ircclient.cpp)
	set(HEADER_FILES ${HEADER_FILES} ${EXTRAS_INCLUDE_PATH}/ircclient.h)
	set(PROJECT_LIBS ${PROJECT_LIBS} ${LIBIRCCLIENT_LIB})
	set(ENABLE_IRCCLIENT 1 PARENT_SCOPE)
endif()

include_directories(${EXTRAS_INCLUDE_PATH})

add_library(winterwind_extras SHARED ${SRC_FILES})
target_link_libraries(winterwind_extras ${PROJECT_LIBS} winterwind)

install(FILES ${HEADER_FILES} DESTINATION ${INCLUDEDIR}/extras)

install(TARGETS winterwind_extras
	LIBRARY DESTINATION ${LIBDIR}
)
