#include "shape_data.h"

submesh_geometry primitive_mesh_generator::create_cube(float size /*= 1.0f*/)
{
	submesh_geometry mesh;
	float s = size * 0.5f;

	DirectX::XMFLOAT3 positions[8] = {
		{-s, -s, -s},
		{ s, -s, -s},
		{ s,  s, -s},
		{-s,  s, -s},
		{-s, -s,  s},
		{ s, -s,  s},
		{ s,  s,  s},
		{-s,  s,  s}
	};

	std::vector<uint32_t> indices = {
		0,1,2,  2,3,0, // back
		4,5,6,  6,7,4, // front
		0,4,7,  7,3,0, // left
		1,5,6,  6,2,1, // right
		3,2,6,  6,7,3, // top
		0,1,5,  5,4,0  // bottom
	};

	DirectX::XMFLOAT3 normals[6] = {
		{0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}
	};

	DirectX::XMFLOAT2 uvs[6] = {
		{0, 0}, {1, 0}, {1, 1}, // Triangle 1
		{1, 1}, {0, 1}, {0, 0}  // Triangle 2
	};

	for (int face = 0; face < 6; ++face)
	{
		int base = face * 6;
		for (int i = 0; i < 6; ++i)
		{
			VERTEX v;
			v.m_position = positions[indices[base + i]];
			v.m_texture_coord = uvs[i];

			mesh.vertices.push_back(v);
			mesh.indices.push_back(static_cast<uint16_t>(mesh.indices.size()));
		}
	}

	return mesh;
}

submesh_geometry primitive_mesh_generator::create_plane(float width, float height, int rows, int cols)
{
	submesh_geometry mesh;
	float dx = width / cols;
	float dz = height / rows;

	// create the plane vertices.
	for (int r = 0; r <= rows; ++r)
	{
		for (int c = 0; c <= cols; ++c)
		{
			VERTEX v;
			v.m_position = XMFLOAT3(-width / 2 + c * dx, 0, -height / 2 + r * dz);
			v.m_texture_coord = XMFLOAT2(static_cast<float>(c) / cols, static_cast<float>(r) / rows);
			mesh.vertices.push_back(v);
		}
	}

	// create the plane indices
	for (int r = 0; r <= rows; ++r)
	{
		for (int c = 0; c <= cols; ++c)
		{
			uint16_t i0 = r * (cols + 1) + c;
			uint16_t i1 = i0 + 1;
			uint16_t i2 = i0 + cols + 1;
			uint16_t i3 = i2 + 1;

			mesh.indices.insert(mesh.indices.end(), { i0, i2, i1, i1, i2, i3 });
		}
	}

	return mesh;
}

submesh_geometry primitive_mesh_generator::create_sphere(float radius, int slices, int stacks)
{
	submesh_geometry mesh;

	for (int stack = 0; stack <= stacks; ++stack)
	{
		// each stack has it's own angle ralative to the center of the sphere.
		float phi = XM_PI * stack / stacks;
		for (int slice = 0; slice <= slices; ++slice)
		{
			float theta = XM_2PI * slice / slices;
			float x = sinf(phi) * cosf(theta);
			float y = cosf(phi);
			float z = sinf(phi) * sinf(theta);

			VERTEX v;
			v.m_position = XMFLOAT3(radius * x, radius * y, radius * z);
			v.m_texture_coord = XMFLOAT2(static_cast<float>(slice) / slices, static_cast<float>(stack) / stacks);
			mesh.vertices.push_back(v);
		}

	}


	// create the sphere indices
	for (int stack = 0; stack <= stacks; ++stack)
	{
		for (int slice = 0; slice <= slices; ++slice)
		{
			uint16_t i0 = stack * (slices + 1) + slice;
			uint16_t i1 = i0 + 1;
			uint16_t i2 = i0 + slices + 1;
			uint16_t i3 = i2 + 1;

			mesh.indices.insert(mesh.indices.end(), { i0, i2, i1, i1, i2, i3 });
		}
	}

	return mesh;
}

submesh_geometry primitive_mesh_generator::create_cylinder(float radius, float height, int slices)
{
	return submesh_geometry();
}

submesh_geometry primitive_mesh_generator::create_cone(float radius, float height, int slices)
{
	return submesh_geometry();
}

submesh_geometry primitive_mesh_generator::create_torrus(
	float major_radius, float minor_radius, int major_segments, int minor_segments
)
{
	submesh_geometry mesh;

	for (int i = 0; i <= major_segments; ++i)
	{
		float phi = XM_2PI * i / major_segments;
		XMMATRIX rot_y = XMMatrixRotationY(phi);
		for (int j = 0; j <= minor_segments; ++j)
		{
			float theta = XM_2PI * j / minor_segments;
			float x = cosf(theta) * minor_radius;
			float y = sinf(theta) * minor_radius;
			float z = 0;


			auto tmp1 = XMFLOAT3(x + major_radius, y, z);
			auto tmp2 = XMFLOAT3(x, y, z);
			XMVECTOR pos = XMVector3Transform(XMLoadFloat3(&tmp1), rot_y);
			XMVECTOR normal = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&tmp2), rot_y));

			XMFLOAT3 p, n;
			XMStoreFloat3(&p, pos);
			XMStoreFloat3(&n, normal);

			VERTEX v;
			v.m_position = p;
			v.m_texture_coord = { static_cast<float>(i) / major_segments, static_cast<float>(j) / minor_segments };
			mesh.vertices.push_back(v);
		}
	}

	// create the indices for the torrus
	int stride = minor_segments + 1;
	for (int i = 0; i <= major_segments; ++i)
	{
		for (int j = 0; j <= minor_segments; ++j)
		{
			uint16_t i0 = i * stride + j;
			uint16_t i1 = i0 + 1;
			uint16_t i2 = i0 + stride;
			uint16_t i3 = i2 + 1;

			mesh.indices.insert(mesh.indices.end(), { i0, i2, i1, i1, i2, i3 });
		}
	}
	return mesh;
}

submesh_geometry primitive_mesh_generator::create_pyramid(float base_size, float height)
{
	return submesh_geometry();
}
