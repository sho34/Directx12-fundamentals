#include "renderer.h"

renderer::renderer(HWND h_wnd, int width, int height)
	: directx12_graphics(h_wnd, width, height)
{
	init_render_items();
}

void renderer::draw_frame(float dt, POINT mouse_pos)
{
	// update mouse position
	m_mouse_position = mouse_pos;

	// #1 get the current back buffer index 
	UINT frame_index{ m_swapchain_4->GetCurrentBackBufferIndex() };

	// #2 get the command allocator for that current frame of rendering 
	auto cmd_allocator{ m_command_allocators[frame_index].Get() };

	// #3 reset the command allocator and command list for drawing commands 
	THROW_GRAPHICS_INFO(cmd_allocator->Reset());
	THROW_GRAPHICS_INFO(m_command_list->Reset(cmd_allocator, nullptr));

	// #4 issue our draw calls.
	render_to_swapchain_buffer();

	// #4 close the command list 
	THROW_GRAPHICS_INFO(m_command_list->Close());

	// #5 add the commands to the command queue for execution
	ADD_TO_QUEUE(m_command_list, m_command_queue);

	// #6 present the rendered frame
	THROW_GRAPHICS_INFO(m_swapchain_4->Present(m_sync_interval, m_present_flags));

	// #7 synchronize with the GPU.
	UINT64& fence_value{ m_frame_fence_values[frame_index] };
	++fence_value;
	ComPtr<ID3D12Fence> fence{ m_fences[frame_index] };
	THROW_GRAPHICS_INFO(m_command_queue->Signal(fence.Get(), fence_value));

	// #8 wait for the current frame to finish rendering to the buffer.
	wait_for_frame(frame_index);

}

