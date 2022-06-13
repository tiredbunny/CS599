// A distribution of individual procedures in vkapp_fns.cpp, starting
// just after the createDevice of the initial distribution, and
// continuing through all procedures needed to achieve the next goal
// "first light".


#include <array>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include <unordered_set>
#include <unordered_map>

#include "vkapp.h"
#include "app.h"
#include "extensions_vk.hpp"


void VkApp::getCommandQueue()
{
    vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_queue);
    // Returns void -- nothing to verify
    // Nothing to destroy -- the queue is owned by the device.
}

// Calling load_VK_EXTENSIONS from extensions_vk.cpp.  A Python script
// from NVIDIA created extensions_vk.cpp from the current Vulkan spec
// for the purpose of loading the symbols for all registered
// extension.  This be (indistinguishable from) magic.
void VkApp::loadExtensions()
{
    load_VK_EXTENSIONS(m_instance, vkGetInstanceProcAddr, m_device, vkGetDeviceProcAddr);
}

//  VkSurface is Vulkan's name for the screen.  Since GLFW creates and
//  manages the window, it creates the VkSurface at our request.
void VkApp::getSurface()
{
    VkBool32 isSupported;   // Supports drawing(presenting) on a screen

    throwIfFailed(glfwCreateWindowSurface(m_instance, app->GLFW_window, nullptr, &m_surface));
    throwIfFailed(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_graphicsQueueIndex,
                                         m_surface, &isSupported));

    assert(isSupported == VK_TRUE);

    //  Verify VK_SUCCESS from both the glfw... and the vk... calls.
    //  Verify isSupported==VK_TRUE, meaning that Vulkan supports presenting on this surface.
    //To destroy: vkDestroySurfaceKHR(m_instance, m_surface, nullptr); ->added
}

// Create a command pool, used to allocate command buffers, which in
// turn are use to gather and send commands to the GPU.  The flag
// makes it possible to reuse command buffers.  The queue index
// determines which queue the command buffers can be submitted to.
// Use the command pool to also create a command buffer.
void VkApp::createCommandPool()
{
    VkCommandPoolCreateInfo poolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = m_graphicsQueueIndex;
    throwIfFailed(vkCreateCommandPool(m_device, &poolCreateInfo, nullptr, &m_cmdPool));
    //  Verify VK_SUCCESS
    // To destroy: vkDestroyCommandPool(m_device, m_cmdPool, nullptr); ->added
    
    // Create a command buffer
    VkCommandBufferAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool        = m_cmdPool;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    throwIfFailed(vkAllocateCommandBuffers(m_device, &allocateInfo, &m_commandBuffer));
    // Verify VK_SUCCESS
    // Nothing to destroy -- the pool owns the command buffer.
}
 
