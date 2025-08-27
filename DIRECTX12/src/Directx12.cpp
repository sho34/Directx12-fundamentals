#include "Directx12.h"

// link the directx12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

directx12_graphics::~directx12_graphics()
{
	// clean up
	for (UINT i{ 0 }; i < m_buffer_count; i++)
	{
		wait_for_frame(i);
	}
}

directx12_graphics::directx12_graphics(HWND hWnd, int width, int height)
	: m_use_warp(false), 
	  m_client_width(width),
	  m_client_height(height),
	  m_hwnd(hWnd)
{
	initiallize_pipeline_resources();

	std::string multibyte_adapter_details_str{ wchar_to_char(adapater_details.c_str()) };

	::OutputDebugString(multibyte_adapter_details_str.c_str());
}

void directx12_graphics::resize_buffers(uint32_t width, uint32_t height)
{

	if (m_client_width != width || m_client_height != height)
	{
		// don't allow zero size swap chain buffers.
		m_client_width = std::max(1u, width);
		m_client_height = std::max(1u, height);


		// release all local references to the buffer resources.
		for (int i{ 0 }; i < m_buffer_count; ++i)
		{
			// wait for all the frames to finish rendering.
			wait_for_frame(i);
			m_swap_chain_buffers[i].Reset();
			m_frame_fence_values[i] = m_frame_fence_values[i];
		}

		DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
		THROW_GRAPHICS_INFO(m_swapchain_4->GetDesc(&swap_chain_desc));

		THROW_GRAPHICS_INFO(
			m_swapchain_4->ResizeBuffers(
				m_buffer_count, m_client_width, m_client_height, 
				swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags
			)
		);

		// update the depth stencil buffer.
		update_depth_stencil_buffer();		
		update_render_target_views();
	}
}

void directx12_graphics::update_fps()
{
	static uint64_t frame_counter{ 0 };
	static double elapsed_seconds{ 0.0 };

	static std::chrono::high_resolution_clock clock;

	static auto t0 = clock.now();

	++frame_counter;

	auto t1 = clock.now();
	auto delta_time = t1 - t0;
	t0 = t1;

	elapsed_seconds += delta_time.count() * 1e-9;

	if (elapsed_seconds > 1.0)
	{
		std::ostringstream oss;
		auto fps = frame_counter / elapsed_seconds;
		oss << "FPS: " << fps << "\n";

		// write the frame stats string to a string.
		m_frame_stats = oss.str();

		frame_counter = 0;
		elapsed_seconds = 0.0;
	}

}

void directx12_graphics::initiallize_pipeline_resources()
{
	// enable the debug layer
	{
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debug_interface;
		THROW_GRAPHICS_INFO(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)));
		debug_interface->EnableDebugLayer();
#endif
	}

	{
		initialize_graphics_adapter_and_factory_device();
	}

	// create device
	{
		THROW_GRAPHICS_INFO(
			D3D12CreateDevice(m_dxgi_adapter_4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_dx12_device))
		);
	}

	// init functions
	{
		create_command_allocators();
		create_command_list();
		create_fences();
		create_fence_event_handle();
		create_command_queue();
		check_tearing_support();
		create_swap_chain();
		get_swap_chain_buffers();
		create_descriptor_heap_rtv();
		create_depth_stencil_buffer();
		create_descriptor_heap_depth_stencil();
		create_descriptor_heap_shared();
	}

}

