#include "UserController.h"



UserControllerBase::UserControllerBase(const Window& window) : _window_context(&window) {}

UserControllerBase::~UserControllerBase() {}

FreeCameraController::FreeCameraController(const Window& window, FreeCamera& camera) :
	UserControllerBase(window),
	_camera(camera){

	glfwGetCursorPos(_window_context->get_window(), &_last_xpos, &_last_ypos);
	glfwSetWindowUserPointer(_window_context->get_window(), this);
	glfwSetInputMode(_window_context->get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwMakeContextCurrent(_window_context->get_window());

	glfwSetKeyCallback(_window_context->get_window(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_REPEAT)
			return;

		auto ptr = reinterpret_cast<FreeCameraController*>(glfwGetWindowUserPointer(window));
		
		switch (key) {
			case GLFW_KEY_W: ptr->_is_W_pressed = action == GLFW_PRESS; break;
			case GLFW_KEY_A: ptr->_is_A_pressed = action == GLFW_PRESS; break;
			case GLFW_KEY_S: ptr->_is_S_pressed = action == GLFW_PRESS; break;
			case GLFW_KEY_D: ptr->_is_D_pressed = action == GLFW_PRESS; break;
			case GLFW_KEY_SPACE: ptr->_is_space_pressed = action == GLFW_PRESS; break;
			case GLFW_KEY_LEFT_CONTROL:ptr-> _is_ctrl_pressed = action == GLFW_PRESS; break;
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
			default: break;
		}
	});

	glfwSetCursorPosCallback(_window_context->get_window(), [](GLFWwindow* window, double xpos, double ypos) {

		auto ptr = reinterpret_cast<FreeCameraController*>(glfwGetWindowUserPointer(window));

		if (ptr->_is_middle_button_pressed) {
			float x_delta = static_cast<float>(xpos - ptr->_last_xpos);
			float y_delta = static_cast<float>(ypos - ptr->_last_ypos);

			ptr->_camera.rotate_camera(x_delta, y_delta);
		}
		ptr->_last_xpos = xpos;
		ptr->_last_ypos = ypos;
	});
	glfwSetMouseButtonCallback(_window_context->get_window(), [](GLFWwindow* window, int button, int action, int mods) {
		if (action == GLFW_REPEAT)
			return;

		auto ptr = reinterpret_cast<FreeCameraController*>(glfwGetWindowUserPointer(window));

		switch (button)
		{
		case GLFW_MOUSE_BUTTON_MIDDLE: ptr->_is_middle_button_pressed = action == GLFW_PRESS; break;
		default: break;
		}
	});
}

void FreeCameraController::process_input(float delta_time) {
	if (_is_middle_button_pressed) {
		glm::vec3 velocity = glm::vec3(0.f);
		if (_is_W_pressed) {
			velocity += _camera.get_forward_vector();
		}
		if (_is_S_pressed) {
			velocity -= _camera.get_forward_vector();
		}
		if (_is_D_pressed) {
			velocity += _camera.get_right_vector();
		}
		if (_is_A_pressed) {
			velocity -= _camera.get_right_vector();
		}
		if (_is_space_pressed) {
			velocity += FreeCamera::camera_up;
		}
		if (_is_ctrl_pressed) {
			velocity -= FreeCamera::camera_up;
		}
		_camera.add_velocity(velocity);
	}
	_camera.process_position(delta_time);
}