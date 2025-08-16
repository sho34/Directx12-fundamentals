#pragma once
#ifndef __imgui_gfx_window__
#define __imgui_gfx_window__

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include <wrl.h>
#include <d3d12.h>


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "imgui_internal.h"

class imgui_gfx
{
public:
	// delete the copy and assignment operators.
	imgui_gfx(const imgui_gfx&) = delete;
	imgui_gfx operator=(const imgui_gfx&) = delete;

public:
	imgui_gfx(
		HWND hWnd,
		ID3D12Device2* pDevice,
		ID3D12DescriptorHeap* srv,
		DXGI_FORMAT bBufferFormat,
		int frames
	);

	~imgui_gfx();

public:
	void init();
	void set_up_window_docking();
	void render_imgui(void*);
	void test_window();
};

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
