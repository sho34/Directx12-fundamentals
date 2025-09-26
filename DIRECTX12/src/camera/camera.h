#pragma once

#include <vector>
#include <DirectXMath.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


// i want to use std versions of min and max.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using namespace DirectX;

class camera
{
public:
	camera();
	~camera() = default;

public:
	void init(XMFLOAT3 position);
	void update(float elapsed_seconds);
	void set_move_speed(float units_per_second);
	void set_rotation_speed(float radians_per_second);
	void on_key_down(WPARAM key);
	void on_key_up(WPARAM key);

public:
	XMMATRIX get_view_matrix() const;
	XMMATRIX get_projection_matrix(float fov, float aspect_ratio, float near_plane = 1.0f, float far_plane = 1000.0f) const;

private:
	void reset();

	// keys to detect when pressed.
	struct key_pressed
	{
		bool w;
		bool a;
		bool s;
		bool d;

		bool left;
		bool right;
		bool up;
		bool down;
	};

private:
	XMFLOAT3 m_initial_position;;
	XMFLOAT3 m_position;
	XMFLOAT3 m_look_at;
	XMFLOAT3 m_up;

	float m_yaw;			
	float m_pitch;
	float m_move_speed;
	float m_rotation_speed;

	key_pressed m_keys;

};