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
float4 PS(in VSOutput frag ) : SV_Target
{
    //return float4(0.5f, 0.5f, 0.5f, 1.0f);
	//return float4(frag.PositionCS.z, frag.PositionCS.z, frag.PositionCS.z, 1.0f);
	
   float3 coord = frag.PositionCS.xyz;
   float3 grid = abs(frac(coord - 0.5) - 0.5) / fwidth(coord);
   float l = min(min(grid.x, grid.y), grid.z);

  // Just visualize the grid lines directly
  float v = 1.0 - min(l, 1.0);
  return float4(float3(v,v,v), 1.0);
}