#pragma once

#ifndef __primitive_camera__
#define __primitive_camera__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DirectXMath.h>

using namespace DirectX;

class primitive_camera
{
public:
	primitive_camera() = default;
	~primitive_camera() = default;

public:
	void update_aspect_ratio(float width, float height)
	{
		m_width = width;
		m_height = height;
		m_aspect_ratio = (width / height);
	}

	XMFLOAT4X4 update_model_view_proj_mat(POINT mouse_pos, float width, float height)
	{
		// update the aspect ratio
		update_aspect_ratio(width, height);

		m_proj_matrix_dx = XMMatrixPerspectiveFovLH(XMConvertToRadians(45), m_aspect_ratio, 1.0f, 100.0f);
		m_cam_pos_dx = DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
		m_cam_look_at_dx = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		m_cam_up_dx = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_view_matrix_dx = XMMatrixLookAtLH(m_cam_pos_dx, m_cam_look_at_dx, m_cam_up_dx);

		m_view_proj_matrix_dx = m_view_matrix_dx * m_proj_matrix_dx;

		float pitch{
		   -XMConvertToRadians(
			   (mouse_pos.x - (static_cast<float>(m_width) / 2.0f)) /
			   (static_cast<float>(m_width) / 2.0f) * 180.0f
		   )
		};

		float roll{
			XMConvertToRadians(
				(mouse_pos.y - (static_cast<float>(m_height) / 2.0f)) /
				(static_cast<float>(m_height) / 2.0f) * 180.0f
			)
		};

		XMMATRIX model_matrix_rotation_dx{ XMMatrixRotationRollPitchYaw(roll, pitch, 0.0f) };
		XMMATRIX model_matrix_translation_dx{ XMMatrixTranslation(0.0f, -1.0f, 0.0f) };
		XMMATRIX model_matrix_dx{ model_matrix_rotation_dx * model_matrix_translation_dx };

		XMFLOAT4X4 mvp_matrix;
		XMStoreFloat4x4(&mvp_matrix, model_matrix_dx * m_view_proj_matrix_dx);
		return mvp_matrix;
	}

	void rotate_camera(float dx, float dy, float dz)
	{
	}
	void translate_camera(float dx, float dy, float dz)
	{
	}
	// add pitch and yaw later on

private:
	float m_aspect_ratio{ 1.0f }; // should never be 0
	float m_width{ 100.0f }, m_height{ 100.0f };

private:
	XMMATRIX m_proj_matrix_dx{ XMMatrixPerspectiveFovLH(XMConvertToRadians(45), m_aspect_ratio, 1.0f, 100.0f) };
	XMVECTOR m_cam_pos_dx = DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
	XMVECTOR m_cam_look_at_dx = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR m_cam_up_dx = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX m_view_matrix_dx{ XMMatrixLookAtLH(m_cam_pos_dx, m_cam_look_at_dx, m_cam_up_dx) };

	XMMATRIX m_view_proj_matrix_dx{ m_view_matrix_dx * m_proj_matrix_dx };
};

#endif