void directx12_graphics::initialize_graphics_adapter_and_factory_device()
{
	// get a compatible adapter for use.
	UINT create_factory_flags{ 0 };
#if defined(_DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	// create the directx12 factory2.
	THROW_GRAPHICS_INFO(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&m_dxgi_factory_4)));

	ComPtr<IDXGIAdapter1> dxgi_adapter_1;

	if (m_use_warp)
	{
		THROW_GRAPHICS_INFO(m_dxgi_factory_4->EnumWarpAdapter(IID_PPV_ARGS(&dxgi_adapter_1)));
		// cast dxgi_adapter_1 interface to a dxgi_adpter_4 interface.
		THROW_GRAPHICS_INFO(dxgi_adapter_1.As(&m_dxgi_adapter_4));
	}
	else
	{
		SIZE_T max_dedicated_video_memory = 0;
		for (UINT i = 0; m_dxgi_factory_4->EnumAdapters1(i, &dxgi_adapter_1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgi_adapter_desc_1;

			dxgi_adapter_1->GetDesc1(&dxgi_adapter_desc_1);
			// check to see if the adapter can create a d3d12 device without actually creating it.
			// the adapter with the largest video memory is favoured.
			if ((dxgi_adapter_desc_1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(
					D3D12CreateDevice(
						dxgi_adapter_1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)
				) && dxgi_adapter_desc_1.DedicatedVideoMemory > max_dedicated_video_memory
				)
			{
				max_dedicated_video_memory = dxgi_adapter_desc_1.DedicatedVideoMemory;
				THROW_GRAPHICS_INFO(dxgi_adapter_1.As(&m_dxgi_adapter_4));
			}
			// save the available adapters in the adapter list.
			THROW_GRAPHICS_INFO(dxgi_adapter_1.As(&m_dxgi_adapter_4));
			m_adapter_list.push_back(m_dxgi_adapter_4);
		}
	}

	{
		log_available_graphics_adapters();
	}

}

void directx12_graphics::log_available_graphics_adapters()
{
	std::wostringstream woss;
	DXGI_ADAPTER_DESC adapterDesc;
	DOUBLE to_gb = 10e-9;

	for (size_t i = 0; i < m_adapter_list.size(); ++i)
	{

		m_adapter_list[i]->GetDesc(&adapterDesc);
		
		woss << "*************** DISPLAY ADAPTER ******************\n";
		woss << "*[description]: " << adapterDesc.Description << "\n";
		woss << "*[sys memory]: " << adapterDesc.DedicatedSystemMemory * to_gb << " GB" << "\n";
		woss << "*[video memory]: " << adapterDesc.DedicatedVideoMemory * to_gb << " GB\n";
		woss << "*[shared sys memory]: " << adapterDesc.SharedSystemMemory * to_gb << " GB\n";
		woss << "**************************************************\n" << std::endl;
	}
	// save the adapter details.
	adapater_details = woss.str();
}

void directx12_graphics::create_command_allocators()
{
	for (UINT i{ 0 }; i < m_buffer_count; i++)
	{
		ComPtr<ID3D12CommandAllocator> command_allocator;
		THROW_GRAPHICS_INFO(
			m_dx12_device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)
			)
		);

		m_command_allocators.push_back(command_allocator);
	}
}

void directx12_graphics::create_command_list()
{
	THROW_GRAPHICS_INFO(
		m_dx12_device->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocators[0].Get(), nullptr,
			IID_PPV_ARGS(&m_command_list)
		)
	);

	THROW_GRAPHICS_INFO(m_command_list->Close());
}

void directx12_graphics::create_fences()
{
	for (UINT i{ 0 }; i < m_buffer_count; i++)
	{
		UINT64 initial_value{ 0 };
		ComPtr<ID3D12Fence> fence;

		THROW_GRAPHICS_INFO(m_dx12_device->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		m_fences.push_back(fence);
		m_frame_fence_values.push_back(initial_value);
	}
}

void directx12_graphics::create_fence_event_handle()
{
	m_fence_event_handle = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fence_event_handle == NULL)
	{
		throw std::runtime_error{ "Error creating fence event" };
	}
}

void directx12_graphics::create_command_queue()
{
	D3D12_COMMAND_QUEUE_DESC queue_descriptor;
	::ZeroMemory(&queue_descriptor, sizeof(queue_descriptor));
	queue_descriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_descriptor.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queue_descriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_descriptor.NodeMask = 0;

	THROW_GRAPHICS_INFO(
		m_dx12_device->CreateCommandQueue(&queue_descriptor, IID_PPV_ARGS(&m_command_queue))
	);
}

