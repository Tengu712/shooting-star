#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 scl;
    mat4 rtx;
    mat4 rty;
    mat4 rtz;
    mat4 trs;
    mat4 view;
    mat4 proj;
    vec4 uv;
    vec4 opt;
} ubo;

layout(location=0) in vec3 in_pos;
layout(location=0) out vec4 out_color;

void main() {
    gl_Position = ubo.proj
        * ubo.view
        * ubo.trs
        * ubo.rtz
        * ubo.rty
        * ubo.rtx
        * ubo.scl
        * vec4(in_pos, 1.0);
    out_color = vec4(1.0);
}
