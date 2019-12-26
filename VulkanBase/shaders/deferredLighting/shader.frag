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
	vec4 cameraPosition;
	vec4 dirLightsDirections[10];
	vec4 pointLightsPositions[100];
	vec4 dirLightsColors[10];
	vec4 pointLightsColors[100];
	vec2 mouse;
	vec2 resolution;
} ubo;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{

	vec3 normal = texture(normalMap, inTexCoord).rgb;
	vec3 color = texture(colorMap, inTexCoord).rgb;
	vec3 position = texture(positionMap, inTexCoord).xyz;

	vec4 endCol = vec4(.0,.0,.0,1.0);
	float ambient = .1;
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

		vec3 pointLightContribution = (ambient + diffuse + specular) * ubo.pointLightsColors[i].xyz * intensity;
		
		endCol += vec4(color * pointLightContribution,.0);
	}
	outColor = clamp(endCol,.0,1.0);
}