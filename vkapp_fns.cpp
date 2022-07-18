
#include <array>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include <unordered_set>
#include <unordered_map>

#include "vkapp.h"
#include "app.h"
#include "extensions_vk.hpp"


void VkApp::destroyAllVulkanResources()
{
    vkDeviceWaitIdle(m_device);  // Uncomment this when you have an m_device created.

    // Destroy all vulkan objects.
    // ...  All objects created on m_device must be destroyed before m_device.

    m_rtBuilder.destroy();

    m_shaderBindingTableBW.destroy(m_device);

	vkDestroyPipelineLayout(m_device, m_rtPipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_rtPipeline, nullptr);

    m_rtColCurrBuffer.destroy(m_device);
    m_rtNdCurrBuffer.destroy(m_device);
    m_rtKdCurrBuffer.destroy(m_device);
    m_rtColPrevBuffer.destroy(m_device);
    m_rtNdPrevBuffer.destroy(m_device);

    vkDestroyDescriptorPool(m_device, m_imguiDescPool, nullptr);
    ImGui_ImplVulkan_Shutdown();

    for (auto& t : m_objText)
        t.destroy(m_device); 

    for (auto& ob : m_objData)
    {
        ob.indexBuffer.destroy(m_device);
        ob.vertexBuffer.destroy(m_device);
        ob.matColorBuffer.destroy(m_device);
        ob.matIndexBuffer.destroy(m_device);
    }


    vkDestroyPipelineLayout(m_device, m_scanlinePipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_scanlinePipeline, nullptr);

    m_scDesc.destroy(m_device);
    vkDestroyRenderPass(m_device, m_scanlineRenderPass, nullptr);
    vkDestroyFramebuffer(m_device, m_scanlineFramebuffer, nullptr);
    m_objDescriptionBW.destroy(m_device);
    m_matrixBW.destroy(m_device);
    m_postDesc.destroy(m_device);
    m_scImageBuffer.destroy(m_device);

    vkDestroyPipelineLayout(m_device, m_postPipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_postPipeline, nullptr);

    for (auto& fb : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, fb, nullptr);
    }

    vkDestroyRenderPass(m_device, m_postRenderPass, nullptr);
    m_depthImage.destroy(m_device);
    destroySwapchain();
    vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void VkApp::recreateSizedResources(VkExtent2D size)
{
    assert(false && "Not ready for onResize events.");
    // Destroy everything related to the window size
    // (RE)Create them all at the new size
}
 
