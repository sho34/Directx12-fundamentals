#pragma once
#ifndef __frame_resources__
#define __frame_resources__

#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class frame_resource
{
public:
	frame_resource(ComPtr<ID3D12Device2> pDevice, UINT passCount, UINT objCount);
	~frame_resource();

public:
	// delete copy and assignment operators.
	frame_resource(const frame_resource&) = delete;
	frame_resource& operator=(const frame_resource&) = delete;

public:
	// create a separate resources for each frame to avoid CPU/GPU synchronization.
	ComPtr<ID3D12CommandAllocator>		m_command_allocator_fr;


};

#endif // !__frame_resources__

