#![allow(non_upper_case_globals)]
#![allow(non_snake_case)]
#![allow(dead_code)]

use super::*;

use std::os::raw::c_char;

pub const PCHAR_VK_LAYER_KHRONOS_validation: *const c_char = [
    'V' as c_char,
    'K' as c_char,
    '_' as c_char,
    'L' as c_char,
    'A' as c_char,
    'Y' as c_char,
    'E' as c_char,
    'R' as c_char,
    '_' as c_char,
    'K' as c_char,
    'H' as c_char,
    'R' as c_char,
    'O' as c_char,
    'N' as c_char,
    'O' as c_char,
    'S' as c_char,
    '_' as c_char,
    'v' as c_char,
    'a' as c_char,
    'l' as c_char,
    'i' as c_char,
    'd' as c_char,
    'a' as c_char,
    't' as c_char,
    'i' as c_char,
    'o' as c_char,
    'n' as c_char,
    '\0' as c_char,
]
.as_ptr();

pub const PCHAR_VK_EXT_debug_report: *const c_char = [
    'V' as c_char,
    'K' as c_char,
    '_' as c_char,
    'E' as c_char,
    'X' as c_char,
    'T' as c_char,
    '_' as c_char,
    'd' as c_char,
    'e' as c_char,
    'b' as c_char,
    'u' as c_char,
    'g' as c_char,
    '_' as c_char,
    'r' as c_char,
    'e' as c_char,
    'p' as c_char,
    'o' as c_char,
    'r' as c_char,
    't' as c_char,
    '\0' as c_char,
]
.as_ptr();

pub const PCHAR_VK_EXT_debug_utils: *const c_char = [
    'V' as c_char,
    'K' as c_char,
    '_' as c_char,
    'E' as c_char,
    'X' as c_char,
    'T' as c_char,
    '_' as c_char,
    'd' as c_char,
    'e' as c_char,
    'b' as c_char,
    'u' as c_char,
    'g' as c_char,
    '_' as c_char,
    'u' as c_char,
    't' as c_char,
    'i' as c_char,
    'l' as c_char,
    's' as c_char,
    '\0' as c_char,
]
.as_ptr();

pub const PCHAR_VK_KHR_surface: *const c_char = [
    'V' as c_char,
    'K' as c_char,
    '_' as c_char,
    'K' as c_char,
    'H' as c_char,
    'R' as c_char,
    '_' as c_char,
    's' as c_char,
    'u' as c_char,
    'r' as c_char,
    'f' as c_char,
    'a' as c_char,
    'c' as c_char,
    'e' as c_char,
    '\0' as c_char,
]
.as_ptr();

pub const PCHAR_VK_KHR_xlib_surface: *const c_char = [
    'V' as c_char,
    'K' as c_char,
    '_' as c_char,
    'K' as c_char,
    'H' as c_char,
    'R' as c_char,
    '_' as c_char,
    'x' as c_char,
    'l' as c_char,
    'i' as c_char,
    'b' as c_char,
    '_' as c_char,
    's' as c_char,
    'u' as c_char,
    'r' as c_char,
    'f' as c_char,
    'a' as c_char,
    'c' as c_char,
    'e' as c_char,
    '\0' as c_char,
]
.as_ptr();

pub const PCHAR_VK_KHR_win32_surface: *const c_char = [
    'V' as c_char,
    'K' as c_char,
    '_' as c_char,
    'K' as c_char,
    'H' as c_char,
    'R' as c_char,
    '_' as c_char,
    'w' as c_char,
    'i' as c_char,
    'n' as c_char,
    '3' as c_char,
    '2' as c_char,
    '_' as c_char,
    's' as c_char,
    'u' as c_char,
    'r' as c_char,
    'f' as c_char,
    'a' as c_char,
    'c' as c_char,
    'e' as c_char,
    '\0' as c_char,
]
.as_ptr();

pub const PCHAR_VK_KHR_swapchain: *const c_char = [
    'V' as c_char,
    'K' as c_char,
    '_' as c_char,
    'K' as c_char,
    'H' as c_char,
    'R' as c_char,
    '_' as c_char,
    's' as c_char,
    'w' as c_char,
    'a' as c_char,
    'p' as c_char,
    'c' as c_char,
    'h' as c_char,
    'a' as c_char,
    'i' as c_char,
    'n' as c_char,
    '\0' as c_char,
]
.as_ptr();

#[inline(always)]
pub fn VK_MAKE_VERSION(major: u32, minor: u32, patch: u32) -> u32 {
    (major << 22) | (minor << 12) | patch
}

#[inline(always)]
pub fn VK_MAKE_API_VERSION(variant: u32, major: u32, minor: u32, patch: u32) -> u32 {
    (variant << 29) | (major << 22) | (minor << 12) | patch
}

impl Default for VkPhysicalDeviceMemoryProperties {
    fn default() -> Self {
        let mem_type = VkMemoryType {
            propertyFlags: 0,
            heapIndex: 0,
        };
        let mem_heap = VkMemoryHeap { size: 0, flags: 0 };
        Self {
            memoryTypeCount: 0,
            memoryTypes: [mem_type; 32],
            memoryHeapCount: 0,
            memoryHeaps: [mem_heap; 16],
        }
    }
}

impl Default for VkQueueFamilyProperties {
    fn default() -> Self {
        Self {
            queueFlags: 0,
            queueCount: 0,
            timestampValidBits: 0,
            minImageTransferGranularity: VkExtent3D {
                width: 0,
                height: 0,
                depth: 0,
            },
        }
    }
}
