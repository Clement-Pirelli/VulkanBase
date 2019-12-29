#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution

const float CLOSE_ENOUGH = 0.001;
const vec3 COLOR = vec3(1.0,.0,.0);
const vec3 BG_COLOR = vec3(.15,.0,.0);
const vec3 SHADOW_COLOR = clamp((COLOR+vec3(.0,.1,.1))/4.0, .0, 1.0);

struct sphere
{
	vec3 c;
    float r;
};


float sphereDist(vec3 p, sphere s)
{
	return distance(p, s.c) - s.r;
}

//from iq : https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float opSmoothUnion( float d1, float d2, float k ) 
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}

float sdf(vec3 p)
{
    float returnVal = .0;
    vec3 rep = vec3(1.0)*(1. + abs(sin(0.41*.11) * 10.0));
	vec3 pr = mod(p, rep) - .5 * rep; 
    
    sphere s1 = sphere(vec3(.0, .1, .0), .07);
    sphere s2 = sphere(vec3(sin(0.41) * .1, cos(0.41)*.2, .1), .1);
    sphere s3 = sphere(vec3(.1,.1,.1), .04);
    returnVal = opSmoothUnion(sphereDist(pr, s1), sphereDist(pr,s2), .2);
    returnVal = opSmoothUnion(returnVal, sphereDist(pr, s3), .15);
	return returnVal; 
}

//from iq https://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcNormal( in vec3 p )
{
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy*sdf( p + k.xyy*CLOSE_ENOUGH ) + 
                      k.yyx*sdf( p + k.yyx*CLOSE_ENOUGH ) + 
                      k.yxy*sdf( p + k.yxy*CLOSE_ENOUGH ) + 
                      k.xxx*sdf( p + k.xxx*CLOSE_ENOUGH ) );
}

const int aa = 2;
void main()
{
    vec3 total = vec3(.0);
    for(int i = 0; i < aa; i++)
    for(int j = 0; j < aa; j++)
    {
       
    // Normalized pixel coordinates (from 0 to 1)
	vec2 uv = (gl_FragCoord.xy+vec2(float(i),float(j))/float(aa)-.5)/iResolution.xy - vec2(.5);
    	uv.x *= iResolution.x/iResolution.y;
	vec3 cameraPos = vec3(.0,.0,-4.);
    
    // Time varying pixel color
    vec3 ro = vec3(cameraPos.x + uv.x, cameraPos.y + uv.y, cameraPos.z);
	vec3 rd = normalize(vec3(uv.x, uv.y, 1.0));
    vec3 col = BG_COLOR;
    float dist = .0;
    
    vec3 lightDir = normalize(vec3(1.0, 0.0, -1.0));
    float renderDistance = 300.0 + abs(sin(0.41*5.0))*300.0;
    while(dist < renderDistance)
    {
        float d = sdf(ro + rd * dist);
        if(d < CLOSE_ENOUGH)
        {
            col = COLOR;
            vec3 hit = ro + rd * dist;
    		col *= mix(SHADOW_COLOR, col, max(dot(calcNormal(hit), lightDir), .0));
        	break;
        }
       	dist += d;
    }
    total += col;
    
    }
    total /= float(aa*aa);
    // Output to screen
    
    // Output to screen
    outColor = vec4(total, 1.0f);
}