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
vkAllocateCommandBuffers|$\
vkCreateFence|$\
vkCreateSemaphore|$\
vkAcquireNextImageKHR|$\
vkWaitForFences|$\
vkResetFences|$\
vkResetCommandBuffer|$\
vkBeginCommandBuffer|$\
vkCmdBeginRenderPass|$\
vkCmdEndRenderPass|$\
vkEndCommandBuffer|$\
vkQueueSubmit|$\
vkQueuePresentKHR|$\
vkDeviceWaitIdle|$\
vkDestroySemaphore|$\
vkDestroyFence|$\
vkFreeCommandBuffers|$\
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
VkImageAspectFlagBits|$\
VkFenceCreateFlagBits|$\
VkCommandBufferResetFlagBits|$\
VkPipelineStageFlagBits

vulkan_var=$\
VK_TRUE

linux_function=$\
XOpenDisplay|$\
XCreateSimpleWindow|$\
XDefaultRootWindow|$\
XStoreName|$\
XSetWMNormalHints|$\
XInternAtom|$\
XSetWMProtocols|$\
XMapWindow|$\
XFlush|$\
XPending|$\
XNextEvent

linux_type=$\

linux_var=$\
PMinSize|$\
PMaxSize|$\
False|$\
ClientMessage

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

ifeq ($(OS),Windows_NT)
  allowlist_function='$(vulkan_function)|$(windows_function)'
  allowlist_type='$(vulkan_type)|$(windows_type)'
  allowlist_var='$(vulkan_var)|$(windows_var)'
else
  allowlist_function='$(vulkan_function)|$(linux_function)'
  allowlist_type='$(vulkan_type)|$(linux_type)'
  allowlist_var='$(vulkan_var)|$(linux_var)'
endif

bindgen:
	bindgen \
	--allowlist-function $(allowlist_function) \
	--allowlist-type $(allowlist_type) \
	--allowlist-var $(allowlist_var) \
	src/tpl.h -o src/tpl.rs