void directx12_graphics::check_tearing_support()
{
	// query the factory1.5 interface
	ComPtr<IDXGIFactory5> dxgi_factory_5;
	if (SUCCEEDED(m_dxgi_factory_4.As(&dxgi_factory_5)))
	{
		if (FAILED(dxgi_factory_5->CheckFeatureSupport(
			DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_is_tearing_supported, sizeof(m_is_tearing_supported)
		)))
		{
			m_is_tearing_supported = FALSE;
		}
	}

	if (m_is_tearing_supported)
	{
		::OutputDebugString("----[TEARING IS SUPPORTED ON THIS DEVICE]---\n");
	}

}

void directx12_graphics::create_swap_chain()
{
	POINT window_size{ m_client_width, m_client_height };

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
	::ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));
	swap_chain_desc.Width = static_cast<UINT>(window_size.x);
	swap_chain_desc.Height = static_cast<UINT>(window_size.y);
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc = { 1, 0 }; // no anti-aliasing
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = m_buffer_count;
	swap_chain_desc.Scaling = DXGI_SCALING_NONE;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// enable tearing if it is supported on this device.
	swap_chain_desc.Flags = m_is_tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : FALSE;
	
	ComPtr<IDXGISwapChain1>	swap_chai_1;
	THROW_GRAPHICS_INFO(
		m_dxgi_factory_4->CreateSwapChainForHwnd(
			m_command_queue.Get(), m_hwnd, &swap_chain_desc, nullptr, nullptr, &swap_chai_1
		)
	);

	// disable the default ALT+ENTER toggle screen feature.
	THROW_GRAPHICS_INFO(m_dxgi_factory_4->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
	THROW_GRAPHICS_INFO(swap_chai_1.As(&m_swapchain_4));
}

void directx12_graphics::get_swap_chain_buffers()
{
	ComPtr<ID3D12Resource> swap_chain_buffer;

	for (uint8_t i{ 0 }; i < m_buffer_count; i++)
	{
		THROW_GRAPHICS_INFO(
			m_swapchain_4->GetBuffer(i, IID_PPV_ARGS(swap_chain_buffer.ReleaseAndGetAddressOf()))
		);

		m_swap_chain_buffers.push_back(swap_chain_buffer);
	}
}

void directx12_graphics::create_descriptor_heap_rtv()
{
	D3D12_DESCRIPTOR_HEAP_DESC heap_descriptor;
	::ZeroMemory(&heap_descriptor, sizeof(heap_descriptor));
	heap_descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heap_descriptor.NumDescriptors = m_buffer_count;
	heap_descriptor.NodeMask = 0;
	heap_descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	
	THROW_GRAPHICS_INFO(m_dx12_device->CreateDescriptorHeap(
		&heap_descriptor, IID_PPV_ARGS(&m_rtv_descriptor_heap))
	);

	m_rtv_descriptor_size = m_dx12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i{ 0 }; i < m_buffer_count; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE d{ m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() };
		d.ptr += i * m_rtv_descriptor_size;

		m_dx12_device->CreateRenderTargetView(m_swap_chain_buffers[i].Get(), nullptr, d);
	}
}

void directx12_graphics::create_descriptor_heap_shared()
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_shared;
	::ZeroMemory(&descriptor_shared, sizeof(descriptor_shared));
	descriptor_shared.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptor_shared.NumDescriptors = 6;
	descriptor_shared.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	THROW_GRAPHICS_INFO(
		m_dx12_device->CreateDescriptorHeap(&descriptor_shared, IID_PPV_ARGS(&m_shared_descriptor_heap))
	);
}

