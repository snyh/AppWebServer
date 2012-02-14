cmake_minimum_required(VERSION 2.8)
find_package(Boost 1.41.0  COMPONENTS system thread filesystem)
find_package(Threads)

set(CMAKE_CXX_FLAGS "-std=c++0x")

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
add_executable(server
	content.cpp
	request_response.cpp
	session.cpp
	server.cpp
	jrpc.cpp
	t.cpp
	)
target_link_libraries(server ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} pthread jsoncpp)
