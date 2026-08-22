cbuffer SpriteVS : register(b0, space1)
{
    float4x4 view_proj;
    float2 basis_x;
    float2 basis_y;
    float2 origin;
    float2 pivot;
};

struct VSInput
{
    float2 pos : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    float2 q = input.pos - pivot;
    float2 local = float2(q.x, -q.y);
    float2 world = origin + basis_x * local.x + basis_y * local.y;

    VSOutput o;
    o.pos = mul(view_proj, float4(world, 0.0, 1.0));
    o.uv = input.uv;
    return o;
}
