#pragma once
#include "Window.h"
#include "RenderUnitSolid.h";

class RendererApplication {
private:
	Window _window;
	Core _core;
	RenderUnitSolid _render_unit_solid;

public:
	RendererApplication(int width, int height, const char* application_name, const char* engine_name);

	void run();

	~RendererApplication();
};