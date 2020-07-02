
//--------------------------------------------------------------------------------------
// Adapted From: 
// https://www.shadertoy.com/view/4dfGDH
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer windowSize : register( b0 )
{
    unsigned int wSize = 5;
};

Texture2D<float4>   InputMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 19


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
	const int kSize = (wSize-1) / 2;
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

struct HS_PATCH_DATA
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
    float center[3] : CENTER;
};

struct HS_CONTROL_POINT
{
    float pos1[3] : POSITION1;
    float pos2[3] : POSITION2;
    float pos3[3] : POSITION3;
    float3 nor1 : NORMAL0;
    float3 nor2 : NORMAL1;
    float3 tex : TEXCOORD0;
};

[domain("tri")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[partitioning("fractional_odd")]
[patchconstantfunc("HullShaderPatchConstant")]
HS_CONTROL_POINT HullShaderControlPointPhase(InputPatch<HS_DATA_INPUT, 3> inputPatch,
    uint tid : SV_OutputControlPointID, uint pid : SV_PrimitiveID)
{

    int next = (1 << tid) & 3; // (tid + 1) % 3
    float3 p1 = inputPatch[tid].position;
    float3 p2 = inputPatch[next].position;
    float3 n1 = inputPatch[tid].normal;
    float3 n2 = inputPatch[next].normal;
    HS_CONTROL_POINT output;
    //control points positions
    output.pos1 = (float[3])p1;
    output.pos2 = (float[3])(2 * p1 + p2 - dot(p2 - p1, n1) * n1);
    output.pos3 = (float[3])(2 * p2 + p1 - dot(p1 - p2, n2) * n2);
    //control points normals
    float3 v12 = 4 * dot(p2 - p1, n1 + n2) / dot(p2 - p1, p2 - p1);
    output.nor1 = n1;
    output.nor2 = n1 + n2 - v12 * (p2 - p1);
    output.tex = inputPatch[tid].texcoord;
    return output;
}

//patch constant data
HS_PATCH_DATA HullShaderPatchConstant(OutputPatch<HS_CONTROL_POINT, 3> controlPoints)
{
    HS_PATCH_DATA patch = (HS_PATCH_DATA)0;
    //calculate Tessellation factors
    HullShaderCalcTessFactor(patch, controlPoints, 0);
    HullShaderCalcTessFactor(patch, controlPoints, 1);
    HullShaderCalcTessFactor(patch, controlPoints, 2);
    patch.inside = max(max(patch.edges[0], patch.edges[1]), patch.edges[2]);
    //calculate center
    float3 center = ((float3)controlPoints[0].pos2 + (float3)controlPoints[0].pos3) * 0.5 -
        (float3)controlPoints[0].pos1 +
        ((float3)controlPoints[1].pos2 + (float3)controlPoints[1].pos3) * 0.5 –
        (float3)controlPoints[1].pos1 +
        ((float3)controlPoints[2].pos2 + (float3)controlPoints[2].pos3) * 0.5 –
        (float3)controlPoints[2].pos1;
    patch.center = (float[3])center;
    return patch;
}

//helper functions
float edgeLod(float3 pos1, float3 pos2) { return dot(pos1, pos2); }

void HullShaderCalcTessFactor(inout HS_PATCH_DATA patch,
    OutputPatch<HS_CONTROL_POINT, 3> controlPoints, uint tid : SV_InstanceID)
{
    int next = (1 << tid) & 3; // (tid + 1) % 3
    patch.edges[tid] = edgeLod((float3)controlPoints[tid].pos1, (float3)controlPoints[next].pos1);
    return;
}

// { Tessellated samples, Control points } ==> Evalated vertex
[domain("triangle")]
DS_DATA_OUTPUT DomainShaderPN(HS_PATCH_DATA patchData,
    const OutputPatch<HS_CONTROL_POINT, 3> input, float3 uvw : SV_DomainLocation)
{
    DS_DATA_OUTPUT output;
    float u = uvw.x;
    float v = uvw.y;
    float w = uvw.z;
    //output position is weighted combination of all 10 position control points
    float3 pos =
        (float3)input[0].pos1 * w*w*w + (float3)input[1].pos1 * u*u*u + (float3)input[2].pos1 * v*v*v +
        (float3)input[0].pos2 * w*w*u + (float3)input[0].pos3 * w*u*u + (float3)input[1].pos2 * u*u*v +
        (float3)input[1].pos3 * u*v*v + (float3)input[2].pos2 * v*v*w + float3)input[2].pos3 * v*w*w +
        (float3)patchData.center * u*v*w;
    //output normal is weighted combination of all 10 position control points
    float3 nor =
        input[0].nor1 * w*w + input[1].nor1 * u*u + input[2].nor1 * v*v +
        input[0].nor2* w*u + input[1].nor2 * u*v + input[2].nor2 * v*w;
    //transform and output data
    output.position = mul(float4(pos, 1), g_mViewProjection);
    output.view = mul(float4(pos, 1), g_mView).xyz;
    output.normal = mul(float4(normalize(nor), 1), g_mNormal).xyz;
    output.vUV = input[0].tex * w + input[1].tex * u + input[2].tex * v;
    return output;
}
