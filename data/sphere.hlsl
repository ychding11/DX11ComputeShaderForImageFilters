// VS VSSphere
// PS PSSphere


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
PS_INPUT VSSphere( in uint VertexIdx : SV_VertexID)
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



//--------------------------------------------------------------------------------------
// sphere: "xyz" is center, "w" is radius.
// ray: origin "ro" with direction "rd"
//--------------------------------------------------------------------------------------
float sphIntersect( in float3 ro, in float3 rd, in float4 sph )
{
	float3 oc = ro - sph.xyz;
	float b = dot( oc, rd );
	float c = dot( oc, oc ) - sph.w*sph.w;
	float h = b*b - c;
	if( h<0.0 ) return -1.0; // no hit
	return -b - sqrt( h );
}

//--------------------------------------------------------------------------------------
// reference  http://iquilezles.org/www/articles/sphereshadow/sphereshadow.htm
// "ro"  hit point.
// "rd"  light direction
// "sph" sphere in test
// "k"   the sharpness of the shadow penumbra. Higher values means sharper.
//--------------------------------------------------------------------------------------
float sphSoftShadow( in float3 ro, in float3 rd, in float4 sph, in float k )
{
    float3 oc = ro - sph.xyz;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - sph.w*sph.w;
    float h = b*b - c;
    
#if 1
    // physically plausible shadow
    float d = sqrt( max(0.0,sph.w*sph.w-h)) - sph.w;
    float t = -b - sqrt( max(h,0.0) );
    return (t<0.0) ? 1.0 : smoothstep(0.0, 1.0, 2.5*k*d/t );
#else
    // cheap but not plausible alternative
    return (b>0.0) ? step(-0.0001,c) : smoothstep( 0.0, 1.0, h*k/b );
#endif    
}    
            
float sphOcclusion( in float3 pos, in float3 nor, in float4 sph )
{
    float3  r = sph.xyz - pos;
    float l = length(r);
    return dot(nor,r)*(sph.w*sph.w)/(l*l*l);
}

float3 sphNormal( in float3 pos, in float4 sph )
{
    return normalize(pos-sph.xyz);
}

//=====================================================

float2 hash2( float n ) { return frac(sin(float2(n,n+1.0))*float2(43758.5453123,22578.1459123)); }

//--------------------------------------------------------------------------------------
// ray intersect plane at y = -1.0 
//--------------------------------------------------------------------------------------
float iPlane( in float3 ro, in float3 rd )
{
    return (-1.0 - ro.y)/rd.y;
}

//--------------------------------------------------------------------------------------
// Pixel Shader: trace sphere on a plane 
//--------------------------------------------------------------------------------------
float4 PSSphere( PS_INPUT input) : SV_Target
{
	float2 resolution = float2(cWidth,cHeight);
    float2 pixelCoord = float2(input.Pos.x, cHeight - input.Pos.y);
    float2 p = (2.0*pixelCoord-resolution.xy) / resolution.y;
    
	// "camera" setting
	float3 ro = float3(0.0, 0.0, 4.0 );
	float3 rd = normalize( float3(p,-2.0) );
	
    // sphere animation
    float4 sph = float4( cos( cTime + float3(2.0,1.0,1.0) + 0.0 )*float3(.5,0.0,1.0), 1.0 );
    sph.x = 1.0;   
	
	// directional light setting
    float3 lig = normalize( float3(0.6,0.3,0.4)); 
	
    float3 col = float3(0.,0,0);

    float tmin = 1e10;
    float3 nor;
    float  occ = 1.0; // 1.0 means no occlude
    
    float t1 = iPlane( ro, rd );
    if( t1>0.0 )
    {
        tmin = t1;
        float3 pos = ro + t1*rd;
        float3 nor = float3(0.0,1.0,0.0);
        occ = 1.0-sphOcclusion( pos, nor, sph );
    }
#if 1
    float t2 = sphIntersect( ro, rd, sph );
    if( t2>0.0 && t2<tmin )
    {
        tmin = t2;
        float3 pos = ro + t2*rd;
        nor = sphNormal( pos, sph );
        occ = 0.5 + 0.5*nor.y;
	}
#endif 
    if( tmin<1000.0 )
    {
        float3 pos = ro + tmin*rd;
        
		col = float3(1.0,1.0,1.0);
        col *= clamp( dot(nor,lig), 0.0, 1.0 );
        col *= sphSoftShadow( pos, lig, sph, 2.0 );
        col += 0.05*occ;
	    col *= exp( -0.05*tmin );
    }

    col = sqrt(col);
    return float4( col, 1.0 );
}