// 
void VkApp::createSwapchain()
{
    VkResult       err;
    VkSwapchainKHR oldSwapchain = m_swapchain;

    vkDeviceWaitIdle(m_device);  // Probably unnecessary

    // Get the surface's capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

    //  Roll your own two step process to retrieve a list of present mode into
    //  by making two calls to
    uint32_t pmCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &pmCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(pmCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &pmCount, presentModes.data());


    // Document your preset modes. I especially want to know if
    // your system offers VK_PRESENT_MODE_MAILBOX_KHR mode.  My
    // high-end windows desktop does; My higher-end Linux laptop
    // doesn't.

    //My laptop does: Alienware M15 R6 (RTX 3070 version)
   
    //Thread 0, Frame 0:
    //vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes) returns VkResult VK_SUCCESS(0) :
    //    physicalDevice : VkPhysicalDevice = 0000024262C292A0
    //    surface : VkSurfaceKHR = 00000242660D3250
    //    pPresentModeCount : uint32_t * = 4
    //    pPresentModes : VkPresentModeKHR * = 00000242660D6840
    //    pPresentModes[0] : VkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR(2)
    //    pPresentModes[1] : VkPresentModeKHR = VK_PRESENT_MODE_FIFO_RELAXED_KHR(3)
    //    pPresentModes[2] : VkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR(1)
    //    pPresentModes[3] : VkPresentModeKHR = VK_PRESENT_MODE_IMMEDIATE_KHR(0)


    // Choose VK_PRESENT_MODE_FIFO_KHR as a default (this must be supported)
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR; // Support is required.
    // But choose VK_PRESENT_MODE_MAILBOX_KHR if it can be found in
    // the retrieved presentModes Several Vulkan tutorials opine that
    // MODE_MAILBOX is the premier mode, but this may not be best for
    // us -- expect more about this later.
  

    // Get the list of VkFormat's that are supported:
    // Do the two step process to retrieve a list of surface formats in
    // with two calls to
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

    // Document the list you get.
 /*   Thread 0, Frame 0:
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats) returns VkResult VK_SUCCESS(0) :
        physicalDevice : VkPhysicalDevice = 000001AB99B525D0
        surface : VkSurfaceKHR = 000001AB9D02C7B0
        pSurfaceFormatCount : uint32_t * = 3
        pSurfaceFormats : VkSurfaceFormatKHR * = 000001ABA5B0BC10
        pSurfaceFormats[0] : VkSurfaceFormatKHR = 000001ABA5B0BC10 :
        format : VkFormat = VK_FORMAT_B8G8R8A8_UNORM(44)
        colorSpace : VkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR(0)
        pSurfaceFormats[1] : VkSurfaceFormatKHR = 000001ABA5B0BC18 :
        format : VkFormat = VK_FORMAT_B8G8R8A8_SRGB(50)
        colorSpace : VkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR(0)
        pSurfaceFormats[2] : VkSurfaceFormatKHR = 000001ABA5B0BC20 :
        format : VkFormat = VK_FORMAT_A2B10G10R10_UNORM_PACK32(64)
        colorSpace : VkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR(0)
        */

    //VkFormat surfaceFormat       = VK_FORMAT_UNDEFINED;               // Temporary value.
    //VkColorSpaceKHR surfaceColor = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; // Temporary value
    //  Replace the above two temporary lines with the following two
    // to choose the first format and its color space as defaults:
     VkFormat surfaceFormat = formats[0].format;
     VkColorSpaceKHR surfaceColor  = formats[0].colorSpace;

    // Then search the formats (from several lines up) to choose
    // format VK_FORMAT_B8G8R8A8_UNORM (and its color space) if such
    // exists.  Document your list of formats/color-spaces, and your
    // particular choice.

     for (auto& format : formats)
     {
         if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
         {
             surfaceFormat = format.format;
             surfaceColor = format.colorSpace;
         }
     }
    
    // Get the swap chain extent
    VkExtent2D swapchainExtent = capabilities.currentExtent;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapchainExtent = capabilities.currentExtent; }
    else {
        // Does this case ever happen?
        int width, height;
        glfwGetFramebufferSize(app->GLFW_window, &width, &height);

        swapchainExtent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        swapchainExtent.width = std::clamp(swapchainExtent.width,
                                           capabilities.minImageExtent.width,
                                           capabilities.maxImageExtent.width);
        swapchainExtent.height = std::clamp(swapchainExtent.height,
                                            capabilities.minImageExtent.height,
                                            capabilities.maxImageExtent.height); }

    // Test against valid size, typically hit when windows are minimized.
    // The app must prevent triggering this code in such a case
    assert(swapchainExtent.width && swapchainExtent.height);
    // If this assert fires, we have some work to do to better deal
    // with the situation.

    // Choose the number of swap chain images, within the bounds supported.
    uint imageCount = capabilities.minImageCount + 1; // Recommendation: minImageCount+1
    if (capabilities.maxImageCount > 0
        && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount; }
    
    assert (imageCount == 3);
    // If this triggers, disable the assert, BUT help me understand
    // the situation that caused it.  

    // Create the swap chain
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                 | VK_IMAGE_USAGE_STORAGE_BIT
                                 | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    VkSwapchainCreateInfoKHR _i = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    _i.surface                  = m_surface;
    _i.minImageCount            = imageCount;
    _i.imageFormat              = surfaceFormat;
    _i.imageColorSpace          = surfaceColor;
    _i.imageExtent              = swapchainExtent;
    _i.imageUsage               = imageUsage;
    _i.preTransform             = capabilities.currentTransform;
    _i.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    _i.imageArrayLayers         = 1;
    _i.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    _i.queueFamilyIndexCount    = 1;
    _i.pQueueFamilyIndices      = &m_graphicsQueueIndex;
    _i.presentMode              = swapchainPresentMode;
    _i.oldSwapchain             = oldSwapchain;
    _i.clipped                  = true;

    throwIfFailed(vkCreateSwapchainKHR(m_device, &_i, nullptr, &m_swapchain));
    //  Verify VK_SUCCESS
    
    //Do the two step process to retrieve the list of (3) swapchain images
    //   m_swapchainImages (of type std::vector<VkImage>)
    // with two calls to
    throwIfFailed(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_imageCount, nullptr));
    m_swapchainImages.resize(m_imageCount);
    throwIfFailed(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_imageCount, m_swapchainImages.data()));

    // Verify success
    // Verify and document that you retrieved the correct number of images.

  /*  Thread 0, Frame 0:
    vkGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages) returns VkResult VK_SUCCESS(0) :
        device : VkDevice = 000001B252966430
        swapchain : VkSwapchainKHR = 000001B25F24D860
        pSwapchainImageCount : uint32_t * = 3
        pSwapchainImages : VkImage * = NULL*/

    
    m_barriers.resize(m_imageCount);
    m_imageViews.resize(m_imageCount);

    // Create an VkImageView for each swap chain image.
    for (uint i=0;  i<m_imageCount;  i++) {
        VkImageViewCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            createInfo.image = m_swapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = surfaceFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(m_device, &createInfo, nullptr, &m_imageViews[i]); }

    // Create three VkImageMemoryBarrier structures (one for each swap
    // chain image) and specify the desired
    // layout (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) for each.
    for (uint i=0;  i<m_imageCount;  i++) {
        VkImageSubresourceRange range = {0};
        range.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel            = 0;
        range.levelCount              = VK_REMAINING_MIP_LEVELS;
        range.baseArrayLayer          = 0;
        range.layerCount              = VK_REMAINING_ARRAY_LAYERS;
        
        VkImageMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        memBarrier.dstAccessMask        = 0;
        memBarrier.srcAccessMask        = 0;
        memBarrier.oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        memBarrier.newLayout            = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        memBarrier.image                = m_swapchainImages[i];
        memBarrier.subresourceRange     = range;
        m_barriers[i] = memBarrier;
    }

    // Create a temporary command buffer. submit the layout conversion
    // command, submit and destroy the command buffer.
    VkCommandBuffer cmd = createTempCmdBuffer();
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0,
                         nullptr, m_imageCount, m_barriers.data());
    submitTempCmdBuffer(cmd);

    // Create the three synchronization objects.  These are not
    // technically part of the swap chain, but they are used
    // exclusively for synchronizing the swap chain, so I include them
    // here.
    VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_waitFence);
    
    VkSemaphoreCreateInfo semCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(m_device, &semCreateInfo, nullptr, &m_readSemaphore);
    vkCreateSemaphore(m_device, &semCreateInfo, nullptr, &m_writtenSemaphore);
    //NAME(m_readSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "m_readSemaphore");
    //NAME(m_writtenSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "m_writtenSemaphore");
    //NAME(m_queue, VK_OBJECT_TYPE_QUEUE, "m_queue");
        
    windowSize = swapchainExtent;
    // To destroy:  Complete and call function destroySwapchain ->done
}

