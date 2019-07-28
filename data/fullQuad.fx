// VS VS
// PS psSampleSrcImage
// PS psSampleResultImage

Texture2D srcTexture  : register( t0 );
Texture2D destTexture : register( t1 );
SamplerState samLinear: register( s0 );

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( in uint VertexIdx : SV_VertexID)
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

float4 psSampleSrcImage( PS_INPUT input) : SV_Target
{
    return srcTexture.Sample( samLinear, input.Tex );
}

float4 psSampleResultImage( PS_INPUT input) : SV_Target
{
	return destTexture.Sample( samLinear, input.Tex );
}
