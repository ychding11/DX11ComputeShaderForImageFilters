// VS VSCanvas
// PS PSCanvas
// PS PSSdfPrimitive
// PS PSClound


//--------------------------------------------------------------------------------------
// reference
// https://xiaoiver.github.io/coding/2018/07/20/%E7%BB%98%E5%88%B6-Pattern.html
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer CB : register(b0)
{
    unsigned int cWidth;
    unsigned int cHeight;
	float cTime;
};


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//-------------------------------------------------------------
// Vertex Shader
//-------------------------------------------------------------
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

//-------------------------------------------------------------
// judge a pixel position[0,1) in circle: 1 in circle, 0 out circle
// create a smooth transition in sphere edge
//-------------------------------------------------------------
float circle(in float2 _st, in float _radius)
{
    float2 l = _st-float2(0.5,0.5);
	float p = 4./cWidth; // transition region is resolution based. hard coded
    return smoothstep( p, -p, length(l)-_radius ); // 1 - smoothstep(a,b,x) = smoothstep(b,a,x)
}

float circlePattern(float2 st, float radius) 
{
    return circle(st+float2(0.,-.5), radius)+
           circle(st+float2(0.,.5), radius) +
           circle(st+float2(-.5,0.), radius)+
           circle(st+float2(.5,0.), radius);
}

//--------------------------------------------------------------------------------------
// Pixel Shader: Draw circles 
//--------------------------------------------------------------------------------------
float4 PSCanvas( PS_INPUT input) : SV_Target
{
	float2 resolution = float2(cWidth,cHeight);
	float2 st = input.Pos.xy / cHeight; // convert pixel coordinate to [0,1)
	st *= 3.0f;
	st = frac(st); // fract() in glsl
	
	float3 color = float3(0.f,0.f,0.f);
	float v = circlePattern(st, 0.25*(sin(cTime*2)+1.)); // animate radius of circle
	color = float3(v, v, v); 
	
	// interplate color
	color = lerp(float3(0.075,0.114,0.329),float3(0.973,0.843,0.675),color);

    return float4(color,1.0f);
}

float random (in float2 _st) 
{
    return frac(sin(dot(_st.xy,float2(12.9898,78.233)))*43758.5453123);
}

float noise (in float2 _st) 
{
    float2 i = floor(_st);
    float2 f = frac(_st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);

    return  lerp(a, b, u.x) +
            (c - a) * u.y * (1.0 - u.x) +
            (d - b) * u.y * u.x;
}

#define NUM_OCTAVES 5

float fbm ( in float2 _st) 
{
    float v = 0.0;
    float a = 0.5;
    float2 shift = float2(100.0, 100.0);
	
    // Rotate to reduce axial bias
    float2x2 rot = float2x2( cos(0.5), sin(0.5),
							-sin(0.5), cos(0.5)); // mat2 in glsl
    for (int i = 0; i < NUM_OCTAVES; ++i) 
	{
        v += a * noise(_st);
        _st =  mul(_st, rot) * 2.0 + shift; // hlsl vector is a row vector. matrix multiplication is done by intrisic function
        a *= 0.5;
    }
    return v;
}

float4 PSClound( PS_INPUT input) : SV_Target
{
    float2 resolution = float2(cWidth,cHeight);
	float2 st = input.Pos.xy / cHeight; // convert pixel coordinate to [0,1)

    float2 q = float2(0,0);
    q.x = fbm( st + 0.00*cTime);
    q.y = fbm( st + float2(1.0, 1.0));

    float2 r = float2(0,0);
    r.x = fbm( st + 1.0*q + float2(1.7, 9.2)+ 0.15*cTime );
    r.y = fbm( st + 1.0*q + float2(8.3, 2.8)+ 0.126*cTime);

    float f = fbm(st+r);

    float3 color = lerp(float3(0.101961,0.619608,0.666667),
                 float3(0.666667,0.666667,0.498039),
                 clamp((f*f)*4.0,0.0,1.0));

    color = lerp(color,
                 float3(0,0,0.164706),
                 float3(length(q),0.0,1.0));

    color = lerp(color,
                float3(0.666667,1,1),
                clamp(length(r.x),0.0,1.0));

    return float4((f*f*f+.6*f*f+.5*f)*color,1.);
}


//--------------------------------------------------------------------------------------
// 2D SDF functions for primitives
//--------------------------------------------------------------------------------------
float sdfCircle(float2 p, float2 center, float radius)
{
	return length(p-center) - radius;
}

//--------------------------------------------------------------------------------------
// Color a pixel
// how to color a pixel with sdf "d" by "color" with stroke width "stroke"
//--------------------------------------------------------------------------------------
float4 sdfColor(float d, float3 color, float stroke) 
{
	float aaWdith = fwidth(d) * 1.0;
	float4 colorLayer = float4(color, 1.0 - smoothstep(-aaWdith, aaWdith, d));
	float4 strokeLayer = float4(float3(0.05, 0.05, 0.05), 1.0 - smoothstep(-aaWdith, aaWdith, d - stroke));
	return float4(lerp(strokeLayer.rgb, colorLayer.rgb, colorLayer.a), strokeLayer.a);
}

//--------------------------------------------------------------------------------------
// Pixel Shader: SDF Primitives
//--------------------------------------------------------------------------------------
float4 PSSdfPrimitive( PS_INPUT input) : SV_Target
{
	float2 resolution = float2(cWidth,cHeight);

	// hard-coded, maybe controled by parameter
    float4 _BackgroundColor = float4(1, 1, 1, 1);
	float4 _Color = float4(1.0, 0, 0, 0);
	
	float d = sdfCircle(input.Pos.xy, float2(0.5, 0.5)* resolution, 100);
	float4 v = sdfColor(d, _Color, fwidth(d) * 2.0);
	return lerp(_BackgroundColor, v, v.a);
}

