#pragma once
#ifndef __root_sugnature__
#define __root_signature__

#include <d3d12.h>
#include <wrl.h>

// exceptions 
#include "../dx_exceptions/Directx12Ex.h"
#include "../dx_exceptions/exception-macros.h"

class root_signature
{
public:
	root_signature(ID3D12Device2* p_device);3
	~root_signature() = default;

private:
	Microsoft::WRL::ComPtr<ID3D12Device2>			m_device;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>		m_root_signature;
	// the order in which the root parameters are added matters.
	std::vector<D3D12_ROOT_PARAMETER>				m_root_parameters;

public:
	void create_descriptor_table(
		D3D12_DESCRIPTOR_RANGE_TYPE desc_range_type, D3D12_SHADER_VISIBILITY shader_visibility,
		UINT num_descriptors, UINT base_shader_register, UINT register_space 
	);
	void create_root_descriptor(
		D3D12_ROOT_PARAMETER_TYPE root_param_type, D3D12_SHADER_VISIBILITY shader_visibility, 
		UINT shader_register, UINT register_space
	);
	void create_root_constant(
		UINT num_constants, D3D12_SHADER_VISIBILITY shader_visibility,
		UINT shader_register, UINT register_space
	);

	void finalize_root_sig_creation(D3D12_ROOT_SIGNATURE_FLAGS flags);

public:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> get_root_signature() { return m_root_signature; };

};

#endif
