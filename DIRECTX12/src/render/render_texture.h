#pragma once
#ifndef __render_texture__
#define __render_texture__

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <stdexcept> // for std::runtime_error
#include "../dx_utils/d3dx12.h"
#include "../dx_utils/directx12_helpers.h"
// TO MANAGE OFFSCREEN RENDER TARGETS

class render_texture
{
public:
	render_texture(DXGI_FORMAT format);

public:
	void set_device(
		ID3D12Device2* device, D3D12_CPU_DESCRIPTOR_HANDLE srv_desc, D3D12_CPU_DESCRIPTOR_HANDLE rtv_desc
	);

	void sizee_resources(size_t width, size_t height);
	void release_device();
	void transition_to(ID3D12GraphicsCommandList* command_list, D3D12_RESOURCE_STATES after_states);
	void begin_scene(ID3D12GraphicsCommandList* command_list);
	void end_scene(ID3D12GraphicsCommandList* command_list);
	void clear(ID3D12GraphicsCommandList* command_list);
	void set_clear_color(DirectX::FXMVECTOR color);
	void update_state(D3D12_RESOURCE_STATES state);
	void set_window(const RECT& rect);

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> get_resource() const;

public:
	DXGI_FORMAT get_format() const;
	D3D12_RESOURCE_STATES get_current_state() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Device2>	m_device;
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_resource;
	D3D12_CPU_DESCRIPTOR_HANDLE				m_srv_desc;
	D3D12_CPU_DESCRIPTOR_HANDLE				m_rtv_desc;
	D3D12_RESOURCE_STATES					m_state;
	float									m_clear_color[4];
	DXGI_FORMAT								m_format;
	size_t									m_width{ 0 }, m_height{ 0 };

};

#endif