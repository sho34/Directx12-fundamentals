#pragma once
#ifndef __scene_editor_h__
#define __scene_editor_h__


#include <d3d12.h>
#include <wrl.h>

// For exception handling.
#include "../dx_exceptions/Directx12Ex.h"
#include "../dx_exceptions/exception-macros.h"

// Include imgui stuff.
#include "../imgui_graphics/imgui_gfx_window.h"

class scene_editor : public imgui_gfx
{
private:
	Microsoft::WRL::ComPtr<ID3D12Device2>			m_device;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_srv_descriptor_heap;

private:
	ImVec2				m_scene_window_dimensions{500, 500};
	ImGuiWindow*		m_scene_window;

public:
	scene_editor(
		HWND hWnd, 
		ID3D12Device2* pDevice, 
		ID3D12DescriptorHeap* srv,
		DXGI_FORMAT b_buffer_format,
		UINT frames, UINT imgui_offset
	);
	~scene_editor() = default;

private:
	void view_port();
	void scissor_rect();
	void set_scene_texture();

public:
	void render(float dt);
	void render_ui();

};

#endif
