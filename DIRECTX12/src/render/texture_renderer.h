#pragma once
#ifndef __texture_renderer__
#define __texture_renderer__

#include <d3d12.h>
#include <wrl.h>

#include "../dx_utils/d3dx12.h"
#include "../dx_exceptions/Directx12Ex.h"
#include "../dx_exceptions/exception-macros.h"

// To be able to use std::max.
#if defined(max)
#undef max
#endif


// THIS FUCKER'S ONLY JOB IS TO CREATE AN OFFSCREEN TETURE THAT I WILL USE LATER.
// EXAMPLE USE CASES ARE RENDERING MY ENTIRE SCENE TO AN IMGUI WINDOW, SAMPLING, ETC.


class texture
{
public:
	texture(
		DXGI_FORMAT format, 
		ID3D12Device2* p_device, 
		ID3D12DescriptorHeap* p_srv, 
		UINT64 srv_heap_offset,
		int width, int height
	);
	~texture() = default;

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> get_resource() const;

public:
	D3D12_CPU_DESCRIPTOR_HANDLE get_rtv_cpu_handle() const; // render target view cpu handle
	D3D12_GPU_DESCRIPTOR_HANDLE get_srv_gpu_handle() const; // shader resource view gpu handle
	D3D12_CPU_DESCRIPTOR_HANDLE get_dsv_cpu_handle() const; // depth stencil view cpu handle

private:
	// depth values if we are rendering to an off-screen texture.
	void create_offscreen_texture();
	void create_texture_descriptor_heap_dsv();
	void create_texture_depth_stencil_buffer();

public:
	void update_texture(float width, float height);
	void clear_texture(ID3D12GraphicsCommandList* p_command_list);
	void update_texture_depth_stencil_buffer(float width, float height);

private:
	Microsoft::WRL::ComPtr<ID3D12Device2>				m_device;
	Microsoft::WRL::ComPtr<ID3D12Resource>				m_off_screen_rt;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_shared_srv_desc;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_off_screen_rtv_heap;

	// for depth stencil 
	Microsoft::WRL::ComPtr<ID3D12Resource>				m_depth_stencil_buffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_dsv_descriptor_heap;

private:
	DXGI_FORMAT											m_format;
	UINT64												m_srv_heap_offset{ 0 };
	size_t												m_width{ 0 }, m_height{ 0 };
	float												m_clear_color[4]{ 0.1f, 0.1f, 0.1f, 1.0f };
};

#endif
