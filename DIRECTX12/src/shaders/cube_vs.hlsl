// world view projection matrix
struct constant_buffer
{
    row_major float4x4 g_world_view_projection;
};

ConstantBuffer<constant_buffer> wvp_c_buffer : register(b0);


struct pixel_shader_input
{
    float4 position  : SV_Position;
    float4 color     : COLOR0;
    float2 uv_coords : TEXCOORD0;
    
    // instance data
    float4 row1 : INSTANCE_TRANSFORM0;
    float4 row2 : INSTANCE_TRANSFORM1;
    float4 row3 : INSTANCE_TRANSFORM2;
    float4 row4 : INSTANCE_TRANSFORM3;
};




pixel_shader_input main(
    float3 pos   : POSITION0, 
    float4 color : COLOR0, 
    float2 uv    : TEXCOORD0, 

    float4 row1 : INSTANCE_TRANSFORM0,
    float4 row2 : INSTANCE_TRANSFORM1,
    float4 row3 : INSTANCE_TRANSFORM2,
    float4 row4 : INSTANCE_TRANSFORM3
)
{
    pixel_shader_input result;
    // transform the position to homogenous clip space
    float4x4 instance_transform = float4x4(row1, row2, row3, row4);
    
    // float4 world_pos = mul(float4(pos, 1.0f), instance_transform);
    float4 world_pos = float4(pos, 1.0f) + row4;
    result.position  = mul(world_pos, wvp_c_buffer.g_world_view_projection);

    result.uv_coords = uv;
    result.color = float4(0.3f, 0.2f, 0.6f, 1.0f);
    
	return result;
}