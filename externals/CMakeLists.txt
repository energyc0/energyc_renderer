set(EXTERNALS_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include" PARENT_SCOPE)

find_package(Vulkan REQUIRED)

message(STATUS "Vulkan version: ${Vulkan_VERSION}")
message(STATUS "Vulkan_INCLUDE_DIR: ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan_LIBRARY: ${Vulkan_LIBRARY}")

add_library(glfwlib IMPORTED SHARED "${CMAKE_CURRENT_LIST_DIR}/libs/glfw3.dll")
set_target_properties(glfwlib 
	PROPERTIES 
		IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/libs/glfw3.lib"
		LINKER_LANGUAGE C
		IMPORTED_GLOBAL TRUE)


add_library(extern::glfwlib ALIAS glfwlib)