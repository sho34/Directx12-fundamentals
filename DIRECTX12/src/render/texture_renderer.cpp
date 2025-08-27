#include "texture_renderer.h"


texture::texture(
	DXGI_FORMAT format,
	ID3D12Device2* p_device,
	ID3D12DescriptorHeap* p_srv,
	UINT64 srv_heap_offset,
	int width, int height
)
	: m_format(format)
	, m_device(p_device)
	, m_shared_srv_desc(p_srv)
	, m_srv_heap_offset(srv_heap_offset)
	, m_width(width)
	, m_height(height)
{
	// CREATE THE RESOURCES 
	create_offscreen_texture();
	create_texture_depth_stencil_buffer();
	create_texture_descriptor_heap_dsv();
}

Microsoft::WRL::ComPtr<ID3D12Resource> texture::get_resource() const { return m_off_screen_rt; }

D3D12_CPU_DESCRIPTOR_HANDLE texture::get_rtv_cpu_handle() const
{
	// we are going to need to use this in the main render loop and set it as the current render target.
	return m_off_screen_rtv_heap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE texture::get_srv_gpu_handle() const
{
	// we are going to use this as input to the imgui texture and display the scene on the imgui window.
	UINT srv_uav_descriptor_size = m_device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	D3D12_GPU_DESCRIPTOR_HANDLE handle{ m_shared_srv_desc->GetGPUDescriptorHandleForHeapStart() };
	handle.ptr += srv_uav_descriptor_size * m_srv_heap_offset; 
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE texture::get_dsv_cpu_handle() const
{
	// get the handle for the start of the descriptor heap of depth stencil views.
	return m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
}

void texture::create_offscreen_texture()
{
	// Create the off-screen render target texture.
	{
		D3D12_RESOURCE_DESC tex_desc = {};
		tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		tex_desc.Width = m_width;
		tex_desc.Height = m_height;
		tex_desc.DepthOrArraySize = 1;
		tex_desc.MipLevels = 1;
		tex_desc.Format = m_format;
		tex_desc.SampleDesc.Count = 1;
		tex_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clear_value = { m_format, {} };
		// copy our set color into the clear_value structure.
		memcpy(clear_value.Color, m_clear_color, sizeof(clear_value.Color));

		// create the texture in the default heap.
		CD3DX12_HEAP_PROPERTIES heap_properties(D3D12_HEAP_TYPE_DEFAULT);
		m_device->CreateCommittedResource(
			&heap_properties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&tex_desc, D3D12_RESOURCE_STATE_COMMON, &clear_value,
			IID_PPV_ARGS(&m_off_screen_rt)
		);
	}


	// Create the off-screen RTV descriptor heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
		rtv_heap_desc.NumDescriptors = 1;
		rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		THROW_GRAPHICS_INFO(
			m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_off_screen_rtv_heap))
		);
	}

	// create the render target view (RTV).
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
		rtv_desc.Format = m_format;
		rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		m_device->CreateRenderTargetView(
			m_off_screen_rt.Get(), &rtv_desc, m_off_screen_rtv_heap->GetCPUDescriptorHandleForHeapStart()
		);
	}

	// Create the shader resource view (SRV) in a shared descriptor heap.
	{
		UINT srv_uav_descriptor_size = m_device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

		// add the srv-desscriptor to an existing shared heap at index 3.
		D3D12_CPU_DESCRIPTOR_HANDLE handle{ m_shared_srv_desc->GetCPUDescriptorHandleForHeapStart() };
		handle.ptr += srv_uav_descriptor_size * m_srv_heap_offset; // after the two descriptors we already have.

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Texture2D.MipLevels = 1;
		m_device->CreateShaderResourceView(
			m_off_screen_rt.Get(), &srv_desc, handle
		);
	}

}

void texture::create_texture_descriptor_heap_dsv()
{
	D3D12_DESCRIPTOR_HEAP_DESC heap_decsriptor;
	::ZeroMemory(&heap_decsriptor, sizeof(heap_decsriptor));
	heap_decsriptor.NumDescriptors = 1;
	heap_decsriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heap_decsriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	THROW_GRAPHICS_INFO(
		m_device->CreateDescriptorHeap(&heap_decsriptor, IID_PPV_ARGS(&m_dsv_descriptor_heap))
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
	::ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));
	depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Flags = D3D12_DSV_FLAG_NONE;


	m_device->CreateDepthStencilView(
		m_depth_stencil_buffer.Get(), &depth_stencil_view_desc,
		m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()
	);
}

void texture::create_texture_depth_stencil_buffer()
{
	D3D12_CLEAR_VALUE depth_optimized_clear_value;
	::ZeroMemory(&depth_optimized_clear_value, sizeof(depth_optimized_clear_value));
	depth_optimized_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
	depth_optimized_clear_value.DepthStencil.Depth = 1.0f;
	depth_optimized_clear_value.DepthStencil.Stencil = 0;

	POINT window_size{ m_width, m_height };

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
		m_device->CreateCommittedResource(
			&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_description, D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depth_optimized_clear_value, IID_PPV_ARGS(&m_depth_stencil_buffer)
		)
	);


}


void texture::update_texture(float width, float height)
{
	// update the window dimensions not less than 100x100.
	m_width = std::max(100.0f, width);
	m_height = std::max(100.0f, height);

	// release the existing off-screen render target texture.
	if (m_off_screen_rt)
	{
		m_off_screen_rt.Reset();
	}
	if (m_off_screen_rtv_heap)
	{
		m_off_screen_rtv_heap.Reset();
	}

	// recreate the off-screen render target texture.
	create_offscreen_texture();
}

void texture::clear_texture(ID3D12GraphicsCommandList* p_command_list)
{
	p_command_list->ClearRenderTargetView(get_rtv_cpu_handle(), m_clear_color, 0, nullptr);
}


void texture::update_texture_depth_stencil_buffer(float width, float height)
{

	// update the current texture dimensions.
	m_width = width; m_height = height;

	create_offscreen_texture();
	create_texture_depth_stencil_buffer();
	create_texture_descriptor_heap_dsv();
}
