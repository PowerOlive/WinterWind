cmake_minimum_required(VERSION 3.0)
project(winterwind)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/winterwind")

include_directories(
	/usr/include/libxml2
	/usr/include/jsoncpp
	/usr/local/include/libxml2
	/usr/local/include/jsoncpp
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(src)

