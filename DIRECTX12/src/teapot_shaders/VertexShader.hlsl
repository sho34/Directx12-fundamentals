struct VertexData
{
	float3 pos : POSITION;
};

struct VertexToHull
{
	float3 pos : POSITION;
};

// this shader will be compiled manually by directx12 from the application side.
VertexToHull main(VertexData input)
{
	VertexToHull output;
	output.pos = input.pos;

	return output;
}