#version 450

layout(binding=1) uniform sampler2D diffuse_map;

layout(location=0) in vec2 in_uv;
layout(location=1) in vec4 in_col;

layout(location=0) out vec4 out_col;

void main() {
    out_col = texture(diffuse_map, in_uv) * in_col;
}
