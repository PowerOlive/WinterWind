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

option(ENABLE_JIRACLIENT "Enable JIRA client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_RATPCLIENT "Enable RATP client (requires ENABLE_HTTPCLIENT)" TRUE)
option(ENABLE_TWITTERCLIENT "Enable Twitter client (required ENABLE_OAUTHCLIENT)" TRUE)

set(SRC_FILES "")

set(HEADER_FILES "")

set(PROJECT_LIBS
	jsoncpp
	xml2)

if (ENABLE_HTTPCLIENT)
	find_package(OpenSSL REQUIRED)

	if (ENABLE_JIRACLIENT)
		set(ENABLE_JIRACLIENT 1 PARENT_SCOPE)
		set(SRC_FILES ${SRC_FILES} jiraclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/core/jiraclient.h)
	endif()

	if (ENABLE_RATPCLIENT)
		set(ENABLE_RATPCLIENT 1 PARENT_SCOPE)
		set(SRC_FILES ${SRC_FILES} ratpclient.cpp)
		set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/extras/ratpclient.h)
	endif()

	if (ENABLE_OAUTHCLIENT)
		if (ENABLE_TWITTERCLIENT)
			set(SRC_FILES ${SRC_FILES} twitterclient.cpp)
			set(HEADER_FILES ${HEADER_FILES} ${INCLUDE_SRC_PATH}/extras/twitterclient.h)
		endif()
	endif()
endif()

if (ENABLE_LUA_ENGINE)
	include_directories(/usr/local/include/lua53)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lua)
	set(SRC_FILES ${SRC_FILES} ${LUA_SRC_FILES})
	if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
		set(PROJECT_LIBS ${PROJECT_LIBS} lua-5.3)
	else()
		set(PROJECT_LIBS ${PROJECT_LIBS} lua)
	endif()
endif()

include_directories(
	${INCLUDE_SRC_PATH}/extras
)

add_library(winterwind_extras SHARED ${SRC_FILES})
target_link_libraries(winterwind_extras ${PROJECT_LIBS} winterwind)

install(FILES ${HEADER_FILES} DESTINATION ${INCLUDEDIR}/extras)

install(TARGETS winterwind_extras
	LIBRARY DESTINATION ${LIBDIR}
)
