#include "teapot_render.h"

teapot_render::teapot_render(HWND hWnd, int width, int height)  

    : directx12_graphics{ hWnd, width, height }
{

    // vertex buffer data
    m_control_points_buffer = utility_functions::create_vertex_buffer(
        m_dx12_device.Get(), TeapotData::points, L"control points"
    );

    // describe the data in the GPU memory
    using point_type = decltype(TeapotData::points)::value_type;

    m_control_points_buffer_view.BufferLocation = m_control_points_buffer->GetGPUVirtualAddress();
    m_control_points_buffer_view.StrideInBytes = static_cast<UINT>(sizeof(point_type));
    m_control_points_buffer_view.SizeInBytes = static_cast<UINT>(
            m_control_points_buffer_view.StrideInBytes * TeapotData::points.size()
        );

    // index buffer data
    m_control_points_index_buffer = utility_functions::create_index_buffer(
        m_dx12_device.Get(), TeapotData::patches, L"patches"
    );

    m_control_points_index_buffer_view.BufferLocation = m_control_points_index_buffer->GetGPUVirtualAddress();
    m_control_points_index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    m_control_points_index_buffer_view.SizeInBytes = static_cast<UINT>(
            TeapotData::patches.size() * sizeof(uint32_t)
        );

    // initialize the transform and colors buffer
    m_transforms_buffer = utility_functions::create_structured_buffer(
        m_dx12_device.Get(), TeapotData::patchesTransforms, L"transforms"
    );
    m_colors_buffer = utility_functions::create_structured_buffer(
        m_dx12_device.Get(), TeapotData::patchesColors, L"colors"
    );

    using transform_type = decltype(TeapotData::patchesTransforms)::value_type;
    using color_type = decltype(TeapotData::patchesColors)::value_type;

    
    //fill the m_transforms_and_colors_desc_heap with 2 descriptors ()
    auto transform_gpu_handle = utility_functions::create_srv<transform_type>(
        m_dx12_device.Get(), m_shared_descriptor_heap.Get(), 0,
        m_transforms_buffer.Get(), TeapotData::patchesTransforms.size()
    );

    auto colors_gpu_handle = utility_functions::create_srv<color_type>(
        m_dx12_device.Get(), m_shared_descriptor_heap.Get(), 1,
        m_colors_buffer.Get(), TeapotData::patchesColors.size()
    );

    // imgui will use descriptors starting at offset 2.
    p_imgui_gfx = std::make_unique<imgui_gfx>(
		m_hwnd, m_dx12_device.Get(), m_shared_descriptor_heap.Get(), DXGI_FORMAT_R8G8B8A8_UNORM, m_buffer_count, 2
    );

    create_constant_buffer();
    load_shaders_to_memory();
    create_root_signature();
    create_pipeline_state_wire_frame();
    create_pipeline_state_solid();
    create_view_port();
    create_scissor_rect();
}

teapot_render::~teapot_render()
{

}

