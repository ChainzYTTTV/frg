// Forge runtime: <Vulkan> implementation. Compiled into games that #include <Vulkan>.
//
// Vulkan's functions come from vulkan-1.dll, which every Vulkan graphics driver installs. It's loaded when the
// window opens, so games build without any Vulkan SDK and only need a Vulkan driver to run.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef LoadImage

#include <cstring>
#include <fstream>
#include <iterator>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "forge_vulkan.hpp"
#include "native_window.inl"

extern "C" {
FrgRlImage LoadImage(const char* fileName);   // raylib's, for loadTexture
void UnloadImage(FrgRlImage image);
void ImageFormat(FrgRlImage* image, int newFormat);
}

namespace frg::Vulkan {

namespace {

// ---- Vulkan's functions, looked up by name from the driver

#define FRG_VK_INSTANCE_FUNCTIONS(X)                \
    X(vkDestroyInstance)                            \
    X(vkEnumeratePhysicalDevices)                   \
    X(vkGetPhysicalDeviceProperties)                \
    X(vkGetPhysicalDeviceQueueFamilyProperties)     \
    X(vkGetPhysicalDeviceMemoryProperties)          \
    X(vkGetPhysicalDeviceFormatProperties)          \
    X(vkGetPhysicalDeviceSurfaceSupportKHR)         \
    X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)    \
    X(vkGetPhysicalDeviceSurfaceFormatsKHR)         \
    X(vkGetPhysicalDeviceSurfacePresentModesKHR)    \
    X(vkEnumerateDeviceExtensionProperties)         \
    X(vkCreateWin32SurfaceKHR)                      \
    X(vkDestroySurfaceKHR)                          \
    X(vkCreateDevice)                               \
    X(vkGetDeviceProcAddr)

#define FRG_VK_DEVICE_FUNCTIONS(X)  \
    X(vkDestroyDevice)              \
    X(vkGetDeviceQueue)             \
    X(vkDeviceWaitIdle)             \
    X(vkCreateSwapchainKHR)         \
    X(vkDestroySwapchainKHR)        \
    X(vkGetSwapchainImagesKHR)      \
    X(vkAcquireNextImageKHR)        \
    X(vkQueuePresentKHR)            \
    X(vkQueueSubmit)                \
    X(vkCreateImageView)            \
    X(vkDestroyImageView)           \
    X(vkCreateImage)                \
    X(vkDestroyImage)               \
    X(vkGetImageMemoryRequirements) \
    X(vkBindImageMemory)            \
    X(vkCreateBuffer)               \
    X(vkDestroyBuffer)              \
    X(vkGetBufferMemoryRequirements)\
    X(vkBindBufferMemory)           \
    X(vkAllocateMemory)             \
    X(vkFreeMemory)                 \
    X(vkMapMemory)                  \
    X(vkUnmapMemory)                \
    X(vkCreateRenderPass)           \
    X(vkDestroyRenderPass)          \
    X(vkCreateFramebuffer)          \
    X(vkDestroyFramebuffer)         \
    X(vkCreateCommandPool)          \
    X(vkDestroyCommandPool)         \
    X(vkAllocateCommandBuffers)     \
    X(vkBeginCommandBuffer)         \
    X(vkEndCommandBuffer)           \
    X(vkResetCommandBuffer)         \
    X(vkCreateSemaphore)            \
    X(vkDestroySemaphore)           \
    X(vkCreateFence)                \
    X(vkDestroyFence)               \
    X(vkWaitForFences)              \
    X(vkResetFences)                \
    X(vkCreateShaderModule)         \
    X(vkDestroyShaderModule)        \
    X(vkCreatePipelineLayout)       \
    X(vkDestroyPipelineLayout)      \
    X(vkCreateGraphicsPipelines)    \
    X(vkDestroyPipeline)            \
    X(vkCmdBeginRenderPass)         \
    X(vkCmdEndRenderPass)           \
    X(vkCmdSetViewport)             \
    X(vkCmdSetScissor)              \
    X(vkCmdBindPipeline)            \
    X(vkCmdBindVertexBuffers)       \
    X(vkCmdBindIndexBuffer)         \
    X(vkCmdPushConstants)           \
    X(vkCmdDraw)                    \
    X(vkCmdDrawIndexed)             \
    X(vkCmdPipelineBarrier)         \
    X(vkCmdCopyImageToBuffer)       \
    X(vkCmdCopyBufferToImage)       \
    X(vkFreeCommandBuffers)         \
    X(vkQueueWaitIdle)              \
    X(vkCreateSampler)              \
    X(vkDestroySampler)             \
    X(vkCreateDescriptorSetLayout)  \
    X(vkDestroyDescriptorSetLayout) \
    X(vkCreateDescriptorPool)       \
    X(vkDestroyDescriptorPool)      \
    X(vkAllocateDescriptorSets)     \
    X(vkUpdateDescriptorSets)       \
    X(vkCmdBindDescriptorSets)

#define FRG_VK_DECLARE(name) PFN_##name name = nullptr;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
PFN_vkCreateInstance vkCreateInstance = nullptr;
FRG_VK_INSTANCE_FUNCTIONS(FRG_VK_DECLARE)
FRG_VK_DEVICE_FUNCTIONS(FRG_VK_DECLARE)
#undef FRG_VK_DECLARE

constexpr int kFramesInFlight = 2;

struct Buffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct Pipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    int32_t pushFloats = 0;
};

