add_library(
	entt
	INTERFACE
)
target_include_directories( entt INTERFACE ${CMAKE_CURRENT_LIST_DIR} )
target_sources( entt INTERFACE ${CMAKE_CURRENT_LIST_DIR} )
