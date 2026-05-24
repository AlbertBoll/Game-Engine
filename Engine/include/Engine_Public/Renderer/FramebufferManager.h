#pragma once

#include"RenderTarget.h"


class TextureManager;

class ENGINE_API FramebufferManager
{
public:
    //FramebufferManager() = default;
    explicit FramebufferManager(TextureManager& textureMgr);
    ~FramebufferManager();

    DELETE_COPY(FramebufferManager)

    FramebufferHandle CreateFromDesc(const RenderTargetDesc& desc,
                                     std::string_view debugName = {});

    void Destroy(FramebufferHandle h);
    void Clear(bool keepCapacity = true);

    bool IsValid(FramebufferHandle h) const;

    const RenderTargetDesc& GetDesc(FramebufferHandle h) const;
    u32 GetGLHandle(FramebufferHandle h) const;

    size_t GetColorAttachmentCount(FramebufferHandle h) const;
    TextureHandle GetColorAttachment(FramebufferHandle h, size_t index) const;
    std::optional<TextureHandle> GetDepthAttachment(FramebufferHandle h) const;

    void ResolveColor(FramebufferHandle srcMsaa,
                      FramebufferHandle dstSingleSample,
                      u32 srcColorIndex,
                      u32 dstColorIndex);

    void ResolveColorToBackBuffer(FramebufferHandle srcMsaa,
                                  u32 srcColorIndex);

    void ResolveColorToBackBuffer(FramebufferHandle srcMsaa,
                                    u32 srcColorIndex,
                                    u32 dstWidth,
                                    u32 dstHeight);

    void Resize(FramebufferHandle h, u32 width, u32 height);

private:
    struct Slot
    {
        bool b_Alive = false;
        u32  m_Generation = 1;

        u32 m_GLFBO = 0;

        RenderTargetDesc m_Desc{};
        std::string      m_DebugName{};

        std::vector<TextureHandle>   m_ColorAttachments;
        std::optional<TextureHandle> m_DepthAttachment;
    };

private:
    FramebufferHandle AllocateSlot();
    Slot*       GetSlot(FramebufferHandle h);
    const Slot* GetSlot(FramebufferHandle h) const;

    void DestroySlotResources(Slot& s);
    bool BuildSlot(Slot& s, const RenderTargetDesc& desc, std::string_view debugName);

private:
    TextureManager& m_TextureMgr;

    std::vector<Slot> m_Slots;
    std::vector<u32>  m_FreeList;
};