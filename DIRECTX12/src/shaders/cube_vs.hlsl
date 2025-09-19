// world view projection matrix
struct constant_buffer
{
    row_major float4x4 g_world_view_projection;
};

ConstantBuffer<constant_buffer> wvp_c_buffer : register(b0);

struct pixel_shader_input
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv_coords : TEXCOORD;
};


pixel_shader_input main(float4 pos : POSITION, float2 uv: TEXCOORD, float4 color : COLOR)
{
    pixel_shader_input result;
    // transform the position to homogenous clip space
    result.position = mul(pos, wvp_c_buffer.g_world_view_projection);
    result.uv_coords = uv;
    result.color = color;
    
	return result;
}