void VkApp::destroySwapchain()
{
    vkDeviceWaitIdle(m_device);

    // Destroy all (3)  m_imageView'Ss with vkDestroyImageView(m_device, imageView, nullptr)

    for (auto& imageView : m_imageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    // Destroy the synchronization items: 
    vkDestroyFence(m_device, m_waitFence, nullptr);
    vkDestroySemaphore(m_device, m_readSemaphore, nullptr);
    vkDestroySemaphore(m_device, m_writtenSemaphore, nullptr);

    // Destroy the actual swapchain with: 
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    m_swapchain = VK_NULL_HANDLE;
    m_imageViews.clear();
    m_barriers.clear();
}



void VkApp::createDepthResource() 
{
    uint mipLevels = 1;

    // Note m_depthImage is type ImageWrap; a tiny wrapper around
    // several related Vulkan objects.
    m_depthImage = createImageWrap(windowSize.width, windowSize.height,
                                    VK_FORMAT_X8_D24_UNORM_PACK32,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    mipLevels);
    m_depthImage.imageView = createImageView(m_depthImage.image,
                                             VK_FORMAT_X8_D24_UNORM_PACK32,
                                             VK_IMAGE_ASPECT_DEPTH_BIT);
    // To destroy: m_depthImage.destroy(m_device); ->done
}

// Gets a list of memory types supported by the GPU, and search
// through that list for one that matches the requested properties
// flag.  The (only?) two types requested here are:
//
// (1) VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: For the bulk of the memory
// used by the GPU to store things internally.
//
// (2) VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
// for memory visible to the CPU  for CPU to GPU copy operations.
uint32_t VkApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i; } }

    throw std::runtime_error("failed to find suitable memory type!");
}


