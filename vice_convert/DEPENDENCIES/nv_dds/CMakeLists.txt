cmake_minimum_required(VERSION 2.6)
project(NV_DDS CXX)


set(NV_DDS_USE_GL OFF CACHE BOOL "Whether we should use OpenGL related stuff")

if(NV_DDS_USE_GL)
	add_definitions(-DNV_DDS_ENABLE_GL_STUFF)
endif(NV_DDS_USE_GL)


# use only C++11 as it uses static_assert

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SRCS 
	nv_dds.h
	nv_dds.cpp
)

add_library(NV_DDS STATIC ${SRCS})