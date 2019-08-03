

Texture2D<float4>   InputMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

[numthreads(32, 32, 1)]
void CSMain( uint3 dispatchThreadID : SV_DispatchThreadID )
{
    uint3 uv1 = uint3(dispatchThreadID.xy + 1, 0);
    uint3 uv2 = uint3(dispatchThreadID.xy - 1, 0);
    float4 data = InputMap.Load(uv1) - InputMap.Load(uv2);
    data.rgb = ( (data.r + data.g + data.b ) / 3.f )* 2.7f;
    OutputMap[dispatchThreadID.xy] = data;
}