void teapot_render::render()
{
    // #1  get the current back buffer index 
    UINT frame_index{ m_swapchain_4->GetCurrentBackBufferIndex() };

    // #2 get the command allocator associated with that frame.
    ComPtr<ID3D12CommandAllocator> command_allocator{ m_command_allocators[frame_index] };

    // reset the command allocator for current usage.
    THROW_GRAPHICS_INFO(command_allocator->Reset());
    THROW_GRAPHICS_INFO(m_command_list->Reset(command_allocator.Get(), nullptr));

    // #3
    m_command_list->SetPipelineState(m_curr_pipeline_state.Get());
    //m_command_list->SetGraphicsRootSignature(m_root_signature.Get());
    m_command_list->RSSetViewports(1, &m_view_port);
    m_command_list->RSSetScissorRects(1, &m_scissor_rect);


    // #4 index into the buffer vector and fetch the buffer for current frame rendering.
    ID3D12Resource* current_buffer{ m_swap_chain_buffers[frame_index].Get() };

    // #5
    CD3DX12_RESOURCE_BARRIER barrier_desc_rt{
        CD3DX12_RESOURCE_BARRIER::Transition(
			current_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
        ) 
    };

    m_command_list->ResourceBarrier(1, &barrier_desc_rt);

    // #6 offsetting to the descriptor handle for that buffer.
    D3D12_CPU_DESCRIPTOR_HANDLE desc_handle_rtv{ m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() };
    desc_handle_rtv.ptr += frame_index * m_rtv_descriptor_size;

    D3D12_CPU_DESCRIPTOR_HANDLE desc_handle_dsv{ m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() };

    // set our current back buffer as the render target.
    m_command_list->OMSetRenderTargets(1, &desc_handle_rtv, FALSE, &desc_handle_dsv);

    // #7 paint it with the color below.
    static float clear_color[]{ 0.4f, 0.3f, 0.6f, 1.0f };
    m_command_list->ClearRenderTargetView(desc_handle_rtv, clear_color, 0, nullptr);
    m_command_list->ClearDepthStencilView(
        m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr
    );


 //   m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

 //   // #8
 //   std::vector<D3D12_VERTEX_BUFFER_VIEW> my_array{ m_control_points_buffer_view };
 //   m_command_list->IASetVertexBuffers(0, static_cast<UINT>(my_array.size()), my_array.data());

 //   // #9
 //   std::vector<int> root_constants{ m_tess_factor, m_tess_factor };
 //   m_command_list->SetGraphicsRoot32BitConstants(
 //       1, static_cast<UINT>(root_constants.size()), root_constants.data(), 0
 //   );


    // #10 pass the address of the first descriptor 
    ID3D12DescriptorHeap* pp_heaps[] = { m_shared_descriptor_heap.Get() };
    m_command_list->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

 //   D3D12_GPU_DESCRIPTOR_HANDLE d{ m_shared_descriptor_heap->GetGPUDescriptorHandleForHeapStart() };
 //   d.ptr += 0;
 //   m_command_list->SetGraphicsRootDescriptorTable(2, d);

 //   // #11
 //   float aspect_ratio{ static_cast<float>(m_client_width) / static_cast<float>(m_client_height) };
 //   XMMATRIX proj_matrix_dx{ XMMatrixPerspectiveFovLH(XMConvertToRadians(45), aspect_ratio, 1.0f, 100.0f) };

	//XMVECTOR cam_pos_dx(XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f));
	//XMVECTOR cam_look_at_dx(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
	//XMVECTOR cam_up_dx(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	//XMMATRIX view_matrix_dx{ XMMatrixLookAtLH(cam_pos_dx, cam_look_at_dx, cam_up_dx) };

 //   XMMATRIX view_proj_matrix_dx{ view_matrix_dx * proj_matrix_dx };

 //   UINT const_data_size_aligned{ (sizeof(XMFLOAT4X4) + 255) & ~255 };
 //   float pitch{
 //       -XMConvertToRadians(
 //           (m_mouse_position.x - (static_cast<float>(m_client_width) / 2.0f)) / 
 //           (static_cast<float>(m_client_width) / 2.0f) * 180.0f
 //       )
 //   };

 //   float roll{
 //       XMConvertToRadians(
 //           (m_mouse_position.y - (static_cast<float>(m_client_height) / 2.0f)) /
 //           (static_cast<float>(m_client_height) / 2.0f) * 180.0f
 //       )
 //   };

	//XMMATRIX model_matrix_rotation_dx{ XMMatrixRotationRollPitchYaw(roll, pitch, 0.0f) };
	//XMMATRIX model_matrix_translation_dx{ XMMatrixTranslation(0.0f, -1.0f, 0.0f) };
	//XMMATRIX model_matrix_dx{ model_matrix_rotation_dx * model_matrix_translation_dx };

 //   XMFLOAT4X4 mvp_matrix;
 //   XMStoreFloat4x4(&mvp_matrix, model_matrix_dx * view_proj_matrix_dx);

 //   // #12
 //   D3D12_RANGE read_range = { 0, 0 };
 //   uint8_t* cbv_data_begin;
 //   m_constant_buffer->Map(0, &read_range, reinterpret_cast<void**>(&cbv_data_begin));
 //   ::memcpy(&cbv_data_begin[frame_index * const_data_size_aligned], &mvp_matrix, sizeof(mvp_matrix));
 //   m_constant_buffer->Unmap(0, nullptr);

 //   // #13
 //   m_command_list->SetGraphicsRootConstantBufferView(
 //       0, m_constant_buffer->GetGPUVirtualAddress() + frame_index * const_data_size_aligned
 //   );

 //   // #14
 //   m_command_list->IASetIndexBuffer(&m_control_points_index_buffer_view);


 //   // #15
 //   uint32_t num_indices{ m_control_points_index_buffer_view.SizeInBytes / sizeof(uint32_t) };
 //   m_command_list->DrawIndexedInstanced(num_indices, 1, 0, 0, 0);


	// render imgui
	p_imgui_gfx->init();
	p_imgui_gfx->scene_stats();
    p_imgui_gfx->render_imgui(m_command_list.Get());

    // #16
	CD3DX12_RESOURCE_BARRIER barrier_desc_present{
	    CD3DX12_RESOURCE_BARRIER::Transition(
		    current_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
	    )
	};

    m_command_list->ResourceBarrier(1, &barrier_desc_present);

    // #17
    THROW_GRAPHICS_INFO(m_command_list->Close());

    // #18
    ID3D12CommandList* cmd_lists{ m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(1, &cmd_lists);

    // #19
    THROW_GRAPHICS_INFO(m_swapchain_4->Present(m_sync_interval, m_present_flags));

    // #20
    UINT64& fence_value{ m_frame_fence_values[frame_index] };
    ++fence_value;
    ComPtr<ID3D12Fence> fence{ m_fences[frame_index] };
    THROW_GRAPHICS_INFO(m_command_queue->Signal(fence.Get(), fence_value));

    // #21 wait for the current frame to finish rendering to the buffer.
    wait_for_frame(m_swapchain_4->GetCurrentBackBufferIndex());

}


void teapot_render::get_mouse_pos(POINT mouse_pos)
{
#if defined(_DEBUG)
    {
		std::ostringstream oss;
		oss << "mouse pos (" << mouse_pos.x << ", " << mouse_pos.y << ")\n";
		//::OutputDebugString(oss.str().c_str());
    }
#endif

    m_mouse_position = mouse_pos;
}


void teapot_render::handle_imgui_messages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

void teapot_render::toggle_pipeline_state_solid()
{
    m_curr_pipeline_state = m_pipeline_state_solid;
}

void teapot_render::toggle_pipeline_state_wire_frame()
{
	m_curr_pipeline_state = m_pipeline_state_wire_frame;
}


void teapot_render::create_constant_buffer()
{
    // will be updated every frame.
    UINT element_size_aligned{ (sizeof(XMFLOAT4X4) + 255) & ~255 };
    UINT64 buffer_size{ element_size_aligned * m_buffer_count };

    D3D12_HEAP_PROPERTIES heap_properties;
    ::ZeroMemory(&heap_properties, sizeof(heap_properties));
    heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_properties.CreationNodeMask = 1;
    heap_properties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resource_description;
    ::ZeroMemory(&resource_description, sizeof((resource_description)));
    resource_description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_description.Alignment = 0;
    resource_description.Width = buffer_size;
    resource_description.Height = 1;
    resource_description.DepthOrArraySize = 1;
    resource_description.MipLevels = 1;
    resource_description.Format = DXGI_FORMAT_UNKNOWN;
    resource_description.SampleDesc.Count = 1;
    resource_description.SampleDesc.Quality = 0;
    resource_description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_description.Flags = D3D12_RESOURCE_FLAG_NONE;

    THROW_GRAPHICS_INFO(
        m_dx12_device->CreateCommittedResource(
            &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_description,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_constant_buffer)
        )
    );

    THROW_GRAPHICS_INFO(m_constant_buffer->SetName(L"per frame constants"));
}

