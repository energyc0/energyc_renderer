#pragma once
#include "Window.h"
#include "RenderManager.h";
#include "CommandManager.h"
#include "SyncManager.h"
#include "UserController.h"
#include "RendererGui.h"
#include "Timer.h"

class EnergycRenderer {
private:
	Window _window;

	Core _core;

	StagingBuffer _staging_buffer;

	CommandManager _command_manager;
	SyncManager _sync_manager;
	std::unique_ptr<RenderManager> _render_manager;

	FreeCamera _camera;
	FreeCameraController _controller;

	std::shared_ptr<class MaterialManager> _material_manager;
	std::shared_ptr<class Scene> _current_scene;
	std::vector<std::shared_ptr<class Scene>> _scenes;

	Timer<> _timer;
	GuiInfo _gui_info;

private:
	void update_uniform(float delta_time);
	void update_render_tasks(float delta_time);
	void draw_frame(float delta_time);

public:
	EnergycRenderer(int width, int height, const char* application_name, const char* engine_name);

	void run();

	~EnergycRenderer();
};