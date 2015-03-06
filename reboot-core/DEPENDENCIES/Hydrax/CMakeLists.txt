cmake_minimum_required(VERSION 2.6)
project(HYDRAX)

if(WIN32)
	 set(CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/CMake/;${CMAKE_MODULE_PATH}")
	 SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "$ENV{BOOST_ROOT}")
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

#set(Boost_USE_STATIC_LIBS ON)
#find_package(Boost COMPONENTS thread REQUIRED)
find_package(OGRE REQUIRED)


link_directories(${OGRE_LIB_DIR})
message(STATUS "Ogre Library dir: " ${OGRE_LIB_DIR})
include_directories(${OGRE_INCLUDE_DIRS} "include/" ${Boost_INCLUDE_DIRS})
set(SRCS
	./Hydrax/DecalsManager.cpp
	./Hydrax/Enums.cpp
	./Hydrax/Help.cpp
	./Hydrax/Hydrax.cpp
	./Hydrax/Image.cpp
	./Hydrax/MaterialManager.cpp
	./Hydrax/Mesh.cpp
	./Hydrax/Prerequisites.cpp
	./Hydrax/Structs.cpp
	./Hydrax/TextureManager.cpp
	./Hydrax/Modules/Module.cpp
	./Hydrax/Noise/Noise.cpp
	./Hydrax/Modules/ProjectedGrid/ProjectedGrid.cpp
	./Hydrax/Modules/SimpleGrid/SimpleGrid.cpp
	./Hydrax/Noise/Perlin/Perlin.cpp
)

set(HEADERS
	./Hydrax/DecalsManager.h
	./Hydrax/Enums.h
	./Hydrax/Help.h
	./Hydrax/Hydrax.h
	./Hydrax/Image.h
	./Hydrax/MaterialManager.h
	./Hydrax/Mesh.h
	./Hydrax/Prerequisites.h
	./Hydrax/Structs.h
	./Hydrax/TextureManager.h
	./Hydrax/Modules/Module.h
	./Hydrax/Noise/Noise.h
	./Hydrax/Modules/ProjectedGrid/ProjectedGrid.h
	./Hydrax/Modules/SimpleGrid/SimpleGrid.h
	./Hydrax/Noise/Perlin/Perlin.h
)

add_library(HYDRAX STATIC ${SRCS} ${HEADERS})
target_link_libraries(HYDRAX ${OGRE_LIBRARIES})
# other project that includes Hydrax is able to directly use HYDRAX_SOURCE_DIR
set(HYDRAX_SOURCE_DIR "${PROJECT_SOURCE_DIR}/include" PARENT_SCOPE)
#message(STATUS ${HYDRAX_SOURCE_DIR})