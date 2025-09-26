#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

#include <d3d12.h>
#include <wrl.h>

#include "../dx_utils/constant_buffer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


using namespace DirectX;
using namespace Microsoft::WRL;

struct VERTEX
{
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT2 m_texture_coord;
	DirectX::XMFLOAT4 m_color;
};

struct submesh_geometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT  BaseVertexLocation = 0;

	std::vector<VERTEX> vertices;
	std::vector<uint16_t> indices;

	std::vector<uint16_t> ralative_start_index_locations;
	std::vector<int16_t> ralative_start_vertex_locations;

	std::unique_ptr<constant_buffer<XMFLOAT4X4>>	p_cb_model_view_proj;

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

	// Bounding box of the geometry defined by this sub-mesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};