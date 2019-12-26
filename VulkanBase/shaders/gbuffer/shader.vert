#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 model;
	mat4 view;
	mat4 projection;							
	vec4 color;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPosition;

void main() {
	mat4 world = ubo.projection * ubo.view * ubo.model;
    gl_Position = world * vec4(inPosition, 1.0);
	outPosition = (ubo.model * vec4(inPosition,1.0)).xyz;
    fragTexCoord = inTexCoord;
	outNormal = (transpose(inverse(ubo.model)) * vec4(inNormal,.0)).xyz;
}