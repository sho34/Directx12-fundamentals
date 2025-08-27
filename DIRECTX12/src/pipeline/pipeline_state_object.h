#pragma once
#ifndef DIRECTX12_PIPELINE_STATE_OBJECT_H
#define DIRECTX12_PIPELINE_STATE_OBJECT_H

#include <d3dcompiler.h>
#include <d3d12.h>
#include <wrl.h>

// some STL libraries to consider.
#include <vector>
#include <fstream>

// Since different components can have a different pipeline state object
// I am encapsulating the pipeline state object creation in this class.

// for exception handling
#include "../dx_exceptions/Directx12Ex.h"
#include "../dx_exceptions/exception-macros.h"

// for file paths
#include "../file_works/file_paths.h"

class pipeline_state_object
{
public:
	pipeline_state_object(ID3D12Device2* p_device, ID3D12RootSignature* p_root_signature);

	virtual ~pipeline_state_object() = default; // to be overridden by derived classes.

public:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> get_pso() { return m_pipeline_state; };

protected:
	Microsoft::WRL::ComPtr<ID3D12Device2>			m_device;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>		m_pipeline_state;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>		m_root_signature;

protected:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC				m_pipeline_state_desc = {};
	D3D12_RASTERIZER_DESC							m_rasterizer_desc = {};
	D3D12_BLEND_DESC								m_blend_desc = {};
	D3D12_DEPTH_STENCIL_DESC						m_depth_stencil_desc = {};
	std::vector<D3D12_INPUT_ELEMENT_DESC>			m_input_element_descriptors;

public:
	void initialize();

protected:
	Microsoft::WRL::ComPtr<ID3DBlob> 	load_shader(const std::wstring file_path);

protected:
	virtual void configure_shaders() = 0;
	virtual void configure_blend_state() = 0;
	virtual void configure_input_layout() = 0;
	virtual void configure_rasterizer_state() = 0;
	virtual void configure_depth_stencil_state() = 0;
	virtual void configure_primitive_topology() = 0;
};

#endif