// A factory function for an ImageWrap, this creates a VkImage and
// creates and binds an associated VkDeviceMemory object.  The
// VkImageView and VkSampler are left empty to be created elsewhere as
// needed.
ImageWrap VkApp::createImageWrap(uint32_t width, uint32_t height,
                                 VkFormat format,
                                 VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties, uint mipLevels)
{
    ImageWrap myImage;
    
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage(m_device, &imageInfo, nullptr, &myImage.image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, myImage.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    vkAllocateMemory(m_device, &allocInfo, nullptr, &myImage.memory);
    
    vkBindImageMemory(m_device, myImage.image, myImage.memory, 0);

    myImage.imageView = VK_NULL_HANDLE;
    myImage.sampler = VK_NULL_HANDLE;

    return myImage;
    // @@ Verify success for vkCreateImage, and vkAllocateMemory
}

VkImageView VkApp::createImageView(VkImage image, VkFormat format,
                                         VkImageAspectFlagBits aspect)
{
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    VkImageView imageView;
    vkCreateImageView(m_device, &viewInfo, nullptr, &imageView);
    // @@ Verify success for vkCreateImageView
    
    return imageView;
}

void VkApp::createPostRenderPass()
{  
    std::array<VkAttachmentDescription, 2> attachments{};
    // Color attachment
    attachments[0].format      = VK_FORMAT_B8G8R8A8_UNORM;
    attachments[0].loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].samples     = VK_SAMPLE_COUNT_1_BIT;

    // Depth attachment
    attachments[1].format        = VK_FORMAT_X8_D24_UNORM_PACK32;
    attachments[1].loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].samples       = VK_SAMPLE_COUNT_1_BIT;

    const VkAttachmentReference colorReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference depthReference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};


    std::array<VkSubpassDependency, 1> subpassDependencies{};
    // Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commands executed outside of the actual renderpass)
    subpassDependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass      = 0;
    subpassDependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
        | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount    = 1;
    subpassDescription.pColorAttachments       = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassInfo.pDependencies   = subpassDependencies.data();

    vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_postRenderPass);
    // To destroy: vkDestroyRenderPass(m_device, m_postRenderPass, nullptr); ->done
}

