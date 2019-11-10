#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 texCol = texture(texSampler, fragTexCoord);
    outColor = texCol*vec4(1.0,.0,.0,1.0);
}