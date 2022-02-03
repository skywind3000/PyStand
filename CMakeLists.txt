cmake_minimum_required(VERSION 3.5)

project (PyStand)


add_executable(PyStand
	WIN32
	PyStand.cpp
	resource.rc
	)

target_link_libraries(PyStand -static)

# static link with vc
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
	if (CMAKE_BUILD_TYPE MATCHES "Debug")
		target_compile_options(PyStand PUBLIC /MTd)
	else()
		target_compile_options(PyStand PUBLIC /MT)
	endif()
	target_link_options(PyStand PUBLIC /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
endif()


