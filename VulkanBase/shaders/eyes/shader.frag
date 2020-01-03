#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution


float pseudorandScalar(vec2 randseed)
{
	return fract(dot(vec2(sin(randseed*194.0154)), vec2(1043.94219, 744.2918)));
}

bool inCircle(vec2 uv, vec2 co, float r)
{
	return (distance(uv, co) <= r);
}

mat2 rot(float angle)
{
	return mat2(cos(angle), -sin(angle), cos(angle), -sin(angle));
}


const int aa = 2;
void main()
{
    vec3 total = vec3(.0);
    for(int i = 0; i < aa; i++)
    for(int j = 0; j < aa; j++)
    {
            
    	vec2 uv = (gl_FragCoord.xy+vec2(float(i),float(j))/float(aa)-.5)/iResolution.xy - vec2(.5);
    	uv.x *= iResolution.x/iResolution.y;
    	
    	uv*= 15.0;
    	vec2 oruv = uv;
    	uv = fract(uv)-.5;
    	
    	vec3 col = vec3(35., 18., 56.)/255.;
    	
    	float r = (.99)*.2;
    	float eyeY = .0;
    	
    	float randAngle = pseudorandScalar(floor(oruv))*360.;
    	mat2 rotation = rot(randAngle);   
    	
    	if(inCircle(uv, rotation*vec2(.0,.1), r) && 
    	    inCircle(uv, rotation*vec2(.0,-.1), r) && 
    	    !inCircle(uv,rotation*vec2(.0,eyeY),.08)
    	  )
    	{
    	    col = vec3(245.,244.,237.)/255.;
    	}
        
        total += col;
    
    }
    total /= float(aa*aa);
    // Output to screen
    outColor = vec4(total,1.0);
}