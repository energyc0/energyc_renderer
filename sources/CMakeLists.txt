add_library(renderer
	"RendererApplication.h"
	"RendererApplication.cpp"

	"tools/Core.cpp"
	"tools/Core.h"
	"tools/Utils.h"
	"tools/Utils.cpp"
	"tools/VulkanDataObjects.h"
	"tools/VulkanDataObjects.cpp"

	"other/Window.cpp"
	"other/Window.h"

	"render_units/RendererBase.h"
	"render_units/RendererSolid.h"
	"render_units/RendererSolid.cpp"
	"render_units/RenderUnitBase.h"
	"render_units/RenderUnitSolid.h"
	"render_units/RenderUnitSolid.cpp"
	)

target_include_directories(renderer PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(renderer PUBLIC ${Vulkan_LIBRARY})
target_include_directories(renderer PUBLIC ${EXTERNALS_INCLUDE_DIRS})
target_link_libraries(renderer PUBLIC extern::glfwlib)

target_include_directories(renderer PUBLIC 
	"${CMAKE_CURRENT_LIST_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/other"
	"${CMAKE_CURRENT_LIST_DIR}/render_units"
	"${CMAKE_CURRENT_LIST_DIR}/tools")

add_library(core::renderer ALIAS renderer)