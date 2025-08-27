#include "pipeline_state_object.h"

pipeline_state_object::pipeline_state_object(ID3D12Device2* p_device, ID3D12RootSignature* p_root_signature)
	: m_device(p_device)
	, m_root_signature(p_root_signature)
{}

void pipeline_state_object::initialize()
{
	configure_shaders();
	configure_blend_state();
	configure_input_layout();
	configure_rasterizer_state();
	configure_depth_stencil_state();
	configure_primitive_topology();

	m_pipeline_state_desc.pRootSignature = m_root_signature.Get();
	m_pipeline_state_desc.SampleMask = UINT_MAX;
	m_pipeline_state_desc.NumRenderTargets = 1;
	m_pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_pipeline_state_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	m_pipeline_state_desc.SampleDesc.Count = 1;

	THROW_GRAPHICS_INFO(
		m_device->CreateGraphicsPipelineState(&m_pipeline_state_desc, IID_PPV_ARGS(&m_pipeline_state))
	);
}

Microsoft::WRL::ComPtr<ID3DBlob> pipeline_state_object::load_shader(const std::wstring file_path)
{
	std::ifstream shader_file(file_path, std::ios::binary | std::ios::ate);
	if (!shader_file)
	{
		throw std::runtime_error{
			"FAILED TO OPEN SHADER FILE: " + std::string(file_path.begin(), file_path.end()) 
		};
	}

	Microsoft::WRL::ComPtr<ID3DBlob> shader_blob;
	THROW_GRAPHICS_INFO(D3DReadFileToBlob(file_path.c_str(), shader_blob.ReleaseAndGetAddressOf()));

	return shader_blob;
}
