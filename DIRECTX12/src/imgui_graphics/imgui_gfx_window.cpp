#include "imgui_gfx_window.h"

imgui_gfx::imgui_gfx(
	HWND hWnd, ID3D12Device2* pDevice, ID3D12DescriptorHeap* srv, 
	DXGI_FORMAT bBufferFormat, int frames, UINT imgui_offset
)
	: m_srv_descriptor_heap(srv),
	  m_imgui_offset(imgui_offset),
	  m_srv_descriptor_size(pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);

	ImGui_ImplDX12_Init(
		pDevice, frames, bBufferFormat, srv,
		get_srv_cpu_handle(0),
		get_srv_gpu_handle(0)
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

void imgui_gfx::setup_window_docking()
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

void imgui_gfx::scene_stats()
{
	ImGui::Begin("scene stats window");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::End();
}

D3D12_CPU_DESCRIPTOR_HANDLE imgui_gfx::get_srv_cpu_handle(UINT local_offset) const
{
	// get the handle to the start of the descriptor heap on the cpu side
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{ m_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()};
	cpu_handle.ptr += m_srv_descriptor_size * (m_imgui_offset + local_offset);
	return cpu_handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE imgui_gfx::get_srv_gpu_handle(UINT local_offset) const
{
	// get the handle to the start of the descriptor heap on the gpu side
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{ m_srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart() };
	gpu_handle.ptr += m_srv_descriptor_size * (m_imgui_offset + local_offset);
	return gpu_handle;
}
