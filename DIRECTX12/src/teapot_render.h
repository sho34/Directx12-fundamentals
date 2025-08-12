#pragma once
#ifndef __teapot_render__
#define __teapot_render__

#include "Directx12.h"


#include "dx_utils/utils.h"
#include "teapot_data/TeapotData.h"
#include "file_works/file_paths.h"
#include "imgui_graphics/imgui_gfx_window.h"

using namespace DirectX;
class teapot_render : public directx12_graphics
{
public:
	teapot_render(HWND hWnd, int width, int height);

public:
	void render();

private:
	void create_transforms_and_colors_desc_heap();
	void create_constant_buffer();
	void load_shaders_to_memory();
	void create_root_signature();
	void create_pipeline_state_wire_frame();
	void create_pipeline_state_solid();
	void create_view_port();
	void create_scissor_rect();

private:
	ComPtr<ID3D12PipelineState> create_pipeline_state(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode);
	// for imgui graphics
	std::unique_ptr<imgui_gfx> p_imgui_gfx;

private:

	// GPU resources/ memory
	ComPtr<ID3D12Resource>					m_control_points_buffer;
	ComPtr<ID3D12Resource>					m_control_points_index_buffer;
	ComPtr<ID3D12Resource>					m_transforms_buffer;
	ComPtr<ID3D12Resource>					m_colors_buffer;
	ComPtr<ID3D12Resource>					m_constant_buffer;

	// descriptor heaps
	ComPtr<ID3D12DescriptorHeap>			m_transforms_and_colors_desc_heap;

	// shader resources
	ComPtr<ID3DBlob>						m_vertex_shader_blob;
	ComPtr<ID3DBlob>						m_hull_shader_blob;
	ComPtr<ID3DBlob>						m_domain_shader_blob;
	ComPtr<ID3DBlob>						m_pixel_shader_blob;

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
	std::wstring		m_shader_dir{ get_file_pathw(L"output\\Debug\\x64")};
};

#endif
