struct VSOutput {
    float4 pos : SV_Position;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VSInput {
    float3 pos   : POSITION;  // location 0
    float3 color : COLOR;     // location 1
    float2 uv    : TEXCOORD;  // location 2
};

struct PushConstants {
    float4x4 ViewPerspectiveTransform;
}
[[vk::push_constant]]
PushConstants pc;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos   = mul(pc.ViewPerspectiveTransform, float4(input.pos, 1.0));
    output.color = input.color;
    output.uv    = input.uv;
    return output;
}



// fragment shader
//[require(spvDescriptorHeapEXT)]
[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target {

    DescriptorHandle<Sampler2D> texture = DescriptorHandle<Sampler2D>(uint2(3024, 16));

    return float4(input.pos.x - 400, input.pos.y - 300, 0, 0); //texture.Sample(input.uv) * float4(input.color, 1);
}