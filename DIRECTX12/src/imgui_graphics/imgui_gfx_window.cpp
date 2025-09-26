#include "imgui_gfx_window.h"

imgui_gfx::imgui_gfx(
	HWND hWnd, ID3D12Device2* pDevice, ID3D12DescriptorHeap* srv, 
	DXGI_FORMAT bBufferFormat, UINT frames, UINT imgui_offset
)
	: m_srv_descriptor_heap(srv)
	, m_imgui_srv_heap_offset(imgui_offset)
	, m_srv_descriptor_size(pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	, m_device(pDevice)
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
	// the widget is placed at the bottom right corner.
	static int location = 3;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if (location >= 0)
	{
		const float PAD = 10.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
		window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
		window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
		window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowViewport(viewport->ID);
		window_flags |= ImGuiWindowFlags_NoMove;
	}
	else if (location == -2)
	{
		// Center window
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		window_flags |= ImGuiWindowFlags_NoMove;
	}
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	if (ImGui::Begin("FRAME STATS WINDOW: ", &m_show_demo_window, window_flags))
	{
		ImGui::Text("Frame stats window\n" "(right-click to change position)");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Separator();
		// optionally show the demo window.
		ImGui::Text("Show demo window");
		ImGui::Checkbox("show", &m_show_demo_window);
		if (m_show_demo_window) { ImGui::ShowDemoWindow(&m_show_demo_window); }
		ImGui::Separator();
		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse Position: <invalid>");

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
			if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
			if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
			if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

void imgui_gfx::object_manager()
{
	ImGui::Begin("objects");
	ImGui::End();
}

void imgui_gfx::draw_scene(const D3D12_GPU_DESCRIPTOR_HANDLE texture_gpu_handle)
{
	ImGui::Begin("Scene view");
	ImVec2 view_port_panel_size = ImGui::GetContentRegionAvail();	// get current window dimensions.
	ImTextureID tex_id = reinterpret_cast<void*>(texture_gpu_handle.ptr);
	ImGui::Image(tex_id, view_port_panel_size);
	ImGui::End();
}

ImGuiWindow* imgui_gfx::get_scene_window() const
{
	return ImGui::FindWindowByName("Scene view");
}

D3D12_CPU_DESCRIPTOR_HANDLE imgui_gfx::get_srv_cpu_handle(UINT local_offset) const
{
	// get the handle to the start of the descriptor heap on the cpu side
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle{ m_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()};
	cpu_handle.ptr += m_srv_descriptor_size * (m_imgui_srv_heap_offset + local_offset);
	return cpu_handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE imgui_gfx::get_srv_gpu_handle(UINT local_offset) const
{
	// get the handle to the start of the descriptor heap on the gpu side
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{ m_srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart() };
	gpu_handle.ptr += m_srv_descriptor_size * (m_imgui_srv_heap_offset + local_offset);
	return gpu_handle;
}

