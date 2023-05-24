#version 450

layout(push_constant) uniform PushConstant {
    vec4 scl;
    vec4 rot;
    vec4 trs;
    vec4 col;
    vec4 uv;
    int param;
} constant;

layout(binding=0) uniform UniformBuffer {
    mat4 view;
    mat4 perse;
    mat4 ortho;
};

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_uv;

layout(location=0) out vec2 out_uv;
layout(location=1) out vec4 out_col;

void main() {
    vec4 pos = vec4(in_pos, 1.0);

    pos = vec4(
        pos.x * constant.scl.x,
        pos.y * constant.scl.y,
        pos.z * constant.scl.z,
        1.0
    );

    float s = sin(constant.rot.x);
    float c = cos(constant.rot.x);
    pos = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0,   c,  -s, 0.0,
        0.0,   s,   c, 0.0,
        0.0, 0.0, 0.0, 1.0
    ) * pos;

    s = sin(constant.rot.y);
    c = cos(constant.rot.y);
    pos = mat4(
          c, 0.0,   s, 0.0,
        0.0, 1.0, 0.0, 0.0,
         -s, 0.0,   c, 0.0,
        0.0, 0.0, 0.0, 1.0
    ) * pos;

    s = sin(constant.rot.y);
    c = cos(constant.rot.y);
    pos = mat4(
          c,  -s, 0.0, 0.0,
          s,   c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    ) * pos;

    pos = vec4(
        pos.x + constant.trs.x,
        pos.y + constant.trs.y,
        pos.z + constant.trs.z,
        1.0
    );

    pos = view * pos;
    pos = perse * pos;

    gl_Position = pos;
    out_uv = in_uv;
    out_col = constant.col;
}
