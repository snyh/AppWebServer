cmake_minimum_required(VERSION 2.8)
find_package(Boost 1.41.0  COMPONENTS system thread filesystem)
find_package(Threads)

#DEFAULT use the DEBUG mode
set(CMAKE_CXX_FLAGS "-std=c++0x -DAWS_DEBUG")

include_directories(3lib/include/)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
add_executable(server
	content.cpp
	request_response.cpp
	session.cpp
	server.cpp
	jrpc.cpp
	DetectBrowser.cpp
	test.cpp
	3lib/json_reader.cpp
	3lib/json_writer.cpp 
	3lib/json_value.cpp
	)
target_link_libraries(server ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} pthread jsoncpp)
