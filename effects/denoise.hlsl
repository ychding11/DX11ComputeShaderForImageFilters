

Texture2D<float4>   InputMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

[numthreads(32, 32, 1)]
void CSMain( uint3 dispatchThreadID : SV_DispatchThreadID )
{
    const float gamma = 0.6f;
    const float numColors = 8.0f;
    uint3 uv = dispatchThreadID.xyz;
    float3 c = InputMap.Load(uv).rgb;
    c = pow(c, float3(gamma, gamma, gamma));
    c = c * numColors;
    c = floor(c);
    c = c / numColors;
    c = pow(c, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));
    OutputMap[dispatchThreadID.xy] = float4(c, 1.0);
}