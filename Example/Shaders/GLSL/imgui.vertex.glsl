#version 450 core

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0)out VertexData
{
    vec4 position;
    vec4 color;
    vec2 uv;
}vertex_out;

layout(push_constant) uniform PushConstants
{
    vec2 inv_display_size;
};

void main()
{
    vec4 out_position = vec4(
        vec2(in_position.xy * inv_display_size * vec2(2.0, -2.0) + vec2(-1.0, 1.0)),
        0, 1);

    vertex_out.position = out_position;
    vertex_out.uv = in_uv;
    vertex_out.color = in_color;

    gl_Position = out_position;
}

