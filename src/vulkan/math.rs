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
        1.0 / width,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0 / height,
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
