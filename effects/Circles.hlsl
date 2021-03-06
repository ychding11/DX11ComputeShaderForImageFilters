

float threashold = 2.90;
Texture2D<float4>   InputMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

[numthreads(32, 32, 1)]
void CSMain( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	int x = dispatchThreadID.x;
	int y = dispatchThreadID.y;
    float4 pixel = InputMap[dispatchThreadID.xy];
	OutputMap[dispatchThreadID.xy] = pixel;
	GroupMemoryBarrierWithGroupSync();
	
	if( dot(pixel.rgb, float3(1.0,1.0,1.0) ) > 2.9 )
	{
		for( float alpha = 0; alpha < 360; alpha += 1 )
		{
			int X = clamp(x + cos(alpha) * 5, 0, 10);
            int Y = clamp(y + sin(alpha) * 5, 0, 10);
            OutputMap[int2(X, Y)] = float4(1.0, 0, 0, 1.0);
		}
	}
}
