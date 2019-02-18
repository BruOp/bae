cmake_minimum_required( VERSION 3.1 )
project(bae VERSION 0.1
        DESCRIPTION "Bruno's Awful Engine"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(GLTF_DIR "${CMAKE_CURRENT_LIST_DIR}/../glTF-Sample-Models/2.0/")
add_subdirectory( deps )
include( deps/bgfx.cmake/cmake/util/ConfigureDebugging.cmake )
add_subdirectory( external )

add_subdirectory( bae )

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
