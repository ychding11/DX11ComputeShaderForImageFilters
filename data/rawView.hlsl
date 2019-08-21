// VS VS
// PS PS


// ================================================================================================
// Constant buffers
// ================================================================================================
cbuffer VSConstants : register(b0)
{
    float4x4 World;
    float4x4 ViewProjection;
	float3 EyePos;
}

// ================================================================================================
// Input/Output structs
// ================================================================================================
struct VSInput
{
    float4 PositionOS 		: POSITION;
	float3 Normal           : NORMAL;
};

struct VSOutput
{
    float4 PositionCS 		: SV_Position;
	float4 color            : Color;
};



// ================================================================================================
// Vertex Shader
// ================================================================================================
VSOutput VS(in VSInput input)
{
    VSOutput output;
    float4 color = float4(0.5f, 0.5f, 0.5f, 1.0f);
	float3 posWorld = mul(input.PositionOS, World);
	float v = dot(input.Normal, EyePos - posWorld);
	color = color * v;
	
    output.PositionCS = mul(mul(input.PositionOS, World), ViewProjection);
	output.color = color;
    return output;
}




// ================================================================================================
// Pixel Shader
// ================================================================================================
float4 PS(in VSOutput input) : SV_Target
{
    return input.color;
}