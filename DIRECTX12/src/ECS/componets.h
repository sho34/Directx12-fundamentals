#pragma once
#ifndef __componets_h__
#define __componets_h__


#include <DirectXMath.h>
#include <string>
#include <vector>


struct transform
{
	DirectX::XMFLOAT4 row1; 
	DirectX::XMFLOAT4 row2; 
	DirectX::XMFLOAT4 row3; 
	DirectX::XMFLOAT4 row4;
};

struct velocity
{
	DirectX::XMFLOAT3 linear_velocity{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 angular_velocity{ 0.0f, 0.0f, 0.0f };
};


struct camera_component
{
	float fov{ 45.0f };
	float aspect_ratio{ 1.0f };
	float near_plane{ 0.1f };
	float far_plane{ 1000.0f };
};

struct renderable
{
	// just a marker component for now.
};

struct light
{
	DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
	float intensity{ 1.0f };
};

struct directional_light
{
	DirectX::XMFLOAT3 direction{ -1.0f, -1.0f, -1.0f };
	DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
	float intensity{ 1.0f };
};

struct point_light
{
	DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
	float intensity{ 1.0f };
	float range{ 10.0f };
};

struct spot_light
{
	DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 direction{ 0.0f, -1.0f, 0.0f };
	DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
	float intensity{ 1.0f };
	float range{ 10.0f };
	float spot_angle{ DirectX::XMConvertToRadians(30.0f) };
};

struct mesh
{
	std::string mesh_file;
};

struct texture_component
{
	std::string texture_file;
};	


struct material
{
	DirectX::XMFLOAT3 albedo{ 1.0f, 1.0f, 1.0f };
	float roughness{ 0.5f };
	float metallic{ 0.0f };
};


struct tag
{
	std::string name;
};

struct parent
{
	uint32_t parent_entity_id{ 0 };
};

struct child
{
	std::vector<uint32_t> children_entity_ids;
};


struct skybox
{
	std::string cubemap_file;
};

struct collider
{
	// just a marker component for now.
};

#endif