// An image from loadTexture, ready to read in a fragment shader: `layout(set = 0, binding = 0) uniform sampler2D`.
// It has two descriptor sets: read smoothly, or with sharp pixels (setTextureFilter).
struct Texture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkDescriptorSet sets[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
};

struct Frame {
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkSemaphore imageReady = VK_NULL_HANDLE;
    VkFence done = VK_NULL_HANDLE;
};

struct State {
    HMODULE lib = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties gpuProps{};
    VkDevice device = VK_NULL_HANDLE;
    uint32_t queueFamily = 0;
    VkQueue queue = VK_NULL_HANDLE;

    // the swapchain: the window's images, drawn into in turn
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkSemaphore> renderDone;  // one per image
    bool canScreenshot = false;
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    VkImage depth = VK_NULL_HANDLE;
    VkDeviceMemory depthMemory = VK_NULL_HANDLE;
    VkImageView depthView = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkCommandPool pool = VK_NULL_HANDLE;
    Frame frames[kFramesInFlight];
    int frame = 0;
    uint32_t image = 0;
    bool recording = false;
    bool submitted = false;  // a finished frame is waiting to be shown
    bool vsync = true;
    bool rebuild = false;    // the swapchain needs making again (vsync changed, or the window changed)

    Buffer shot;             // the screenshot is copied here
    bool shotTaken = false;

    // settings for the next createPipeline
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    bool depthTest = true;
    VkCullModeFlags cull = VK_CULL_MODE_NONE;
    bool blending = false;

    std::vector<VkShaderModule> shaders;
    std::vector<Pipeline> pipelines;
    std::vector<Buffer> buffers;

    // textures: every pipeline has room for one (set 0, binding 0)
    VkDescriptorSetLayout textureLayout = VK_NULL_HANDLE;
    VkSampler samplers[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};   // smooth, sharp pixels
    std::vector<VkDescriptorPool> descriptorPools;
    uint32_t setsLeft = 0;                // in the newest pool
    std::vector<Texture> textures;
    bool smooth = true;
    int32_t currentPipeline = 0;          // bound with bindPipeline (bindTexture needs its layout)
};

State& st() {
    static State s;
    return s;
}

void need(const char* what) {
    if (!st().device) native::fail(what, "the window isn't open yet - call Vulkan.openWindow(width, height, \"title\") first");
}

void needRecording(const char* what) {
    need(what);
    if (!st().recording) native::fail(what, "this only works between Vulkan.beginFrame() and Vulkan.endFrame()");
}

void check(VkResult r, const char* what, const char* doing) {
    if (r != VK_SUCCESS) native::fail(what, std::string("Vulkan couldn't ") + doing + " (error " + str((int64_t)r) + ")");
}

// Handles are positions in these lists, counting from 1 (0 means "nothing").
template <class T>
T& lookup(std::vector<T>& v, int32_t handle, const char* what, const char* kind) {
    if (handle < 1 || handle > (int32_t)v.size()) native::fail(what, std::string("there's no ") + kind + " with handle " + str(handle));
    return v[handle - 1];
}

