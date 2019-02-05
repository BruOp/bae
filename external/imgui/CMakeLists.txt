add_library(
	imgui
	STATIC
	${CMAKE_CURRENT_LIST_DIR}/src/imgui.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/imgui_demo.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/imgui_draw.cpp
)
target_include_directories( imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/imgui )
target_sources( imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/imgui )
set_target_properties( imgui PROPERTIES FOLDER "imgui" )
