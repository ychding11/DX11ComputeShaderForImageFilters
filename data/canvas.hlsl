// VS VSCanvas
// PS PSCanvas


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer CB : register(b0)
{
    unsigned int cWidth;
    unsigned int cHeight;
};


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VSCanvas( in uint VertexIdx : SV_VertexID)
{
	PS_INPUT output;
    
	if(VertexIdx == 0)
    {
        output.Pos = float4(-1.0f, 1.0f, 1.0f, 1.0f);
        output.Tex = float2(0.0f, 0.0f);
    }
    else if(VertexIdx == 1)
    {
        output.Pos = float4(3.0f, 1.0f, 1.0f, 1.0f);
        output.Tex = float2(2.0f, 0.0f);
    }
    else
    {
        output.Pos = float4(-1.0f, -3.0f, 1.0f, 1.0f);
        output.Tex = float2(0.0f, 2.0f);
    }
    return output;
}

float circle(in float2 _st, in float _radius)
{
    float2 l = _st-float2(0.5,0.5);
    return 1.f - smoothstep(
        _radius-(_radius*0.01),
        _radius+(_radius*0.01),
        dot(l,l)*4.0);
}

//--------------------------------------------------------------------------------------
// Pixel Shader: Draw circles 
//--------------------------------------------------------------------------------------
float4 PSCanvas( PS_INPUT input) : SV_Target
{
	float2 resolution = float2(cWidth,cHeight);
	float2 st = input.Pos.xy / resolution;
	float3 color = float3(0.f,0.f,0.f);
	st *= 3.0f;
	st = frac(st); // fract() in glsl
	float v = circle(st, 0.5);
	color = float3(v, v, v); 
    return float4(color,1.0f);
}
