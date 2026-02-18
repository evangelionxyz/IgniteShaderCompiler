#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec2 in_tilingFactor;
layout(location = 3) in vec4 in_color;
layout(location = 4) in uint in_texIndex;

layout(set = 0, binding = 0) uniform CameraBuffer
{
    mat4 projection;
    mat4 view;
    vec4 position;
} camera;

layout(location = 0) out VertexOut
{
    vec2 texCoord;
    vec2 tilingFactor;
    vec4 color;
    flat uint texIndex;
} vertex_out;

void main()
{
    vec4 pos = vec4(in_position, 1.0);
    gl_Position = camera.projection * camera.view * pos;

    vertex_out.texCoord = in_texCoord;
    vertex_out.tilingFactor = in_tilingFactor;
    vertex_out.color = in_color;
    vertex_out.texIndex = in_texIndex;
}
