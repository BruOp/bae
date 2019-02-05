add_library(
	entt
	INTERFACE
)
target_include_directories( entt INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
 )
target_sources( entt INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include )
