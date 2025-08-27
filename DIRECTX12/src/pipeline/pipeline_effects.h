#pragma once
#ifndef __pipeline_effects__
#define __pipeline_effects__

#include "pipeline_state_object.h"

using namespace Microsoft::WRL;

//------------------------ FOR WIREFRAME RENDERING ------------------------//
class wire_frame_pso : public pipeline_state_object
{
public:
	wire_frame_pso(ID3D12Device2* p_device, ID3D12RootSignature* p_root_signature)
		: pipeline_state_object(p_device, p_root_signature)
	{}

private:
	ComPtr<ID3DBlob> m_vertex_shader{ load_shader(LOAD_SHADER(L"VertexShader.cso")) };
	ComPtr<ID3DBlob> m_pixel_shader{ load_shader(LOAD_SHADER(L"PixelShader.cso")) };
	ComPtr<ID3DBlob> m_hull_shader{ load_shader(LOAD_SHADER(L"HullShader.cso")) };
	ComPtr<ID3DBlob> m_domain_shader{ load_shader(LOAD_SHADER(L"DomainShader.cso")) };

protected:
	void configure_shaders() override
	{
		// we need to lead the shaders into memory first.
		// ... load shaders into memory
		// ...
		// ... compile them if needed

		// then set them in the pipeline state description.
		m_pipeline_state_desc.VS = { m_vertex_shader->GetBufferPointer(), m_vertex_shader->GetBufferSize() }; 
		m_pipeline_state_desc.PS = { m_pixel_shader->GetBufferPointer(), m_pixel_shader->GetBufferSize() }; 
		m_pipeline_state_desc.DS = { m_domain_shader->GetBufferPointer(), m_domain_shader->GetBufferSize() }; 
		m_pipeline_state_desc.HS = { m_hull_shader->GetBufferPointer(), m_hull_shader->GetBufferSize() }; 
	}

	void configure_blend_state() override
	{
		m_blend_desc.AlphaToCoverageEnable = FALSE;
		m_blend_desc.IndependentBlendEnable = FALSE;
		m_blend_desc.RenderTarget[0] = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		// set the blend state in the pipeline state description.
		m_pipeline_state_desc.BlendState = m_blend_desc;
	}

	void configure_input_layout() override
	{
		m_input_element_descriptors = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// set the input layout in the pipeline state description.
		m_pipeline_state_desc.InputLayout = {
			m_input_element_descriptors.data(), static_cast<UINT>(m_input_element_descriptors.size())
		};
	}

	void configure_rasterizer_state() override
	{
		m_rasterizer_desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		m_rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
		m_rasterizer_desc.FrontCounterClockwise = FALSE;
		m_rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		m_rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		m_rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		m_rasterizer_desc.DepthClipEnable = TRUE;
		m_rasterizer_desc.MultisampleEnable = FALSE;
		m_rasterizer_desc.AntialiasedLineEnable = FALSE;
		m_rasterizer_desc.ForcedSampleCount = 0;
		m_rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// set the rasterizer state in the pipeline state description.
		m_pipeline_state_desc.RasterizerState = m_rasterizer_desc;
	}

	void configure_depth_stencil_state() override
	{
		m_depth_stencil_desc.DepthEnable = TRUE;
		m_depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		m_depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		m_depth_stencil_desc.StencilEnable = FALSE;
		m_depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		m_depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		m_depth_stencil_desc.FrontFace = {
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_COMPARISON_FUNC_ALWAYS
		};
		m_depth_stencil_desc.BackFace = {
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_COMPARISON_FUNC_ALWAYS
		};

		// set the depth-stencil state in the pipeline state description.
		m_pipeline_state_desc.DepthStencilState = m_depth_stencil_desc;
	}

	void configure_primitive_topology() override
	{
		m_pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
};


//------------------------ FOR SOLID RENDERING ------------------------//
class solid_pso : public pipeline_state_object
{
public:
	solid_pso(ID3D12Device2* p_device, ID3D12RootSignature* p_root_signatue)
		: pipeline_state_object(p_device, p_root_signatue)
	{}

private:
	ComPtr<ID3DBlob> m_vertex_shader{ load_shader(LOAD_SHADER(L"VertexShader.cso")) };
	ComPtr<ID3DBlob> m_pixel_shader{ load_shader(LOAD_SHADER(L"PixelShader.cso")) };
	ComPtr<ID3DBlob> m_hull_shader{ load_shader(LOAD_SHADER(L"HullShader.cso")) };
	ComPtr<ID3DBlob> m_domain_shader{ load_shader(LOAD_SHADER(L"DomainShader.cso")) };

protected:
	void configure_shaders() override
	{
		// we need to lead the shaders into memory first.
		// ... load shaders into memory
		// ...
		// ... compile them if needed

		// then set them in the pipeline state description.
		m_pipeline_state_desc.VS = { m_vertex_shader->GetBufferPointer(), m_vertex_shader->GetBufferSize() };
		m_pipeline_state_desc.PS = { m_pixel_shader->GetBufferPointer(), m_pixel_shader->GetBufferSize() };
		m_pipeline_state_desc.DS = { m_domain_shader->GetBufferPointer(), m_domain_shader->GetBufferSize() };
		m_pipeline_state_desc.HS = { m_hull_shader->GetBufferPointer(), m_hull_shader->GetBufferSize() };

	}

	void configure_blend_state() override
	{
		m_blend_desc.AlphaToCoverageEnable = FALSE;
		m_blend_desc.IndependentBlendEnable = FALSE;
		m_blend_desc.RenderTarget[0] = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		// set the blend state in the pipeline state description.
		m_pipeline_state_desc.BlendState = m_blend_desc;
	}

	void configure_input_layout() override
	{
		m_input_element_descriptors = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// set the input layout in the pipeline state description.
		m_pipeline_state_desc.InputLayout = { 
			m_input_element_descriptors.data(), static_cast<UINT>(m_input_element_descriptors.size())
		};
	}

	void configure_rasterizer_state() override
	{
		m_rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
		m_rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
		m_rasterizer_desc.FrontCounterClockwise = FALSE;
		m_rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		m_rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		m_rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		m_rasterizer_desc.DepthClipEnable = TRUE;
		m_rasterizer_desc.MultisampleEnable = FALSE;
		m_rasterizer_desc.AntialiasedLineEnable = FALSE;
		m_rasterizer_desc.ForcedSampleCount = 0;
		m_rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// set the rasterizer state in the pipeline state description.
		m_pipeline_state_desc.RasterizerState = m_rasterizer_desc;
	}

	void configure_depth_stencil_state() override
	{
		m_depth_stencil_desc.DepthEnable = TRUE;
		m_depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		m_depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		m_depth_stencil_desc.StencilEnable = FALSE;
		m_depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		m_depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		m_depth_stencil_desc.FrontFace = {
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_COMPARISON_FUNC_ALWAYS
		};
		m_depth_stencil_desc.BackFace = {
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_COMPARISON_FUNC_ALWAYS
		};

		// set the depth-stencil state in the pipeline state description.
		m_pipeline_state_desc.DepthStencilState = m_depth_stencil_desc;
	}

	void configure_primitive_topology() override
	{
		m_pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
};

#endif // !__pipeline_effects__