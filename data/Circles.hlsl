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
	int x = dispatchThreadID.x;
	int y = dispatchThreadID.y;
    float4 pixel = InputMap[dispatchThreadID.xy]; //readPixel(dispatchThreadID.x, dispatchThreadID.y);

    if (pixel.a < 0.5) OutputMap[dispatchThreadID.xy] = pixel;
	GroupMemoryBarrierWithGroupSync();
	
	if( dot(pixel.rgb,float3(1,1,1)) > 2.9 )
	{
		for( float alpha = 0; alpha<360; alpha += 1 )
		{
			int X = clamp(x + cos(alpha) * 15, 0, g_iWidth);
            int Y = clamp(y + sin(alpha) * 15, 0, g_iHeight);
            OutputMap[int2(X, Y)] = float4(0.8, 0, 0.2, 1.0);
		}
	}
}
