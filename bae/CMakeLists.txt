set(SLD2_PATH ${CMAKE_CURRENT_LIST_DIR}/../external/sdl2/)
find_package(SDL2 REQUIRED)

file(GLOB_RECURSE BAE_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE BAE_INCLUDE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h)

file(GLOB BAE_VERT_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*_vs.sc)
file(GLOB BAE_FRAG_SHADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*_fs.sc)

add_executable( bae ${BAE_SOURCE} ${BAE_INCLUDE} ${BAE_VERT_SHADERS} ${BAE_FRAG_SHADERS} )

target_link_libraries( bae PUBLIC bgfx imgui glm entt ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} tiny_obj_loader tinygltf)
target_include_directories( bae PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${SDL2_INCLUDE_DIR})
set_target_properties( bae PROPERTIES FOLDER "bae" )

set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../assets)

foreach(SHADER IN LISTS BAE_VERT_SHADERS)
    add_shader( ${SHADER} VERTEX OUTPUT ${SHADER_OUTPUT_DIR} DX11_MODEL 5_0 GLSL 140 )
endforeach(SHADER)


foreach(SHADER IN LISTS BAE_FRAG_SHADERS)
    add_shader( ${SHADER} FRAGMENT OUTPUT ${SHADER_OUTPUT_DIR} DX11_MODEL 5_0 GLSL 140 )
endforeach(SHADER)

target_compile_definitions(bae PUBLIC SHADER_OUTPUT_DIR=\"${SHADER_OUTPUT_DIR}\")
target_compile_definitions(bae PUBLIC ASSETS_DIR=\"${ASSETS_DIR}\")
target_compile_definitions(bae PUBLIC GLTF_DIR=\"${GLTF_DIR}\")

configure_debugging( bae WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR} )

source_group("src" FILES ${BAE_SOURCE})
source_group("include" FILES ${BAE_INCLUDE})

# Copy the SDL2.dll file to the executable directory
if (WIN32)
    add_custom_command(TARGET bae POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SLD2_PATH}/libs/SDL2.dll
            $<TARGET_FILE_DIR:bae>)
endif()
