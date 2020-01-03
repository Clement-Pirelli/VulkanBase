#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution

#define YELLOW vec3(250., 236., 155.)/255.

vec2 pseudorandPos(vec2 randseed)
{
    return abs(cos(fract(randseed * dot(sin(randseed * 242.589), vec2(42.4391588, 3.91711988))) * 294419.4857));
}

vec2 getGradient(vec2 coord)
{
	return 2. * normalize(pseudorandPos(floor(coord))) - 1.0;
}

//from : https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds%20/perlin-noise-part-2?url=procedural-generation-virtual-worlds%20/perlin-noise-part-2
float smoothFloat(float t)
{
	return t * t * (3. - 2. * t);
}

//from : https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds%20/perlin-noise-part-2?url=procedural-generation-virtual-worlds%20/perlin-noise-part-2
float getPerlin(vec2 coord)
{
	int xi0 = int(floor(coord.x)); 
 	int yi0 = int(floor(coord.y)); 
 	int xi1 = xi0 + 1; 
 	int yi1 = yi0 + 1;
 	float tx = coord.x - floor(coord.x); 
 	float ty = coord.y - floor(coord.y); 	
 	float u = smoothFloat(tx);
 	float v = smoothFloat(ty);
 	// gradients at the corner of the cell
 	vec2 c00 = getGradient(vec2(xi0, yi0));
 	vec2 c10 = getGradient(vec2(xi1, yi0)); 
 	vec2 c01 = getGradient(vec2(xi0, yi1)); 
 	vec2 c11 = getGradient(vec2(xi1, yi1));
 	// generate vectors going from the grid points to p
 	float x0 = tx, x1 = tx - 1.;
 	float y0 = ty, y1 = ty - 1.;
 	vec2 p00 = vec2(x0, y0); 
 	vec2 p10 = vec2(x1, y0); 
 	vec2 p01 = vec2(x0, y1); 
 	vec2 p11 = vec2(x1, y1);	
 	// linear interpolation
 	float a = mix(dot(c00, p00), dot(c10, p10), u); 
 	float b = mix(dot(c01, p01), dot(c11, p11), u);
 	return abs(mix(a, b, v)); // g 
}

void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = gl_FragCoord.xy/iResolution.xy - .5;
    uv.x *= iResolution.x/iResolution.y;   
    uv *= 100.0;
    
    vec2 st = (uv+141.15) * .8;
    
    
    float perlin1 = getPerlin(uv);
    float perlin2 = getPerlin(st);
    float perlin = mix(perlin1, perlin2, .5);
    
    
    // Time varying pixel color
    vec3 col = (YELLOW * (1.0f-perlin));

    // Output to screen
    outColor = vec4(col,1.0);
}