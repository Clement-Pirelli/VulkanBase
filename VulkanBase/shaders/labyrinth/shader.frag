#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution

#define r(a) fract(dot(vec2(sin(a*194.154)),vec2(104.919,74.218)))
void main(){
vec2 i=iResolution;
vec2 c=9.*(gl_FragCoord.xy-.5*i.xy)/i.y;
vec2 n=floor(c)*(.5);
c=fract(c)-.5;
c.x*=r(n+1.)>.5?-1.:1.;
c.y*=r(n)>.5?-1.:1.;
outColor=vec4(abs(c.x+c.y)<.01?.0:1.);
}