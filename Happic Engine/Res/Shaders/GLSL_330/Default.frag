#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 texCoord0;

uniform sampler2D Albedo;

void main() 
{
    outColor = texture(Albedo, texCoord0);
}