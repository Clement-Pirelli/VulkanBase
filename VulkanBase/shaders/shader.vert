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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 outNormal;

void main() {
    gl_Position = ubo.world * vec4(inPosition, 1.0);
    fragColor = ubo.color;
    fragTexCoord = inTexCoord;
	outNormal = ubo.world * vec4(inNormal, .0);
}