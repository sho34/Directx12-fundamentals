#include "imgui_gfx_window.h"

imgui_gfx::imgui_gfx(
	HWND hWnd, ID3D12Device2* pDevice, ID3D12DescriptorHeap* srv, DXGI_FORMAT bBufferFormat, int frames
)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);

	ImGui_ImplDX12_Init(
		pDevice, frames, bBufferFormat, srv,
		srv->GetCPUDescriptorHandleForHeapStart(),
		srv->GetGPUDescriptorHandleForHeapStart()
	);

}

imgui_gfx::~imgui_gfx()
{
	// clean up imgui
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void imgui_gfx::init()
{
	// start the imgui frame in here 
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void imgui_gfx::set_up_window_docking()
{

}

void imgui_gfx::render_imgui(void* renderArgs)
{
	// render the imgui graphics 
	ImGui::Render();

	/// render imgui in here 
	ImGui_ImplDX12_RenderDrawData(
		ImGui::GetDrawData(), reinterpret_cast<ID3D12GraphicsCommandList*>(renderArgs)
	);
}

void imgui_gfx::test_window()
{
	ImGui::Begin("test window");
	ImGui::End();
}