void VkApp::createInstance(bool doApiDump)
{
    uint32_t countGLFWextensions{0};
    const char** reqGLFWextensions = glfwGetRequiredInstanceExtensions(&countGLFWextensions);

    // Append each GLFW required extension in reqGLFWextensions to reqInstanceExtensions
    // Print them out while your are at it
    printf("GLFW required extensions:\n");
    // ...
    for (uint i = 0; i < countGLFWextensions; ++i)
    {
        reqInstanceExtensions.push_back(reqGLFWextensions[i]);
        printf("%s\n", reqGLFWextensions[i]);

    }

    // Suggestion: Parse a command line argument to set/unset doApiDump
    if (doApiDump)
        reqInstanceLayers.push_back("VK_LAYER_LUNARG_api_dump");
  
    uint32_t count;
    // The two step procedure for getting a variable length list
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> availableLayers(count);
    vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

    // Print out the availableLayers
    printf("InstanceLayer count: %d\n", count);
    // ...  use availableLayers[i].layerName
    for (auto& layer : availableLayers)
    {
        printf("%s\n", layer.layerName);
    }
    // Another two step dance
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

    // Print out the availableExtensions
    printf("InstanceExtensions count: %d\n", count);
    // ...  use availableExtensions[i].extensionName
    for (auto& extension : availableExtensions)
    {
        printf("%s\n", extension.extensionName);
    }

    VkApplicationInfo applicationInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    applicationInfo.pApplicationName = "rtrt";
    applicationInfo.pEngineName      = "no-engine";
    applicationInfo.apiVersion       = VK_MAKE_VERSION(1, 3, 0);

    VkInstanceCreateInfo instanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceCreateInfo.pNext                   = nullptr;
    instanceCreateInfo.pApplicationInfo        = &applicationInfo;
    
    instanceCreateInfo.enabledExtensionCount   = reqInstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = reqInstanceExtensions.data();
    
    instanceCreateInfo.enabledLayerCount       = reqInstanceLayers.size();
    instanceCreateInfo.ppEnabledLayerNames     = reqInstanceLayers.data();

    throwIfFailed(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

    // Document with a cut-and-paste of the three list printouts above.
    //    GLFW required extensions:
    // 
    //    VK_KHR_surface
    //    VK_KHR_win32_surface
    // 
    //    InstanceLayer count : 10
    // 
    //    VK_LAYER_NV_optimus
    //    VK_LAYER_VALVE_steam_overlay
    //    VK_LAYER_VALVE_steam_fossilize
    //    VK_LAYER_LUNARG_api_dump
    //    VK_LAYER_LUNARG_gfxreconstruct
    //    VK_LAYER_KHRONOS_synchronization2
    //    VK_LAYER_KHRONOS_validation
    //    VK_LAYER_LUNARG_monitor
    //    VK_LAYER_LUNARG_screenshot
    //    VK_LAYER_KHRONOS_profiles
    // 
    //    InstanceExtensions count : 16
    // 
    //    VK_KHR_device_group_creation
    //    VK_KHR_display
    //    VK_KHR_external_fence_capabilities
    //    VK_KHR_external_memory_capabilities
    //    VK_KHR_external_semaphore_capabilities
    //    VK_KHR_get_display_properties2
    //    VK_KHR_get_physical_device_properties2
    //    VK_KHR_get_surface_capabilities2
    //    VK_KHR_surface
    //    VK_KHR_surface_protected_capabilities
    //    VK_KHR_win32_surface
    //    VK_EXT_debug_report
    //    VK_EXT_debug_utils
    //    VK_EXT_direct_mode_display
    //    VK_EXT_swapchain_colorspace
    //    VK_NV_external_memory_capabilities
}

void VkApp::createPhysicalDevice()
{
    uint physicalDevicesCount;
    vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, physicalDevices.data());

    std::vector<uint32_t> compatibleDevices;
  
    printf("%d devices\n", physicalDevicesCount);
    int i = 0;

    // For each GPU:
    for (auto physicalDevice : physicalDevices) 
    {

        // Get the GPU's properties
        VkPhysicalDeviceProperties GPUproperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &GPUproperties);

        // Get the GPU's extension list;  Another two-step list retrieval procedure:
        uint extCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                             &extCount, extensionProperties.data());
        

        if (GPUproperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
          
            bool isCompatible = false;
            for (auto& reqExtension : reqDeviceExtensions)
            {
                isCompatible = false;
                for (auto& extensionProperty : extensionProperties)
                {
                    if (strcmp(reqExtension, extensionProperty.extensionName) == 0)
                    {
                        isCompatible = true;
                    }
                }

                //missing one of the required extension
                if (!isCompatible)
                    break;
                
            }

            //return the first compatible device
            if (isCompatible)
            {
                m_physicalDevice = physicalDevice;
                return;
            }
        }

        // This code is in a loop iterating variable physicalDevice
        // through a list of all physicalDevices.  The
        // physicalDevice's properties (GPUproperties) and a list of
        // its extension properties (extensionProperties) are retrieve
        // above, and here we judge if the physicalDevice (i.e.. GPU)
        // is compatible with our requirements. We consider a GPU to be
        // compatible if it satisfies both:
        
        //    GPUproperties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        
        // and
        
        //    All reqDeviceExtensions can be found in the GPUs extensionProperties list
        //      That is: for all i, there exists a j such that:
        //                 reqDeviceExtensions[i] == extensionProperties[j].extensionName

        //  If a GPU is found to be compatible
        //  Return it (physicalDevice), or continue the search and then return the best GPU.
        //    raise an exception of none were found
        //    tell me all about your system if more than one was found.
    }


    //not found
    throw std::exception();
  
}

