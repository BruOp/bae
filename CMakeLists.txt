# This is free and unencumbered software released into the public domain.

cmake_minimum_required( VERSION 3.0 )
project( bae )
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory( deps )
include( deps/bgfx.cmake/cmake/util/ConfigureDebugging.cmake )

add_subdirectory( bae )
