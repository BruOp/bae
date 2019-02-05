add_library( tiny_obj_loader INTERFACE )
target_include_directories( tiny_obj_loader INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include )
target_sources( tiny_obj_loader INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include )
