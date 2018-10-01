file(GLOB BAE_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB BAE_INCLUDE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_executable( bae ${BAE_SOURCE} ${BAE_INCLUDE} )
target_link_libraries( bae PUBLIC bgfx imgui glfw glm )
add_dependencies( bae bgfx imgui glfw glm )
target_include_directories( bae PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
set_target_properties( bae PROPERTIES FOLDER "bae" )
configure_debugging( bae WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR} )
