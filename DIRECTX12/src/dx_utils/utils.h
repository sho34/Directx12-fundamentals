#pragma once

// create a helper macro for adding commands to the command queue
#ifndef ADD_TO_QUEUE
#define ADD_TO_QUEUE(commandlist, command_queue_gpu)							\
{																				\
	ID3D12CommandList* cmd_lists{ commandlist.Get() };						\
	command_queue_gpu->ExecuteCommandLists(1, &cmd_lists);	\
}
#endif		


#ifndef __utils__
#define __utils__

#include <wrl/client.h>
#include <vector>
#include <d3d12.h>

#if defined(max)
#undef max
#endif


namespace utility_functions
{
	template<typename T>
	Microsoft::WRL::ComPtr<ID3D12Resource> create_default_buffer(
		ID3D12Device2* device, const std::vector<T>& data, D3D12_RESOURCE_STATES final_state, 
		const wchar_t* name = L""
	)
	{
		UINT element_size{ static_cast<UINT>(sizeof(T)) };
		UINT buffer_size{ static_cast<UINT>(data.size() * element_size) };

		// #1
		D3D12_HEAP_PROPERTIES heap_properties;
		::ZeroMemory(&heap_properties, sizeof(heap_properties));
		heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 1;
		heap_properties.VisibleNodeMask = 1;

		// #2
		D3D12_RESOURCE_DESC resource_descriptor;
		::ZeroMemory(&resource_descriptor, sizeof(resource_descriptor));
		resource_descriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resource_descriptor.Alignment = 0;
		resource_descriptor.Width = std::max((UINT)1, buffer_size);
		resource_descriptor.Height = 1;
		resource_descriptor.DepthOrArraySize = 1;
		resource_descriptor.MipLevels = 1;
		resource_descriptor.Format = DXGI_FORMAT_UNKNOWN;
		resource_descriptor.SampleDesc.Count = 1;
		resource_descriptor.SampleDesc.Quality = 0;
		resource_descriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resource_descriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

		// #3
		Microsoft::WRL::ComPtr<ID3D12Resource> default_buffer;

		THROW_GRAPHICS_INFO(
			device->CreateCommittedResource(
				&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_descriptor, D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr, IID_PPV_ARGS(default_buffer.ReleaseAndGetAddressOf())
			)
		);

		THROW_GRAPHICS_INFO(default_buffer->SetName(name));

		heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;

		// #4 The intermediate upload buffer.
		Microsoft::WRL::ComPtr<ID3D12Resource> upload_buffer;
		THROW_GRAPHICS_INFO(
			device->CreateCommittedResource(
				&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_descriptor, D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(upload_buffer.ReleaseAndGetAddressOf())
			)
		);

		const wchar_t* intermed_buff_name{ L"intermediate_upload_buffer" };
		THROW_GRAPHICS_INFO(upload_buffer->SetName(intermed_buff_name));

		// #5
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
		THROW_GRAPHICS_INFO(
			device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(command_allocator.ReleaseAndGetAddressOf())
			)
		);

		// #6
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;
		THROW_GRAPHICS_INFO(
			device->CreateCommandList(
				0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(),
				nullptr, IID_PPV_ARGS(command_list.ReleaseAndGetAddressOf())
			)
		);

		// #7
		D3D12_COMMAND_QUEUE_DESC queue_descriptor;
		::ZeroMemory(&queue_descriptor, sizeof(queue_descriptor));
		queue_descriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queue_descriptor.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queue_descriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queue_descriptor.NodeMask = 0;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
		THROW_GRAPHICS_INFO(
			device->CreateCommandQueue(&queue_descriptor, IID_PPV_ARGS(command_queue.ReleaseAndGetAddressOf()))
		);

		// #8
		void* p_data;
		THROW_GRAPHICS_INFO(upload_buffer->Map(0, NULL, &p_data));
		// copy the input data into the intermediate upload buffer memory.
		memcpy(p_data, data.data(), buffer_size);

		// #9 then copy the contents(data) of the intermediate upload buffer to the default buffer.
		command_list->CopyBufferRegion(default_buffer.Get(), 0, upload_buffer.Get(), 0, buffer_size);

