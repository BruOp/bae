add_library(
	glm
	INTERFACE
)
target_include_directories( glm INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include )
target_sources( glm INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include )
