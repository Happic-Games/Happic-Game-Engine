#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

layout(std140) uniform PerDrawInstance
{
	mat4 mvp;
} perDrawInstance;

out vec2 texCoord0;

void main() 
{
	texCoord0 = texCoord;
    gl_Position = perDrawInstance.mvp * vec4(pos, 1.0);
}