		// #10
		D3D12_RESOURCE_BARRIER barrier_descriptor;
		::ZeroMemory(&barrier_descriptor, sizeof(barrier_descriptor));
		barrier_descriptor.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_descriptor.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_descriptor.Transition.pResource = default_buffer.Get();
		barrier_descriptor.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier_descriptor.Transition.StateAfter = final_state;
		barrier_descriptor.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		command_list->ResourceBarrier(1, &barrier_descriptor);

		// #11
		command_list->Close();
		std::vector<ID3D12CommandList*> pp_command_lists{ command_list.Get() };
		command_queue->ExecuteCommandLists(
			static_cast<UINT>(pp_command_lists.size()), pp_command_lists.data()
		);

		// #12
		UINT64 initial_value{ 0 };
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		THROW_GRAPHICS_INFO(device->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

		// #13
		HANDLE fence_event_handle{ CreateEvent(nullptr, FALSE, FALSE, nullptr) };
		if (fence_event_handle == NULL)
		{
			throw std::runtime_error{ "error creating a fence event." };
		}

		// #14 set the value on the GPU, fence will be this value after the cmd list is executed.
		THROW_GRAPHICS_INFO(command_queue->Signal(fence.Get(), 1));
		// #15 raise the fence_event_handle when (fence == 1)
		THROW_GRAPHICS_INFO(fence->SetEventOnCompletion(1, fence_event_handle));

		// #16
		DWORD wait{ ::WaitForSingleObject(fence_event_handle, 10000) };
		if (wait != WAIT_OBJECT_0)
		{
			throw( std::runtime_error{ "Failed WaitForSingleObject()." } );
		}

		return default_buffer;
	}


	// our vertex buffer
	template<typename T>
	Microsoft::WRL::ComPtr<ID3D12Resource> create_vertex_buffer(
		ID3D12Device2* device, const std::vector<T>& data, const wchar_t* name = L""
	)
	{
		return utility_functions::create_default_buffer(
			device, data, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, name
		);
	}

	// our index buffer
	template<typename T>
	Microsoft::WRL::ComPtr<ID3D12Resource> create_index_buffer(
		ID3D12Device2* device, const std::vector<T>& data, const wchar_t* name = L""
	)
	{
		return utility_functions::create_default_buffer( device, data, D3D12_RESOURCE_STATE_INDEX_BUFFER, name );
	}

	// for the structured buffer
	template<typename T>
	Microsoft::WRL::ComPtr<ID3D12Resource> create_structured_buffer(
		ID3D12Device2* device, const std::vector<T>& data, const wchar_t* name = L""
	)
	{
		return utility_functions::create_default_buffer(
			device, data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, name
		);
	}

	template<typename T>
	D3D12_GPU_DESCRIPTOR_HANDLE create_srv(
		ID3D12Device2* device, ID3D12DescriptorHeap* descHeap, int offset,
		ID3D12Resource* resource,
		size_t numElements
	)
	{
		// #1
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_descriptor;
		::ZeroMemory(&srv_descriptor, sizeof(srv_descriptor));
		srv_descriptor.Format = DXGI_FORMAT_UNKNOWN;
		srv_descriptor.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_descriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_descriptor.Buffer.FirstElement = 0;
		srv_descriptor.Buffer.NumElements = static_cast<UINT>(numElements);
		srv_descriptor.Buffer.StructureByteStride = static_cast<UINT>(sizeof(T));
		srv_descriptor.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		// #2
		static UINT descriptor_size{
			device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		};

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_desciptor_handle{ descHeap->GetCPUDescriptorHandleForHeapStart() };

		// #3 a place in the heap where we can create a descriptor.
		cpu_desciptor_handle.ptr += descriptor_size * offset;

		// #4
		device->CreateShaderResourceView(resource, &srv_descriptor, cpu_desciptor_handle);

		// return the descriptor handle for the shader resource view.
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor_handle{ descHeap->GetGPUDescriptorHandleForHeapStart() };
		gpu_descriptor_handle.ptr += descriptor_size * offset;

		// #5
		return gpu_descriptor_handle;
	}
}

#endif
