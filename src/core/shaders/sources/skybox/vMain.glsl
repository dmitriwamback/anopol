#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
uniform vec3 position;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos + position, 1.0);
}