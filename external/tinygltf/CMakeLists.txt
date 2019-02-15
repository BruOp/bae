add_library( tinygltf INTERFACE )
target_include_directories( tinygltf INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include )
target_sources( tinygltf INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include )
