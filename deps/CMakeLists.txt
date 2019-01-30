# bgfx
set( BGFX_BUILD_TOOLS     ON  CACHE INTERNAL "" )
set( BGFX_BUILD_EXAMPLES  OFF CACHE INTERNAL "" )
set( BGFX_CUSTOM_TARGETS  OFF CACHE INTERNAL "" )
add_subdirectory( bgfx.cmake )

# glfw
set( GLFW_BUILD_EXAMPLES  OFF CACHE INTERNAL "" )
set( GLFW_BUILD_TESTS     OFF CACHE INTERNAL "" )
set( GLFW_BUILD_DOCS      OFF CACHE INTERNAL "" )
set( GLFW_INSTALL         OFF CACHE INTERNAL "" )
add_subdirectory( glfw )

# glm
include( glm.cmake )

# imgui
include( imgui.cmake )

# entt
add_subdirectory( entt )

set(TINY_OBJ_LOADER_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/headers/tiny_obj_loader.h" )
add_library(
	tiny_obj_loader
	INTERFACE
)
target_include_directories( tiny_obj_loader INTERFACE ${CMAKE_CURRENT_LIST_DIR}/headers/tiny_obj_loader )
