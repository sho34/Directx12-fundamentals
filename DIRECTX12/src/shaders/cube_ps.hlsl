// texture stuff
Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

struct pixel_shader_input
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv_coords : TEXCOORD;
};

float4 main(pixel_shader_input input) : SV_TARGET
{
    float4 custom_color = float4(0.7f, 0.2f, 0.3f, 1.0f);
    return t1.Sample(s1, input.uv_coords) * input.color;
}