/**
 * @file ImageLoader.h
 * @brief 图像文件加载器接口
 * @author hxxcxx
 * @date 2026-04-17
 */
#pragma once

#include "ImageData.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace MulanGeo::Document {

DOCUMENT_API ImageData loadImage(std::string_view path, int forceChannels = 0);
DOCUMENT_API ImageData loadImageFromMemory(const uint8_t* data, size_t size, int forceChannels = 0);
DOCUMENT_API bool saveImagePNG(std::string_view path, const ImageData& image);

} // namespace MulanGeo::Document
