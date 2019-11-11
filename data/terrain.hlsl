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



//------------------------------------------------------------------------------------------
// global
//------------------------------------------------------------------------------------------

const float3  kSunDir = float3(-0.624695,0.468521,-0.624695); // actually it is a reverse direction.
const float   kMaxTreeHeight = 2.0;

float3 fog( in float3 col, float t )
{
    float3 fogCol = float3(0.4,0.6,1.15); //hand-coded
    return lerp( col, fogCol, 1.0-exp(-0.000001*t*t) );
}

const mat2 m2 = mat2(  0.80,  0.60,
                      -0.60,  0.80 );

float hash1( float2 p )
{
    p  = 50.0*frac( p*0.3183099 );
    return frac( p.x*p.y*(p.x+p.y) );
}

float hash1( float n )
{
    return frac( n*17.0*frac( n*0.3183099 ) );
}

float2 hash2( float n ) { return frac(sin(float2(n,n+1.0))*float2(43758.5453123,22578.1459123)); }


float2 hash2( float2 p ) 
{
    const float2 k = float2( 0.3183099, 0.3678794 );
    p = p*k + k.yx;
    return frac( 16.0 * k*frac( p.x*p.y*(p.x+p.y)) );
}
				  
float noise( in float2 x )
{
    float2 p = floor(x);
    float2 w = frac(x);
    float2 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    
    float a = hash1(p+float2(0,0));
    float b = hash1(p+float2(1,0));
    float c = hash1(p+float2(0,1));
    float d = hash1(p+float2(1,1));
    
    return -1.0+2.0*( a + (b-a)*u.x + (c-a)*u.y + (a - b - c + d)*u.x*u.y );
}

float fbm_9( in float2 x )
{
    float f = 1.9;
    float s = 0.55;
    float a = 0.0;
    float b = 0.5;
    for( int i=0; i<9; i++ )
    {
        float n = noise(x);
        a += b*n;
        b *= s;
        x = f*m2*x;
    }
	return a;
}

//------------------------------------------------------------------------------------------
// terrain
//------------------------------------------------------------------------------------------

float2 terrainMap( in float2 p )
{
    const float sca = 0.0010;
    const float amp = 300.0;

    p *= sca;
    float e = fbm_9( p + float2(1.0,-2.0)); // terrain height
    float a = 1.0-smoothstep( 0.12, 0.13, abs(e+0.12) ); // flag high-slope areas (-0.25, 0.0)
    e = e + 0.15*smoothstep( -0.08, -0.01, e );
    e *= amp;
    return float2(e,a);
}

vec4 terrainMapD( in vec2 p )
{
	const float sca = 0.0010;
    const float amp = 300.0;
    p *= sca;
    vec3 e = fbmd_9( p + vec2(1.0,-2.0) );
    vec2 c = smoothstepd( -0.08, -0.01, e.x );
	e.x = e.x + 0.15*c.x;
	e.yz = e.yz + 0.15*c.y*e.yz;    
    e.x *= amp;
    e.yz *= amp*sca;
    return vec4( e.x, normalize( vec3(-e.y,1.0,-e.z) ) );
}

vec3 terrainNormal( in vec2 pos )
{
#if 1
    return terrainMapD(pos).yzw;
#else    
    vec2 e = vec2(0.03,0.0);
	return normalize( vec3(terrainMap(pos-e.xy).x - terrainMap(pos+e.xy).x,
                           2.0*e.x,
                           terrainMap(pos-e.yx).x - terrainMap(pos+e.yx).x ) );
#endif    
}

float terrainShadow( in vec3 ro, in vec3 rd, in float mint )
{
    float res = 1.0;
    float t = mint;
#ifdef LOWQUALITY
    for( int i=ZERO; i<32; i++ )
    {
        vec3  pos = ro + t*rd;
        vec2  env = terrainMap( pos.xz );
        float hei = pos.y - env.x;
        res = min( res, 32.0*hei/t );
        if( res<0.0001 ) break;
        t += clamp( hei, 1.0+t*0.1, 50.0 );
    }
#else
    for( int i=ZERO; i<128; i++ )
    {
        vec3  pos = ro + t*rd;
        vec2  env = terrainMap( pos.xz );
        float hei = pos.y - env.x;
        res = min( res, 32.0*hei/t );
        if( res<0.0001 ) break;
        t += clamp( hei, 0.5+t*0.05, 25.0 );
    }
#endif
    return clamp( res, 0.0, 1.0 );
}

