#sources
add_library(renderer
	"managers/EnergycRenderer.h"
	"managers/EnergycRenderer.cpp"
	"managers/CommandManager.h"
	"managers/CommandManager.cpp"
	"managers/SyncManager.h"
	"managers/SyncManager.cpp"
	"managers/MaterialManager.h"
	"managers/MaterialManager.cpp"

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
	
	"render/Renderer/RendererBlur.h"
	"render/Renderer/RendererBlur.cpp"
	"render/Renderer/RendererPostProcess.h"
	"render/Renderer/RendererPostProcess.cpp"
	"render/Renderer/RendererBase.h"
	"render/Renderer/RendererSolid.h"
	"render/Renderer/RendererSolid.cpp"
	"render/Renderer/RendererLight.h"
	"render/Renderer/RendererLight.cpp"
	"render/Renderer/RendererGui.h"
	"render/Renderer/RendererGui.cpp"
	"render/Renderer/RendererEquirectangularProj.h"
	"render/Renderer/RendererEquirectangularProj.cpp"
	"render/RenderUnit/RenderUnitBase.h"
	"render/RenderUnit/RenderUnitSolid.h"
	"render/RenderUnit/RenderUnitSolid.cpp"
	"render/RenderUnit/RenderUnitPostProcess.h"
	"render/RenderUnit/RenderUnitPostProcess.cpp"
	"render/RenderUnit/RenderUnitEquirectangularProj.h"
	"render/RenderUnit/RenderUnitEquirectangularProj.cpp"
	"render/RenderManager.h"
	"render/RenderManager.cpp"
	)

target_include_directories(renderer PUBLIC ${Vulkan_INCLUDE_DIR})
target_link_libraries(renderer PUBLIC ${Vulkan_LIBRARY})
target_include_directories(renderer PUBLIC ${EXTERNALS_INCLUDE_DIRS})
target_link_libraries(renderer PUBLIC extern::glfwlib)
target_link_libraries(renderer PUBLIC extern::ImGui)


target_include_directories(renderer PUBLIC 
	"${CMAKE_CURRENT_LIST_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/other"
	"${CMAKE_CURRENT_LIST_DIR}/render"
	"${CMAKE_CURRENT_LIST_DIR}/render/Renderer"
	"${CMAKE_CURRENT_LIST_DIR}/render/RenderUnit"
	"${CMAKE_CURRENT_LIST_DIR}/tools"
	"${CMAKE_CURRENT_LIST_DIR}/scene"
	"${CMAKE_CURRENT_LIST_DIR}/managers"
)
	

add_library(core::renderer ALIAS renderer)