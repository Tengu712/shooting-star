use super::*;
use super::additions::*;

use std::os::raw::c_char;

const APP_NAME: *const c_char = [
    'V' as c_char,
    'k' as c_char,
    'A' as c_char,
    'p' as c_char,
    'p' as c_char,
    '\0' as c_char,
]
.as_ptr();

#[cfg(debug_assertions)]
const INST_LAYER_NAMES: [*const c_char; 1] = [PCHAR_VK_LAYER_KHRONOS_validation];
#[cfg(not(debug_assertions))]
const INST_LAYER_NAMES: [*const c_char; 0] = [];

#[cfg(target_os = "linux")]
const INST_EXT_NAME_SURFACE: *const c_char = PCHAR_VK_KHR_xlib_surface;
#[cfg(target_os = "windows")]
const INST_EXT_NAME_SURFACE: *const c_char = PCHAR_VK_KHR_win32_surface;

#[cfg(debug_assertions)]
const INST_EXT_NAMES: [*const c_char; 4] = [
    PCHAR_VK_EXT_debug_report,
    PCHAR_VK_EXT_debug_utils,
    PCHAR_VK_KHR_surface,
    INST_EXT_NAME_SURFACE,
];
#[cfg(not(debug_assertions))]
const INST_EXT_NAMES: [*const c_char; 2] = [PCHAR_VK_KHR_surface, INST_EXT_NAME_SURFACE];

const DEVICE_EXT_NAMES: [*const c_char; 1] = [PCHAR_VK_KHR_swapchain];

impl VulkanApp {
    pub fn new() -> Self {
        let instance = {
            let ai = VkApplicationInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_APPLICATION_INFO,
                pNext: null(),
                pApplicationName: APP_NAME,
                applicationVersion: 0,
                pEngineName: APP_NAME,
                engineVersion: VK_MAKE_VERSION(0, 1, 0),
                apiVersion: VK_MAKE_API_VERSION(0, 1, 2, 0),
            };
            let ci = VkInstanceCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                pApplicationInfo: &ai,
                enabledLayerCount: INST_LAYER_NAMES.len() as u32,
                ppEnabledLayerNames: INST_LAYER_NAMES.as_ptr(),
                enabledExtensionCount: INST_EXT_NAMES.len() as u32,
                ppEnabledExtensionNames: INST_EXT_NAMES.as_ptr(),
            };
            let mut instance = null_mut();
            check_vk!(
                vkCreateInstance(&ci, null(), &mut instance),
                "failed to create a Vulkan instance."
            );
            instance
        };

        let (phys_device, phys_device_mem_prop) = {
            // physical device
            let mut cnt = 0;
            check_vk!(
                vkEnumeratePhysicalDevices(instance, &mut cnt, null_mut()),
                "failed to get the number of physical devices."
            );
            let mut phys_devices = vec![null_mut(); cnt as usize];
            check_vk!(
                vkEnumeratePhysicalDevices(instance, &mut cnt, phys_devices.as_mut_ptr()),
                "failed to enumerate physical devices."
            );
            // TODO: select a physical device better.
            let phys_device = if let Some(n) = phys_devices.get(0) {
                n.clone()
            } else {
                ss_error("tried to get a physical device out of index.");
            };

            // physical device memory properties
            let mut phys_device_mem_prop = VkPhysicalDeviceMemoryProperties::default();
            unsafe { vkGetPhysicalDeviceMemoryProperties(phys_device, &mut phys_device_mem_prop) };

            // finish
            (phys_device, phys_device_mem_prop)
        };

        let queue_family_index = {
            let mut cnt = 0;
            unsafe { vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &mut cnt, null_mut()) };
            let mut props = vec![VkQueueFamilyProperties::default(); cnt as usize];
            unsafe {
                vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &mut cnt, props.as_mut_ptr())
            };
            let mut queue_family_index = -1;
            for (i, prop) in props.into_iter().enumerate() {
                if (prop.queueFlags & VkQueueFlagBits_VK_QUEUE_GRAPHICS_BIT) > 0 {
                    queue_family_index = i as i32;
                }
            }
            check!(
                queue_family_index >= 0,
                "failed to find a queue family index."
            );
            queue_family_index as u32
        };

        let device = {
            let queue_cis = [VkDeviceQueueCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                queueFamilyIndex: queue_family_index,
                queueCount: 1,
                pQueuePriorities: [1.0].as_ptr(),
            }];
            let ci = VkDeviceCreateInfo {
                sType: VkStructureType_VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                queueCreateInfoCount: 1,
                pQueueCreateInfos: queue_cis.as_ptr(),
                enabledLayerCount: 0,
                ppEnabledLayerNames: null(),
                enabledExtensionCount: DEVICE_EXT_NAMES.len() as u32,
                ppEnabledExtensionNames: DEVICE_EXT_NAMES.as_ptr(),
                pEnabledFeatures: null(),
            };
            let mut device = null_mut();
            check_vk!(
                vkCreateDevice(phys_device, &ci, null(), &mut device),
                "failed to create a device."
            );
            device
        };

        // finish
        Self {
            instance,
            phys_device_mem_prop,
            device,
        }
    }
}
