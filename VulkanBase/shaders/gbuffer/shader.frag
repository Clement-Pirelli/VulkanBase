#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 model;
	mat4 view;
	mat4 projection;
	vec4 color;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inPosition;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

void main() {
	vec4 texCol = texture(texSampler, fragTexCoord);
    vec4 col = (texCol*ubo.color);
	
	outColor = vec4(col.rgb,1.0);
	outNormal = vec4(normalize(inNormal),1.0);
	outPosition = vec4(inPosition, 1.0);
}