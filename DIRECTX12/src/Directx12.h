#pragma once
#ifndef __Directx12__
#define __Directx12__

#define WIN32_LEAN_AND_MEAN

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <d3dcompiler.h>

// std template libraries
#include <vector>
#include <chrono>
#include <algorithm>

// for the error checking and debugging.
#include "dx_exceptions/Directx12Ex.h"
#include "dx_exceptions/exception-macros.h"

// Directx12 extension library
#include "dx_utils/d3dx12.h"
#include "dx_utils/string_works.h"

#if defined(max)
#undef max
#endif

#if defined(min)
#undef min
#endif

using namespace Microsoft::WRL;

class directx12_graphics
{

public:
	// delete the copy and copy assignment operators.
	directx12_graphics(const directx12_graphics&) = delete;
	directx12_graphics operator=(const directx12_graphics&) = delete;
	~directx12_graphics();

public:
	directx12_graphics(HWND hWnd, int width, int height);
	// resize the buffers.
	void resize_buffers(uint32_t width, uint32_t height);
	void update_fps();

private:
	void initiallize_pipeline_resources();
	void initialize_graphics_adapter_and_factory_device();
	void log_available_graphics_adapters();
	void create_command_allocators();
	void create_command_list();
	void create_fences();
	void create_fence_event_handle();
	void create_command_queue();
	void check_tearing_support();
	void create_swap_chain();
	void get_swap_chain_buffers();
	void create_descriptor_heap_rtv();
	void create_descriptor_heap_shared();
	void create_depth_stencil_buffer();
	void create_descriptor_heap_depth_stencil();

protected:
	void wait_for_frame(UINT frameIndex);
	void update_render_target_views();

public:
	void toggle_v_sync();
	void activate_v_sync_parameters();
	void update_depth_stencil_buffer();


public:
	std::wstring get_adapter_details() { return adapater_details; };

protected:
	static const uint8_t	m_buffer_count{ 3 };
	bool					m_use_warp{ false };
	uint32_t				m_client_width;
	uint32_t				m_client_height;
	HWND					m_hwnd;

protected:
	ComPtr<ID3D12Device2>				m_dx12_device;
	ComPtr<ID3D12CommandQueue>			m_command_queue;
	ComPtr<IDXGISwapChain4>				m_swapchain_4;
	ComPtr<ID3D12GraphicsCommandList>	m_command_list;
	ComPtr<ID3D12DescriptorHeap>		m_rtv_descriptor_heap;
	ComPtr<ID3D12DescriptorHeap>		m_dsv_descriptor_heap;
	ComPtr<ID3D12DescriptorHeap>		m_shared_descriptor_heap;
	ComPtr<IDXGIAdapter4>				m_dxgi_adapter_4;
	ComPtr<IDXGIFactory4>				m_dxgi_factory_4;
	ComPtr<ID3D12Resource>				m_depth_stencil_buffer;

	std::vector<ComPtr<ID3D12Resource>>				m_swap_chain_buffers;
	std::vector<ComPtr<ID3D12CommandAllocator>>		m_command_allocators;

protected:
	UINT		m_rtv_descriptor_size;
	UINT		m_current_back_buffer_index;
	UINT		m_present_flags;
	UINT		m_sync_interval;
protected:
	// synchronization objects.
	ComPtr<ID3D12Fence> m_fence;
	uint64_t			m_fence_value;
	HANDLE				m_fence_event_handle;

	std::vector<uint64_t>				m_frame_fence_values;
	std::vector<ComPtr<ID3D12Fence>>	m_fences;

protected:
	std::string 						m_frame_stats;

private:
	// to log the list of available adapters.
	std::vector<ComPtr<IDXGIAdapter4>>		m_adapter_list;

protected:
	// variables to control the swap-chain present method.
	bool m_is_vsync_enabled{ true };
	BOOL m_is_tearing_supported{ FALSE };
	bool m_is_fullscreen_active{ false };

private:
	std::wstring adapater_details;

};

#endif
