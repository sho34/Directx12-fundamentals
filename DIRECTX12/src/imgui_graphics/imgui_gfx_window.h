#pragma once

#ifndef __imgui_gfx_window__
#define __imgui_gfx_window__

#include <wrl.h>
#include <d3d12.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_internal.h"

class imgui_gfx
{
public:
	// delete the copy and assignment operators.
	imgui_gfx(const imgui_gfx&) = delete;
	imgui_gfx operator=(const imgui_gfx&) = delete;

public:
	imgui_gfx(
		HWND hWnd, ID3D12Device2* pDevice, ID3D12DescriptorHeap* srv,
		DXGI_FORMAT bBufferFormat, int frames, UINT imgui_offset
	);

	~imgui_gfx();

public:
	void init();
	void setup_window_docking();
	void render_imgui(void*);// pass a pointer to a command-list
	void scene_stats();
	void draw_scene(UINT texture_offset, ImVec2 dimensions);

	// check if the window is being resized.
	bool is_imgui_window_resized(ImGuiWindow* im_win);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srv_descriptor_heap;
	UINT m_imgui_srv_heap_offset;
	UINT m_srv_descriptor_size;

private:
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device;

private:
	// get the handles to the descriptor heaps.
	D3D12_CPU_DESCRIPTOR_HANDLE get_srv_cpu_handle(UINT local_offset) const;
	D3D12_GPU_DESCRIPTOR_HANDLE get_srv_gpu_handle(UINT local_offset) const;
	D3D12_GPU_DESCRIPTOR_HANDLE get_tex_gpu_handle(UINT texture_heap_offset) const;
};

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