float raymarchTerrain( in float3 ro, in vec3 rd, float tmin, float tmax )
{
    float dis, th;
    float t = tmin; 
    float ot = t;
    float odis = 0.0;

    for( int i=0; i<400; i++ )
    {
        th = 0.001*t;

        vec3  pos = ro + t*rd;
        vec2  env = terrainMap( pos.xz );
        
        // terrain
        dis = pos.y - env.x;
        if( dis<th ) break; // below isosurface means find it.
        
        ot = t;
        odis = dis;
        t += dis*0.8*(1.0-0.75*env.y); // slow down in step areas
        if( t>tmax ) break;
    }

    if( t>tmax ) t = -1.0; // missed.
    else t = ot + (th-odis)*(t-ot)/(dis-odis); // linear interpolation for better accuracy
    return t;
}

vec4 renderTerrain( in vec3 ro, in vec3 rd, in vec2 tmima, out float teShadow, out vec2 teDistance, inout float resT )
{
    vec4 res = vec4(0.0);
    teShadow = 0.0;
    teDistance = vec2(0.0);
    
    vec2 t = raymarchTerrain( ro, rd, tmima.x, tmima.y );
    if( t.x>0.0 )
    {
        vec3 pos = ro + t.x*rd;
        vec3 nor = terrainNormal( pos.xz );

        // bump map
        nor = normalize( nor + 0.8*(1.0-abs(nor.y))*0.8*fbmd_8( pos*0.3*vec3(1.0,0.2,1.0) ).yzw );
        
        vec3 col = vec3(0.18,0.11,0.10)*.75;
        col = 1.0*mix( col, vec3(0.1,0.1,0.0)*0.3, smoothstep(0.7,0.9,nor.y) );      
        
		//col *= 1.0 + 2.0*fbm( pos*0.2*vec3(1.0,4.0,1.0) );
        
        float sha = 0.0;
        float dif =  clamp( dot( nor, kSunDir), 0.0, 1.0 ); 
        if( dif>0.0001 ) 
        {
            sha = terrainShadow( pos+nor*0.01, kSunDir, 0.01 );
            //if( sha>0.0001 ) sha *= cloudsShadow( pos+nor*0.01, kSunDir, 0.01, 1000.0 );
            dif *= sha;
        }
        vec3  ref = reflect(rd,nor);
    	float bac = clamp( dot(normalize(vec3(-kSunDir.x,0.0,-kSunDir.z)),nor), 0.0, 1.0 )*clamp( (pos.y+100.0)/100.0, 0.0,1.0);
        float dom = clamp( 0.5 + 0.5*nor.y, 0.0, 1.0 );
        vec3  lin  = 1.0*0.2*mix(0.1*vec3(0.1,0.2,0.0),vec3(0.7,0.9,1.0),dom);//pow(vec3(occ),vec3(1.5,0.7,0.5));
              lin += 1.0*5.0*vec3(1.0,0.9,0.8)*dif;        
              lin += 1.0*0.35*vec3(1.0)*bac;
        
	    col *= lin;

        col = fog(col,t.x);

        teShadow = sha;
        teDistance = t;
        res = vec4( col, 1.0 );
        resT = t.x;
    }
    
    return res;
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
	
    // sphere animation
	float4 sph = float4(float3(0,0.0,1.0), 1.0 );
	if (cAnimateSphere == 1)
		sph = float4(cos(cTime)*float3(0,0.0,1.0), 1.0 );
    sph.x = 1.0;   
	
	// directional light setting
	float pi = 3.141592;
    float delta = pi / 99999;
	float theta = 1.75*pi;
	if (cAnimateLight == 1)
	{
		int i = int(cTime) % 99998;
		theta = pi + delta * i;
	}
	float3 lig = normalize( float3(cos(theta),sin(theta),0));
	
    float3 col = float3(0,0,0);

    float tmin = 1e10;
    float3 nor = float3(0,1,0);
    float  occ = 1.0; // 1.0 means no occlude
    
    float t1 = iPlane( ro, rd );
    if( t1>0.0 )
    {
        tmin = t1;
        float3 pos = ro + t1*rd;
        float3 nor = float3(0,1,0);
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

    if( tmin<1e4 ) // Not infinity
    {
        float3 pos = ro + tmin*rd;
        
		col = cLightColor; //float3(1.0,1.0,1.0); //light intensity.
        col *= clamp( dot(nor,-lig), 0.0, 1.0 );
        col *= sphSoftShadow( pos, lig, sph, 2.0 );
        col += 0.05*occ; // AO
	    col *= exp( -0.05*tmin );
    }

    col = sqrt(col);
    return float4( col, 1.0 );
}
