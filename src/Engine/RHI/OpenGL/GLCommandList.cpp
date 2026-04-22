/**
 * @file GLCommandList.cpp
 * @brief OpenGL 命令列表实现
 * @author terry
 * @date 2026-04-16
 */

#include "GLCommandList.h"
#include "GLBuffer.h"
#include "GLPipelineState.h"
#include "../Buffer.h"
#include "../RenderTypes.h"
#include <cstdio>
#include <cstring>

namespace MulanGeo::Engine {

GLCommandList::GLCommandList() = default;

GLCommandList::~GLCommandList() = default;

void GLCommandList::begin() {
    // OpenGL 立即模式，begin/end 不做实质工作
    m_pipelineStateApplied = false;
    m_viewportDirty = true;
    m_scissorDirty = true;
}

void GLCommandList::end() {
    // OpenGL 立即模式，end 不做实质工作
}

void GLCommandList::setPipelineState(PipelineState* pso) {
    if (m_currentPipeline == pso) return;

    m_currentPipeline = pso;
    m_pipelineStateApplied = false;  // 标记需要应用
}

void GLCommandList::setViewport(const Viewport& vp) {
    if (std::memcmp(&m_viewport, &vp, sizeof(Viewport)) == 0) {
        return;  // 无变化
    }

    m_viewport = vp;
    m_viewportDirty = true;
}

void GLCommandList::setScissorRect(const ScissorRect& rect) {
    if (std::memcmp(&m_scissorRect, &rect, sizeof(ScissorRect)) == 0) {
        return;  // 无变化
    }

    m_scissorRect = rect;
    m_scissorDirty = true;
}

void GLCommandList::setVertexBuffer(uint32_t slot, Buffer* buffer,
                                    uint32_t offset) {
    if (slot >= MAX_VERTEX_BUFFERS) {
        std::fprintf(stderr, "[GLCommandList] setVertexBuffer: slot %u out of range\n", slot);
        return;
    }

    if (m_vertexBuffers[slot] == buffer && 
        m_vertexBufferOffsets[slot] == offset) {
        return;  // 无变化
    }

    m_vertexBuffers[slot] = buffer;
    m_vertexBufferOffsets[slot] = offset;

    if (slot >= m_vertexBufferCount) {
        m_vertexBufferCount = slot + 1;
    }

    // OpenGL 立即执行
    if (buffer) {
        auto* glBuf = static_cast<GLBuffer*>(buffer);
        glBindBuffer(GL_ARRAY_BUFFER, glBuf->handle());
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void GLCommandList::setVertexBuffers(uint32_t startSlot, uint32_t count,
                                     Buffer** buffers, uint32_t* offsets) {
    for (uint32_t i = 0; i < count; ++i) {
        setVertexBuffer(startSlot + i, buffers[i], 
                       offsets ? offsets[i] : 0);
    }
}

void GLCommandList::setIndexBuffer(Buffer* buffer, uint32_t offset,
                                   IndexType type) {
    if (m_indexBuffer == buffer && 
        m_indexBufferOffset == offset &&
        m_indexType == type) {
        return;  // 无变化
    }

    m_indexBuffer = buffer;
    m_indexBufferOffset = offset;
    m_indexType = type;

    // OpenGL 立即执行
    if (buffer) {
        auto* glBuf = static_cast<GLBuffer*>(buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuf->handle());
    } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void GLCommandList::draw(const DrawAttribs& attribs) {
    // 应用管线状态
    applyPipelineState();

    // 应用视口
    if (m_viewportDirty) {
        glViewport(static_cast<GLint>(m_viewport.x),
                   static_cast<GLint>(m_viewport.y),
                   static_cast<GLint>(m_viewport.width),
                   static_cast<GLint>(m_viewport.height));
        glDepthRange(m_viewport.minDepth, m_viewport.maxDepth);
        m_viewportDirty = false;
    }

    // 应用裁剪矩形
    if (m_scissorDirty) {
        glScissor(static_cast<GLint>(m_scissorRect.x),
                  static_cast<GLint>(m_scissorRect.y),
                  static_cast<GLint>(m_scissorRect.width),
                  static_cast<GLint>(m_scissorRect.height));
        m_scissorDirty = false;
    }

    // 执行绘制
    glDrawArrays(GL_TRIANGLES, attribs.startVertex, attribs.vertexCount);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::fprintf(stderr, "[GLCommandList] draw failed (error: 0x%X)\n", err);
    }
}

void GLCommandList::drawIndexed(const DrawIndexedAttribs& attribs) {
    // 应用管线状态
    applyPipelineState();

    // 应用视口
    if (m_viewportDirty) {
        glViewport(static_cast<GLint>(m_viewport.x),
                   static_cast<GLint>(m_viewport.y),
                   static_cast<GLint>(m_viewport.width),
                   static_cast<GLint>(m_viewport.height));
        glDepthRange(m_viewport.minDepth, m_viewport.maxDepth);
        m_viewportDirty = false;
    }

    // 应用裁剪矩形
    if (m_scissorDirty) {
        glScissor(static_cast<GLint>(m_scissorRect.x),
                  static_cast<GLint>(m_scissorRect.y),
                  static_cast<GLint>(m_scissorRect.width),
                  static_cast<GLint>(m_scissorRect.height));
        m_scissorDirty = false;
    }

    // 执行索引绘制
    GLenum indexFormat = indexTypeToGLFormat(m_indexType);
    glDrawElements(GL_TRIANGLES, 
                   static_cast<GLsizei>(attribs.indexCount), 
                   indexFormat,
                   reinterpret_cast<const void*>(
                       static_cast<uintptr_t>(m_indexBufferOffset + attribs.startIndex)));

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::fprintf(stderr, "[GLCommandList] drawIndexed failed (error: 0x%X)\n", err);
    }
}

void GLCommandList::updateBuffer(Buffer* buffer, uint32_t offset,
                                 uint32_t size, const void* data,
                                 ResourceTransitionMode /*mode*/) {
    if (!buffer || !data) return;

    buffer->update(offset, size, data);
}

void GLCommandList::transitionResource(Buffer* buffer,
                                       ResourceState newState) {
    // GL 无需显式资源状态转换
    (void)buffer;
    (void)newState;
}

void GLCommandList::transitionResource(Texture* texture,
                                       ResourceState newState) {
    // GL 无需显式资源状态转换
    (void)texture;
    (void)newState;
}

void GLCommandList::copyTextureToBuffer(Texture* src, Buffer* dst) {
    // TODO: 实现纹理→缓冲区复制
    // 使用 glReadPixels 或 glGetTexImage
    (void)src;
    (void)dst;
}

void GLCommandList::clearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLCommandList::clearDepth(float depth) {
    glClearDepthf(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GLCommandList::clearStencil(uint8_t stencil) {
    glClearStencil(stencil);
    glClear(GL_STENCIL_BUFFER_BIT);
}

Buffer* GLCommandList::currentVertexBuffer(uint32_t slot) const {
    if (slot >= MAX_VERTEX_BUFFERS) return nullptr;
    return m_vertexBuffers[slot];
}

void GLCommandList::applyPipelineState() {
    if (!m_currentPipeline || m_pipelineStateApplied) return;

    auto* glPipeline = static_cast<GLPipelineState*>(m_currentPipeline);

    // 应用着色器程序
    glUseProgram(glPipeline->program());

    // 应用渲染状态（栅格化、深度测试、混合等）
    glPipeline->applyRenderState();

    m_pipelineStateApplied = true;
}

GLenum GLCommandList::indexTypeToGLFormat(IndexType type) {
    switch (type) {
    case IndexType::UInt16: return GL_UNSIGNED_SHORT;
    case IndexType::UInt32: return GL_UNSIGNED_INT;
    default:                return GL_UNSIGNED_INT;
    }
}

} // namespace MulanGeo::Engine
