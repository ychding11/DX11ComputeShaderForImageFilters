//
//https://www.geeks3d.com/20091020/shader-library-lens-circle-post-processing-effect-glsl/
//  

SamplerState samLinear: register(s0);
//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer CB : register(b0)
{
    unsigned int g_iWidth;
    unsigned int g_iHeight;
};

Texture2D<float4>   InputMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

[numthreads(32, 32, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{                                     
    float3 uv = float3(dispatchThreadID.xyz) / float3(g_iWidth, g_iHeight, 1.f);
    float dist = distance(uv.xy, float2(0.5, 0.5) );
    float4 data = InputMap.SampleLevel(samLinear, uv.xy, 0);
    //float4 data = InputMap.Sample(samLinear, uv.xy); // not work in cs
    data.rgb *= smoothstep( 0.48, 0.38,dist); // does this fucntion work in glsl ? https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/smoothstep.xhtml
    OutputMap[dispatchThreadID.xy] = data;
}
