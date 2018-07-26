//
// https://www.geeks3d.com/20110428/shader-library-swirl-post-processing-filter-in-glsl
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

//static const uint2 center = uint2(200, 200);
static const float radius = 200.0f;
static const float swirlPower = 6.f;
static const float angle = .8f;

float2 swirlSample1( float2 uv, float time)
{
    float2 center = float2(g_iWidth * .5f, g_iHeight * .5f);
    uv -= center;
    float r = length(uv);
    if (r < radius)
    {
        float theta = atan2(uv.y, uv.x);
        float percent = (radius - r) / radius;
        float distortion = pow(swirlPower * percent, 2);
        theta += distortion;
        uv.x = r * cos(theta);
        uv.y = r * sin(theta);
    }
    uv += center;
    return float2( float2(uv.x, uv.y) / float2(g_iWidth, g_iHeight) );
}

float2 swirlSample2( float2 uv, float time)
{
    float2 center = float2(g_iWidth * .5f, g_iHeight * .5f);
    uv -= center;
    float r = length(uv);
    if (r < radius)
    {
        float percent = (radius - r) / radius;
        float theta = percent * percent * angle * 8.0;
        float s = sin(theta);
        float c = cos(theta);
        uv = float2(dot(uv, float2(c, -s)), dot(uv, float2(s, c)));
    }
    uv += center;
    return float2( float2(uv.x, uv.y) / float2(g_iWidth, g_iHeight) );
}

[numthreads(32, 32, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{                                     
    float2 uv = swirlSample2( float2(dispatchThreadID.x, dispatchThreadID.y), 0.0f);
    float4 data = InputMap.SampleLevel(samLinear, uv.xy, 0);
    OutputMap[dispatchThreadID.xy] = data;
}
