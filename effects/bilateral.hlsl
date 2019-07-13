
//--------------------------------------------------------------------------------------
// Adapted From: 
// https://www.shadertoy.com/view/4dfGDH
//--------------------------------------------------------------------------------------

SamplerState samLinear: register( s0 );

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

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 15


//--------------------------------------------------------------------------------------
// Normal distribution: http://mathworld.wolfram.com/NormalDistribution.html
// Mean : 0
// Variance : sigma * sigma
//--------------------------------------------------------------------------------------
float normpdf(in float x, in float sigma)
{
    return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}
float normpdf3(in float3 v, in float sigma)
{
    return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

//! Load
//! The texture coordinates; the last component specifies the mipmap level. This method uses a 0-based coordinate system and not a 0.0-1.0 UV system. 
[numthreads(32, 32, 1)]
void CSMain( uint3 dispatchThreadID : SV_DispatchThreadID )
{
    uint3 uv = dispatchThreadID.xyz;
	float3 c = InputMap.Load(uv).rgb; // current pixel
	const int kSize = (MSIZE-1) / 2;
	float kernel[MSIZE];
	float3 final_colour;
		
	//create the 1-D kernel
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), SIGMA);
	}
		
	float3 cc;
	float factor;
	float bZ = 1.0 / normpdf(0.0, BSIGMA);

	//read out the pixel 
	for (int i=-kSize; i <= kSize; ++i)
	{
		for (int j=-kSize; j <= kSize; ++j)
		{
			//cc = texture(iChannel0, vec2(0.0, 1.0)-(fragCoord.xy+vec2(float(i),float(j))) / iResolution.xy).rgb;
            cc = InputMap.Load(uv + int3(i, j, 0)).rgb;
			factor = normpdf3(cc-c, BSIGMA) * bZ * kernel[kSize+j] * kernel[kSize+i];
			Z += factor;
			final_colour += factor * cc;
		}
	}
    OutputMap[dispatchThreadID.xy] = float4(final_colour/Z, 1.0);
}
