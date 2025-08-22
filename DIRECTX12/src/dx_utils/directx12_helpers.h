#pragma once
#include <d3d12.h>
#include <cassert>

namespace dx12_utils
{
	// Helper function to transition a resource from one state to another.
	inline void transition_resource(
		_In_ ID3D12GraphicsCommandList* command_list,
		_In_ ID3D12Resource* resource,
		D3D12_RESOURCE_STATES state_before,
		D3D12_RESOURCE_STATES state_after
	) noexcept
	{
		assert(command_list != nullptr);
		assert(resource != nullptr);
		if (state_before == state_after)
		{
			return;
		}
		D3D12_RESOURCE_BARRIER barrier_desc{};
		barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_desc.Transition.pResource = resource;
		barrier_desc.Transition.StateBefore = state_before;
		barrier_desc.Transition.StateAfter = state_after;
		barrier_desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		command_list->ResourceBarrier(1, &barrier_desc);
	}
}