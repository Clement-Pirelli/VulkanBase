#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 outTexCoord;

//from : https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
void main()
{
    outTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outTexCoord * 2.0f + -1.0f, 0.0f, 1.0f);
}