# This source is a part of vice convert

cmake_minimum_required(VERSION 2.6)
project(prj_vice_convert CXX)

# IN WINDOWS PREFER STATIC LINKING
# IN UNIX ENVIRONMENT PREFER DYNAMIC LINKING

include(RebootUtil)

# Try to locate Ogre
if(WIN32)
	set(CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/CMake/;${CMAKE_MODULE_PATH}")
	
		 #stupid temporary
	 SET(BOOST_ROOT "F:/sources/boost_1_55_0/" CACHE STRING "Path to Boost Libraries")
	 SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${BOOST_DIR}")
endif(WIN32)

if(UNIX)
	if(EXISTS "/usr/local/lib/OGRE/cmake")
		set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
	elseif(EXISTS "/usr/lib/OGRE/cmake")
		set(CMAKE_MODULE_PATH "/usr/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
	else()
		message(SEND_ERROR "Failed to find Ogre CMake Module Path")
	endif()
endif(UNIX)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost COMPONENTS system filesystem thread program_options REQUIRED)
find_package(OGRE REQUIRED)

add_subdirectory(DEPENDENCIES)

# use add_sources(to add sources)
set(SRCS "")

add_sources(
	main.cpp 
	main.h
	DffToMeshExporter.h
	DffToMeshExporter.cpp
	IplToDotSceneExporter.h
	TextureToDDSExporter.cpp
	TextureToDDSExporter.h
	MaterialDatabase.h
	MaterialDatabase.cpp
	Exception.h
	ItemDefination.cpp
	ItemDefination.h
	ItemDatabase.h
	ItemDatabase.cpp
)

# we are building using C++ 11
if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")
endif()

if(UNIX)
	# On my arch linux setup boost filesystem is not being found in Boost_LIBRARIES
	set(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})
endif()

#smessage(STATUS "HELLO: " ${NV_DDS_SOURCE_DIR} " " ${NV_DDS_LIBRARIES})


include_directories(${prj-gtaformats_SOURCE_DIR} ${OGRE_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${NV_DDS_SOURCE_DIR} ${LIBCRXUTIL_SOURCE_DIR})
add_executable(prj_vice_convert ${SRCS})
target_link_libraries(prj_vice_convert gtaformats ${OGRE_LIBRARIES} ${Boost_LIBRARIES} NV_DDS LIBCRXUTIL)

cotire(prj_vice_convert)

##########################
#Following can be removed

# enable warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set_target_properties(prj_vice_convert PROPERTIES COMPILE_FLAGS "/Wall")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set_target_properties(prj_vice_convert PROPERTIES COMPILE_FLAGS "-Weverything")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set_target_properties(prj_vice_convert PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
endif()

# cotire sets the following properties 
get_target_property(_unitySource prj_vice_convert COTIRE_CXX_UNITY_SOURCE)
get_target_property(_prefixHeader prj_vice_convert COTIRE_CXX_PREFIX_HEADER)
get_target_property(_precompiledHeader prj_vice_convert COTIRE_CXX_PRECOMPILED_HEADER)
get_target_property(_unityTargetName prj_vice_convert COTIRE_UNITY_TARGET_NAME)

if (_unitySource)
	message(STATUS "prj_vice_convert unity source: ${_unitySource}")
endif()
if (_prefixHeader)
	message(STATUS "prj_vice_convert prefix header: ${_prefixHeader}")
endif()
if (_precompiledHeader)
	message(STATUS "prj_vice_convert precompiled header: ${_precompiledHeader}")
endif()
if (TARGET ${_unityTargetName})
	message(STATUS "prj_vice_convert unity target: ${_unityTargetName}")
endif()
