.PHONY: bindgen

vulkan_function=$\
vkCreateInstance|$\
vkEnumeratePhysicalDevices|$\
vkGetPhysicalDeviceMemoryProperties|$\
vkGetPhysicalDeviceQueueFamilyProperties|$\
vkCreateDevice|$\
vkGetDeviceQueue|$\
vkCreateCommandPool|$\
vkCreateXlibSurfaceKHR|$\
vkCreateWin32SurfaceKHR|$\
vkGetPhysicalDeviceSurfaceFormatsKHR|$\
vkGetPhysicalDeviceSurfaceCapabilitiesKHR|$\
vkCreateSwapchainKHR|$\
vkGetSwapchainImagesKHR|$\
vkCreateImageView|$\
vkCreateRenderPass|$\
vkCreateFramebuffer|$\
vkDeviceWaitIdle|$\
vkDestroyFramebuffer|$\
vkDestroyRenderPass|$\
vkDestroyImageView|$\
vkDestroySwapchainKHR|$\
vkDestroySurfaceKHR|$\
vkDestroyCommandPool|$\
vkDestroyDevice|$\
vkDestroyInstance

vulkan_type=$\
VkQueueFlagBits|$\
VkCommandPoolCreateFlagBits|$\
VkImageUsageFlagBits|$\
VkImageAspectFlagBits

vulkan_var=$\
VK_TRUE

windows_function=$\
GetModuleHandleW|$\
RegisterClassExW|$\
AdjustWindowRect|$\
CreateWindowExW|$\
ShowWindow|$\
UpdateWindow|$\
PeekMessageW|$\
TranslateMessage|$\
DispatchMessageW|$\
DestroyWindow|$\
UnregisterClassW|$\
PostQuitMessage|$\
DefWindowProcW

windows_type=$\
RECT

windows_var=$\
CS_CLASSDC|$\
WS_OVERLAPPED|$\
WS_CAPTION|$\
WS_SYSMENU|$\
WS_MINIMIZEBOX|$\
SW_SHOWDEFAULT|$\
PM_REMOVE|$\
WM_QUIT|$\
WM_DESTROY

allowlist_function='$(vulkan_function)|$(windows_function)'
allowlist_type='$(vulkan_type)|$(windows_type)'
allowlist_var='$(vulkan_var)|$(windows_var)'

bindgen:
	bindgen \
	--allowlist-function $(allowlist_function) \
	--allowlist-type $(allowlist_type) \
	--allowlist-var $(allowlist_var) \
	src/tpl.h -o src/tpl.rs