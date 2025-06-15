#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 vTexCoord;

out vec3 FragPos;
out vec3 worldPosition;
out vec3 normal;
out vec2 texCoord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main() 
{
    FragPos = vec3(world * vec4(aPos, 1.0));
    normal = normalize(mat3(inverse(transpose(world))) * aNormal);
    texCoord = vTexCoord;

    gl_Position = projection * view * world * vec4(aPos, 1.0);
    worldPosition = mat3(world) * aPos + vec3(-1000, 0, -1000);
}
