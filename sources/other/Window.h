#pragma once

#include <GLFW/glfw3.h>

class Window{
private:
	static uint32_t _windows_count;

	GLFWwindow* _window;


public:
	Window(int width, int height, const char* title);
	~Window();
	inline GLFWwindow* get_window() const noexcept { return _window; }
	//extent get_framebuffer_size() const noexcept;
};