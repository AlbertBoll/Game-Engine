#include "Renderer/FramebufferManager.h"
#include "Assets/Texture/TextureManager.h"
#include <glad/gl.h>

namespace
{
    static inline void GLDeleteFramebuffer(u32& id)
    {
        if (!id) return;
        GLuint fbo = static_cast<GLuint>(id);
        glDeleteFramebuffers(1, &fbo);
        id = 0;
    }

    static inline void SetDebugLabel(GLenum identifier, u32 id, std::string_view name)
    {
        if (name.empty()) return;
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
            glObjectLabel(identifier, static_cast<GLuint>(id),
                          static_cast<GLsizei>(name.size()), name.data());
    }

    static inline GLenum ToGLAttachment(AttachmentKind kind, u32 colorIndex = 0)
    {
        switch (kind)
        {
        case AttachmentKind::Color:        return GL_COLOR_ATTACHMENT0 + colorIndex;
        case AttachmentKind::Depth:        return GL_DEPTH_ATTACHMENT;
        case AttachmentKind::DepthStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
        }
        return GL_COLOR_ATTACHMENT0;
    }
}

FramebufferManager::FramebufferManager(TextureManager& textureMgr)
    : m_TextureMgr(textureMgr)
{
}

FramebufferManager::~FramebufferManager()
{
    Clear(false);
}

FramebufferHandle FramebufferManager::AllocateSlot()
{
    if (!m_FreeList.empty())
    {
        const u32 idx = m_FreeList.back();
        m_FreeList.pop_back();
        return FramebufferHandle{ idx + 1, m_Slots[idx].m_Generation };
    }

    m_Slots.push_back({});
    const u32 idx = static_cast<u32>(m_Slots.size()) - 1;
    return FramebufferHandle{ idx + 1, m_Slots[idx].m_Generation };
}

FramebufferManager::Slot* FramebufferManager::GetSlot(FramebufferHandle h)
{
    if (!h) return nullptr;

    const u32 idx = h.m_Id - 1;
    if (idx >= m_Slots.size()) return nullptr;

    Slot& s = m_Slots[idx];
    if (!s.b_Alive) return nullptr;
    if (s.m_Generation != h.m_Generation) return nullptr;

    return &s;
}

const FramebufferManager::Slot* FramebufferManager::GetSlot(FramebufferHandle h) const
{
    return const_cast<FramebufferManager*>(this)->GetSlot(h);
}

bool FramebufferManager::IsValid(FramebufferHandle h) const
{
    return GetSlot(h) != nullptr;
}

const RenderTargetDesc& FramebufferManager::GetDesc(FramebufferHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetDesc: invalid FramebufferHandle");
    return s->m_Desc;
}

u32 FramebufferManager::GetGLHandle(FramebufferHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetGLHandle: invalid FramebufferHandle");
    return s->m_GLFBO;
}

size_t FramebufferManager::GetColorAttachmentCount(FramebufferHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetColorAttachmentCount: invalid FramebufferHandle");
    return s->m_ColorAttachments.size();
}

TextureHandle FramebufferManager::GetColorAttachment(FramebufferHandle h, size_t index) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetColorAttachment: invalid FramebufferHandle");
    CORE_ASSERT(index < s->m_ColorAttachments.size(), "GetColorAttachment: index out of range");
    return s->m_ColorAttachments[index];
}

std::optional<TextureHandle> FramebufferManager::GetDepthAttachment(FramebufferHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetDepthAttachment: invalid FramebufferHandle");
    return s->m_DepthAttachment;
}

void FramebufferManager::DestroySlotResources(Slot& s)
{
    for (TextureHandle h : s.m_ColorAttachments)
    {
        if (h)
            m_TextureMgr.Destroy(h);
    }
    s.m_ColorAttachments.clear();

    if (s.m_DepthAttachment.has_value() && *s.m_DepthAttachment)
    {
        m_TextureMgr.Destroy(*s.m_DepthAttachment);
    }
    s.m_DepthAttachment.reset();

    GLDeleteFramebuffer(s.m_GLFBO);
}