void VkApp::chooseQueueIndex()
{
    VkQueueFlags requiredQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT
                                      | VK_QUEUE_TRANSFER_BIT;
    
    uint32_t mpCount;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &mpCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProperties(mpCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &mpCount, queueProperties.data());

    //  Use the api_dump to document the results of the above two
    // step.  How many queue families does your Vulkan offer.  Which
    // of them, by index, has the above three required flags?

    //3 Queue families in my machine. Index 0 queue family has all the required flags.

    //Thread 0, Frame 0:
    //    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties) returns void :
    //    physicalDevice : VkPhysicalDevice = 000001D5E994BEF0
    //    pQueueFamilyPropertyCount : uint32_t * = 3
    //    pQueueFamilyProperties : VkQueueFamilyProperties * = NULL

    //    Thread 0, Frame 0 :
    //    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties) returns void :
    //    physicalDevice : VkPhysicalDevice = 000001D5E994BEF0
    //    pQueueFamilyPropertyCount : uint32_t * = 3
    //    pQueueFamilyProperties : VkQueueFamilyProperties * = 000001D5E9769230
    //    pQueueFamilyProperties[0] : VkQueueFamilyProperties = 000001D5E9769230 :
    //    queueFlags : VkQueueFlags = 15 (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)
    //    queueCount : uint32_t = 16
    //    timestampValidBits : uint32_t = 64
    //    minImageTransferGranularity : VkExtent3D = 000001D5E976923C :
    //    width : uint32_t = 1
    //    height : uint32_t = 1
    //    depth : uint32_t = 1
    //    pQueueFamilyProperties[1] : VkQueueFamilyProperties = 000001D5E9769248 :
    //    queueFlags : VkQueueFlags = 12 (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)
    //    queueCount : uint32_t = 2
    //    timestampValidBits : uint32_t = 64
    //    minImageTransferGranularity : VkExtent3D = 000001D5E9769254 :
    //    width : uint32_t = 1
    //    height : uint32_t = 1
    //    depth : uint32_t = 1
    //    pQueueFamilyProperties[2] : VkQueueFamilyProperties = 000001D5E9769260 :
    //    queueFlags : VkQueueFlags = 14 (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)
    //    queueCount : uint32_t = 8
    //    timestampValidBits : uint32_t = 64
    //    minImageTransferGranularity : VkExtent3D = 000001D5E976926C :
    //    width : uint32_t = 1
    //    height : uint32_t = 1
    //    depth : uint32_t = 1

    
    
    // Search the list for (the index of) the first queue family that has the required flags.
    // Verity that your search choose the correct queue family.
    // Record the index in m_graphicsQueueIndex.
    // Nothing to destroy as m_graphicsQueueIndex is just an integer.
    //m_graphicsQueueIndex = you chosen index;

    for (uint i = 0; i < queueProperties.size(); ++i)
    {
        if ((queueProperties[i].queueFlags & requiredQueueFlags) == requiredQueueFlags)
        {
            m_graphicsQueueIndex = i;
            break;
        }
    }

    printf("m_graphicsQueueIndex = %d\n", m_graphicsQueueIndex);
}


