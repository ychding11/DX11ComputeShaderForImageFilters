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
	unsigned int cAnimateSphere;
	unsigned int cAnimateLight;
	float3 cLightColor;
};


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader for a full screen quad
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
// return shadow factor: 1: No shadow, 0: All shadow
//--------------------------------------------------------------------------------------
float sphSoftShadow( in float3 ro, in float3 rd, in float4 sph, in float k )
{
    float3 oc = ro - sph.xyz;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - sph.w*sph.w;
    float h = b*b - c;
    float esp = 1e-3;
#if 1
    // physically plausible shadow
    float d = sqrt( max(0.0,sph.w*sph.w-h)) - sph.w;
    float t = -b - sqrt( max(h,0.0) );
	return (t<-esp) ? smoothstep(0.0, 1.0, 2.5*k*d/-t ) : 1.0 ;
#else
    // cheap but not plausible alternative
    return (b>0.0) ? step(-0.0001,c) : smoothstep( 0.0, 1.0, h*k/b );
	//return (b>0.0) ?  smoothstep( 0.0, 1.0, h*k/b ): step(-0.0001,c);
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

void animateCamera( out float3 origin, out float3 target )
{
    float det = 3.1415926 / 100.;
	float an = (cTime % 200) * det;
	origin = float3( 8.0*cos(an), 0., 8.0*sin(an) );
    target = float3( 0.0, 0, 0.0 );
}

//--------------------------------------------------------------------------------------
// calculate a ray for a specified "samplePoint" (-1, 1) on image plane
//--------------------------------------------------------------------------------------
void calcRayForPixel( in float2 samplePoint, out float3 rayOri, out float3 rayDir )
{
     
	float3 ro = float3(0, 0, 8), ta = float3(0, 0, 0);
	animateCamera( ro, ta ); // camera sphere	
	
    // camera matrix
    float3 ww = normalize( ta - ro );
    float3 uu = normalize( cross(ww,float3(0.0,1.0,0.0) ) );
    float3 vv = normalize( cross(uu,ww));
	
	// create view ray
	float3 rd = normalize( samplePoint.x*uu + samplePoint.y*vv + 2.0*ww );
	
	rayOri = ro;
	rayDir = rd;
}


//--------------------------------------------------------------------------------------
// calculate the box filtered texture value by a tecture coordinate "p"
//--------------------------------------------------------------------------------------
float checkerboardBoxFiltered( in float2 p, in float2 ddx, in float2 ddy )
{
    // filter kernel
    float2 w = max(abs(ddx), abs(ddy)) + 0.01;  
	
    // analytical integral (box filter)
    float2 i = 2.0*(abs(frac((p-0.5*w)/2.0)-0.5)-abs(frac((p+0.5*w)/2.0)-0.5))/w;
	
    // xor pattern
    return 0.5 - 0.5*i.x*i.y;                  
}

//--------------------------------------------------------------------------------------
// calculate the raw texture value by a tecture coordinate "uv"
//--------------------------------------------------------------------------------------
float checkerboard( in float2 uv )
{
    float2 q = floor(uv*12.f);
    return fmod( q.x+q.y, 2.0 );    // xor pattern
}

float3 animateDirectLight()
{
	float pi = 3.141592;
    float delta = pi / 200;
	float theta = 1.75*pi;
	if (cAnimateLight == 1)
	{
		int i = int(cTime) % 200;
		theta = pi + delta * i;
	}
	float3 lig = normalize( float3(cos(theta),sin(theta),0));
	return lig;
}

float4 animateSphere()
{
	float4 sph = float4(float3(0,0.0,1.0), 1.0 );
	if (cAnimateSphere == 1)
		sph = float4(cos(cTime)*float3(0,0.0,1.0), 1.0 );
    sph.x = 1.0;   
	return sph;
}



static float4  mysphere;  // global variable
static float3  baseColor = float3(0,0,0);

//--------------------------------------------------------------------------------------
// hit test: ray(ro, rd) with primitives in scene
//         return (distance, objectID)
//--------------------------------------------------------------------------------------
float2  worldIntersect( in float3 ro, in float3 rd, in float maxlen )
{
    float tmin = 1e10;
    float2 ret = float2(tmin, -1.0);
    
    float t1 = iPlane( ro, rd );
    if( t1>0.0 )
    {
        tmin = t1;
		ret.y = 0;
    }
#if 1
    float t2 = sphIntersect( ro, rd, mysphere );
    if( t2>0.0 && t2<tmin )
    {
        tmin = t2;
		ret.y = 1;
	}
#endif 

    ret.x = tmin;
	if (tmin > maxlen) ret.y = -1.0;
	return ret;
}

//--------------------------------------------------------------------------------------
// calculate the texture coordinate at a specified "hit-point" on sphere surface
//           param "n" is the unit normal on hit point
//
//  Not Very clear the "value scope" of hlsl atan2() and acos().
//  So The UV coordiante is NOT accuarate.
//--------------------------------------------------------------------------------------
float2 calcSphereTexCoord( in float3 n)
{
    float2 uv = float2( atan2(n.z, n.x), acos(n.y ));
	return uv;
}


float3 Shading( in float3 hitpoint, in float3 lightdir, in float objectID )
{
	float3 col = float3(0,0,0);
	float3 hitnormal = float3(0,0,0);
	float  occ = 1.0; // 1.0 means no occlude
	float  texturedColor = 0;
	
	if (objectID == 0)
	{	
		hitnormal = float3(0,1,0);
		occ = 1.0-sphOcclusion( hitpoint, hitnormal, mysphere );
		texturedColor = 1.0;
	}
	else if (objectID == 1)
    {
		hitnormal = sphNormal( hitpoint, mysphere );
		occ = 0.5 + 0.5*hitnormal.y;
	    texturedColor = checkerboard(calcSphereTexCoord(hitnormal));
	}
	
    col += clamp( dot(hitnormal,-lightdir), 0.0, 1.0 ) * cLightColor * texturedColor;
    col *= sphSoftShadow( hitpoint, lightdir, mysphere, 2.0 );
    col += 0.05*occ * texturedColor; // AO
	return col;
}

//--------------------------------------------------------------------------------------
// Pixel Shader: trace sphere on a plane 
//--------------------------------------------------------------------------------------
float4 PSSphere( PS_INPUT input) : SV_Target
{
	float2 resolution = float2(cWidth,cHeight);
    float2 pixelCoord = float2(input.Pos.x, cHeight-input.Pos.y);
    float2 p = (2.0*pixelCoord-resolution.xy) / resolution.y;
    
	// "camera" setting
	float3 ro = float3(0.0, 0.0, 6.0 );
	float3 rd = normalize( float3(p,-2.0) );
	
	calcRayForPixel(p, ro, rd);
	
	mysphere = animateSphere();
	
    float3 col = float3(0,0,0);

    float2 ret = worldIntersect( ro, rd, 1e4 );
	if (ret.y >= 0)
	{
	  col = Shading( ro + ret.x*rd, animateDirectLight(), ret.y );
	  col *= exp( -0.05*ret.x );
	}
	
    col = sqrt(col);
    return float4( col, 1.0 );
}
