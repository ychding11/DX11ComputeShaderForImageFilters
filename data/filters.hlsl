//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer CB : register( b0 )
{
    unsigned int g_iWidth;
    unsigned int g_iHeight;
};

#define THREAD_GROUP_SIZE_X 960

Texture2D<float4>   InputMap : register(t0);
RWTexture2D<float4> OutputMap : register(u8);

static const float filter[7] = {
    0.030078323, 0.104983664, 0.222250419, 0.285375187, 0.222250419,
    0.104983664, 0.030078323
};

groupshared float4 sharedData[THREAD_GROUP_SIZE_X];

[numthreads(THREAD_GROUP_SIZE_X, 1, 1)]
void main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
    float4 data = InputMap.Load(dispatchThreadID);
    sharedData[dispatchThreadID.x] = data;
		
	GroupMemoryBarrierWithGroupSync();
	
    int3 texturelocation = dispatchThreadID - int3(3, 0, 0);
    float4 Color = float4(0.0, 0.0, 0.0, 0.0);
    for (int x = 0; x < 7; x++)
        Color += sharedData[texturelocation.x + x] * filter[x];

    OutputMap[dispatchThreadID.xy] = Color;
}
