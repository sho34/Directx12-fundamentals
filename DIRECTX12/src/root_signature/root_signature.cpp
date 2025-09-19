#include "root_signature.h"

root_signature::root_signature(ID3D12Device2* p_device)
	:m_device(p_device)
{}

void root_signature::create_static_samplers(UINT shader_register, UINT register_space)
{
	// configure the static samplers in here 
	D3D12_STATIC_SAMPLER_DESC  static_sampler_descriptor;
	::ZeroMemory(&static_sampler_descriptor, sizeof(static_sampler_descriptor));
	static_sampler_descriptor.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	static_sampler_descriptor.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	static_sampler_descriptor.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	static_sampler_descriptor.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	static_sampler_descriptor.MipLODBias = 0;
	static_sampler_descriptor.MaxAnisotropy = 0;
	static_sampler_descriptor.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	static_sampler_descriptor.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	static_sampler_descriptor.MinLOD = 0.0f;
	static_sampler_descriptor.MaxLOD = D3D12_FLOAT32_MAX;
	static_sampler_descriptor.ShaderRegister = shader_register;
	static_sampler_descriptor.RegisterSpace = register_space;
	static_sampler_descriptor.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// add the samplers to the samplers vector.
	m_static_samplers.push_back(static_sampler_descriptor);
}

void root_signature::create_descriptor_table(
	D3D12_DESCRIPTOR_RANGE_TYPE desc_range_type, D3D12_SHADER_VISIBILITY shader_visibility, 
	UINT num_descriptors, UINT base_shader_register, UINT register_space
)
{
	D3D12_DESCRIPTOR_RANGE descriptor_range;
	::ZeroMemory(&descriptor_range, sizeof(descriptor_range));

	descriptor_range.RangeType = desc_range_type;
	descriptor_range.NumDescriptors = num_descriptors; // we have two structured buffers and two descriptors.
	descriptor_range.BaseShaderRegister = base_shader_register; // starting from the first register t0.
	descriptor_range.RegisterSpace = register_space; // allows us to use the same register by using diff space.
	descriptor_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// #2 pass it to the parameter description.
	D3D12_ROOT_PARAMETER descriptor_range_root_param;
	::ZeroMemory(&descriptor_range_root_param, sizeof(descriptor_range_root_param));
	descriptor_range_root_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	descriptor_range_root_param.DescriptorTable = { 1, &descriptor_range }; // one range.
	descriptor_range_root_param.ShaderVisibility = shader_visibility; // used by the domain shader.

	// add it to the root parameters vector.
	m_root_parameters.push_back(descriptor_range_root_param);
}

void root_signature::create_root_descriptor(
	D3D12_ROOT_PARAMETER_TYPE root_param_type, D3D12_SHADER_VISIBILITY shader_visibility,
	UINT shader_register, UINT register_space
)
{
	D3D12_ROOT_PARAMETER root_descriptor;
	::ZeroMemory(&root_descriptor, sizeof(root_descriptor));
	root_descriptor.ParameterType = root_param_type;
	root_descriptor.Descriptor = { shader_register, register_space };    // first register b0 in first space.
	root_descriptor.ShaderVisibility = shader_visibility; // only used by the domain shader.

	// add it to the root parameters vector.
	m_root_parameters.push_back(root_descriptor);
}

void root_signature::create_root_constant(
	UINT num_constants, D3D12_SHADER_VISIBILITY shader_visibility, UINT shader_register, UINT register_space
)
{
	// #4 creating a root constant for the hull shader tessellation factors.
	D3D12_ROOT_PARAMETER root_constant;
	::ZeroMemory(&root_constant, sizeof(root_constant));
	root_constant.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	// 2 constants in first register b0 in first register space.
	root_constant.Constants = { shader_register, register_space, num_constants }; 
	root_constant.ShaderVisibility = shader_visibility; // only used in the hull shader.

	// add it to the root parameters vector.
	m_root_parameters.push_back(root_constant);
}

void root_signature::finalize_root_sig_creation(D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	D3D12_ROOT_SIGNATURE_DESC root_signature_desc;
	::ZeroMemory(&root_signature_desc, sizeof(root_signature_desc));
	root_signature_desc.NumParameters = static_cast<UINT>(m_root_parameters.size());
	root_signature_desc.pParameters = m_root_parameters.data();
	root_signature_desc.NumStaticSamplers = static_cast<UINT>(m_static_samplers.size()); 
	root_signature_desc.pStaticSamplers = m_static_samplers.data();
	root_signature_desc.Flags = flags;

	// #7
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;

	THROW_GRAPHICS_INFO(
		D3D12SerializeRootSignature(
			&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
			signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()

		)
	);

	// #8 create the root signature.
	THROW_GRAPHICS_INFO(
		m_device->CreateRootSignature(
			0, signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(m_root_signature.ReleaseAndGetAddressOf())
		)
	);
}

void root_signature::create_default()
{
	CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
	root_signature_desc.Init(
		0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	// #7
	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;

	THROW_GRAPHICS_INFO(
		D3D12SerializeRootSignature(
			&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
			signature.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf()

		)
	);

	// #8 create the root signature.
	THROW_GRAPHICS_INFO(
		m_device->CreateRootSignature(
			0, signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(m_root_signature.ReleaseAndGetAddressOf())
		)
	);
}
