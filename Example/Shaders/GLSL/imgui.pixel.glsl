#version 450 core

layout(location = 0) out vec4 out_color;

struct VertexData
{
    vec4 position;
    vec4 color;
    vec2 uv;
};

layout(location = 0) in VertexData vertex_in;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    out_color = texture(tex, vertex_in.uv) * vertex_in.color;
}