void VkApp::createDevice()
{
    // 
    // Build a pNext chain of the following six "feature" structures:
    //   features2->features11->features12->features13->accelFeature->rtPipelineFeature->NULL

    // Hint: Keep it simple; add a second parameter (the pNext pointer) to each
    // structure point up to the previous structure.
    
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, nullptr};
    
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &rtPipelineFeature};
    
    VkPhysicalDeviceVulkan13Features features13{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, &accelFeature};
    
    VkPhysicalDeviceVulkan12Features features12{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &features13};
    
    VkPhysicalDeviceVulkan11Features features11{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &features12};
    
    VkPhysicalDeviceFeatures2 features2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &features11};

    

    // Fill in all structures on the pNext chain
    vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features2);
    // 
    // Document the whole filled in pNext chain using an api_dump
    // Examine all the many features.  Do any of them look familiar?

    //Thread 0, Frame 0:
    //vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice) returns VkResult VK_SUCCESS(0) :
    //    physicalDevice : VkPhysicalDevice = 0000023E1A715DD0
    //    pCreateInfo : const VkDeviceCreateInfo * = 000000BD89F5E0D0 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO(3)
    //    pNext : const void* = NULL
    //    flags : VkDeviceCreateFlags = 0
    //    queueCreateInfoCount : uint32_t = 1
    //    pQueueCreateInfos : const VkDeviceQueueCreateInfo * = 0000023E1ACEB9C8
    //    pQueueCreateInfos[0] : const VkDeviceQueueCreateInfo = 0000023E1ACEB9C8 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO(2)
    //    pNext : const void* = NULL
    //    flags : VkDeviceQueueCreateFlags = 0
    //    queueFamilyIndex : uint32_t = 0
    //    queueCount : uint32_t = 1
    //    pQueuePriorities : const float* = 0000023E1A00C350
    //    pQueuePriorities[0] : const float = 1
    //    enabledLayerCount : uint32_t = 0
    //    ppEnabledLayerNames : const char* const* = 0000023E1A00C270
    //    enabledExtensionCount : uint32_t = 4
    //    ppEnabledExtensionNames : const char* const* = 0000023E1A716AF0
    //    ppEnabledExtensionNames[0] : const char* const = "VK_KHR_swapchain"
    //    ppEnabledExtensionNames[1] : const char* const = "VK_KHR_acceleration_structure"
    //    ppEnabledExtensionNames[2] : const char* const = "VK_KHR_ray_tracing_pipeline"
    //    ppEnabledExtensionNames[3] : const char* const = "VK_KHR_deferred_host_operations"
    //    pEnabledFeatures : const VkPhysicalDeviceFeatures * = NULL
    //    pNext : VkPhysicalDeviceFeatures2 = 0000023E1A9F0130 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2(1000059000)
    //    pNext : const void* = VkPhysicalDeviceVulkan11Features
    //    features : VkPhysicalDeviceFeatures = 0000023E1A9F0140 :
    //    robustBufferAccess : VkBool32 = 0
    //    fullDrawIndexUint32 : VkBool32 = 1
    //    imageCubeArray : VkBool32 = 1
    //    independentBlend : VkBool32 = 1
    //    geometryShader : VkBool32 = 1
    //    tessellationShader : VkBool32 = 1
    //    sampleRateShading : VkBool32 = 1
    //    dualSrcBlend : VkBool32 = 1
    //    logicOp : VkBool32 = 1
    //    multiDrawIndirect : VkBool32 = 1
    //    drawIndirectFirstInstance : VkBool32 = 1
    //    depthClamp : VkBool32 = 1
    //    depthBiasClamp : VkBool32 = 1
    //    fillModeNonSolid : VkBool32 = 1
    //    depthBounds : VkBool32 = 1
    //    wideLines : VkBool32 = 1
    //    largePoints : VkBool32 = 1
    //    alphaToOne : VkBool32 = 1
    //    multiViewport : VkBool32 = 1
    //    samplerAnisotropy : VkBool32 = 1
    //    textureCompressionETC2 : VkBool32 = 0
    //    textureCompressionASTC_LDR : VkBool32 = 0
    //    textureCompressionBC : VkBool32 = 1
    //    occlusionQueryPrecise : VkBool32 = 1
    //    pipelineStatisticsQuery : VkBool32 = 1
    //    vertexPipelineStoresAndAtomics : VkBool32 = 1
    //    fragmentStoresAndAtomics : VkBool32 = 1
    //    shaderTessellationAndGeometryPointSize : VkBool32 = 1
    //    shaderImageGatherExtended : VkBool32 = 1
    //    shaderStorageImageExtendedFormats : VkBool32 = 1
    //    shaderStorageImageMultisample : VkBool32 = 1
    //    shaderStorageImageReadWithoutFormat : VkBool32 = 1
    //    shaderStorageImageWriteWithoutFormat : VkBool32 = 1
    //    shaderUniformBufferArrayDynamicIndexing : VkBool32 = 1
    //    shaderSampledImageArrayDynamicIndexing : VkBool32 = 1
    //    shaderStorageBufferArrayDynamicIndexing : VkBool32 = 1
    //    shaderStorageImageArrayDynamicIndexing : VkBool32 = 1
    //    shaderClipDistance : VkBool32 = 1
    //    shaderCullDistance : VkBool32 = 1
    //    shaderFloat64 : VkBool32 = 1
    //    shaderInt64 : VkBool32 = 1
    //    shaderInt16 : VkBool32 = 1
    //    shaderResourceResidency : VkBool32 = 1
    //    shaderResourceMinLod : VkBool32 = 1
    //    sparseBinding : VkBool32 = 1
    //    sparseResidencyBuffer : VkBool32 = 1
    //    sparseResidencyImage2D : VkBool32 = 1
    //    sparseResidencyImage3D : VkBool32 = 1
    //    sparseResidency2Samples : VkBool32 = 1
    //    sparseResidency4Samples : VkBool32 = 1
    //    sparseResidency8Samples : VkBool32 = 1
    //    sparseResidency16Samples : VkBool32 = 1
    //    sparseResidencyAliased : VkBool32 = 1
    //    variableMultisampleRate : VkBool32 = 1
    //    inheritedQueries : VkBool32 = 1
    //    pNext : VkPhysicalDeviceVulkan11Features = 0000023E1A68C380 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES(49)
    //    pNext : const void* = VkPhysicalDeviceVulkan12Features
    //    storageBuffer16BitAccess : VkBool32 = 1
    //    uniformAndStorageBuffer16BitAccess : VkBool32 = 1
    //    storagePushConstant16 : VkBool32 = 1
    //    storageInputOutput16 : VkBool32 = 0
    //    multiview : VkBool32 = 1
    //    multiviewGeometryShader : VkBool32 = 1
    //    multiviewTessellationShader : VkBool32 = 1
    //    variablePointersStorageBuffer : VkBool32 = 1
    //    variablePointers : VkBool32 = 1
    //    protectedMemory : VkBool32 = 0
    //    samplerYcbcrConversion : VkBool32 = 1
    //    shaderDrawParameters : VkBool32 = 1
    //    pNext : VkPhysicalDeviceVulkan12Features = 0000023E19EEF140 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES(51)
    //    pNext : const void* = VkPhysicalDeviceVulkan13Features
    //    samplerMirrorClampToEdge : VkBool32 = 1
    //    drawIndirectCount : VkBool32 = 1
    //    storageBuffer8BitAccess : VkBool32 = 1
    //    uniformAndStorageBuffer8BitAccess : VkBool32 = 1
    //    storagePushConstant8 : VkBool32 = 1
    //    shaderBufferInt64Atomics : VkBool32 = 1
    //    shaderSharedInt64Atomics : VkBool32 = 1
    //    shaderFloat16 : VkBool32 = 1
    //    shaderInt8 : VkBool32 = 1
    //    descriptorIndexing : VkBool32 = 1
    //    shaderInputAttachmentArrayDynamicIndexing : VkBool32 = 1
    //    shaderUniformTexelBufferArrayDynamicIndexing : VkBool32 = 1
    //    shaderStorageTexelBufferArrayDynamicIndexing : VkBool32 = 1
    //    shaderUniformBufferArrayNonUniformIndexing : VkBool32 = 1
    //    shaderSampledImageArrayNonUniformIndexing : VkBool32 = 1
    //    shaderStorageBufferArrayNonUniformIndexing : VkBool32 = 1
    //    shaderStorageImageArrayNonUniformIndexing : VkBool32 = 1
    //    shaderInputAttachmentArrayNonUniformIndexing : VkBool32 = 1
    //    shaderUniformTexelBufferArrayNonUniformIndexing : VkBool32 = 1
    //    shaderStorageTexelBufferArrayNonUniformIndexing : VkBool32 = 1
    //    descriptorBindingUniformBufferUpdateAfterBind : VkBool32 = 1
    //    descriptorBindingSampledImageUpdateAfterBind : VkBool32 = 1
    //    descriptorBindingStorageImageUpdateAfterBind : VkBool32 = 1
    //    descriptorBindingStorageBufferUpdateAfterBind : VkBool32 = 1
    //    descriptorBindingUniformTexelBufferUpdateAfterBind : VkBool32 = 1
    //    descriptorBindingStorageTexelBufferUpdateAfterBind : VkBool32 = 1
    //    descriptorBindingUpdateUnusedWhilePending : VkBool32 = 1
    //    descriptorBindingPartiallyBound : VkBool32 = 1
    //    descriptorBindingVariableDescriptorCount : VkBool32 = 1
    //    runtimeDescriptorArray : VkBool32 = 1
    //    samplerFilterMinmax : VkBool32 = 1
    //    scalarBlockLayout : VkBool32 = 1
    //    imagelessFramebuffer : VkBool32 = 1
    //    uniformBufferStandardLayout : VkBool32 = 1
    //    shaderSubgroupExtendedTypes : VkBool32 = 1
    //    separateDepthStencilLayouts : VkBool32 = 1
    //    hostQueryReset : VkBool32 = 1
    //    timelineSemaphore : VkBool32 = 1
    //    bufferDeviceAddress : VkBool32 = 1
    //    bufferDeviceAddressCaptureReplay : VkBool32 = 1
    //    bufferDeviceAddressMultiDevice : VkBool32 = 1
    //    vulkanMemoryModel : VkBool32 = 1
    //    vulkanMemoryModelDeviceScope : VkBool32 = 1
    //    vulkanMemoryModelAvailabilityVisibilityChains : VkBool32 = 1
    //    shaderOutputViewportIndex : VkBool32 = 1
    //    shaderOutputLayer : VkBool32 = 1
    //    subgroupBroadcastDynamicId : VkBool32 = 1
    //    pNext : VkPhysicalDeviceVulkan13Features = 0000023E1A82D850 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES(53)
    //    pNext : const void* = VkPhysicalDeviceAccelerationStructureFeaturesKHR
    //    robustImageAccess : VkBool32 = 1
    //    inlineUniformBlock : VkBool32 = 1
    //    descriptorBindingInlineUniformBlockUpdateAfterBind : VkBool32 = 1
    //    pipelineCreationCacheControl : VkBool32 = 1
    //    privateData : VkBool32 = 1
    //    shaderDemoteToHelperInvocation : VkBool32 = 1
    //    shaderTerminateInvocation : VkBool32 = 1
    //    subgroupSizeControl : VkBool32 = 1
    //    computeFullSubgroups : VkBool32 = 1
    //    synchronization2 : VkBool32 = 1
    //    textureCompressionASTC_HDR : VkBool32 = 0
    //    shaderZeroInitializeWorkgroupMemory : VkBool32 = 1
    //    dynamicRendering : VkBool32 = 1
    //    shaderIntegerDotProduct : VkBool32 = 1
    //    maintenance4 : VkBool32 = 1
    //    pNext : VkPhysicalDeviceAccelerationStructureFeaturesKHR = 0000023E1A7166A0 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR(1000150013)
    //    pNext : const void* = VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    //    accelerationStructure : VkBool32 = 1
    //    accelerationStructureCaptureReplay : VkBool32 = 1
    //    accelerationStructureIndirectBuild : VkBool32 = 0
    //    accelerationStructureHostCommands : VkBool32 = 0
    //    descriptorBindingAccelerationStructureUpdateAfterBind : VkBool32 = 1
    //    pNext : VkPhysicalDeviceRayTracingPipelineFeaturesKHR = 0000023E1A7166D0 :
    //    sType : VkStructureType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR(1000347000)
    //    pNext : void* = NULL
    //    rayTracingPipeline : VkBool32 = 1
    //    rayTracingPipelineShaderGroupHandleCaptureReplay : VkBool32 = 0
    //    rayTracingPipelineShaderGroupHandleCaptureReplayMixed : VkBool32 = 0
    //    rayTracingPipelineTraceRaysIndirect : VkBool32 = 1
    //    rayTraversalPrimitiveCulling : VkBool32 = 1
    //    pAllocator : const VkAllocationCallbacks * = NULL
    //    pDevice : VkDevice * = 0000023E1ACED580


    // Turn off robustBufferAccess (WHY?)
    features2.features.robustBufferAccess = VK_FALSE;

    float priority = 1.0;
    VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueInfo.queueFamilyIndex = m_graphicsQueueIndex;
    queueInfo.queueCount       = 1;
    queueInfo.pQueuePriorities = &priority;
    
    VkDeviceCreateInfo deviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.pNext            = &features2; // This is the whole pNext chain
  
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos    = &queueInfo;
    
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(reqDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = reqDeviceExtensions.data();

    throwIfFailed(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));
    // 
    // Verify VK_SUCCESS
    // To destroy: vkDestroyDevice(m_device, nullptr); ->added
}

// That's all for now!
// Many more procedures will follow ...
