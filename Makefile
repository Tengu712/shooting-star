.PHONY: all
.PHONY: bindgen
.PHONY: glslc

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
vkCreateShaderModule|$\
vkCreateSampler|$\
vkCreateDescriptorSetLayout|$\
vkCreateDescriptorPool|$\
vkAllocateDescriptorSets|$\
vkCreatePipelineLayout|$\
vkCreateGraphicsPipelines|$\
vkDeviceWaitIdle|$\
vkDestroyInstance|$\
vkDestroyDevice|$\
vkDestroyCommandPool|$\
vkDestroySurfaceKHR|$\
vkDestroySwapchainKHR|$\
vkDestroyImageView|$\
vkDestroyRenderPass|$\
vkDestroyFramebuffer|$\
vkFreeCommandBuffers|$\
vkDestroyFence|$\
vkDestroySemaphore|$\
vkDestroyShaderModule|$\
vkDestroySampler|$\
vkDestroyDescriptorSetLayout|$\
vkDestroyDescriptorPool|$\
vkDestroyPipelineLayout|$\
vkDestroyPipeline|$\
vkDestroyBuffer|$\
vkFreeMemory|$\
vkDestroyImage|$\
vkAcquireNextImageKHR|$\
vkWaitForFences|$\
vkResetFences|$\
vkResetCommandBuffer|$\
vkBeginCommandBuffer|$\
vkCmdBeginRenderPass|$\
vkCmdBindPipeline|$\
vkCmdBindVertexBuffers|$\
vkCmdBindIndexBuffer|$\
vkCmdBindDescriptorSets|$\
vkCmdPushConstants|$\
vkCmdDrawIndexed|$\
vkCmdEndRenderPass|$\
vkEndCommandBuffer|$\
vkQueueSubmit|$\
vkQueuePresentKHR|$\
vkCreateBuffer|$\
vkGetBufferMemoryRequirements|$\
vkAllocateMemory|$\
vkBindBufferMemory|$\
vkMapMemory|$\
vkUnmapMemory|$\
vkCreateImage|$\
vkGetImageMemoryRequirements|$\
vkAllocateMemory|$\
vkBindImageMemory|$\
vkCmdPipelineBarrier|$\
vkCmdCopyBufferToImage|$\
vkCmdPipelineBarrier|$\
vkUpdateDescriptorSets

vulkan_type=$\
VkQueueFlagBits|$\
VkCommandPoolCreateFlagBits|$\
VkImageUsageFlagBits|$\
VkImageAspectFlagBits|$\
VkFenceCreateFlagBits|$\
VkCommandBufferResetFlagBits|$\
VkPipelineStageFlagBits|$\
VkCullModeFlagBits|$\
VkColorComponentFlagBits|$\
VkBufferUsageFlagBits|$\
VkMemoryPropertyFlagBits|$\
VkAccessFlagBits

vulkan_var=$\
VK_FALSE|$\
VK_TRUE|$\
VK_QUEUE_FAMILY_IGNORED|$\
VK_WHOLE_SIZE

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
  allowlist_function='$(vulkan_function)|$(windows_function)|memcpy'
  allowlist_type='$(vulkan_type)|$(windows_type)'
  allowlist_var='$(vulkan_var)|$(windows_var)'
else
  allowlist_function='$(vulkan_function)|$(linux_function)|memcpy'
  allowlist_type='$(vulkan_type)|$(linux_type)'
  allowlist_var='$(vulkan_var)|$(linux_var)'
endif

all: bindgen glslc

bindgen:
	bindgen \
	--allowlist-function $(allowlist_function) \
	--allowlist-type $(allowlist_type) \
	--allowlist-var $(allowlist_var) \
	src/tpl.h -o src/tpl.rs

glslc:
	glslc src/shader.vert -o shader.vert.spv
	glslc src/shader.frag -o shader.frag.spv
