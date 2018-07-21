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
    uint3 uv = dispatchThreadID.xyz;
    float4 data = InputMap.Load( uv);
    OutputMap[dispatchThreadID.xy] = data * (float(dispatchThreadID.x) / float(g_iWidth));
}
