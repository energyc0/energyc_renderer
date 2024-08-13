#include "RendererApplication.h"

RendererApplication::RendererApplication(int width, int height, const char* application_name, const char* engine_name) :
	_window(width,height,application_name),
	_core(_window.get_window(), application_name,engine_name){
	LOG_STATUS("Application start.");
}

void RendererApplication::run() {
	while (!glfwWindowShouldClose(_window.get_window())) {
		glfwPollEvents();
	}
}

RendererApplication::~RendererApplication() {
	LOG_STATUS("Application shutdown.");
}