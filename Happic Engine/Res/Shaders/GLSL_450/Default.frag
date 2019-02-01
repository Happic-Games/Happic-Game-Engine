#version 450

out vec4 outColor;

in vec2 texCoord0;

layout(set = 1, binding = 0) uniform sampler2D Albedo;

void main() 
{
    outColor = texture(Albedo, texCoord0);
}