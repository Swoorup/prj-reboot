cmake_minimum_required(VERSION 2.6)
project(LIBCRXUTIL CXX)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost COMPONENTS system filesystem thread program_options REQUIRED)

set(SRCS 
	CrxFile.h
	CrxFile.cpp
)

if(UNIX)
	# On my arch linux setup boost filesystem is not being found in Boost_LIBRARIES
	set(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY})
endif()

add_library(LIBCRXUTIL STATIC ${SRCS})
target_link_libraries(LIBCRXUTIL ${Boost_LIBRARIES})