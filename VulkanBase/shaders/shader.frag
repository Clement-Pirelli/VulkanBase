#version 450
#extension GL_ARB_separate_shader_objects : enable

struct DirectionalLight
{
	vec4 direction;
	vec4 color;
	float intensity;
};

struct PointLight
{
	vec4 position;
	vec4 color;
	float intensity;
};


layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 world;
	mat4 camera;
	vec4 color;
	vec2 mouse;
	vec2 resolution;
	vec4 cameraPosition;
	DirectionalLight dirLights[10];
	PointLight pointLights[10];
	int dirLightAmount;
	int pointLightAmount;
	float time;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 inNormal;


layout(location = 0) out vec4 outColor;

void main() {
	vec4 texCol = texture(texSampler, fragTexCoord);
    vec3 col = (texCol*fragColor).xyz;
	vec4 endCol = vec4(.0,.0,.0,1.0);

	for(int i = 0; i < ubo.dirLightAmount; i++)
	{
		if(i > 10) discard;
		endCol += vec4(1.0,1.0,1.0,1.0);//vec4(col * max(.0, dot(-ubo.dirLights[i].direction, inNormal.xyz))*ubo.dirLights[i].color*ubo.dirLights[i].intensity, .0);
	}

	for(int i = 0; i < ubo.pointLightAmount;i++)
	{
		if(i > 10) discard;
		//vec3 direction = (ubo.pointLights[i].position-gl_FragCoord.xyz);
		//float distanceSquared = dot(direction, direction);
		//direction = normalize(direction);
		//endCol += vec4(col * max(.0, dot(direction, inNormal.xyz)) * ubo.pointLights[i].color / distanceSquared,.0);
	}
	outColor = endCol;
}