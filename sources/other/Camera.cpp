#include "Camera.h"
#include "Utils.h"

FreeCamera::FreeCamera() :
	CameraBase(),
	_pitch(0.0f),
	_yaw(0.0f),
	_velocity(0.0f),
	_right(glm::cross(-camera_up, _forward)),
	_acceleration(d_acceleration),
	_deceleration(d_deceleration),
	_max_velocity(d_max_velocity),
	_sensetivity(d_sensetivity){}

FreeCamera::FreeCamera(const glm::vec3& pos) :
	CameraBase(pos),
	_pitch(0.0f),
	_yaw(0.0f),
	_velocity(0.0f),
	_right(glm::cross(-camera_up, _forward)),
	_acceleration(d_acceleration),
	_deceleration(d_deceleration),
	_max_velocity(d_max_velocity),
	_sensetivity(d_sensetivity) {}

FreeCamera::FreeCamera(const glm::vec3& pos, const glm::vec3& forward) :
	CameraBase(pos,forward),
	_velocity(0.0f),
	_right(glm::cross(-camera_up, _forward)),
	_acceleration(d_acceleration),
	_deceleration(d_deceleration),
	_max_velocity(d_max_velocity),
	_sensetivity(d_sensetivity) {
	_pitch = asin(-_forward.y) / glm::pi<float>() * 180.f;
	float cosinus = cos(glm::radians(_pitch));
	if (cosinus != 0.f) {
		_yaw = acos(_forward.z / cosinus);
	}
	else {
		_yaw = 0.f;
	}
}

FreeCamera::FreeCamera(const glm::vec3& pos, const glm::vec3& forward, float acceleration, float deceleration, float sensetivity, float max_velocity) :
	CameraBase(pos, forward),
	_velocity(0.0f),
	_right(glm::cross(-camera_up, _forward)),
	_acceleration(acceleration),
	_deceleration(deceleration),
	_max_velocity(max_velocity),
	_sensetivity(sensetivity) {
	_pitch = asin(-forward.y) * (glm::pi<float>() / 180.f);
	float cosinus = cos(glm::radians(_pitch));
	if (cosinus != 0.f) {
		_yaw = acos(forward.z / cosinus);
	}
	else {
		_yaw = 0.f;
	}
}

void FreeCamera::add_velocity(const glm::vec3& velocity) {
	_velocity += velocity * _acceleration;
	if (glm::dot(_velocity, _velocity) > _max_velocity) {
		_velocity = glm::normalize(_velocity) * _max_velocity;
	}

}

void FreeCamera::rotate_camera(float x_delta, float y_delta) {
	_yaw += x_delta * _sensetivity;
	_pitch += y_delta * _sensetivity;

	if (_pitch > 89.f) {
		_pitch = 89.f;
	}
	else if (_pitch < -89.f) {
		_pitch = -89.f;
	}

	_forward.x = cos(glm::radians(_yaw + 90.f)) * cos(glm::radians(_pitch));
	//vulkan -y
	_forward.y = -sin(glm::radians(_pitch));
	_forward.z = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward = glm::normalize(_forward);

	_right = glm::cross(-camera_up, _forward);
}

void FreeCamera::process_position(float delta_time) {
	_world_pos += _velocity * delta_time * _max_velocity;
	_velocity -= _velocity * delta_time * _deceleration;
}