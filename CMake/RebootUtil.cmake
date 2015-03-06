# add sources to the nearest project defined with project() in CMakeLists.txt
# Brief: Useful for adding sources that are inside subdirectories of the current project
# Todo: ngladitz from IRC insist on using include to add sources that are inside subdirectory 
#	since include is more of the prefered way and variable scope is shared between the 
#	parent cmake script. 
#	However, the current macro works only for scripts that uses add_subdirectory.
#	add_subdirectory is actually meant for adding scripts that build a target.
#	I have to figure out to find the directory for the script that is being included
macro (add_sources)
	#message(STATUS ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${}) 
	file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
	
	foreach (_src ${ARGN})
		if (_relPath)
			list (APPEND SRCS "${_relPath}/${_src}")
		else()
			list (APPEND SRCS "${_src}")
		endif()
	endforeach()
	
	if (_relPath)
		# propagate SRCS to parent directory
		set (SRCS ${SRCS} PARENT_SCOPE)
	endif()
endmacro()