uint32_t findMemory(uint32_t typeBits, VkMemoryPropertyFlags want, const char* what) {
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(st().gpu, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
        if ((typeBits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & want) == want) return i;
    native::fail(what, "the graphics card has no suitable memory");
}

Buffer makeBuffer(VkDeviceSize bytes, VkBufferUsageFlags usage, const void* data, const char* what) {
    State& s = st();
    Buffer b;
    VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bi.size = bytes;
    bi.usage = usage;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    check(vkCreateBuffer(s.device, &bi, nullptr, &b.buffer), what, "create the buffer");
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(s.device, b.buffer, &req);
    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = req.size;
    // Memory both the CPU and the graphics card can see: simple, and fine for a game's few buffers.
    ai.memoryTypeIndex = findMemory(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, what);
    check(vkAllocateMemory(s.device, &ai, nullptr, &b.memory), what, "get memory for the buffer");
    vkBindBufferMemory(s.device, b.buffer, b.memory, 0);
    if (data) {
        void* p = nullptr;
        check(vkMapMemory(s.device, b.memory, 0, bytes, 0, &p), what, "write to the buffer");
        std::memcpy(p, data, (size_t)bytes);
        vkUnmapMemory(s.device, b.memory);
    }
    return b;
}

void destroyBuffer(Buffer& b) {
    if (b.buffer) vkDestroyBuffer(st().device, b.buffer, nullptr);
    if (b.memory) vkFreeMemory(st().device, b.memory, nullptr);
    b = Buffer{};
}

VkImageView makeView(VkImage image, VkFormat format, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vi.image = image;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = format;
    vi.subresourceRange = {aspect, 0, 1, 0, 1};
    VkImageView view = VK_NULL_HANDLE;
    check(vkCreateImageView(st().device, &vi, nullptr, &view), "Vulkan.openWindow()", "make an image view");
    return view;
}

// ---- textures

// The descriptor set layout every pipeline has (one texture), and the two samplers. Made the first time either is needed.
void ensureTextureLayout() {
    State& s = st();
    if (s.textureLayout) return;
    const char* what = "Vulkan.createPipeline()";
    VkDescriptorSetLayoutBinding b{};
    b.binding = 0;
    b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b.descriptorCount = 1;
    b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    li.bindingCount = 1;
    li.pBindings = &b;
    check(vkCreateDescriptorSetLayout(s.device, &li, nullptr, &s.textureLayout), what, "make the texture layout");
    for (int i = 0; i < 2; i++) {
        VkSamplerCreateInfo si{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        si.magFilter = si.minFilter = i == 0 ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        si.maxLod = 0;
        check(vkCreateSampler(s.device, &si, nullptr, &s.samplers[i]), what, "make a sampler");
    }
}

// Two descriptor sets for a texture, from the newest pool (a new pool when it's full).
void allocateSets(VkDescriptorSet out[2], const char* what) {
    State& s = st();
    if (s.setsLeft < 2) {
        VkDescriptorPoolSize size{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128};
        VkDescriptorPoolCreateInfo pi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        pi.maxSets = 128;
        pi.poolSizeCount = 1;
        pi.pPoolSizes = &size;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        check(vkCreateDescriptorPool(s.device, &pi, nullptr, &pool), what, "make a descriptor pool");
        s.descriptorPools.push_back(pool);
        s.setsLeft = 128;
    }
    VkDescriptorSetLayout layouts[2] = {s.textureLayout, s.textureLayout};
    VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ai.descriptorPool = s.descriptorPools.back();
    ai.descriptorSetCount = 2;
    ai.pSetLayouts = layouts;
    check(vkAllocateDescriptorSets(s.device, &ai, out), what, "get descriptor sets for the texture");
    s.setsLeft -= 2;
}

// Records one-off commands (copying a picture to the graphics card), runs them and waits for them to finish.
template <class F>
void runOnce(F record, const char* what) {
    State& s = st();
    VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cai.commandPool = s.pool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    check(vkAllocateCommandBuffers(s.device, &cai, &cmd), what, "make a command buffer");
    VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &bi);
    record(cmd);
    check(vkEndCommandBuffer(cmd), what, "finish the commands");
    VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    check(vkQueueSubmit(s.queue, 1, &si, VK_NULL_HANDLE), what, "send the commands to the graphics card");
    vkQueueWaitIdle(s.queue);
    vkFreeCommandBuffers(s.device, s.pool, 1, &cmd);
}

void destroyTextures() {
    State& s = st();
    for (Texture& t : s.textures) {
        if (t.view) vkDestroyImageView(s.device, t.view, nullptr);
        if (t.image) vkDestroyImage(s.device, t.image, nullptr);
        if (t.memory) vkFreeMemory(s.device, t.memory, nullptr);
    }
    s.textures.clear();
    for (VkDescriptorPool p : s.descriptorPools) vkDestroyDescriptorPool(s.device, p, nullptr);
    s.descriptorPools.clear();
    s.setsLeft = 0;
    for (VkSampler& sm : s.samplers) {
        if (sm) vkDestroySampler(s.device, sm, nullptr);
        sm = VK_NULL_HANDLE;
    }
    if (s.textureLayout) vkDestroyDescriptorSetLayout(s.device, s.textureLayout, nullptr);
    s.textureLayout = VK_NULL_HANDLE;
}

// ---- the swapchain, and everything sized to match it

void destroySwapchain() {
    State& s = st();
    for (auto f : s.framebuffers) vkDestroyFramebuffer(s.device, f, nullptr);
    for (auto v : s.views) vkDestroyImageView(s.device, v, nullptr);
    for (auto sem : s.renderDone) vkDestroySemaphore(s.device, sem, nullptr);
    s.framebuffers.clear();
    s.views.clear();
    s.renderDone.clear();
    s.images.clear();
    if (s.depthView) vkDestroyImageView(s.device, s.depthView, nullptr);
    if (s.depth) vkDestroyImage(s.device, s.depth, nullptr);
    if (s.depthMemory) vkFreeMemory(s.device, s.depthMemory, nullptr);
    s.depthView = VK_NULL_HANDLE;
    s.depth = VK_NULL_HANDLE;
    s.depthMemory = VK_NULL_HANDLE;
    destroyBuffer(s.shot);
    if (s.swapchain) vkDestroySwapchainKHR(s.device, s.swapchain, nullptr);
    s.swapchain = VK_NULL_HANDLE;
}

// False when the window can't be drawn into right now (it's minimized).
bool createSwapchain() {
    const char* what = "Vulkan.openWindow()";
    State& s = st();
    VkSurfaceCapabilitiesKHR caps;
    check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s.gpu, s.surface, &caps), what, "ask about the window");
    s.extent = caps.currentExtent;
    if (s.extent.width == 0xFFFFFFFFu) s.extent = {(uint32_t)native::width(), (uint32_t)native::height()};
    if (s.extent.width == 0 || s.extent.height == 0) return false;

    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(s.gpu, s.surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(s.gpu, s.surface, &count, modes.data());
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;  // waits for the screen: always available
    if (!s.vsync) {
        for (auto m : modes)
            if (m == VK_PRESENT_MODE_IMMEDIATE_KHR) mode = m;
        for (auto m : modes)
            if (m == VK_PRESENT_MODE_MAILBOX_KHR) mode = m;
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;
    s.canScreenshot = (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;

    VkSwapchainCreateInfoKHR si{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    si.surface = s.surface;
    si.minImageCount = imageCount;
    si.imageFormat = s.format;
    si.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    si.imageExtent = s.extent;
    si.imageArrayLayers = 1;
    si.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (s.canScreenshot ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0);
    si.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    si.preTransform = caps.currentTransform;
    si.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    si.presentMode = mode;
    si.clipped = VK_TRUE;
    check(vkCreateSwapchainKHR(s.device, &si, nullptr, &s.swapchain), what, "set up the window's images");

    vkGetSwapchainImagesKHR(s.device, s.swapchain, &count, nullptr);
    s.images.resize(count);
    vkGetSwapchainImagesKHR(s.device, s.swapchain, &count, s.images.data());

    // the depth buffer
    VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = s.depthFormat;
    ii.extent = {s.extent.width, s.extent.height, 1};
    ii.mipLevels = 1;
    ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
    ii.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    check(vkCreateImage(s.device, &ii, nullptr, &s.depth), what, "make the depth buffer");
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(s.device, s.depth, &req);
    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = findMemory(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, what);
    check(vkAllocateMemory(s.device, &ai, nullptr, &s.depthMemory), what, "get memory for the depth buffer");
    vkBindImageMemory(s.device, s.depth, s.depthMemory, 0);
    s.depthView = makeView(s.depth, s.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkSemaphoreCreateInfo semInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (VkImage img : s.images) {
        s.views.push_back(makeView(img, s.format, VK_IMAGE_ASPECT_COLOR_BIT));
        VkImageView attachments[2] = {s.views.back(), s.depthView};
        VkFramebufferCreateInfo fi{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fi.renderPass = s.renderPass;
        fi.attachmentCount = 2;
        fi.pAttachments = attachments;
        fi.width = s.extent.width;
        fi.height = s.extent.height;
        fi.layers = 1;
        VkFramebuffer fb = VK_NULL_HANDLE;
        check(vkCreateFramebuffer(s.device, &fi, nullptr, &fb), what, "make a framebuffer");
        s.framebuffers.push_back(fb);
        VkSemaphore sem = VK_NULL_HANDLE;
        check(vkCreateSemaphore(s.device, &semInfo, nullptr, &sem), what, "make a semaphore");
        s.renderDone.push_back(sem);
    }
    return true;
}

bool rebuildSwapchain() {
    vkDeviceWaitIdle(st().device);
    destroySwapchain();
    st().rebuild = false;
    return createSwapchain();
}

// ---- what the shared window calls once a frame

void present() {
    State& s = st();
    if (!s.submitted) return;
    s.submitted = false;
    VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &s.renderDone[s.image];
    pi.swapchainCount = 1;
    pi.pSwapchains = &s.swapchain;
    pi.pImageIndices = &s.image;
    VkResult r = vkQueuePresentKHR(s.queue, &pi);
    if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR) s.rebuild = true;
    s.frame = (s.frame + 1) % kFramesInFlight;
}

void screenshot(const std::string& path) {
    State& s = st();
    if (!s.shotTaken) return;
    s.shotTaken = false;
    vkWaitForFences(s.device, 1, &s.frames[s.frame].done, VK_TRUE, UINT64_MAX);
    void* p = nullptr;
    if (vkMapMemory(s.device, s.shot.memory, 0, VK_WHOLE_SIZE, 0, &p) != VK_SUCCESS) return;
    size_t n = (size_t)s.extent.width * s.extent.height * 4;
    std::vector<unsigned char> px((unsigned char*)p, (unsigned char*)p + n);
    vkUnmapMemory(s.device, s.shot.memory);
    if (s.format == VK_FORMAT_B8G8R8A8_UNORM)  // blue-green-red: swap to red-green-blue
        for (size_t i = 0; i < n; i += 4) std::swap(px[i], px[i + 2]);
    native::savePng(path, px.data(), (int)s.extent.width, (int)s.extent.height);
}

}  // namespace

void openWindow_(int32_t width, int32_t height, const std::string& title) {
    const char* what = "Vulkan.openWindow()";
    State& s = st();
    s.lib = LoadLibraryA("vulkan-1.dll");
    if (!s.lib) native::fail(what, "this computer has no Vulkan driver - updating the graphics card's driver usually adds one");
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)(void*)GetProcAddress(s.lib, "vkGetInstanceProcAddr");
    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
    if (!vkCreateInstance) native::fail(what, "the Vulkan driver is broken (it has no vkCreateInstance)");

    native::open(width, height, title, &present, &screenshot);

    // the instance: Forge's connection to Vulkan
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = title.c_str();
    app.pEngineName = "Forge";
    app.apiVersion = VK_API_VERSION_1_0;
    const char* instanceExts[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo = &app;
    ici.enabledExtensionCount = 2;
    ici.ppEnabledExtensionNames = instanceExts;
    check(vkCreateInstance(&ici, nullptr, &s.instance), what, "start (no driver supports drawing into windows)");
#define FRG_VK_LOAD(name)                                                 \
    name = (PFN_##name)vkGetInstanceProcAddr(s.instance, #name);          \
    if (!name) native::fail(what, "the Vulkan driver is missing " #name);
    FRG_VK_INSTANCE_FUNCTIONS(FRG_VK_LOAD)
#undef FRG_VK_LOAD

    VkWin32SurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    sci.hinstance = GetModuleHandleW(nullptr);
    sci.hwnd = (HWND)native::handle();
    check(vkCreateWin32SurfaceKHR(s.instance, &sci, nullptr, &s.surface), what, "draw into the window");

    // the graphics card: one that can draw into this window, with a real (discrete) card preferred
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(s.instance, &count, nullptr);
    std::vector<VkPhysicalDevice> gpus(count);
    vkEnumeratePhysicalDevices(s.instance, &count, gpus.data());
    int bestScore = -1;
    for (VkPhysicalDevice gpu : gpus) {
        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> exts(extCount);
        vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extCount, exts.data());
        bool hasSwapchain = false;
        for (auto& e : exts)
            if (std::strcmp(e.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) hasSwapchain = true;
        if (!hasSwapchain) continue;
        uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &qCount, nullptr);
        std::vector<VkQueueFamilyProperties> qs(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &qCount, qs.data());
        for (uint32_t i = 0; i < qCount; i++) {
            VkBool32 canPresent = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, s.surface, &canPresent);
            if (!(qs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) || !canPresent) continue;
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(gpu, &props);
            int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 2 : props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 1 : 0;
            if (score > bestScore) {
                bestScore = score;
                s.gpu = gpu;
                s.gpuProps = props;
                s.queueFamily = i;
            }
            break;
        }
    }
    if (!s.gpu) native::fail(what, "no graphics card here can draw into a window with Vulkan");

    float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = s.queueFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &priority;
    const char* deviceExts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = deviceExts;
    check(vkCreateDevice(s.gpu, &dci, nullptr, &s.device), what, "start the graphics card");
#define FRG_VK_LOAD(name)                                          \
    name = (PFN_##name)vkGetDeviceProcAddr(s.device, #name);       \
    if (!name) native::fail(what, "the Vulkan driver is missing " #name);
    FRG_VK_DEVICE_FUNCTIONS(FRG_VK_LOAD)
#undef FRG_VK_LOAD
    vkGetDeviceQueue(s.device, s.queueFamily, 0, &s.queue);

    // the image format: plain 8-bit color, as the other toolkits use
    vkGetPhysicalDeviceSurfaceFormatsKHR(s.gpu, s.surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(s.gpu, s.surface, &count, formats.data());
    s.format = formats.empty() || formats[0].format == VK_FORMAT_UNDEFINED ? VK_FORMAT_B8G8R8A8_UNORM : formats[0].format;
    for (auto& f : formats)
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_R8G8B8A8_UNORM) {
            s.format = f.format;
            break;
        }

    for (VkFormat f : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM}) {
        VkFormatProperties fp;
        vkGetPhysicalDeviceFormatProperties(s.gpu, f, &fp);
        if (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            s.depthFormat = f;
            break;
        }
    }

    // the render pass: each frame clears the color and depth, draws, then is shown in the window
    VkAttachmentDescription att[2]{};
    att[0].format = s.format;
    att[0].samples = VK_SAMPLE_COUNT_1_BIT;
    att[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    att[1] = att[0];
    att[1].format = s.depthFormat;
    att[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub{};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &colorRef;
    sub.pDepthStencilAttachment = &depthRef;
    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo rpi{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rpi.attachmentCount = 2;
    rpi.pAttachments = att;
    rpi.subpassCount = 1;
    rpi.pSubpasses = &sub;
    rpi.dependencyCount = 1;
    rpi.pDependencies = &dep;
    check(vkCreateRenderPass(s.device, &rpi, nullptr, &s.renderPass), what, "make the render pass");

    createSwapchain();

    // per-frame command buffers and syncing, so the CPU can record one frame while the GPU draws the last
    VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pci.queueFamilyIndex = s.queueFamily;
    check(vkCreateCommandPool(s.device, &pci, nullptr, &s.pool), what, "make the command pool");
    for (Frame& f : s.frames) {
        VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cai.commandPool = s.pool;
        cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cai.commandBufferCount = 1;
        check(vkAllocateCommandBuffers(s.device, &cai, &f.cmd), what, "make a command buffer");
        VkSemaphoreCreateInfo semInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        check(vkCreateSemaphore(s.device, &semInfo, nullptr, &f.imageReady), what, "make a semaphore");
        VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        check(vkCreateFence(s.device, &fci, nullptr, &f.done), what, "make a fence");
    }
    addHook(shutdownHooks(), &closeWindow_);
}

void closeWindow_() {
    State& s = st();
    if (s.device) {
        vkDeviceWaitIdle(s.device);
        for (auto& p : s.pipelines) {
            vkDestroyPipeline(s.device, p.pipeline, nullptr);
            vkDestroyPipelineLayout(s.device, p.layout, nullptr);
        }
        for (auto m : s.shaders) vkDestroyShaderModule(s.device, m, nullptr);
        for (auto& b : s.buffers) destroyBuffer(b);
        destroyTextures();
        s.currentPipeline = 0;
        s.pipelines.clear();
        s.shaders.clear();
        s.buffers.clear();
        for (Frame& f : s.frames) {
            vkDestroySemaphore(s.device, f.imageReady, nullptr);
            vkDestroyFence(s.device, f.done, nullptr);
            f = Frame{};
        }
        if (s.pool) vkDestroyCommandPool(s.device, s.pool, nullptr);
        s.pool = VK_NULL_HANDLE;
        destroySwapchain();
        if (s.renderPass) vkDestroyRenderPass(s.device, s.renderPass, nullptr);
        s.renderPass = VK_NULL_HANDLE;
        vkDestroyDevice(s.device, nullptr);
        s.device = VK_NULL_HANDLE;
    }
    if (s.surface) vkDestroySurfaceKHR(s.instance, s.surface, nullptr);
    if (s.instance) vkDestroyInstance(s.instance, nullptr);
    s.surface = VK_NULL_HANDLE;
    s.instance = VK_NULL_HANDLE;
    s.recording = s.submitted = false;
    native::close();
}

void setVSync_(bool on) {
    if (st().vsync == on) return;
    st().vsync = on;
    st().rebuild = true;  // takes effect at the next beginFrame
}

std::string deviceName_() {
    need("Vulkan.deviceName()");
    return st().gpuProps.deviceName;
}

void setTopology_(int32_t topology) {
    if (topology < 0 || topology > 5) native::fail("Vulkan.setTopology()", "use Vulkan.TRIANGLES, Vulkan.LINES, Vulkan.POINTS and so on");
    st().topology = (VkPrimitiveTopology)topology;
}
void setDepthTest_(bool on) { st().depthTest = on; }
void setBlending_(bool on) { st().blending = on; }
void setCulling_(int32_t mode) {
    if (mode < 0 || mode > 2) native::fail("Vulkan.setCulling()", "use Vulkan.CULL_NONE, Vulkan.CULL_FRONT or Vulkan.CULL_BACK");
    st().cull = (VkCullModeFlags)mode;
}

// A compiled shader (.spv). `frg build` makes a .spv next to each .vert/.frag file in the game's folder.
int32_t loadShader_(const std::string& path) {
    const char* what = "Vulkan.loadShader()";
    need(what);
    std::string file = native::findGameFile(path);
    if (file.empty()) {
        std::string hint = path.size() > 4 && path.compare(path.size() - 4, 4, ".spv") == 0
                               ? " - frg build makes it from the shader file without the .spv (like player.vert)"
                               : " - put it in the game's folder (next to main.frg)";
        native::fail(what, "can't find the file '" + path + "'" + hint);
    }
    std::ifstream in(file, std::ios::binary);
    std::vector<char> code((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (code.size() < 4 || code.size() % 4 != 0 || *(const uint32_t*)code.data() != 0x07230203u)
        native::fail(what, "'" + path + "' isn't a compiled SPIR-V shader - load the .spv file, not the .vert/.frag");
    VkShaderModuleCreateInfo mi{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    mi.codeSize = code.size();
    mi.pCode = (const uint32_t*)code.data();
    VkShaderModule m = VK_NULL_HANDLE;
    check(vkCreateShaderModule(st().device, &mi, nullptr, &m), what, "load the shader");
    st().shaders.push_back(m);
    return (int32_t)st().shaders.size();
}

// The vertex shader's inputs are locations 0, 1, 2... in one vertex buffer; attributeSizes says how many floats
// each takes (1-4). pushConstantFloats is how many floats pushConstants() can send (up to 32 everywhere).
int32_t createPipeline_(int32_t vertexShader, int32_t fragmentShader, const List<int32_t>& attributeSizes, int32_t pushConstantFloats) {
    const char* what = "Vulkan.createPipeline()";
    need(what);
    State& s = st();
    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = lookup(s.shaders, vertexShader, what, "shader");
    stages[0].pName = "main";
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = lookup(s.shaders, fragmentShader, what, "shader");
    stages[1].pName = "main";

    static const VkFormat formats[] = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
    std::vector<VkVertexInputAttributeDescription> attrs;
    uint32_t offset = 0;
    for (size_t i = 0; i < attributeSizes.size(); i++) {
        int n = attributeSizes[i];
        if (n < 1 || n > 4) native::fail(what, "each attribute size is how many floats it takes: 1, 2, 3 or 4");
        attrs.push_back({(uint32_t)i, 0, formats[n - 1], offset});
        offset += (uint32_t)n * 4;
    }
    VkVertexInputBindingDescription binding{0, offset, VK_VERTEX_INPUT_RATE_VERTEX};
    VkPipelineVertexInputStateCreateInfo vin{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    if (!attrs.empty()) {
        vin.vertexBindingDescriptionCount = 1;
        vin.pVertexBindingDescriptions = &binding;
        vin.vertexAttributeDescriptionCount = (uint32_t)attrs.size();
        vin.pVertexAttributeDescriptions = attrs.data();
    }

    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = s.topology;
    VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = s.cull;
    // Corners going counter-clockwise on screen face you (the same as OpenGL, thanks to perspective()'s flip).
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    ds.depthTestEnable = s.depthTest;
    ds.depthWriteEnable = s.depthTest;
    ds.depthCompareOp = VK_COMPARE_OP_LESS;
    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (s.blending) {
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1;
    cb.pAttachments = &blend;
    VkDynamicState dyn[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dy{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dy.dynamicStateCount = 2;
    dy.pDynamicStates = dyn;

    Pipeline p;
    p.pushFloats = pushConstantFloats;
    uint32_t maxFloats = s.gpuProps.limits.maxPushConstantsSize / 4;
    if (pushConstantFloats < 0 || (uint32_t)pushConstantFloats > maxFloats)
        native::fail(what, "this graphics card allows 0 to " + str((int64_t)maxFloats) + " push-constant floats");
    VkPushConstantRange range{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint32_t)pushConstantFloats * 4};
    VkPipelineLayoutCreateInfo li{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    ensureTextureLayout();   // every pipeline can read one texture (set 0, binding 0), whether it does or not
    li.setLayoutCount = 1;
    li.pSetLayouts = &s.textureLayout;
    if (pushConstantFloats > 0) {
        li.pushConstantRangeCount = 1;
        li.pPushConstantRanges = &range;
    }
    check(vkCreatePipelineLayout(s.device, &li, nullptr, &p.layout), what, "make the pipeline layout");

    VkGraphicsPipelineCreateInfo gi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    gi.stageCount = 2;
    gi.pStages = stages;
    gi.pVertexInputState = &vin;
    gi.pInputAssemblyState = &ia;
    gi.pViewportState = &vp;
    gi.pRasterizationState = &rs;
    gi.pMultisampleState = &ms;
    gi.pDepthStencilState = &ds;
    gi.pColorBlendState = &cb;
    gi.pDynamicState = &dy;
    gi.layout = p.layout;
    gi.renderPass = s.renderPass;
    gi.subpass = 0;
    VkResult r = vkCreateGraphicsPipelines(s.device, VK_NULL_HANDLE, 1, &gi, nullptr, &p.pipeline);
    if (r != VK_SUCCESS) {
        vkDestroyPipelineLayout(s.device, p.layout, nullptr);
        check(r, what, "make the pipeline - do the shaders' inputs match the attribute sizes?");
    }
    s.pipelines.push_back(p);
    return (int32_t)s.pipelines.size();
}

int32_t createVertexBuffer_(const List<float>& data) {
    const char* what = "Vulkan.createVertexBuffer()";
    need(what);
    if (data.empty()) native::fail(what, "a buffer can't be empty");
    st().buffers.push_back(makeBuffer(data.size() * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data.data(), what));
    return (int32_t)st().buffers.size();
}

int32_t createIndexBuffer_(const List<int32_t>& indices) {
    const char* what = "Vulkan.createIndexBuffer()";
    need(what);
    if (indices.empty()) native::fail(what, "a buffer can't be empty");
    st().buffers.push_back(makeBuffer(indices.size() * 4, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices.data(), what));
    return (int32_t)st().buffers.size();
}

// Starts recording this frame's drawing, cleared to one color. False means skip drawing this frame
// (the window is minimized, or its images are being remade) - just go round the loop again.
// Loads a picture from the game's folder onto the graphics card. (0, 0) in the shader is its top-left corner.
int32_t loadTexture_(const std::string& path) {
    const char* what = "Vulkan.loadTexture()";
    need(what);
    State& s = st();
    std::string file = native::findGameFile(path);
    if (file.empty()) native::fail(what, "can't find the file '" + path + "' - put it in the game's folder (next to main.frg)");
    FrgRlImage img = LoadImage(file.c_str());
    if (!img.data) native::fail(what, "couldn't read '" + path + "' as an image");
    ImageFormat(&img, 7);  // RGBA, 8 bits each
    uint32_t w = (uint32_t)img.width, h = (uint32_t)img.height;
    Buffer staging = makeBuffer((VkDeviceSize)w * h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, img.data, what);
    UnloadImage(img);
    ensureTextureLayout();

    Texture t;
    VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = VK_FORMAT_R8G8B8A8_UNORM;
    ii.extent = {w, h, 1};
    ii.mipLevels = 1;
    ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
    ii.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    check(vkCreateImage(s.device, &ii, nullptr, &t.image), what, "create the texture");
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(s.device, t.image, &req);
    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = findMemory(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, what);
    check(vkAllocateMemory(s.device, &ai, nullptr, &t.memory), what, "get memory for the texture");
    vkBindImageMemory(s.device, t.image, t.memory, 0);

    // copy it in: undefined -> ready to copy into -> copied -> ready for shaders to read
    runOnce([&](VkCommandBuffer cmd) {
        VkImageMemoryBarrier bar{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        bar.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        bar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        bar.srcQueueFamilyIndex = bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bar.image = t.image;
        bar.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        bar.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &bar);
        VkBufferImageCopy region{};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = {w, h, 1};
        vkCmdCopyBufferToImage(cmd, staging.buffer, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        bar.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        bar.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        bar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &bar);
    }, what);
    destroyBuffer(staging);

    VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vi.image = t.image;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = VK_FORMAT_R8G8B8A8_UNORM;
    vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    check(vkCreateImageView(s.device, &vi, nullptr, &t.view), what, "make the texture's view");

    allocateSets(t.sets, what);
    for (int i = 0; i < 2; i++) {
        VkDescriptorImageInfo info{s.samplers[i], t.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        VkWriteDescriptorSet wr{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        wr.dstSet = t.sets[i];
        wr.dstBinding = 0;
        wr.descriptorCount = 1;
        wr.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        wr.pImageInfo = &info;
        vkUpdateDescriptorSets(s.device, 1, &wr, 0, nullptr);
    }
    s.textures.push_back(t);
    return (int32_t)s.textures.size();
}

// The draws after this read this texture (in the pipeline bound with bindPipeline).
void bindTexture_(int32_t texture) {
    const char* what = "Vulkan.bindTexture()";
    needRecording(what);
    State& s = st();
    if (s.currentPipeline == 0) native::fail(what, "bind a pipeline first (Vulkan.bindPipeline), then its texture");
    Texture& t = lookup(s.textures, texture, what, "texture");
    Pipeline& p = lookup(s.pipelines, s.currentPipeline, what, "pipeline");
    vkCmdBindDescriptorSets(s.frames[s.frame].cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p.layout, 0, 1, &t.sets[s.smooth ? 0 : 1], 0, nullptr);
}

// true (the start): smooth when stretched; false: sharp pixels, for pixel art. For the textures bound after it.
void setTextureFilter_(bool smooth) {
    need("Vulkan.setTextureFilter()");
    st().smooth = smooth;
}

bool beginFrame_(float r, float g, float b, float a) {
    const char* what = "Vulkan.beginFrame()";
    need(what);
    State& s = st();
    if (s.recording) native::fail(what, "the last frame wasn't finished - call Vulkan.endFrame() after drawing");
    if (s.submitted) native::fail(what, "a frame was already drawn this time round the loop - check Vulkan.shouldClose() between frames");
    if ((s.rebuild || !s.swapchain) && !rebuildSwapchain()) return false;

    Frame& f = s.frames[s.frame];
    vkWaitForFences(s.device, 1, &f.done, VK_TRUE, UINT64_MAX);
    VkResult res = vkAcquireNextImageKHR(s.device, s.swapchain, UINT64_MAX, f.imageReady, VK_NULL_HANDLE, &s.image);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        s.rebuild = true;
        return false;
    }
    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) check(res, what, "get the window's next image");
    vkResetFences(s.device, 1, &f.done);
    vkResetCommandBuffer(f.cmd, 0);
    VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(f.cmd, &bi);

    VkClearValue clears[2];
    clears[0].color = {{r, g, b, a}};
    clears[1].depthStencil = {1.0f, 0};
    VkRenderPassBeginInfo rbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rbi.renderPass = s.renderPass;
    rbi.framebuffer = s.framebuffers[s.image];
    rbi.renderArea = {{0, 0}, s.extent};
    rbi.clearValueCount = 2;
    rbi.pClearValues = clears;
    vkCmdBeginRenderPass(f.cmd, &rbi, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport{0, 0, (float)s.extent.width, (float)s.extent.height, 0, 1};
    VkRect2D scissor{{0, 0}, s.extent};
    vkCmdSetViewport(f.cmd, 0, 1, &viewport);
    vkCmdSetScissor(f.cmd, 0, 1, &scissor);
    s.recording = true;
    return true;
}

void bindPipeline_(int32_t pipeline) {
    needRecording("Vulkan.bindPipeline()");
    State& s = st();
    vkCmdBindPipeline(s.frames[s.frame].cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lookup(s.pipelines, pipeline, "Vulkan.bindPipeline()", "pipeline").pipeline);
    s.currentPipeline = pipeline;
}

void bindVertexBuffer_(int32_t buffer) {
    needRecording("Vulkan.bindVertexBuffer()");
    State& s = st();
    VkBuffer b = lookup(s.buffers, buffer, "Vulkan.bindVertexBuffer()", "buffer").buffer;
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(s.frames[s.frame].cmd, 0, 1, &b, &offset);
}

void bindIndexBuffer_(int32_t buffer) {
    needRecording("Vulkan.bindIndexBuffer()");
    State& s = st();
    vkCmdBindIndexBuffer(s.frames[s.frame].cmd, lookup(s.buffers, buffer, "Vulkan.bindIndexBuffer()", "buffer").buffer, 0, VK_INDEX_TYPE_UINT32);
}

// Sends a few floats straight to the shaders (their `layout(push_constant) uniform` block) for the next draws.
void pushConstants_(int32_t pipeline, const List<float>& data) {
    const char* what = "Vulkan.pushConstants()";
    needRecording(what);
    State& s = st();
    Pipeline& p = lookup(s.pipelines, pipeline, what, "pipeline");
    if ((int32_t)data.size() > p.pushFloats)
        native::fail(what, "this pipeline was made for " + str(p.pushFloats) + " push-constant floats, but got " + str((int64_t)data.size()));
    if (data.empty()) return;
    vkCmdPushConstants(s.frames[s.frame].cmd, p.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       (uint32_t)data.size() * 4, data.data());
}

void draw_(int32_t vertexCount, int32_t firstVertex) {
    needRecording("Vulkan.draw()");
    vkCmdDraw(st().frames[st().frame].cmd, (uint32_t)vertexCount, 1, (uint32_t)firstVertex, 0);
}

void drawIndexed_(int32_t indexCount, int32_t firstIndex) {
    needRecording("Vulkan.drawIndexed()");
    vkCmdDrawIndexed(st().frames[st().frame].cmd, (uint32_t)indexCount, 1, (uint32_t)firstIndex, 0, 0);
}

// Finishes the frame and sends it to the graphics card. It appears in the window at the next shouldClose().
void endFrame_() {
    const char* what = "Vulkan.endFrame()";
    needRecording(what);
    State& s = st();
    Frame& f = s.frames[s.frame];
    vkCmdEndRenderPass(f.cmd);

    if (s.canScreenshot && native::screenshotDue()) {  // copy the finished image out for FRG_AUTOTEST_SHOT
        if (!s.shot.buffer)
            s.shot = makeBuffer((VkDeviceSize)s.extent.width * s.extent.height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, what);
        VkImageMemoryBarrier bar{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        bar.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        bar.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        bar.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        bar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        bar.srcQueueFamilyIndex = bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bar.image = s.images[s.image];
        bar.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCmdPipelineBarrier(f.cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &bar);
        VkBufferImageCopy region{};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = {s.extent.width, s.extent.height, 1};
        vkCmdCopyImageToBuffer(f.cmd, s.images[s.image], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, s.shot.buffer, 1, &region);
        bar.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        bar.dstAccessMask = 0;
        bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        bar.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vkCmdPipelineBarrier(f.cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &bar);
        s.shotTaken = true;
    }

    check(vkEndCommandBuffer(f.cmd), what, "finish recording the frame");
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &f.imageReady;
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &f.cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &s.renderDone[s.image];
    check(vkQueueSubmit(s.queue, 1, &si, f.done), what, "send the frame to the graphics card");
    s.recording = false;
    s.submitted = true;
}

}  // namespace frg::Vulkan
