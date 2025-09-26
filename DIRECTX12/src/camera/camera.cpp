#include "camera.h"

// a simple camera system

camera::camera()
	: m_initial_position(0.0f, 10.0f, -10.0f)
	, m_position(m_initial_position)
	, m_look_at(0.0f, 0.0f, -1.0f)
	, m_up(0.0f, 1.0f, 0.0f)
	, m_yaw(XM_PI)
	, m_pitch(0.0f)
	, m_move_speed(20.0f)
	, m_rotation_speed(XM_PIDIV2)
	, m_keys{ }
{}

void camera::init(XMFLOAT3 position)
{
	m_initial_position = position;
	reset();
}

void camera::update(float elapsed_seconds)
{
	// calculate the move vector in the camera space.
	XMFLOAT3 move_vector(0.0f, 0.0f, 0.0f);

	if (m_keys.a)
		move_vector.x -= 1.0f;
	if (m_keys.d)
		move_vector.x += 1.0f;
	if (m_keys.w)
		move_vector.z -= 1.0f;
	if (m_keys.s)
		move_vector.z += 1.0f;

	if (fabs(move_vector.x) > 0.1f && fabs(move_vector.y) > 0.1f)
	{
		XMVECTOR v = XMVector3Normalize(XMLoadFloat3(&move_vector));
		move_vector.x = XMVectorGetX(v);
		move_vector.z = XMVectorGetZ(v);
	}

	float move_interval = m_move_speed * elapsed_seconds;
	float rotate_interval = m_rotation_speed * elapsed_seconds;

	if (m_keys.left)
		m_yaw += rotate_interval;
	if (m_keys.right)
		m_yaw -= rotate_interval;
	if (m_keys.up)
		m_pitch += rotate_interval;
	if (m_keys.down)
		m_pitch -= rotate_interval;

	// prevent looking too far up or down.
	m_pitch = std::min(m_pitch, XM_PIDIV4);
	m_pitch = std::max(-XM_PIDIV4, m_pitch);

	// move the camera in model space.
	float x = move_vector.x * -cosf(m_yaw) - move_vector.z * sinf(m_yaw);
	float z = move_vector.x * sinf(m_yaw) - move_vector.z * cosf(m_yaw);

	m_position.x += x * move_interval;
	m_position.z += z * move_interval;

	// determine the look direction.
	float r = cosf(m_pitch);
	m_look_at.x = r * sinf(m_yaw);
	m_look_at.y = sinf(m_pitch);
	m_look_at.z = r * cosf(m_yaw);

}

void camera::set_move_speed(float units_per_second)
{
	m_move_speed = units_per_second;
}

void camera::set_rotation_speed(float radians_per_second)
{
	m_rotation_speed = radians_per_second;
}

void camera::on_key_down(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keys.w = true;
		break;
	case 'A':
		m_keys.a = true;
		break;
	case 'S':
		m_keys.s = true;
		break;
	case 'D':
		m_keys.d = true;
		break;
	case VK_LEFT:
		m_keys.left = true;
		break;
	case VK_RIGHT:
		m_keys.right = true;
		break;
	case VK_UP:
		m_keys.up = true;
		break;
	case VK_DOWN:
		m_keys.down = true;
		break;
	case VK_ESCAPE:
		reset();
		break;
	}
}

void camera::on_key_up(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keys.w = false;
		break;
	case 'A':
		m_keys.a = false;
		break;
	case 'S':
		m_keys.s = false;
		break;
	case 'D':
		m_keys.d = false;
		break;
	case VK_LEFT:
		m_keys.left = false;
		break;
	case VK_RIGHT:
		m_keys.right = false;
		break;
	case VK_UP:
		m_keys.up = false;
		break;
	case VK_DOWN:
		m_keys.down = false;
		break;
	case VK_ESCAPE:
		reset();
		break;
	}
}

// this will be updated per frame.
DirectX::XMMATRIX camera::get_view_matrix() const
{
	return XMMatrixLookToRH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_look_at), XMLoadFloat3(&m_up));
}

DirectX::XMMATRIX camera::get_projection_matrix(float fov, float aspect_ratio, float near_plane, float far_plane) const
{
	return XMMatrixPerspectiveFovRH(fov, aspect_ratio, near_plane, far_plane);
}

void camera::reset()
{
	// reset the camera orientation and position back to where it was 
	m_position = m_initial_position;
	m_yaw = XM_PI;
	m_pitch = 0.0f;
	m_look_at = { 0, 0, -1 };
}

