# This file is a part of Project Reboot
cmake_minimum_required(VERSION 2.6)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
# https://github.com/sakra/cotire
# Speed up builds :)
include(cotire)

set(VICE_CONVERT_BUILD OFF CACHE BOOL "Whether you want to build vice_convert")
set(REBOOT_CORE_BUILD ON CACHE BOOL "Whether you want to build reboot-core")

if(VICE_CONVERT_BUILD)
	add_subdirectory(vice_convert)
	add_subdirectory(libcrxutil)
endif()



if(REBOOT_CORE_BUILD)
	add_subdirectory(reboot-core)
endif()