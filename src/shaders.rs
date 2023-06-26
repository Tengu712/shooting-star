use crate::log::*;

use std::fs::File;
use std::io::Read;

#[cfg(feature = "with-default-shaders")]
const VERTEX_SHADER: Option<&'static [u8]> = Some(include_bytes!(concat!(
    env!("OUT_DIR"),
    "/shader.vert.spv"
)));
#[cfg(feature = "with-default-shaders")]
const FRAGMENT_SHADER: Option<&'static [u8]> = Some(include_bytes!(concat!(
    env!("OUT_DIR"),
    "/shader.frag.spv"
)));
#[cfg(not(feature = "with-default-shaders"))]
const VERTEX_SHADER: Option<&'static [u8]> = None;
#[cfg(not(feature = "with-default-shaders"))]
const FRAGMENT_SHADER: Option<&'static [u8]> = None;

fn get_shader_binary(path: &str, shader: Option<&'static [u8]>) -> Vec<u8> {
    match File::open(path) {
        Ok(mut file) => {
            let mut res = Vec::new();
            file.read_to_end(&mut res)
                .unwrap_or_else(|_| ss_error(&format!("failed to read {path}.")));
            res
        }
        Err(_) => {
            if let Some(n) = shader {
                ss_info(&format!("{path} not found. default shader will be used."));
                n.to_vec()
            } else {
                ss_error(&format!("{path} and default shader not found."));
            }
        }
    }
}

pub(crate) fn get_vertex_shader_binary() -> Vec<u8> {
    get_shader_binary("shader.vert.spv", VERTEX_SHADER)
}

pub(crate) fn get_fragment_shader_binary() -> Vec<u8> {
    get_shader_binary("shader.frag.spv", FRAGMENT_SHADER)
}