void teapot_render::load_shaders_to_memory()
{
    THROW_GRAPHICS_INFO(
        D3DReadFileToBlob(
          AnsiToWString(_FP_("VertexShader.cso")).c_str(),
            m_vertex_shader_blob.ReleaseAndGetAddressOf())
    );
    THROW_GRAPHICS_INFO(
        D3DReadFileToBlob(
           AnsiToWString(_FP_("PixelShader.cso")).c_str(), 
            m_pixel_shader_blob.ReleaseAndGetAddressOf())
    );
    THROW_GRAPHICS_INFO(
        D3DReadFileToBlob(
            AnsiToWString(_FP_("HullShader.cso")).c_str(),
            m_hull_shader_blob.ReleaseAndGetAddressOf())
    );
    THROW_GRAPHICS_INFO(
        D3DReadFileToBlob(
            AnsiToWString(_FP_("DomainShader.cso")).c_str(), 
            m_domain_shader_blob.ReleaseAndGetAddressOf())
    );
}

void teapot_render::create_root_signature()
{
    // * THIS IS A FUNCTION WHERE THE INPUTS TO THE SHADERS ARE DECLARED.

    // #1
    D3D12_DESCRIPTOR_RANGE dsv_transfrm_and_color_srv_range;
    ::ZeroMemory(&dsv_transfrm_and_color_srv_range, sizeof(dsv_transfrm_and_color_srv_range));

    dsv_transfrm_and_color_srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    dsv_transfrm_and_color_srv_range.NumDescriptors = 2; // we have two structured buffers and two descriptors.
    dsv_transfrm_and_color_srv_range.BaseShaderRegister = 0; // starting from the first register t0.
    dsv_transfrm_and_color_srv_range.RegisterSpace = 0; // allows us to use the same register by using diff space.
    dsv_transfrm_and_color_srv_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // #2
    D3D12_ROOT_PARAMETER ds_transform_and_color_srv;
    ::ZeroMemory(&ds_transform_and_color_srv, sizeof(ds_transform_and_color_srv));
    ds_transform_and_color_srv.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    ds_transform_and_color_srv.DescriptorTable = { 1, &dsv_transfrm_and_color_srv_range }; // one range.
    ds_transform_and_color_srv.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN; // used by the domain shader.

    // #3
    D3D12_ROOT_PARAMETER ds_obj_cb;
    ::ZeroMemory(&ds_obj_cb, sizeof(ds_obj_cb));
    ds_obj_cb.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    ds_obj_cb.Descriptor = { 0, 0 };    // first register b0 in first space.
    ds_obj_cb.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN; // only used by the domain shader.

    // #4
    D3D12_ROOT_PARAMETER hs_tess_factors_cb;
    ::ZeroMemory(&hs_tess_factors_cb, sizeof(hs_tess_factors_cb));
    hs_tess_factors_cb.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    hs_tess_factors_cb.Constants = { 0, 0, 2 }; // 2 constants in first register b0 in first register space.
    hs_tess_factors_cb.ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL; // only used in the hull shader.

    std::vector<D3D12_ROOT_PARAMETER> root_parameters{ ds_obj_cb, hs_tess_factors_cb, ds_transform_and_color_srv };

    // #5
    D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags{
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |  // using vertex and index buffers.
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS     |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS   |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
    };

    // #6
    D3D12_ROOT_SIGNATURE_DESC root_signature_desc;
    ::ZeroMemory(&root_signature_desc, sizeof(root_signature_desc));
    root_signature_desc.NumParameters = static_cast<UINT>(root_parameters.size());
    root_signature_desc.pParameters = root_parameters.data();
    root_signature_desc.NumStaticSamplers = 0; // can be stored in root signature separately and consume no space.
    root_signature_desc.pStaticSamplers = nullptr; // we are not using texturing.
    root_signature_desc.Flags = root_signature_flags;

    // #7
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    THROW_GRAPHICS_INFO(
        D3D12SerializeRootSignature(
            &root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, 
            signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()

        )
    );

    // #8 create the root signature.
    THROW_GRAPHICS_INFO(
        m_dx12_device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(m_root_signature.ReleaseAndGetAddressOf())
        )
    );

}

