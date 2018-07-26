//
//  https://www.geeks3d.com/20140213/glsl-shader-library-fish-eye-and-dome-and-barrel-distortion-post-processing-filters/
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

static const float PI = 3.1415926535;

float2 fishEyeSample(float2 uvCoord)
{
    float aperture = 178.0;
    float apertureHalf = 0.5 * aperture * (PI / 180.0);
    float maxFactor = sin(apertureHalf);

    float2 uv;
    float2 xy = 2.0 * uvCoord.xy - 1.0;
    float d = length(xy);
    if (d < (2.0 - maxFactor))
    {
        d = length(xy * maxFactor);
        float z = sqrt(1.0 - d * d);
        float r = atan2(d, z) / PI;
        float phi = atan2(xy.y, xy.x);

        uv.x = r * cos(phi) + 0.5;
        uv.y = r * sin(phi) + 0.5;
    }
    else
    {
        uv = uvCoord.xy;
    }
    return uv;
}

[numthreads(32, 32, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{                                     
    float2 uv = fishEyeSample( float2(dispatchThreadID.x, dispatchThreadID.y) / float2(g_iWidth, g_iHeight) );
    float4 data = InputMap.SampleLevel(samLinear, uv.xy, 0);
    OutputMap[dispatchThreadID.xy] = data;
}
