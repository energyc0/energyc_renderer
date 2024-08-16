#pragma once
#include "Window.h"
#include "RenderUnitSolid.h";
#include "CommandManager.h"
#include "SyncManager.h"
#include "UserController.h"
#include "Timer.h"

class RendererApplication {
private:
	Window _window;
	Core _core;
	CommandManager _command_manager;
	SyncManager _sync_manager;
	RenderUnitSolid* _render_unit_solid;
	FreeCamera _camera;
	FreeCameraController _controller;
	std::vector<class Scene*> _scenes;
	Timer<> _timer;

private:
	void update_uniform();
	void update_render_tasks(float delta_time);
	void render(float delta_time);

public:
	RendererApplication(int width, int height, const char* application_name, const char* engine_name);

	void run();

	~RendererApplication();
};