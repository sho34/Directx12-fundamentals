#pragma once
#ifndef __math_helpers__
#define __math_helpers__

#include <DirectXMath.h>

struct view_space_matrix
{
	// view matrix represents the camera XMMatrixLookAtLH() to get the view matrix.
	DirectX::XMMATRIX world_space = DirectX::XMMatrixIdentity();
};

struct world_space_matrix
{
	// world matrix contains all the transformations which positions scales and orients into our virtual world.
	// every object will have its own world matrix.
	// needs to be updated per 3D model.
};

struct projection_space_matrix
{
	// no matter how far an object is it will always appear to be the same size.
	// use XMMatrixOrthographicLH() to build. 
	DirectX::XMMATRIX projection_matrix_orthographic = DirectX::XMMatrixIdentity();
	// use XMMatrixPerspectiveLH() or XMMatrixPerspectiveForLH() to build.
	DirectX::XMMATRIX projection_matrix_perspective = DirectX::XMMatrixIdentity();
};

struct wvp_matrix
{
	// projection matrix moves all the vertices into projection space.
};

#endif