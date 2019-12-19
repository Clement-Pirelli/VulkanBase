#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 model;
	mat4 view;
	mat4 projection;
	vec4 color;
	vec2 mouse;
	vec2 resolution;
	vec4 cameraPosition;
	vec4 dirLightsDirections[10];
	vec4 pointLightsPositions[10];
	vec4 dirLightsColors[10];
	vec4 pointLightsColors[10];
	float time;						
	int dirLightAmount;
	int pointLightAmount;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor; //todo : remove this - ubo is accessible from here anyways
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inPosition;


layout(location = 0) out vec4 outColor;

void main() {
	vec4 texCol = texture(texSampler, fragTexCoord);
    vec3 col = (texCol*fragColor).xyz;
	float ambient = .1;
	vec4 endCol = vec4(.0, .0, .0, 1.0);
	vec3 norm = normalize(inNormal);
	vec3 viewDirection = normalize(inPosition - ubo.cameraPosition.xyz);
	

	for(int i = 0; i < ubo.dirLightAmount; i++)
	{
		float intensity = ubo.dirLightsColors[i].w;
		vec3 direction = -ubo.dirLightsDirections[i].xyz;

		float diffuse = max(.0, dot(direction, norm));
		
		vec3 reflection = reflect(direction, norm);
		float specular = pow(max(dot(reflection, viewDirection),.0),64.0);
		
		vec3 dirLightContribution = (ambient + diffuse + specular)*ubo.dirLightsColors[i].xyz*intensity;
		
		endCol += vec4(col*dirLightContribution,.0);
	}

	for(int i = 0; i < ubo.pointLightAmount;i++)
	{
		vec3 direction = ubo.pointLightsPositions[i].xyz - inPosition;
		float distanceSquared = dot(direction, direction);
		direction = normalize(direction); //normalize the direction after taking distance

		float intensity = ubo.pointLightsColors[i].w / (.001 + distanceSquared); //inverse square law
																				 //.001 : so that there is no division by 0
		float diffuse = max(.0, dot(direction, norm));
		
		vec3 reflection = reflect(direction, norm);
		float specular = pow(max(dot(reflection, viewDirection),.0),64.0);

		vec3 pointLightContribution = (diffuse + specular) * ubo.pointLightsColors[i].xyz * intensity;
		
		endCol += vec4(col * pointLightContribution,.0);
	}


	outColor = clamp(endCol,.0,1.0);
}