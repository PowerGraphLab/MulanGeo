/**
 * @file ImageData.h
 * @brief 图像数据容器 — CPU 端解码后的像素缓冲
 * @author hxxcxx
 * @date 2026-04-17
 */
#pragma once

#include "DocumentExport.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace MulanGeo::Document {

struct DOCUMENT_API ImageData {
    uint32_t             width    = 0;
    uint32_t             height   = 0;
    uint32_t             channels = 0;   ///< 1=灰度, 2=灰度+A, 3=RGB, 4=RGBA
    std::vector<uint8_t> pixels;         ///< row-major, top-left origin

    bool valid() const {
        return width > 0 && height > 0 && channels > 0
            && pixels.size() == static_cast<size_t>(width) * height * channels;
    }

    uint32_t rowBytes() const { return width * channels; }
    size_t totalBytes() const { return pixels.size(); }

    void flipVertically() {
        if (!valid()) return;
        uint32_t rowLen = rowBytes();
        std::vector<uint8_t> tmp(rowLen);
        for (uint32_t top = 0, bot = height - 1; top < bot; ++top, --bot) {
            uint8_t* pTop = pixels.data() + top * rowLen;
            uint8_t* pBot = pixels.data() + bot * rowLen;
            std::memcpy(tmp.data(), pTop, rowLen);
            std::memcpy(pTop, pBot, rowLen);
            std::memcpy(pBot, tmp.data(), rowLen);
        }
    }

    ImageData toRGBA() const {
        if (channels == 4) return *this;
        if (!valid()) return {};

        ImageData out;
        out.width    = width;
        out.height   = height;
        out.channels = 4;
        out.pixels.resize(static_cast<size_t>(width) * height * 4);

        size_t pixelCount = static_cast<size_t>(width) * height;
        for (size_t i = 0; i < pixelCount; ++i) {
            const uint8_t* src = pixels.data() + i * channels;
            uint8_t*       dst = out.pixels.data() + i * 4;

            switch (channels) {
            case 1:
                dst[0] = dst[1] = dst[2] = src[0];
                dst[3] = 255;
                break;
            case 2:
                dst[0] = dst[1] = dst[2] = src[0];
                dst[3] = src[1];
                break;
            case 3:
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
                dst[3] = 255;
                break;
            default:
                break;
            }
        }
        return out;
    }
};

} // namespace MulanGeo::Document