void directx12_graphics::create_depth_stencil_buffer()
{
	D3D12_CLEAR_VALUE depth_optimized_clear_value;
	::ZeroMemory(&depth_optimized_clear_value, sizeof(depth_optimized_clear_value));
	depth_optimized_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
	depth_optimized_clear_value.DepthStencil.Depth = 1.0f;
	depth_optimized_clear_value.DepthStencil.Stencil = 0;

	POINT window_size{ m_client_width, m_client_height };

	D3D12_HEAP_PROPERTIES heap_properties;
	::ZeroMemory(&heap_properties, sizeof(heap_properties));
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resource_description;
	::ZeroMemory(&resource_description, sizeof(resource_description));
	resource_description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resource_description.Alignment = 0; 
	resource_description.Width = window_size.x;
	resource_description.Height = window_size.y;
	resource_description.DepthOrArraySize = 1;
	resource_description.MipLevels = 0;
	resource_description.Format = DXGI_FORMAT_D32_FLOAT;
	resource_description.SampleDesc.Count = 1;
	resource_description.SampleDesc.Quality = 0;
	resource_description.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resource_description.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	THROW_GRAPHICS_INFO(
		m_dx12_device->CreateCommittedResource(
			&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_description, D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depth_optimized_clear_value, IID_PPV_ARGS(&m_depth_stencil_buffer)
		)
	);


}

void directx12_graphics::create_descriptor_heap_depth_stencil()
{
	D3D12_DESCRIPTOR_HEAP_DESC heap_decsriptor;
	::ZeroMemory(&heap_decsriptor, sizeof(heap_decsriptor));
	heap_decsriptor.NumDescriptors = 1;
	heap_decsriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heap_decsriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	THROW_GRAPHICS_INFO(
		m_dx12_device->CreateDescriptorHeap(&heap_decsriptor, IID_PPV_ARGS(&m_dsv_descriptor_heap))
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
	::ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));
	depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Flags = D3D12_DSV_FLAG_NONE;


	m_dx12_device->CreateDepthStencilView(
		m_depth_stencil_buffer.Get(), &depth_stencil_view_desc,
		m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()
	);

}

void directx12_graphics::wait_for_frame(UINT frameIndex)
{
	UINT64 fence_value{ m_frame_fence_values[frameIndex] };
	ComPtr<ID3D12Fence> fence{ m_fences[frameIndex] };

	THROW_GRAPHICS_INFO(fence->SetEventOnCompletion(fence_value, m_fence_event_handle));

	DWORD wait{ ::WaitForSingleObject(m_fence_event_handle, 10000) };
	if (wait != WAIT_OBJECT_0)
	{
		throw( std::runtime_error{ "[wait_for_frame()] : FAILED...!" } );
	}
}

void directx12_graphics::update_render_target_views()
{
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

	for (int i{ 0 }; i < m_buffer_count; i++)
	{
		ComPtr<ID3D12Resource> back_buffer;

		THROW_GRAPHICS_INFO(m_swapchain_4->GetBuffer(i, IID_PPV_ARGS(&back_buffer)));
		m_dx12_device->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_handle);

		m_swap_chain_buffers[i] = back_buffer;

		rtv_handle.Offset(m_rtv_descriptor_size);
	}
}

void directx12_graphics::toggle_v_sync()
{
	// THE FRAME RATE WILL INCREASE, DUE TO THE APPLICATION NOT WAITING FOR THE VERTICAL REFRESH TO PRESENT 
	// THE RENDERED IMAGE.
	m_is_vsync_enabled = !m_is_vsync_enabled;
	if (m_is_vsync_enabled)
	{
		::OutputDebugString("V-Sync Enabled.\n");
	}
	else
	{
		::OutputDebugString("V-Sync Disabled.\n");
	};
}

void directx12_graphics::activate_v_sync_parameters()
{
	m_sync_interval = m_is_vsync_enabled ? 1 : 0;
	m_present_flags = m_is_tearing_supported && !m_is_vsync_enabled ? DXGI_PRESENT_ALLOW_TEARING : FALSE;
}

void directx12_graphics::update_depth_stencil_buffer()
{
	if (m_depth_stencil_buffer)
	{
		// release the old depth stencil buffer.
		m_depth_stencil_buffer.Reset();
	}
	if (m_dsv_descriptor_heap)
	{
		// release the old descriptor heap.
		m_dsv_descriptor_heap.Reset();
	}
	create_depth_stencil_buffer();
	create_descriptor_heap_depth_stencil();
}






