#pragma once
#ifndef __shape_data__
#define __shape_data__

#include <vector>
#include <DirectXMath.h>
#include <DirectXColors.h>


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../geometry/vertex_attributes.h"


using namespace DirectX;

class primitive_mesh_generator
{
public:
	static submesh_geometry create_cube(float size = 1.0f); // done
	static submesh_geometry create_plane(float width = 1.0f, float height = 1.0f, int rows = 1, int cols = 1); // done
	static submesh_geometry create_sphere(float radius = 1.0f, int slices = 16, int stacks = 16); // done
	static submesh_geometry create_cylinder(float radius = 0.5f, float height = 1.0f, int slices = 16);
	static submesh_geometry create_cone(float radius = 0.5f, float height = 1.0f, int slices = 16);
	static submesh_geometry create_torrus(
		float major_radius = 1.0f, float minor_radius = 0.3f, int major_segments = 32, int minor_segments = 16
	); // done
	static submesh_geometry create_pyramid(float base_size = 1.0f, float height = 1.0f);
};



#endif