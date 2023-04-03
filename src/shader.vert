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
    vec4 view_pos;
    vec4 view_rot;
    vec4 ortho;
    vec4 perse;
} camera;

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_uv;
layout(location=0) out vec4 out_color;
layout(location=1) out vec2 out_uv;

mat4 my_scale(vec3 v) {
    return mat4(
        v.x, 0.0, 0.0, 0.0,
        0.0, v.y, 0.0, 0.0,
        0.0, 0.0, v.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_rotate_x(float ang) {
    float c = cos(ang);
    float s = sin(ang);
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0,   c,  -s, 0.0,
        0.0,   s,   c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_rotate_y(float ang) {
    float c = cos(ang);
    float s = sin(ang);
    return mat4(
          c, 0.0,   s, 0.0,
        0.0, 1.0, 0.0, 0.0,
         -s, 0.0,   c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_rotate_z(float ang) {
    float c = cos(ang);
    float s = sin(ang);
    return mat4(
          c,  -s, 0.0, 0.0,
          s,   c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_translate(vec3 v) {
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        v.x, v.y, v.z, 1.0
    );
}

void main() {
    vec3 view_pos_inv = -1.0 * camera.view_pos.xyz;
    vec3 view_rot_inv = -1.0 * camera.view_rot.xyz;
    mat4 m = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    m = my_scale(constant.scl.xyz) * m;
    m = my_rotate_x(constant.rot.x) * m;
    m = my_rotate_y(constant.rot.y) * m;
    m = my_rotate_z(constant.rot.z) * m;
    m = my_translate(constant.trs.xyz) * m;
    m = my_rotate_x(view_rot_inv.x) * m;
    m = my_rotate_y(view_rot_inv.y) * m;
    m = my_rotate_z(view_rot_inv.z) * m;
    m = my_translate(view_pos_inv) * m;
    vec4 new_pos = m * vec4(in_pos, 1.0);
    if (constant.param[0] == 0.0) {
        gl_Position = mat4(
            2.0 / camera.ortho.x, 0.0, 0.0, 0.0,
            0.0, 2.0 / camera.ortho.y, 0.0, 0.0,
            0.0, 0.0, 1.0 / camera.ortho.z, 0.0,
            0.0, 0.0, 0.0, 1.0
        ) * new_pos;
    } else {
        float div_tanpov = 1.0 / tan(camera.perse.x);
        float div_depth = 1.0 / (camera.perse.w - camera.perse.z);
        gl_Position = mat4(
            div_tanpov, 0.0, 0.0, 0.0,
            0.0, camera.perse.y * div_tanpov, 0.0, 0.0,
            0.0, 0.0, (camera.perse.w + camera.perse.z) * div_depth, 1.0,
            0.0, 0.0, 2.0 * camera.perse.w * camera.perse.z * div_depth, 0.0
        ) * new_pos;
    }
    out_color = vec4(1.0) * constant.col;
    out_uv = vec2(constant.uv.x + (constant.uv.z - constant.uv.x) * in_uv.x, constant.uv.y + (constant.uv.w - constant.uv.y) * in_uv.y);
}
