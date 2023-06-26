#version 450

layout(push_constant) uniform PushConstant {
    vec4 scl;
    vec4 rot;
    vec4 trs;
    vec4 col;
    vec4 uv;
    ivec4 param;
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

    float sr = sin(constant.rot.x);
    float sp = sin(constant.rot.y);
    float sy = sin(constant.rot.z);
    float cr = cos(constant.rot.x);
    float cp = cos(constant.rot.y);
    float cy = cos(constant.rot.z);
    pos = mat4(
        // col 1
        cp * cy,
        -cp * sy,
        sp,
        0.0,
        // col 2
        cr * sy + sr * sp * cy,
        cr * cy - sr * sp * sy,
        -sr * cp,
        0.0,
        // col 3
        sr * sy - cr * sp * cy,
        sr * cy + cr * sp * sy,
        cr * cp,
        0.0,
        // col 4
        0.0,
        0.0,
        0.0,
        1.0
    ) * pos;

    pos = vec4(
        pos.x + constant.trs.x,
        pos.y + constant.trs.y,
        pos.z + constant.trs.z,
        1.0
    );

    if (constant.param.x == 1) {
        pos = ortho * pos;
    } else {
        pos = view * pos;
        pos = perse * pos;
    }

    gl_Position = pos;
    out_uv = vec2(
        constant.uv.x + (constant.uv.z - constant.uv.x) * in_uv.x,
        constant.uv.y + (constant.uv.w - constant.uv.y) * in_uv.y
    );
    out_col = constant.col;
}
