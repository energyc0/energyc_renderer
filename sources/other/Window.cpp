#include "Window.h"
#include <iostream>
uint32_t Window::_windows_count = 0;

Window::Window(int width, int height, const char* title) {
	if (_windows_count == 0) {
		glfwInit();
	}
	_windows_count++;
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_FALSE);
	_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

Window::~Window() {
	_windows_count--;
	glfwDestroyWindow(_window);
	if (_windows_count == 0) {
		glfwTerminate();
	}
}