// A VkFrameBuffer wraps several images into a render target --
// usually a color buffer and a depth buffer.
void VkApp::createPostFrameBuffers()
{
    std::array<VkImageView, 2> fbattachments{};
    
    // Create frame buffers for every swap chain image
    VkFramebufferCreateInfo _ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    _ci.renderPass      = m_postRenderPass;
    _ci.width           = windowSize.width;
    _ci.height          = windowSize.height;
    _ci.layers          = 1;
    _ci.attachmentCount = 2;
    _ci.pAttachments    = fbattachments.data();

    // Each of the three swapchain images gets an associated frame
    // buffer, all sharing one depth buffer.
    m_framebuffers.resize(m_imageCount);
    for(uint32_t i = 0; i < m_imageCount; i++) 
    {
        fbattachments[0] = m_imageViews[i];         // A color attachment from the swap chain
        fbattachments[1] = m_depthImage.imageView;  // A depth attachment

        throwIfFailed(vkCreateFramebuffer(m_device, &_ci, nullptr, &m_framebuffers[i])); 
    }

    // To destroy: In a loop, call: vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr); ->done
    // Verify success
}


void VkApp::createPostPipeline()
{

    // Creating the pipeline layout
    VkPipelineLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    // What we eventually want:
    //createInfo.setLayoutCount         = 1;
    //createInfo.pSetLayouts            = &m_scDesc.descSetLayout;
    // Push constants in the fragment shader
    //VkPushConstantRange pushConstantRanges = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float)};
    //createInfo.pushConstantRangeCount = 1;
    //createInfo.pPushConstantRanges    = &pushConstantRanges;
    
    // What we can do now as a first pass:
    createInfo.setLayoutCount = 1;
    createInfo.pSetLayouts = &m_postDesc.descSetLayout;
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges    = nullptr;
    
    vkCreatePipelineLayout(m_device, &createInfo, nullptr, &m_postPipelineLayout);

    ////////////////////////////////////////////
    // Create the shaders
    ////////////////////////////////////////////
    VkShaderModule vertShaderModule = createShaderModule(loadFile("spv/post.vert.spv"));
    VkShaderModule fragShaderModule = createShaderModule(loadFile("spv/post.frag.spv"));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    //auto bindingDescription = Vertex::getBindingDescription();
    //auto attributeDescriptions = Vertex::getAttributeDescriptions();

    // No geometry in this pipeline's draw.
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
        
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) windowSize.width;
    viewport.height = (float) windowSize.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = VkExtent2D{windowSize.width, windowSize.height};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; //??
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;// BEWARE!!  NECESSARY!!
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_postPipelineLayout;
    pipelineInfo.renderPass = m_postRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                              &m_postPipeline);

    // The pipeline has fully compiled copies of the shaders, so these
    // intermediate (SPV) versions can be destroyed.
    // 
    // For the two modules fragShaderModule and vertShaderModule
    // destroy right *here* via vkDestroyShaderModule(m_device, ..., nullptr);

    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    
    // To destroy:  vkDestroyPipelineLayout(m_device, m_postPipelineLayout, nullptr); ->done
    //  and:        vkDestroyPipeline(m_device, m_postPipeline, nullptr); ->done
    // Document the vkCreateGraphicsPipelines call with an api_dump.  

    //Thread 0, Frame 0:
    //vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines) returns VkResult VK_SUCCESS(0) :
    //    device : VkDevice = 00000187009BFED0
    //    pipelineCache : VkPipelineCache = 0000000000000000
    //    createInfoCount : uint32_t = 1
    //    pCreateInfos : const VkGraphicsPipelineCreateInfo * = 000001870CF6A2B8
    //    pCreateInfos[0]:                const VkGraphicsPipelineCreateInfo = 000001870CF6A2B8 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO(28)
    //    pNext : const void* = NULL
    //    flags : VkPipelineCreateFlags = 0
    //    stageCount : uint32_t = 2
    //    pStages : const VkPipelineShaderStageCreateInfo * = 000001870D0AA1B8
    //    pStages[0]:                     const VkPipelineShaderStageCreateInfo = 000001870D0AA1B8 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO(18)
    //    pNext : const void* = NULL
    //    flags : VkPipelineShaderStageCreateFlags = 0
    //    stage : VkShaderStageFlagBits = 1 (VK_SHADER_STAGE_VERTEX_BIT)
    //    module : VkShaderModule = 000001870CF4F4F0
    //    pName : const char* = "main"
    //    pSpecializationInfo : const VkSpecializationInfo * = NULL
    //    pStages[1] : const VkPipelineShaderStageCreateInfo = 000001870D0AA1E8 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO(18)
    //    pNext : const void* = NULL
    //    flags : VkPipelineShaderStageCreateFlags = 0
    //    stage : VkShaderStageFlagBits = 16 (VK_SHADER_STAGE_FRAGMENT_BIT)
    //    module : VkShaderModule = 000001870CF4FBF0
    //    pName : const char* = "main"
    //    pSpecializationInfo : const VkSpecializationInfo * = NULL
    //    pVertexInputState : const VkPipelineVertexInputStateCreateInfo * = 0000018703DCC170 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO(19)
    //    pNext : const void* = NULL
    //    flags : VkPipelineVertexInputStateCreateFlags = 0
    //    vertexBindingDescriptionCount : uint32_t = 0
    //    pVertexBindingDescriptions : const VkVertexInputBindingDescription * = NULL
    //    vertexAttributeDescriptionCount : uint32_t = 0
    //    pVertexAttributeDescriptions : const VkVertexInputAttributeDescription * = NULL
    //    pInputAssemblyState : const VkPipelineInputAssemblyStateCreateInfo * = 000001877A032D20 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO(20)
    //    pNext : const void* = NULL
    //    flags : VkPipelineInputAssemblyStateCreateFlags = 0
    //    topology : VkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST(3)
    //    primitiveRestartEnable : VkBool32 = 0
    //    pTessellationState : const VkPipelineTessellationStateCreateInfo * = NULL
    //    pViewportState : const VkPipelineViewportStateCreateInfo * = 0000018703DCCA30 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO(22)
    //    pNext : const void* = NULL
    //    flags : VkPipelineViewportStateCreateFlags = 0
    //    viewportCount : uint32_t = 1
    //    pViewports : const VkViewport * = 0000018700704190
    //    pViewports[0]:                  const VkViewport = 0000018700704190 :
    //    x : float = 0
    //    y : float = 0
    //    width : float = 1280
    //    height : float = 768
    //    minDepth : float = 0
    //    maxDepth : float = 1
    //    scissorCount : uint32_t = 1
    //    pScissors : const VkRect2D * = 00000187007041D0
    //    pScissors[0]:                   const VkRect2D = 00000187007041D0 :
    //    offset : VkOffset2D = 00000187007041D0 :
    //    x : int32_t = 0
    //    y : int32_t = 0
    //    extent : VkExtent2D = 00000187007041D8 :
    //    width : uint32_t = 1280
    //    height : uint32_t = 768
    //    pRasterizationState : const VkPipelineRasterizationStateCreateInfo * = 0000018703B14E80 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO(23)
    //    pNext : const void* = NULL
    //    flags : VkPipelineRasterizationStateCreateFlags = 0
    //    depthClampEnable : VkBool32 = 0
    //    rasterizerDiscardEnable : VkBool32 = 0
    //    polygonMode : VkPolygonMode = VK_POLYGON_MODE_FILL(0)
    //    cullMode : VkCullModeFlags = 0 (VK_CULL_MODE_NONE)
    //    frontFace : VkFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE(0)
    //    depthBiasEnable : VkBool32 = 0
    //    depthBiasConstantFactor : float = 0
    //    depthBiasClamp : float = 0
    //    depthBiasSlopeFactor : float = 0
    //    lineWidth : float = 1
    //    pMultisampleState : const VkPipelineMultisampleStateCreateInfo * = 0000018703DCC270 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO(24)
    //    pNext : const void* = NULL
    //    flags : VkPipelineMultisampleStateCreateFlags = 0
    //    rasterizationSamples : VkSampleCountFlagBits = 1 (VK_SAMPLE_COUNT_1_BIT)
    //    sampleShadingEnable : VkBool32 = 0
    //    minSampleShading : float = 0
    //    pSampleMask : const VkSampleMask * = NULL
    //    alphaToCoverageEnable : VkBool32 = 0
    //    alphaToOneEnable : VkBool32 = 0
    //    pDepthStencilState : const VkPipelineDepthStencilStateCreateInfo * = 000001870D0A9E30 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO(25)
    //    pNext : const void* = NULL
    //    flags : VkPipelineDepthStencilStateCreateFlags = 0
    //    depthTestEnable : VkBool32 = 1
    //    depthWriteEnable : VkBool32 = 1
    //    depthCompareOp : VkCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL(3)
    //    depthBoundsTestEnable : VkBool32 = 0
    //    stencilTestEnable : VkBool32 = 0
    //    front : VkStencilOpState = 000001870D0A9E58 :
    //    failOp : VkStencilOp = VK_STENCIL_OP_KEEP(0)
    //    passOp : VkStencilOp = VK_STENCIL_OP_KEEP(0)
    //    depthFailOp : VkStencilOp = VK_STENCIL_OP_KEEP(0)
    //    compareOp : VkCompareOp = VK_COMPARE_OP_NEVER(0)
    //    compareMask : uint32_t = 0
    //    writeMask : uint32_t = 0
    //    reference : uint32_t = 0
    //    back : VkStencilOpState = 000001870D0A9E74 :
    //    failOp : VkStencilOp = VK_STENCIL_OP_KEEP(0)
    //    passOp : VkStencilOp = VK_STENCIL_OP_KEEP(0)
    //    depthFailOp : VkStencilOp = VK_STENCIL_OP_KEEP(0)
    //    compareOp : VkCompareOp = VK_COMPARE_OP_NEVER(0)
    //    compareMask : uint32_t = 0
    //    writeMask : uint32_t = 0
    //    reference : uint32_t = 0
    //    minDepthBounds : float = 0
    //    maxDepthBounds : float = 0
    //    pColorBlendState : const VkPipelineColorBlendStateCreateInfo * = 0000018703DCC470 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO(26)
    //    pNext : const void* = NULL
    //    flags : VkPipelineColorBlendStateCreateFlags = 0
    //    logicOpEnable : VkBool32 = 0
    //    logicOp : VkLogicOp = VK_LOGIC_OP_COPY(3)
    //    attachmentCount : uint32_t = 1
    //    pAttachments : const VkPipelineColorBlendAttachmentState * = 000001877A032750
    //    pAttachments[0]:                const VkPipelineColorBlendAttachmentState = 000001877A032750 :
    //    blendEnable : VkBool32 = 0
    //    srcColorBlendFactor : VkBlendFactor = VK_BLEND_FACTOR_ZERO(0)
    //    dstColorBlendFactor : VkBlendFactor = VK_BLEND_FACTOR_ZERO(0)
    //    colorBlendOp : VkBlendOp = VK_BLEND_OP_ADD(0)
    //    srcAlphaBlendFactor : VkBlendFactor = VK_BLEND_FACTOR_ZERO(0)
    //    dstAlphaBlendFactor : VkBlendFactor = VK_BLEND_FACTOR_ZERO(0)
    //    alphaBlendOp : VkBlendOp = VK_BLEND_OP_ADD(0)
    //    colorWriteMask : VkColorComponentFlags = 15 (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
    //    blendConstants : float[4] = 0000018703DCC498
    //    blendConstants[0]:              float = 0
    //    blendConstants[1] : float = 0
    //    blendConstants[2] : float = 0
    //    blendConstants[3] : float = 0
    //    pDynamicState : const VkPipelineDynamicStateCreateInfo * = NULL
    //    layout : VkPipelineLayout = 000001870D37B200
    //    renderPass : VkRenderPass = 0000018700B989D0
    //    subpass : uint32_t = 0
    //    basePipelineHandle : VkPipeline = 0000000000000000
    //    basePipelineIndex : int32_t = 0
    //    pAllocator : const VkAllocationCallbacks * = NULL
    //    pPipelines : VkPipeline * = 0000007D3A6FF680
    //    pPipelines[0] : VkPipeline = 000001870D3675D0

}

