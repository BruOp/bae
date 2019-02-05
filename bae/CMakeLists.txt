find_package(SDL2 REQUIRED ${CMAKE_CURRENT_LIST_DIR}/../externals/sdl2/)

file(GLOB_RECURSE BAE_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE BAE_INCLUDE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h)

file(GLOB BAE_VERT_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*_vs.sc)
file(GLOB BAE_FRAG_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*_fs.sc)

add_executable( bae ${BAE_SOURCE} ${BAE_INCLUDE} shaders/basic_vs.sc shaders/basic_fs.sc )
target_link_libraries( bae PUBLIC bgfx imgui glm entt ${SDL2_LIBRARY} tiny_obj_loader)
target_include_directories( bae PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${SDL2_INCLUDE_DIR})
set_target_properties( bae PROPERTIES FOLDER "bae" )

set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../assets)

add_shader( shaders/basic_vs.sc VERTEX OUTPUT ${SHADER_OUTPUT_DIR} DX11_MODEL 5_0 GLSL 140 )
add_shader( shaders/basic_fs.sc FRAGMENT OUTPUT ${SHADER_OUTPUT_DIR} DX11_MODEL 5_0 GLSL 140 )

target_compile_definitions(bae PUBLIC SHADER_OUTPUT_DIR=\"${SHADER_OUTPUT_DIR}\")
target_compile_definitions(bae PUBLIC ASSETS_DIR=\"${ASSETS_DIR}\")

configure_debugging( bae WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR} )

source_group("src" FILES ${BAE_SOURCE})
source_group("include" FILES ${BAE_INCLUDE})

# Copy the SDL2.dll file to the executable directory
if (WIN32)
    add_custom_command(TARGET bae POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL2_LIBRARY}
            $<TARGET_FILE_DIR:bae>)
endif()
