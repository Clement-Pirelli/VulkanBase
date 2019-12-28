#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D colorMap;
layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform sampler2D depthMap;
layout(binding = 3) uniform sampler2D positionMap;

layout(std140, binding = 4) uniform UniformBufferObject
{
	float time;						
	int dirLightAmount;
	int pointLightAmount;
	int ssaoOn;
	mat4 projection;
	vec4 cameraPosition;
	vec4 dirLightsDirections[10];
	vec4 pointLightsPositions[100];
	vec4 dirLightsColors[10];
	vec4 pointLightsColors[100];
	vec4 ssaoKernel[32];
	vec2 mouse;
	vec2 resolution;
} ubo;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

//from : https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float hash12(vec2 seed){
    return fract(sin(dot(seed, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{

	vec3 normal = texture(normalMap, inTexCoord).rgb;
	vec3 color = texture(colorMap, inTexCoord).rgb;
	vec3 position = texture(positionMap, inTexCoord).xyz;
	float depth = position.z;

	vec3 randomVec = vec3(hash12(inTexCoord), hash12(inTexCoord + vec2(12.5915, 9.2841)), .0);
	float occlusion = .0;
	if(ubo.ssaoOn == 1)
	{
		//from : https://learnopengl.com/Advanced-Lighting/SSAO
		
		vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
			vec3 bitangent = cross(normal, tangent);
			mat3 TBN       = mat3(tangent, bitangent, normal);  
			
			const int kernelSize = 32;
			const float bias = .02;
			const float radius = .5;


			for(int i = 0; i < kernelSize; ++i)
			{
				vec3 kernelSample = TBN * ubo.ssaoKernel[i].xyz; // From tangent to view-space
				kernelSample = position + kernelSample * radius;
				vec4 offset = vec4(kernelSample, 1.0);
				offset      = ubo.projection * offset;  // from view to clip-space
				offset.xyz /= offset.w;					// perspective divide
				offset.xyz  = offset.xyz * 0.5 + 0.5;	// transform to range 0.0 - 1.0  
				float sampleDepth = texture(positionMap, offset.xy).z;
				float rangeCheck = smoothstep(0.0, 1.0, radius / abs(depth - sampleDepth));
				occlusion += (sampleDepth >= kernelSample.z + bias ? 1.0 : 0.0)*rangeCheck;
			} 
			occlusion = pow(1.0 - (occlusion / float(kernelSize)),1.0);

	} else 
	{
		occlusion = 1.0;
	}
	





	vec4 endCol = vec4(.0,.0,.0,1.0);
	float ambient = .1 * occlusion;
	vec3 viewDirection = normalize(position - ubo.cameraPosition.xyz);
	
 
	for(int i = 0; i < ubo.dirLightAmount; i++)
	{
		const float intensity = ubo.dirLightsColors[i].w;
		const vec3 direction = -ubo.dirLightsDirections[i].xyz;

		const float diffuse = max(.0, dot(direction, normal));
		
		const vec3 reflection = reflect(direction, normal);
		const float specular = pow(max(dot(reflection, viewDirection),.0),64.0);
		
		const vec3 dirLightContribution = (ambient + .5*diffuse + .5*specular)*ubo.dirLightsColors[i].xyz*intensity;
		
		endCol += vec4(color*dirLightContribution,.0);
	}
 
	for(int i = 0; i < ubo.pointLightAmount;i++)
	{
		vec3 direction = ubo.pointLightsPositions[i].xyz - position;
		float distanceSquared = dot(direction, direction);
		direction = normalize(direction); //normalize the direction after taking distance

		float intensity = ubo.pointLightsColors[i].w / (.001 + distanceSquared); //inverse square law
																				 //.001 : so that there is no division by 0
		float diffuse = max(.0, dot(normal, direction));
		
		vec3 reflection = reflect(direction, normal);
		float specular = pow(max(dot(reflection, viewDirection),.0),64.0);

		vec3 pointLightContribution = (ambient + .5*diffuse + .5*specular) * ubo.pointLightsColors[i].xyz * intensity;
		
		endCol += vec4(color * pointLightContribution,.0);
	}

	outColor = clamp(endCol,.0,1.0);
	outColor = vec4(occlusion,occlusion,occlusion,1.0);
}