void teapot_render::create_pipeline_state_wire_frame()
{
    m_pipeline_state_wire_frame = create_pipeline_state(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE);
    m_curr_pipeline_state = m_pipeline_state_wire_frame;
}

void teapot_render::create_pipeline_state_solid()
{
    m_pipeline_state_solid = create_pipeline_state(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);
}

void teapot_render::create_view_port()
{
	m_view_port.TopLeftX = 0.0f;
    m_view_port.TopLeftY = 0.0f;
    m_view_port.Width = static_cast<FLOAT>(m_client_width);
    m_view_port.Height = static_cast<FLOAT>(m_client_height);
    m_view_port.MinDepth = 0.0f;
    m_view_port.MaxDepth = 1.0f;
}

void teapot_render::create_scissor_rect()
{
	RECT rect;
	if (!GetClientRect(m_hwnd, &rect))
	{
		throw(std::runtime_error{ "Error getting window size." });
	}

	m_scissor_rect.left = 0;
	m_scissor_rect.top = 0;
	m_scissor_rect.right = rect.right - rect.left;
	m_scissor_rect.bottom = rect.bottom - rect.top;
}


ComPtr<ID3D12PipelineState> teapot_render::create_pipeline_state(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode)
{
    // #1
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_element_descriptors
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // #2
    D3D12_RASTERIZER_DESC rasterizer_desc;
    ::ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
    rasterizer_desc.FillMode = fillMode;
    rasterizer_desc.CullMode = cullMode;
    rasterizer_desc.FrontCounterClockwise = FALSE;
    rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizer_desc.DepthClipEnable = TRUE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	rasterizer_desc.ForcedSampleCount = 0;
    rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


    // #3
    D3D12_BLEND_DESC  blend_desc;
    ::ZeroMemory(&blend_desc, sizeof(blend_desc));
    blend_desc.AlphaToCoverageEnable = FALSE;
    blend_desc.IndependentBlendEnable = FALSE;
    blend_desc.RenderTarget[0] = {
        FALSE, FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };

    // #4 
    D3D12_DEPTH_STENCIL_DESC depth_stencil_desc;
    ::ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
	depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depth_stencil_desc.StencilEnable = FALSE;
    depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC default_stencil_op = { 
        D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS 
    };
    depth_stencil_desc.FrontFace = default_stencil_op;
    depth_stencil_desc.BackFace = default_stencil_op;

    // #5
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc;
	ZeroMemory(&pipeline_state_desc, sizeof(pipeline_state_desc));

	pipeline_state_desc.InputLayout = { 
        input_element_descriptors.data(), static_cast<UINT>(input_element_descriptors.size()) 
    };

	pipeline_state_desc.pRootSignature = m_root_signature.Get();

	pipeline_state_desc.VS = { m_vertex_shader_blob->GetBufferPointer(), m_vertex_shader_blob->GetBufferSize() };
	pipeline_state_desc.HS = { m_hull_shader_blob->GetBufferPointer(), m_hull_shader_blob->GetBufferSize() };
	pipeline_state_desc.DS = { m_domain_shader_blob->GetBufferPointer(), m_domain_shader_blob->GetBufferSize() };
	pipeline_state_desc.PS = { m_pixel_shader_blob->GetBufferPointer(), m_pixel_shader_blob->GetBufferSize() };

	pipeline_state_desc.RasterizerState = rasterizer_desc;
	pipeline_state_desc.BlendState = blend_desc;
	pipeline_state_desc.DepthStencilState = depth_stencil_desc;
	pipeline_state_desc.SampleMask = UINT_MAX;
	pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	pipeline_state_desc.NumRenderTargets = 1;
	pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipeline_state_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipeline_state_desc.SampleDesc.Count = 1;

    ComPtr<ID3D12PipelineState> pipeline_state;
    THROW_GRAPHICS_INFO(
        m_dx12_device->CreateGraphicsPipelineState(
            &pipeline_state_desc, IID_PPV_ARGS(pipeline_state.ReleaseAndGetAddressOf())
        )
    );

    return pipeline_state;
}
