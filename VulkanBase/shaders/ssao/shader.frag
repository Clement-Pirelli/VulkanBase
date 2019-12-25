#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D colorMap;
layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform sampler2D depthMap;
layout(binding = 3) uniform sampler2D positionMap;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

vec3 hash33(vec3 randseed)
{
    return abs(cos(fract(randseed * dot(sin(randseed * 242.589), vec3(42.48391588, 24.47571723, .91711988))) * 294419.4857));
}

vec3 hash32(vec2 randseed)
{
	return hash33(cross(randseed.xyx, vec3(1.4155,2.24115,1.94815)));
}

void main()
{



	//const int sampleBoxSize = 16;
	//const ivec2 resolution = ivec2(1920,1080); //TODO : uniform buffer with this
	//const vec2 pixelSize = vec2(1./float(resolution.x), 1./float(resolution.y));
	//const float bias = .0001;
	//const float depthSample = texture(depthMap, inTexCoord).r;
	//
	//float occlusion = .0;
	//
	//for(int i = -sampleBoxSize; i <= sampleBoxSize; i++)
	//for(int j = -sampleBoxSize; j <= sampleBoxSize; j++)
	//{
	//	vec2 sampleUV = inTexCoord + vec2(pixelSize.x*float(i),pixelSize.y*float(j));
	//	float sampleDepth = texture(depthMap, sampleUV).r;
	//	float rangeCheck = smoothstep(0.0, 1.0, (sampleBoxSize*sampleBoxSize*2*2) / abs(depthSample - sampleDepth));
	//	occlusion += ((sampleDepth >= depthSample + bias) ? 1.0 : 0.0)*rangeCheck; 
	//}
	//occlusion = 1.0 - (occlusion / pow(sampleBoxSize, sampleBoxSize));

	vec3 normal = texture(normalMap, inTexCoord).rgb;
	vec3 color = texture(colorMap, inTexCoord).rgb;
	vec3 position = texture(positionMap, inTexCoord).rgb;
	vec3 randomVec = hash32(inTexCoord);
	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);  



	outColor = vec4(0.0,.0,.0, 1.0);
}