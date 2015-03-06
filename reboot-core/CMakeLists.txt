cmake_minimum_required(VERSION 2.6)
project(reboot-core CXX)

include(RebootUtil)

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

find_package(OGRE REQUIRED)
find_package(OIS REQUIRED)

add_subdirectory(DEPENDENCIES)

# its fucking me up on windows

set(Boost_USE_STATIC_LIBS ON)

if(UNIX)
	set(Boost_USE_MULTITHREADED ON)
	set(Boost_USE_STATIC_RUNTIME ON)
endif()

find_package(Boost COMPONENTS system filesystem thread program_options REQUIRED)
find_package(OGRE REQUIRED)

# we are building using C++ 11
if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")
endif()

# set the ZM flag PCH is screwed up constantly F
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Zm167")
endif()

message(STATUS "Message: Boost Libraries = " ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})
if(UNIX)
	# On my arch linux setup boost filesystem is not being found in Boost_LIBRARIES
	set(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})
endif()

set(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY})

set(SRCS "")
add_subdirectory(sources)
message(STATUS ${SRC})

include_directories(${OGRE_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OIS_INCLUDE_DIR} ${OGRE_Overlay_INCLUDE_DIRS} ${HYDRAX_SOURCE_DIR})
add_executable(reboot-core ${SRCS})
target_link_libraries(reboot-core HYDRAX ${OGRE_LIBRARIES} ${Boost_LIBRARIES} ${OIS_LIBRARIES} ${OGRE_Overlay_LIBRARIES})
cotire(reboot-core)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/bin/media)

if(MINGW OR UNIX)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/dist/bin)
endif(MINGW OR UNIX)

add_custom_command(TARGET reboot-core PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/dist $<TARGET_FILE_DIR:reboot-core>)
                       
#install(TARGETS reboot-core	RUNTIME DESTINATION bin)

#install(FILES dist/bin/plugins.cfg
	#dist/bin/resources.cfg
	#RUNTIME DESTINATION bin
#)
install(DIRECTORY dist
	DESTINATION ${PROJECT_BINARY_DIR}
)