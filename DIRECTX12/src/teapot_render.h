#pragma once
#ifndef __teapot_render__
#define __teapot_render__

#include "Directx12.h"


#include "dx_utils/utils.h"
#include "teapot_data/TeapotData.h"
#include "file_works/file_paths.h"

#include "imgui_graphics/imgui_gfx_window.h"
#include "render/texture_renderer.h"
#include "pipeline/pipeline_effects.h"

using namespace DirectX;
class teapot_render : public directx12_graphics
{
public:
	teapot_render(HWND hWnd, int width, int height);
	~teapot_render();

public:
	void render();
	void update(float delta_time);
	void get_mouse_pos(POINT);
	void handle_imgui_messages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void toggle_pipeline_state_solid();
	void toggle_pipeline_state_wire_frame();

private:
	void create_constant_buffer();
	void create_root_signature();
	void create_pipeline_state_wire_frame();
	void create_pipeline_state_solid();
	void create_view_port();
	void create_scissor_rect();


private:
	// for imgui graphics
	std::unique_ptr<imgui_gfx> p_imgui_gfx;

private:
	std::unique_ptr<texture>		m_p_off_screen_rt;
	ImVec2							m_imgui_window_dimensions{ 100.0f, 100.0f };
	ImGuiWindow*					m_imgui_window;
	ImRect							m_im_scissor_rect;

private:

	// GPU resources/ memory
	ComPtr<ID3D12Resource>					m_control_points_buffer;
	ComPtr<ID3D12Resource>					m_control_points_index_buffer;
	ComPtr<ID3D12Resource>					m_transforms_buffer;
	ComPtr<ID3D12Resource>					m_colors_buffer;
	ComPtr<ID3D12Resource>					m_constant_buffer;

	// root signature
	ComPtr<ID3D12RootSignature>				m_root_signature;

	// pipeline states
	ComPtr<ID3D12PipelineState>				m_pipeline_state_wire_frame;
	ComPtr<ID3D12PipelineState>				m_pipeline_state_solid;
	ComPtr<ID3D12PipelineState>				m_curr_pipeline_state;


private:
	D3D12_VERTEX_BUFFER_VIEW				m_control_points_buffer_view;
	D3D12_INDEX_BUFFER_VIEW					m_control_points_index_buffer_view;

	D3D12_VIEWPORT							m_view_port;
	D3D12_RECT								m_scissor_rect;

private:
	int					m_tess_factor{ 8 };
	const int			m_num_parts{ 28 };
	POINT				m_mouse_position;
};

#endif
