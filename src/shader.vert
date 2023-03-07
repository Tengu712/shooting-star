#version 450

layout(push_constant) uniform PushConstant {
    vec4 scl;
    vec4 rot;
    vec4 trs;
    vec4 col;
    vec4 uv;
    vec4 param;
} constant;

layout(binding = 0) uniform CameraUniformData {
    vec3 view_pos;
    vec3 view_rot;
    int is_perse;
    vec4 proj;
} camera;

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_uv;
layout(location=0) out vec4 out_color;
layout(location=1) out vec2 out_uv;

void main() {
    vec3 scled = constant.scl.xyz * in_pos;
    // TODO: rotate
    vec3 trsed = constant.trs.xyz + scled;
    // TODO: view_rot and proj
    vec3 view_pos_inv = -1.0f * camera.view_pos;
    vec3 view_trsed = view_pos_inv + trsed;
    gl_Position = vec4(view_trsed, 1.0);
    out_color = vec4(1.0) * constant.col;
    out_uv = in_uv; // TODO: use constant.uv
}
