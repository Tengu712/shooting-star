#version 450

layout(location=0) in vec4 in_color;
layout(location=1) in vec2 in_uv;
layout(location=0) out vec4 out_color;

layout(binding=1) uniform sampler2D diffuse_map;

void main() {
    out_color = in_color * texture(diffuse_map, in_uv);
}
