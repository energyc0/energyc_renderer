#sources
add_library(renderer
	"managers/EnergycRenderer.h"
	"managers/EnergycRenderer.cpp"
	"managers/CommandManager.h"
	"managers/CommandManager.cpp"
	"managers/SyncManager.h"
	"managers/SyncManager.cpp"
	"managers/DescriptorManager.h"
	"managers/DescriptorManager.cpp"

	"scene/Scene.h"
	"scene/Scene.cpp"
	"scene/SceneObject.h"
	"scene/SceneObject.cpp"

	"tools/Core.cpp"
	"tools/Core.h"
	"tools/Utils.h"
	"tools/Utils.cpp"
	"tools/VulkanDataObjects.h"
	"tools/VulkanDataObjects.cpp"

	"other/Window.cpp"
	"other/Window.h"
	"other/Camera.h"
	"other/Camera.cpp"
	"other/UserController.h"
	"other/UserController.cpp"
	"other/Timer.h"

	"render/RendererBase.h"
	"render/RendererSolid.h"
	"render/RendererSolid.cpp"
	"render/RenderUnitBase.h"
	"render/RenderUnitSolid.h"
	"render/RenderUnitSolid.cpp"
	"render/RenderManager.h"
	"render/RenderManager.cpp"
	)

target_include_directories(renderer PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(renderer PUBLIC ${Vulkan_LIBRARY})
target_include_directories(renderer PUBLIC ${EXTERNALS_INCLUDE_DIRS})
target_link_libraries(renderer PUBLIC extern::glfwlib)

target_include_directories(renderer PUBLIC 
	"${CMAKE_CURRENT_LIST_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/other"
	"${CMAKE_CURRENT_LIST_DIR}/render"
	"${CMAKE_CURRENT_LIST_DIR}/tools"
	"${CMAKE_CURRENT_LIST_DIR}/scene"
	"${CMAKE_CURRENT_LIST_DIR}/managers"
)
	

add_library(core::renderer ALIAS renderer)