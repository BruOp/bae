find_package(SDL2 REQUIRED)

file(GLOB_RECURSE BAE_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE BAE_INCLUDE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

file(GLOB BAE_VERT_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*_vs.sc)
file(GLOB BAE_FRAG_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*_fs.sc)

add_executable( bae ${BAE_SOURCE} ${BAE_INCLUDE} shaders/basic_vs.sc shaders/basic_fs.sc )
target_link_libraries( bae PUBLIC bgfx imgui glm ${SDL2_LIBRARY})
add_dependencies( bae bgfx imgui glm )
target_include_directories( bae PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${SDL2_INCLUDE_DIR})
set_target_properties( bae PROPERTIES FOLDER "bae" )


set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_shader( shaders/basic_vs.sc VERTEX OUTPUT ${SHADER_OUTPUT_DIR} DX11_MODEL 5_0 GLSL 140 )
add_shader( shaders/basic_fs.sc FRAGMENT OUTPUT ${SHADER_OUTPUT_DIR} DX11_MODEL 5_0 GLSL 140 )

add_compile_definitions(SHADER_OUTPUT_DIR=\"${SHADER_OUTPUT_DIR}\")

configure_debugging( bae WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR} )
