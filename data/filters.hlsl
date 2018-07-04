//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer CB : register( b0 )
{
    unsigned int iWidth;
    unsigned int iHeight;
    unsigned int iDimX;
};

#define THREAD_GROUP_SIZE_X 960 
#define THREAD_GROUP_SIZE_Y 540

Texture2D<float4>   InputMap : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

static const float filter[7] = {
    0.030078323, 0.104983664, 0.222250419, 0.285375187, 0.222250419,
    0.104983664, 0.030078323
};

groupshared float4 sharedData[THREAD_GROUP_SIZE_X];

[numthreads(iWidth, 1, 1)]
void mainX( uint3 dispatchThreadID : SV_DispatchThreadID )
{
    float4 data = InputMap.Load(dispatchThreadID);
    sharedData[dispatchThreadID.x] = data;
		
	GroupMemoryBarrierWithGroupSync();
	
    int3 texturelocation = dispatchThreadID - int3(3, 0, 0);
    float4 Color = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < 7; i++)
        Color += sharedData[texturelocation.x + i] * filter[i];

    OutputMap[dispatchThreadID.xy] = Color;
}

[numthreads(1, iHeight, 1)]
void mainY( uint3 dispatchThreadID : SV_DispatchThreadID )
{
    float4 data = InputMap.Load(dispatchThreadID);
    sharedData[dispatchThreadID.y] = data;
		
	GroupMemoryBarrierWithGroupSync();
	
    int3 texturelocation = dispatchThreadID - int3(0, 3, 0);
    float4 Color = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < 7; i++)
        Color += sharedData[texturelocation.y + i] * filter[i];

    OutputMap[dispatchThreadID.xy] = Color;
}

[numthreads(1, 1, 1)]
void main()
{

}