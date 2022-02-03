cmake_minimum_required(VERSION 3.5)

project (PyStand)


add_executable(PyStand
	WIN32
	PyStand.cpp
	resource.rc
	)

target_link_libraries(PyStand -static)


