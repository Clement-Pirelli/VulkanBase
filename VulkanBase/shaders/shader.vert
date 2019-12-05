#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 world;
	vec4 color;
	float time;
	vec2 mouse;
	vec2 resolution;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.world * vec4(inPosition, 1.0);
    fragColor = ubo.color;
    fragTexCoord = inTexCoord;
}