void FramebufferManager::BuildSlot(Slot& s, const RenderTargetDesc& desc, std::string_view debugName)
{
    ValidateRenderTargetDesc(desc);

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    CORE_ASSERT(fbo != 0, "glGenFramebuffers failed");

    s.m_GLFBO = static_cast<u32>(fbo);
    s.m_Desc = desc;
    s.m_DebugName = std::string(debugName);
    s.m_ColorAttachments.clear();
    s.m_DepthAttachment.reset();

    SetDebugLabel(GL_FRAMEBUFFER, s.m_GLFBO, debugName);

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(s.m_GLFBO));

    // create and attach color textures
    std::vector<GLenum> drawBuffers;
    drawBuffers.reserve(desc.m_ColorAttachments.size());

    for (u32 i = 0; i < static_cast<u32>(desc.m_ColorAttachments.size()); ++i)
    {
        const AttachmentSpec& spec = desc.m_ColorAttachments[i];
        TextureDesc td = ToTextureDesc(desc, spec);

        std::string texName = s.m_DebugName.empty()
            ? ("RT_Color" + std::to_string(i))
            : (s.m_DebugName + "_Color" + std::to_string(i));

        TextureHandle tex = m_TextureMgr.CreateEmpty(td, texName);
        CORE_ASSERT(tex, "Failed to create color attachment texture");

        const TextureDesc& createdDesc = m_TextureMgr.GetDesc(tex);
        CORE_ASSERT(createdDesc.m_Type == TextureType::Tex2D, "Framebuffer currently requires 2D attachments");

        const u32 glTex = m_TextureMgr.GetNativeTexture(tex);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               ToGLAttachment(AttachmentKind::Color, i),
                               GL_TEXTURE_2D,
                               static_cast<GLuint>(glTex),
                               0);

        s.m_ColorAttachments.push_back(tex);
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    // create and attach depth/depth-stencil texture
    if (desc.m_DepthAttachment.has_value())
    {
        const AttachmentSpec& spec = *desc.m_DepthAttachment;
        TextureDesc td = ToTextureDesc(desc, spec);

        std::string texName = s.m_DebugName.empty()
            ? "RT_Depth"
            : (s.m_DebugName + "_Depth");

        TextureHandle tex = m_TextureMgr.CreateEmpty(td, texName);
        CORE_ASSERT(tex, "Failed to create depth attachment texture");

        const TextureDesc& createdDesc = m_TextureMgr.GetDesc(tex);
        CORE_ASSERT(createdDesc.m_Type == TextureType::Tex2D, "Framebuffer currently requires 2D attachments");

        const u32 glTex = m_TextureMgr.GetNativeTexture(tex);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               ToGLAttachment(spec.m_Kind),
                               GL_TEXTURE_2D,
                               static_cast<GLuint>(glTex),
                               0);

        s.m_DepthAttachment = tex;
    }

    if (drawBuffers.empty())
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    else
    {
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    CORE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FramebufferHandle FramebufferManager::CreateFromDesc(const RenderTargetDesc& desc,
                                                     std::string_view debugName)
{
    FramebufferHandle h = AllocateSlot();
    Slot& s = m_Slots[h.m_Id - 1];

    BuildSlot(s, desc, debugName);

    s.b_Alive = true;
    s.m_Generation = h.m_Generation;
    return h;
}

void FramebufferManager::Destroy(FramebufferHandle h)
{
    Slot* s = GetSlot(h);
    if (!s) return;

    DestroySlotResources(*s);

    s->b_Alive = false;
    ++s->m_Generation;
    s->m_Desc = RenderTargetDesc{};
    s->m_DebugName.clear();

    m_FreeList.push_back(h.m_Id - 1);
}

void FramebufferManager::Resize(FramebufferHandle h, u32 width, u32 height)
{
    Slot* s = GetSlot(h);
    CORE_ASSERT(s, "Resize: invalid FramebufferHandle");

    if (s->m_Desc.m_Width == width && s->m_Desc.m_Height == height)
        return;

    RenderTargetDesc newDesc = s->m_Desc;
    newDesc.m_Width = width;
    newDesc.m_Height = height;

    DestroySlotResources(*s);
    BuildSlot(*s, newDesc, s->m_DebugName);
}

void FramebufferManager::Clear(bool keepCapacity)
{
    for (auto& s : m_Slots)
    {
        if (s.b_Alive)
        {
            DestroySlotResources(s);
            s.b_Alive = false;
            ++s.m_Generation;
            s.m_Desc = RenderTargetDesc{};
            s.m_DebugName.clear();
        }
    }

    m_FreeList.clear();

    if (keepCapacity)
    {
        m_FreeList.reserve(m_Slots.size());
        for (u32 i = 0; i < static_cast<u32>(m_Slots.size()); ++i)
            m_FreeList.push_back(i);
    }
    else
    {
        m_Slots.clear();
        m_FreeList.clear();
    }
}