void renderer::render_to_swapchain_buffer()
{
	// #1 get the current back buffer index 
	UINT frame_index{ m_swapchain_4->GetCurrentBackBufferIndex() };

	// in here we will prepare the commands needed to draw our frame
	m_command_list->SetPipelineState(m_pipeline_state.Get());
	m_command_list->SetGraphicsRootSignature(m_root_signature.Get());
	m_command_list->RSSetViewports(1, &m_view_port);
	m_command_list->RSSetScissorRects(1, &m_scissor_rect);

	// #4 get the current back buffer and then render to it.
	ID3D12Resource* current_buffer{ m_swap_chain_buffers[frame_index].Get() };

	// #5 set the resource barrier
	CD3DX12_RESOURCE_BARRIER barrier_desc_rt{
		CD3DX12_RESOURCE_BARRIER::Transition(
			current_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	};

	m_command_list->ResourceBarrier(1, &barrier_desc_rt);


	// #6 offsetting to the descriptor handle for that buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE desc_handle_rtv{ m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() };
	desc_handle_rtv.ptr += static_cast<unsigned long long>(frame_index) * m_rtv_descriptor_size;

	D3D12_CPU_DESCRIPTOR_HANDLE desc_handle_dsv{ m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() };
	D3D12_GPU_DESCRIPTOR_HANDLE tex_srv_gpu{ m_shared_descriptor_heap->GetGPUDescriptorHandleForHeapStart() };
	tex_srv_gpu.ptr += 0;

	// #7 pass the address of the first descriptor 
	ID3D12DescriptorHeap* pp_heaps[] = { m_shared_descriptor_heap.Get() };
	m_command_list->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

	m_command_list->SetGraphicsRootDescriptorTable(1, tex_srv_gpu);

	// set our current back buffer as the render target.
	m_command_list->OMSetRenderTargets(1, &desc_handle_rtv, FALSE, &desc_handle_dsv);

	// #7 paint it with the color below.
	static float clear_color[]{ 0.2f, 0.2f, 0.2f, 1.0f };
	m_command_list->ClearRenderTargetView(desc_handle_rtv, clear_color, 0, nullptr);
	m_command_list->ClearDepthStencilView(
		m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr
	);

	// world view projection matrix
	{
		XMFLOAT4X4 mvp_matrix;
		mvp_matrix = p_camera->update_model_view_proj_mat(m_mouse_position, m_client_width, m_client_height);

		p_cb_model_view_proj->copy_data_to_buffer(&mvp_matrix, frame_index);
		m_command_list->SetGraphicsRootConstantBufferView(
			0, p_cb_model_view_proj->get_gpu_virtual_address_aligned(frame_index)
		);
	}


	m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_command_list->IASetIndexBuffer(&m_index_buffer_view);
	m_command_list->IASetVertexBuffers(0, 1, &m_vertex_buffer_view);
	m_command_list->DrawIndexedInstanced(
		mesh_geometry_data.indices.size(), 1, 0, 0, 0	
	);


	// render imgui
	p_imgui_gfx->init();
	p_imgui_gfx->dock_space();
	p_imgui_gfx->scene_stats();
	p_imgui_gfx->render_imgui(m_command_list.Get());

	CD3DX12_RESOURCE_BARRIER barrier_desc_present{
	CD3DX12_RESOURCE_BARRIER::Transition(
		current_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
	)
	};

	m_command_list->ResourceBarrier(1, &barrier_desc_present);
}

void renderer::render_to_offscreen_texture()
{

}

void renderer::init_render_items()
{
	// create the pipeline state and the root signature.
	root_signature shader_inputs(m_dx12_device.Get());
	shader_inputs.create_root_descriptor(
		D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0, 0
	);
	shader_inputs.create_static_samplers(0, 0);
	shader_inputs.create_descriptor_table(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 2, 0, 0);
	shader_inputs.finalize_root_sig_creation(
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS 
	);
	m_root_signature = shader_inputs.get_root_signature();

	simple_pso pipeline_simple(m_dx12_device.Get(), m_root_signature.Get());
	pipeline_simple.initialize();
	m_pipeline_state = pipeline_simple.get_pso();

	mesh_geometry_data.vertices = triangle_mesh_textured();
	mesh_geometry_data.indices = triangle_indices_texture();

	// create the wood texture structure to hold its meta data.
	p_crate_texture = std::make_unique<texture>();
	p_crate_texture->name = "wood crate texture";
	p_crate_texture->file_name = L"C:\\Users\\Gumball\\Desktop\\projects\\REV-1\\DIRECTX12\\src\\assets\\WoodCrate01.dds";

	D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor{ m_shared_descriptor_heap->GetCPUDescriptorHandleForHeapStart() };
	hDescriptor.ptr += 0;

	// load the texture from file.
	THROW_GRAPHICS_INFO(m_command_list->Reset(m_command_allocators[1].Get(), nullptr));
	THROW_GRAPHICS_INFO(
		CreateDDSTextureFromFile12(
			m_dx12_device.Get(), m_command_list.Get(),
			p_crate_texture->file_name.c_str(), p_crate_texture->texture_resouce, p_crate_texture->upload_heap
		)
	);

	THROW_GRAPHICS_INFO(p_crate_texture->upload_heap->SetName(L"Upload heap crate texture"));
	THROW_GRAPHICS_INFO(p_crate_texture->texture_resouce->SetName(L"Crate texture"));

	// create the actual resource description for that texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = p_crate_texture->texture_resouce->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = p_crate_texture->texture_resouce->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_dx12_device->CreateShaderResourceView(p_crate_texture->texture_resouce.Get(), &srvDesc, hDescriptor);

	// #5 add the commands to the command queue for execution
	THROW_GRAPHICS_INFO(m_command_list->Close());
	ADD_TO_QUEUE(m_command_list, m_command_queue);


	m_vertex_buffer = utility_functions::create_vertex_buffer(
		m_dx12_device.Get(), mesh_geometry_data.vertices, L"triangle vertices"
	);

	m_vertex_buffer_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
	m_vertex_buffer_view.StrideInBytes = static_cast<UINT>(sizeof(VERTEX));
	m_vertex_buffer_view.SizeInBytes = static_cast<UINT>(
		m_vertex_buffer_view.StrideInBytes * mesh_geometry_data.vertices.size()
	);

	// create the index buffer
	m_index_buffer = utility_functions::create_index_buffer(
		m_dx12_device.Get(), mesh_geometry_data.indices, L"triangle indices"
	);

	m_index_buffer_view.BufferLocation = m_index_buffer->GetGPUVirtualAddress();
	m_index_buffer_view.Format = DXGI_FORMAT_R16_UINT;
	m_index_buffer_view.SizeInBytes = mesh_geometry_data.indices.size() * sizeof(uint16_t);

	// create the constant buffer for the world view projection matrix.
	p_cb_model_view_proj = std::make_unique<constant_buffer<XMFLOAT4X4>>(m_dx12_device.Get());

	// create and imgui graphics object using offset 0 of the shared descriptor heap.
	p_imgui_gfx = std::make_unique<imgui_gfx>(
		m_hwnd, m_dx12_device.Get(), m_shared_descriptor_heap.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM, m_buffer_count, 1
	);

	p_camera = std::make_unique<camera>();


}

void renderer::create_srv_descriptors()
{

}
