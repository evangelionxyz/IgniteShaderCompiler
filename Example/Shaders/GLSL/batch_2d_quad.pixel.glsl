#version 450 core
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in VertexOut
{
    vec2 texCoord;
    vec2 tilingFactor;
    vec4 color;
    flat uint texIndex;
} vertex_in;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler2D textures[];

void main()
{
    vec4 texColor = texture(textures[nonuniformEXT(vertex_in.texIndex)], vertex_in.texCoord * vertex_in.tilingFactor);
    vec4 finalColor = vertex_in.color * texColor;

    if (finalColor.a == 0.0)
    {
        discard;
    }

    out_color = finalColor;
}
