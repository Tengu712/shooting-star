/// A function to create a camera view matrix.
///
/// - `pos` - camera position `[x, y, z]`
/// - `rot` - camera rotation `[x, y, z]` (rad)
pub fn create_view(pos: [f32; 3], rot: [f32; 3]) -> [f32; 16] {
    let sr = f32::sin(-rot[0]);
    let sp = f32::sin(-rot[1]);
    let sy = f32::sin(-rot[2]);
    let cr = f32::cos(-rot[0]);
    let cp = f32::cos(-rot[1]);
    let cy = f32::cos(-rot[2]);
    [
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
        -pos[0],
        -pos[1],
        -pos[2],
        1.0,
    ]
}

/// A function to create a persepective projection matrix.
pub fn create_perse(pov: f32, aspect: f32, near: f32, far: f32) -> [f32; 16] {
    let div_tanpov = 1.0 / f32::tan(std::f32::consts::PI * pov / 180.0);
    let div_depth = 1.0 / (far - near);
    [
        div_tanpov,
        0.0,
        0.0,
        0.0,
        0.0,
        div_tanpov * aspect,
        0.0,
        0.0,
        0.0,
        0.0,
        far * div_depth,
        1.0,
        0.0,
        0.0,
        -far * near * div_depth,
        0.0,
    ]
}

/// A function to create a orthographic projection matrix.
pub fn create_ortho(width: f32, height: f32, depth: f32) -> [f32; 16] {
    [
        2.0 / width,
        0.0,
        0.0,
        0.0,
        0.0,
        2.0 / height,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0 / depth,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0,
    ]
}
