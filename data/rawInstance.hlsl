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
 
    float4 r0   : INSTMAT0;
	float4 r1   : INSTMAT1;
	float4 r2   : INSTMAT2;
	float4 r3   : INSTMAT3;
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
	float4x4 Inst = float4x4(input.r0, input.r1, input.r2, input.r3);
	float3 posWorld = mul(input.PositionOS, Inst);
	float v = dot(input.Normal, EyePos - posWorld);
	color = color * v;
	
    output.PositionCS = mul(mul(input.PositionOS, Inst), ViewProjection);
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