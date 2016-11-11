set(SRC_FILES
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
