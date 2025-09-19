#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


using namespace DirectX;

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
	INT BaseVertexLocation = 0;

	std::vector<VERTEX> vertices;
	std::vector<uint16_t> indices;

	// Bounding box of the geometry defined by this sub-mesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};