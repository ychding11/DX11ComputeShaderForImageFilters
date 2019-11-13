// VS VSCanvas
// PS PSMandelbrot


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
	float cFrequency;
	float cAltitude;
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


// This shader computes the distance to the Mandelbrot Set for everypixel, and colorizes
// it accoringly.
// 
// Z -> Z²+c, Z0 = 0. 
// therefore Z' -> 2·Z·Z' + 1
//
// The Hubbard-Douady potential G(c) is G(c) = log Z/2^n
// G'(c) = Z'/Z/2^n
//
// So the distance is |G(c)|/|G'(c)| = |Z|·log|Z|/|Z'|
//
// More info here: http://www.iquilezles.org/www/articles/distancefractals/distancefractals.htm


float4 PSMandelbrot( PS_INPUT input) : SV_Target
{
    float2 resolution = float2(cWidth,cHeight);
	input.Pos.y = cHeight- input.Pos.y;
	float2 p = input.Pos.xy /resolution; // convert pixel coordinate to [0,1]
	p = 2 * p - 1.0;
    p.x *= resolution.x/resolution.y;
	
	
    // animation	
	float tz = cAltitude *(1.0 - cos(cFrequency*cTime)); // can be controled
	//tz = 0;
    float zoo = pow( 0.5, tz );
	float2 c = p*zoo + float2(-0.5,.618); //should be control

    // iterate
    int   inSet =  1;
	float m2 = 0.0;
    float2 z  = float2(0,0);
    float2 dz = float2(0,0);
    for( int i=0; i<400; i++ )
    {
        if( m2>100.0 ) { inSet=0; break; }

		// Z' -> 2·Z·Z' + 1
        dz = 2.0*float2(z.x*dz.x-z.y*dz.y, z.x*dz.y + z.y*dz.x) + float2(1.0,0.0);
			
        // Z -> Z² + c			
        z = float2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
			
        m2 = dot(z,z);
    }

    // distance	
	// d(c) = |Z|·log|Z|/|Z'|
	float d = 0.5*sqrt(dot(z,z)/dot(dz,dz))*log(dot(z,z));
    if( inSet==1 ) d=0.0;
	
    // do some soft coloring based on distance
	d = clamp( pow(4.0*d/zoo,0.2), 0.0, 1.0 );

    float3 col = float3( d,d,d );
    
    return float4( col, 1.0 );
}