#ifdef GUI
void VkApp::initGUI()
{
    uint subpassID = 0;

    // UI
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    std::vector<VkDescriptorPoolSize> poolSize{ {VK_DESCRIPTOR_TYPE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1} };
    VkDescriptorPoolCreateInfo        poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.maxSets = 2;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize.data();
    vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_imguiDescPool);

    // Setup Platform/Renderer back ends
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_instance;
    init_info.PhysicalDevice = m_physicalDevice;
    init_info.Device = m_device;
    init_info.QueueFamily = m_graphicsQueueIndex;
    init_info.Queue = m_queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_imguiDescPool;
    init_info.Subpass = subpassID;
    init_info.MinImageCount = 2;
    init_info.ImageCount = m_imageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = nullptr;
    init_info.Allocator = nullptr;

    ImGui_ImplVulkan_Init(&init_info, m_postRenderPass);

    // Upload Fonts
    VkCommandBuffer cmdbuf = createTempCmdBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(cmdbuf);
    submitTempCmdBuffer(cmdbuf);

    ImGui_ImplGlfw_InitForVulkan(app->GLFW_window, true);
}
#endif

std::string VkApp::loadFile(const std::string& filename)
{
    std::string   result;
    std::ifstream stream(filename, std::ios::ate | std::ios::binary);  //ate: Open at file end

    if(!stream.is_open())
        return result;

    result.reserve(stream.tellg()); // tellg() is last char position in file (i.e.,  length)
    stream.seekg(0, std::ios::beg);

    result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return result;
}

