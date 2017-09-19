
//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer CB : register( b0 )
{
    unsigned int g_iWidth;
    unsigned int g_iHeight;
};

Texture2D<float4>   InputMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

[numthreads(32, 18, 1)]
void CSMain( uint3 dispatchThreadID : SV_DispatchThreadID )
{
    float4 data = InputMap.Load(dispatchThreadID);
    data = data * 0.9;
    OutputMap[dispatchThreadID.xy] = data;
}
