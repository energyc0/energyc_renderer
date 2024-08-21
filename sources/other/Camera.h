#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraBase {
protected:
	glm::vec3 _world_pos;
	glm::vec3 _forward;
public:
	static constexpr glm::vec3 camera_up = glm::vec3(0.f,1.f,0.f);

	CameraBase() : _world_pos(0.0f), _forward(0.0f,0.0f,1.0f) {}
	CameraBase(const glm::vec3& pos) : _world_pos(pos), _forward(0.0f, 0.0f, 1.0f) {}
	CameraBase(const glm::vec3& pos, const glm::vec3& forward) : _world_pos(pos), _forward(glm::normalize(forward)) {}

	inline glm::vec3 get_world_position() const noexcept { return _world_pos; }
	inline glm::vec3 get_forward_vector() const noexcept { return _forward; }
	inline glm::mat4 get_view_matrix() const noexcept { return glm::lookAt(_world_pos, _world_pos + _forward, camera_up); }

	virtual ~CameraBase() {}
};

class FreeCamera : public CameraBase{
protected:
	//default values
	constexpr static float d_max_velocity = 0.05f;
	constexpr static float d_acceleration = d_max_velocity;
	constexpr static float d_deceleration = d_max_velocity / 5.0f;
	constexpr static float d_sensetivity = 0.8f;

	const float _acceleration;
	const float _deceleration;
	const float _sensetivity;
	const float _max_velocity;

	float _yaw;
	float _pitch;
	glm::vec3 _right;
	glm::vec3 _velocity;

public:
	FreeCamera();
	FreeCamera(const glm::vec3& pos);
	FreeCamera(const glm::vec3& pos, const glm::vec3& forward);
	FreeCamera(const glm::vec3& pos, const glm::vec3& forward, float acceleration, float deceleration, float sensetivity, float max_velocity);

	inline glm::vec3 get_right_vector() const noexcept { return _right; }

	void rotate_camera(float x_delta, float y_delta);
	void add_velocity(const glm::vec3& velocity);
	void process_position(float delta_time);
};