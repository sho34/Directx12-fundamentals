#pragma once
#ifndef __frame_resources__
#define __frame_resources__

#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>

// include the upload buffer
#include "../dx_utils/constant_buffer.h"

// include the throw macros
#include "../dx_exceptions/Directx12Ex.h"
#include "../dx_exceptions/exception-macros.h"

// off-screen render targets 
#include "../render/texture_renderer.h"

// include the utility functions.
#include "../dx_utils/utils.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct render_item
{
	// it will contain its own vertex and index buffers 
	// it will have its own pipeline state object 
	// it will have its own 

	// shape resources
	ComPtr<ID3D12Resource>					m_index_buffer;
	ComPtr<ID3D12Resource>					m_vertex_buffer;

	// views to these buffers.
	D3D12_INDEX_BUFFER_VIEW					m_index_buffer_view;
	D3D12_VERTEX_BUFFER_VIEW				m_vertex_buffer_view;

	// root signature
	ComPtr<ID3D12RootSignature>				m_root_signature;

	// render item variables 
	ComPtr<ID3D12Resource>					m_constant_buffer;

	// pipeline states
	ComPtr<ID3D12PipelineState>				m_pipeline_state;
};


struct texture
{
	std::string name;
	std::wstring file_name;
	Microsoft::WRL::ComPtr<ID3D12Resource>  texture_resouce;
	Microsoft::WRL::ComPtr<ID3D12Resource>  upload_heap;
};

#endif // !__frame_resources__

