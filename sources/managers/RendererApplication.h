#pragma once
#include "Window.h"
#include "RenderUnitSolid.h";
#include "CommandManager.h"
#include "SyncManager.h"

class RendererApplication {
private:
	Window _window;
	Core _core;
	CommandManager _command_manager;
	SyncManager _sync_manager;
	RenderUnitSolid* _render_unit_solid;
	std::vector<class Scene*> _scenes;

private:
	void update_render_tasks();
	void render();

public:
	RendererApplication(int width, int height, const char* application_name, const char* engine_name);

	void run();

	~RendererApplication();
};