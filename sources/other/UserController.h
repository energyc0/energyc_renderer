#pragma once

#include "Window.h"
#include "Camera.h"

class UserControllerBase {
protected:
	const Window* _window_context;

public:
	UserControllerBase(const Window& window);

	virtual void process_input(float delta_time) = 0;

	virtual ~UserControllerBase();
};

class FreeCameraController : public UserControllerBase {
protected:
	FreeCamera& _camera;

	bool _is_W_pressed = false;
	bool _is_A_pressed = false;
	bool _is_S_pressed = false;
	bool _is_D_pressed = false;

	bool _is_space_pressed = false;
	bool _is_ctrl_pressed = false;

	bool _is_middle_button_pressed = false;

	double _last_xpos;
	double _last_ypos;
public:
	FreeCameraController(const Window& window, FreeCamera& camera);

	inline bool is_middle_button_pressed()const noexcept { return _is_middle_button_pressed; }
	const FreeCamera& get_camera() const noexcept { return _camera; }

	virtual void process_input(float delta_time);
};