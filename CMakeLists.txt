# This is free and unencumbered software released into the public domain.

cmake_minimum_required( VERSION 3.1...3.12 )
project(bae VERSION 0.1
        DESCRIPTION "Bruno's Awful Engine"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory( deps )
include( deps/bgfx.cmake/cmake/util/ConfigureDebugging.cmake )

add_subdirectory( bae )
