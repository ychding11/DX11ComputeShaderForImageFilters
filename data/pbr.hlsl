// VS VS
// PS PS


static const float PI = 3.14159265359;
static const float Epsilon = 0.00001;
static const uint  NumLights = 3;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

struct DirectionalLight
{
	float3 direction;
	float3 radiance;
	uint   enabled;
};

// ================================================================================================
// Constant buffers
// ================================================================================================
cbuffer VSConstants : register(b0)
{
    float4x4 WorldMatrix;
    float4x4 ViewProjectionMatrix;
}

cbuffer PSConstants : register(b0)
{
	float3 eyePosition;
	uint    cShadingFlag;
}

cbuffer PSConstants : register(b1)
{
	DirectionalLight lights[NumLights];
}

#define ENABLE_IBL_AMBIENT   (0x1 << 0)
#define ENABLE_IBL_SPECULAR  (0x1 << 1)

// ================================================================================================
// Textures
// ================================================================================================

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D roughnessTexture : register(t2);
Texture2D metalnessTexture : register(t3);

TextureCube specularTexture   : register(t4);
TextureCube irradianceTexture : register(t5);
Texture2D   specularBRDF_LUT  : register(t6);

SamplerState defaultSampler : register(s0);
SamplerState spBRDF_Sampler : register(s1);



// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// The Fresnel reflectance tells us much how much of the light is reflected.
// As angle increases, the Fresnel reflectance stays almost the same, 
// But for very glancing angles it goes to 100% at all wavelengths.
// F(0) is the surface’s characteristic specular color: Cspec 
// Shlick's approximation of the Fresnel reflectance.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Returns number of mipmap levels for specular IBL environment map.
uint querySpecularTextureLevels()
{
	uint width, height, levels;
	specularTexture.GetDimensions(0, width, height, levels);
	return levels;
}

// ================================================================================================
// Input/Output structs
// ================================================================================================
struct VSInput
{
    float3 position 		: POSITION;
	float3 normal           : NORMAL;
	float2 texcoord         : TEXCOORD;
	float3 tangent          : TANGENT;
	float3 bitangent        : BITANGENT;
};

struct PixelShaderInput
{
	float4 pixelPosition  : SV_POSITION;
	float3 position       : POSITION;
	float2 texcoord       : TEXCOORD;
	float3x3 tangentBasis : TBASIS;
};


// ================================================================================================
// Vertex Shader
// ================================================================================================

PixelShaderInput VS(VSInput vin)
{
	PixelShaderInput vout;
	vout.position = mul(float4(vin.position, 1.0), WorldMatrix).xyz;
	vout.texcoord = float2(vin.texcoord.x, 1.0-vin.texcoord.y);

	// Pass tangent space basis vectors (for normal mapping).
	float3x3 TBN = float3x3(vin.tangent, vin.bitangent, vin.normal);
	vout.tangentBasis = mul((float3x3)WorldMatrix, transpose(TBN));

	float4x4 mvpMatrix = mul(WorldMatrix, ViewProjectionMatrix);
	vout.pixelPosition = mul(float4(vin.position, 1.0), mvpMatrix);
	return vout;
}



// ================================================================================================
// Pixel shader
// ================================================================================================

float4 PS(PixelShaderInput pin) : SV_Target
{
	// Sample input textures to get shading model params.
	float3 albedo   = albedoTexture.Sample(defaultSampler, pin.texcoord).rgb;
	float metalness = metalnessTexture.Sample(defaultSampler, pin.texcoord).r;
	float roughness = roughnessTexture.Sample(defaultSampler, pin.texcoord).r;

    // Get current fragment's normal and transform to world space.
	float3 N = normalize(2.0 * normalTexture.Sample(defaultSampler, pin.texcoord).rgb - 1.0);
	N = normalize(mul( N, pin.tangentBasis));
	
	// Outgoing light direction (vector from world-space fragment position to the "eye").
	float3 Lo = normalize(eyePosition - pin.position);

	// Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));
		
	// Specular reflection vector.
	float3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	float3 F0 = lerp(Fdielectric, albedo, metalness);

	// Direct lighting calculation for directional lights.
	float3 directLighting = 0.0;
	for(uint i=0; i<NumLights && lights[i].enabled; ++i)
	{
		float3 Li = -lights[i].direction;
		float3 Lradiance = lights[i].radiance;

		// Half-vector between Li and Lo.
		float3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		float3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, roughness);
		
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);

        // There are two very different light-material interactions: 
		// 	 - surface reflection == reflection term
		//   - refraction-absorption-scattering == diffuse term
		// Diffuse is always subsurface scattering, the difference is the scale("pixel size" compared to "entry-exit distances")
		// Non-metals cause various degrees of absorption and scattering, it is refraction-absorption-scattering.
		// Metals immediately absorb all refracted light. So Metals either reflect or absorb energy.
		// Thus metal's diffuse contribution is zero.
		// To be energy conserving, diffuse BRDF contribution is based on Fresnel factor & metalness.
		float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		float3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
		float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}

	// Ambient lighting (IBL).
	float3 ambientLighting = float3(0,0,0);
	if (cShadingFlag & ENABLE_IBL_AMBIENT)
	{
		// Sample diffuse irradiance at normal direction from a cube map
		float3 irradiance = irradianceTexture.Sample(defaultSampler, N).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since using pre-filtered cubemap(s) and irradiance is from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		float3 F = fresnelSchlick(F0, cosLo);

		// Get diffuse contribution factor (as with direct lighting).
		float3 kd = lerp(1.0 - F, 0.0, metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		float3 diffuseIBL = kd * albedo * irradiance;
		ambientLighting += diffuseIBL;
	}
	if (cShadingFlag & ENABLE_IBL_AMBIENT)
	{
		// Sample pre-filtered specular reflection environment at correct mipmap level.
		uint specularTextureLevels = querySpecularTextureLevels();
		float3 specularIrradiance = specularTexture.SampleLevel(defaultSampler, Lr, roughness * specularTextureLevels).rgb;

		// Split-sum approximation factors for Cook-Torrance specular BRDF.
		float2 specularBRDF = specularBRDF_LUT.Sample(spBRDF_Sampler, float2(cosLo, roughness)).rg;

		// Total specular IBL contribution.
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		ambientLighting += specularIBL;
	}

	// Final fragment color.
	return float4(directLighting + ambientLighting, 1.0);
}
