#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution

struct line
{
    vec2 start;
    vec2 end;
};
    
vec2 getDirection(line l)
{
	return normalize(l.end - l.start);
}

vec2 getPoint(line l, float dist)
{
	return l.start + getDirection(l) * dist;
}

float sdfCircle(vec2 p, vec2 o, float r)
{
	return distance(o, p) - r;
}

//from iq : https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float opSmoothUnion( float d1, float d2, float k ) 
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}

float getSdf(vec2 p)
{

	float radius = .02;
    float s = opSmoothUnion(sdfCircle(p,vec2(.0,.0), radius), sdfCircle(p, vec2(.2,.2), .05), abs(sin(30.46))*.4 );
    s = opSmoothUnion(s, sdfCircle(p, vec2(.1, -.1), .1), abs(sin(30.46))*.3 );
    return s;
}

void main()
{

    const float t = -.1;
    const int aa = 2;

    vec3 total = vec3(.0);
    for(int i = 0; i < aa; i++)
    for(int j = 0; j < aa; j++)
    {


    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = (gl_FragCoord.xy+vec2(float(i),float(j))/float(aa)-.5)/iResolution.xy - vec2(.5);
    	uv.x *= iResolution.x/iResolution.y;
    
    float s = getSdf(uv);
    vec3 col;
    if(s <= .0)
        col = vec3(.0,.0,.0);
    else
    {
    	col = vec3(1.0,1.0, 1.0);
        
        
        vec2 mouse = vec2(sin(t), cos(t))*.5;

        vec2 p = vec2(.3, .0);
        float dist = distance(uv, mouse);
        
        col /= 1. + dist*dist;
        
        line l = line(uv, mouse);
        
        float pd = .0;
        while(pd < dist)
        {
            float d = getSdf(getPoint(l, pd));
            if(d < .0001)
            {
            	col *= .21;
                break;
            }
        	pd += d;
            
        }
    }
        
    
        total += col;
    
    }
    total /= float(aa*aa);
    
    // Output to screen
    outColor = vec4(total,1.0);
}