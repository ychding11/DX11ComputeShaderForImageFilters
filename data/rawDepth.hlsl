// VS VS
// PS PS


// ================================================================================================
// Constant buffers
// ================================================================================================
cbuffer VSConstants : register(b0)
{
    float4x4 ViewProjection;
}

// ================================================================================================
// Input/Output structs
// ================================================================================================
struct VSInput
{
    float4 PositionOS 		: POSITION;
};

struct VSOutput
{
    float4 PositionCS 		: SV_Position;
};

// ================================================================================================
// Vertex Shader
// ================================================================================================
VSOutput VS(in VSInput input)
{
    VSOutput output;

    // Calc the clip-space position
    output.PositionCS = mul(input.PositionOS, ViewProjection);
    //output.PositionCS = output.PositionCS.xyzw / output.PositionCS.w;
    return output;
}

// ================================================================================================
// Pixel Shader
// ================================================================================================
float4 PS() : SV_Target
{
    return float4(0.5f, 0.5f, 0.5f, 1.0f);
}