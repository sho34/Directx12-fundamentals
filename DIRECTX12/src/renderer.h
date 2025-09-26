#pragma once
#ifndef __renderer__
#define __renderer__

#include "Directx12.h"

#include "dx_utils/utils.h"
#include "file_works/file_paths.h"

#include "imgui_graphics/imgui_gfx_window.h"
#include "mesh_data/shape_data.h"
#include "render/texture_renderer.h"
#include "pipeline/pipeline_effects.h"
#include "root_signature/root_signature.h"
#include "dx_utils/constant_buffer.h"
#include "render/render_items.h"
#include "math_helpers/primitive_camera.h"
#include "texture-loader/DDSTextureLoader.h"
#include "ECS/componets.h"
#include "math_helpers/math_helpers.h"
#include "camera/camera.h"

//#include <DDSTextureLoader.h>



class renderer : public directx12_graphics
{
public:
	// delete the copy and assignment operator
	renderer(const renderer&) = delete;
	renderer operator=(const renderer&) = delete;

public:
	renderer(HWND h_wnd, int width, int height);
	~renderer() = default;

private:
	// i am going to keep track of the changing time for later animations
	float dt{ 0.0f };
	POINT m_mouse_position;

public:
	void draw_frame(float dt, POINT mouse_pos, UINT8 key);

private:
	void render_to_swapchain_buffer();
	void render_to_offscreen_texture();
	void init_render_items();
	void create_srv_descriptors();
	void create_instance_transforms();

private:
	std::unique_ptr<imgui_gfx>						p_imgui_gfx;
	std::unique_ptr<constant_buffer<XMFLOAT4X4>>	p_cb_model_view_proj;
	std::unique_ptr<offscreen_texture>				p_offscreen_texture;
	//std::unique_ptr<primitive_camera>				p_camera;

private:
	submesh_geometry mesh_geometry_data;
	std::vector<submesh_geometry> mesh_collection;
	std::unique_ptr<texture> p_crate_texture;

public:
	std::unique_ptr<camera> m_p_camera;

	// instance data
	std::vector<transform> m_transform_data;

	// make 3 of everything.
	int m_instance_count{ 1 };
private:
	// shape resources
	ComPtr<ID3D12Resource>					m_index_buffer;
	ComPtr<ID3D12Resource>					m_vertex_buffer;

	// instance buffer data
	ComPtr<ID3D12Resource>					m_instance_buffer;
	D3D12_VERTEX_BUFFER_VIEW				m_instance_buffer_view;


	// views to these buffers.
	D3D12_INDEX_BUFFER_VIEW					m_index_buffer_view;
	D3D12_VERTEX_BUFFER_VIEW				m_vertex_buffer_view;

	// root signature
	ComPtr<ID3D12RootSignature>				m_root_signature;

	// render item variables 
	ComPtr<ID3D12Resource>					m_constant_buffer;

	// pipeline states
	ComPtr<ID3D12PipelineState>				m_pipeline_state;
};

#endif
