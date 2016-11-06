include_directories(
	/usr/include/libxml2
	/usr/include/jsoncpp
	/usr/local/include/libxml2
	/usr/local/include/jsoncpp
)

set(LIB_PROJECT_NAME winterwind)
set(LIB_SOURCE_FILES
	utils/base64.cpp
	utils/semaphore.cpp
	utils/stringutils.cpp
	utils/threads.cpp
	caldavclient.cpp
	console.cpp
	elasticsearchclient.cpp
	gitlabapiclient.cpp
	httpclient.cpp
	namegenerator.cpp
	postgresqlclient.cpp
	redisclient.cpp
	xmlparser.cpp)

set(LIB_PROJECT_LIBS
	crypto
	curl
	hiredis
	jsoncpp
	pq
	ssl
	xml2
)

add_library(${LIB_PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})
target_link_libraries(${LIB_PROJECT_NAME} ${LIB_PROJECT_LIBS})

install(TARGETS ${LIB_PROJECT_NAME}
	LIBRARY DESTINATION ${LIBDIR}
)
