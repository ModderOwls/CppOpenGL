#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

out vec2 texCoord;
out vec3 worldPosition;

uniform mat4 world, view, projection;

uniform sampler2D texture1;

void main()
{
//     vec3 pos = aPos;
//     Object space offset.

//     vec4 worldPos = world * vec4(aPos, 1.0);
//     World position offsets.
//     worldPos.y += texture(texture1, vTexCoord).r * 100.0f;

    gl_Position = projection * view * world * vec4(aPos, 1.0);
    texCoord = vTexCoord;

    worldPosition = mat3(world) * aPos;
}