cmake_minimum_required(VERSION 3.5)

project (PyStand)


# sources
add_executable(PyStand
	WIN32
	PyStand.cpp
	resource.rc
	)


# static link
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
	target_compile_options(PyStand
		PUBLIC
		$<$<CONFIG:Debug>:/MTd>
		$<$<CONFIG:Release>:/MT>
		$<$<CONFIG:MinSizeRel>:/MT>
		)
	target_link_options(PyStand PUBLIC /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
else()
	# for mingw
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_RELEASE} -s")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -s")
	target_link_libraries(PyStand 
		-static
		shlwapi
		winmm
		)
endif()



