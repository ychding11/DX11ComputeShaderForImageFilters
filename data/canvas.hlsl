// VS VSCanvas
// PS PSCanvas
// PS PSSdfPrimitive


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
	// create a smooth transition in sphere edge
	// smoothstep(a,b,x) = smoothstep(b,a,x)
    //return 1.f - smoothstep(_radius-(_radius*0.01),_radius+(_radius*0.01), dot(l,l)*4.0);
	//return smoothstep(_radius+(_radius*0.01), _radius-(_radius*0.01), dot(l,l)*4.0);
	//return 1.f - step(_radius*_radius, dot(l,l)*4.0);
	
	// transition region is resolution based. it is more robust
	float p = 4./cWidth;
    return smoothstep( p, - p, length(l)-_radius );

}

float circlePattern(float2 st, float radius) 
{
    return circle(st+float2(0.,-.5), radius)+
        circle(st+float2(0.,.5), radius)+
        circle(st+float2(-.5,0.), radius)+
        circle(st+float2(.5,0.), radius);
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
	
	//float v = circle(st, 0.25);
	float v = circlePattern(st, 0.25*(sin(cTime*2)+1.));
	//color = float3(v, v, v); 
	
	// interplate color
	color += lerp(float3(0.075,0.114,0.329),float3(0.973,0.843,0.675),float3(v, v, v));

    return float4(color,1.0f);
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
	float4 _Color = float4(1., 0, 0, 0);
	
	float d = sdfCircle(input.Pos.xy, float2(0.5, 0.5)* resolution, 100);
	float4 v = sdfColor(d, _Color, fwidth(d) * 2.0);
	return lerp(_BackgroundColor, v, v.a);
}

