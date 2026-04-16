/**
 * @file VKTexture.h
 * @brief Vulkan纹理实现
 * @author hxxcxx
 * @date 2026-04-15
 */

#pragma once

#include "../Texture.h"
#include "VkConvert.h"

namespace MulanGeo::Engine {

class VKTexture : public Texture {
public:
    VKTexture(const TextureDesc& desc, vk::Device device, VmaAllocator allocator);
    ~VKTexture();

    const TextureDesc& desc() const override { return m_desc; }

    vk::Image image() const { return m_image; }
    vk::ImageView view() const { return m_view; }
    vk::ImageViewCreateInfo viewForFramebuffer() const;

    static bool isDepthFormat(TextureFormat f);

private:
    TextureDesc     m_desc;
    vk::Device      m_device;
    VmaAllocator    m_allocator = nullptr;
    vk::Image       m_image;
    VmaAllocation   m_allocation = nullptr;
    vk::ImageView   m_view;
};

} // namespace MulanGeo::Engine
