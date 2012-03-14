cmake_minimum_required(VERSION 2.8)
find_package(Boost 1.41.0  COMPONENTS system thread filesystem REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++0x")

include_directories(include)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)

set(SRC_LIST 
	3lib/json_reader.cpp
	3lib/json_writer.cpp 
	3lib/json_value.cpp

	request_response.cpp
	session.cpp
	DetectBrowser.cpp
	jrpc.cpp
	server.cpp
	)
add_library(awserver ${SRC_LIST})
target_link_libraries(awserver ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_library(awserver_d ${SRC_LIST})
target_link_libraries(awserver_d ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(awserver_d PROPERTIES COMPILE_FLAGS "-DAWServer_DEBUG")


add_executable(test test.cpp)
target_link_libraries(test ${Boost_LIBRARIES} awserver_d)
