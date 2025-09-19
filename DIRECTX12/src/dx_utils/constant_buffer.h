#pragma once
#ifndef __constant_buffer__
#define __constant_buffer__

#include<d3d12.h>
#include<wrl.h>

// for exception handling.
#include "../dx_exceptions/Directx12Ex.h"
#include "../dx_exceptions/exception-macros.h"

// some dx12 helper functions.
#include "../dx_utils/d3dx12.h"

using namespace Microsoft::WRL;

template<typename buffer_type>
class constant_buffer
{
private:
	ComPtr<ID3D12Device2>		m_dx12_device;
	ComPtr<ID3D12Resource>		m_constant_buffer;
	static constexpr UINT		m_buffer_count{ 3 }; // assuming a triple buffering system is set up.

	UINT m_aligned_size_of_elem{ (sizeof(buffer_type) + 255) & ~255 };

public:
	constant_buffer(ID3D12Device2* p_device)
		: m_dx12_device{ p_device }
	{
		// will be updated every frame.
		// will be aligned to 256 bytes.
		UINT64 buffer_size{ static_cast<unsigned long long>(m_aligned_size_of_elem) * m_buffer_count };

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
	~constant_buffer() = default;

public:
	void copy_data_to_buffer(const buffer_type* p_data, UINT frame_index)
	{
		UINT   aligned_size_of_elem{ (sizeof(buffer_type) + 255) & ~255 };
		UINT8* p_mapped_data{ nullptr };

		// map the resource to write to it.
		CD3DX12_RANGE read_range{ 0, 0 }; // we do not intend to read from this resource on the CPU.
		THROW_GRAPHICS_INFO(m_constant_buffer->Map(0, &read_range, reinterpret_cast<void**>(&p_mapped_data)));

		if (p_mapped_data != nullptr)
		{
			::memcpy(&p_mapped_data[frame_index * aligned_size_of_elem], p_data, sizeof(buffer_type));
			// unmap the resource.
			CD3DX12_RANGE written_range{
				frame_index * aligned_size_of_elem, (frame_index * aligned_size_of_elem) + sizeof(buffer_type) 
			};
			m_constant_buffer->Unmap(0, &written_range);
		}
	}

	// get the underlying buffer.
	ID3D12Resource* get_buffer() const { return m_constant_buffer.Get(); }

	D3D12_GPU_VIRTUAL_ADDRESS get_gpu_virtual_address_aligned(UINT frame_index) const
	{
		return { m_constant_buffer->GetGPUVirtualAddress() + frame_index * m_aligned_size_of_elem };
	}

	// 
public:
	// disable copy constructor and copy assignment operator.
	constant_buffer(constant_buffer&) = delete;
	constant_buffer& operator=(constant_buffer&) = delete;

	// disable move constructor and move assignment operator.
	constant_buffer(constant_buffer&&) = delete; 
	constant_buffer& operator=(constant_buffer&&) = delete;

};

#endif
