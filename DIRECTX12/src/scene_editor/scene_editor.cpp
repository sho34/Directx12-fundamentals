#include "scene_editor.h"

scene_editor::scene_editor(
	HWND hWnd,
	ID3D12Device2* p_device,
	ID3D12DescriptorHeap* srv,
	DXGI_FORMAT b_buffer_format,
	UINT frames, UINT imgui_offset
)
	: imgui_gfx{ hWnd, p_device, srv, b_buffer_format, frames, imgui_offset }
{}

void scene_editor::view_port()
{
}

void scene_editor::scissor_rect()
{
}

void scene_editor::render(float dt)
{
	// responsible for rendering graphics to the scene editor window.
}

void scene_editor::render_ui()
{
	// responsible for rendering the imgui editor.
}