//-------------------------------------------------------------------------------------------------
// Post processing pass: tone mapper, UI
void VkApp::postProcess()
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color        = {{1,1,1,1}};
    clearValues[1].depthStencil = {1.0f, 0};
            
    VkRenderPassBeginInfo _i{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    _i.clearValueCount = 2;
    _i.pClearValues    = clearValues.data();
    _i.renderPass      = m_postRenderPass;
    _i.framebuffer     = m_framebuffers[m_swapchainIndex];
    _i.renderArea      = {{0, 0}, windowSize};
    
    vkCmdBeginRenderPass(m_commandBuffer, &_i, VK_SUBPASS_CONTENTS_INLINE);
    {   // extra indent for renderpass commands
        //VkViewport viewport{0.0f, 0.0f,
        //    static_cast<float>(windowSize.width), static_cast<float>(windowSize.height),
        //    0.0f, 1.0f};
        //vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
        //
        //VkRect2D scissor{{0, 0}, {windowSize.width, windowSize.height}};
        //vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

        auto aspectRatio = static_cast<float>(windowSize.width)
            / static_cast<float>(windowSize.height);
        //vkCmdPushConstants(m_commandBuffer, m_postPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        //                   sizeof(float), &aspectRatio);

        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_postPipelineLayout, 0, 1, &m_postDesc.descSet, 0, nullptr);

        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_postPipeline);
        //vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        //                       m_postPipelineLayout, 0, 0, nullptr, 0, nullptr);

        // Weird! This draws 3 vertices but with no vertices/triangles buffers bound in.
        // Hint: The vertex shader fabricates vertices from gl_VertexIndex
        vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

        #ifdef GUI
        ImGui::Render();  // Rendering UI
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_commandBuffer);
        #endif
    }
    vkCmdEndRenderPass(m_commandBuffer);
}

// That's all for now!
// Many more procedures will follow ...
