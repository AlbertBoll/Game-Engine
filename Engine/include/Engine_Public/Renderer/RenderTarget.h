#pragma once

#include <optional>
#include <vector>
#include"Assets/Texture/TextureDesc.h"
#include"Math/Math.h"

using namespace Math;

enum class AttachmentKind : u8
{
    Color,
    Depth,
    DepthStencil
};

enum class MipPolicy : u8
{
    OneLevel,          // only allocate mip 0
    AllocateFullChain, // allocate full chain, but do not auto-generate
    AutoGenerate       // allocate full chain and auto-generate in EndPass()
};

struct AttachmentSpec
{
    AttachmentKind m_Kind   = AttachmentKind::Color;
    TextureFormat  m_Format = TextureFormat::RGBA16F;
    SamplerDesc    m_Sampler{};

    MipPolicy m_MipPolicy = MipPolicy::OneLevel;
};

struct RenderTargetDesc
{
    u32 m_Width  = 1;
    u32 m_Height = 1;

    std::vector<AttachmentSpec>   m_ColorAttachments;

    // May hold either Depth or DepthStencil attachment spec.
    std::optional<AttachmentSpec> m_DepthAttachment;
};

struct FramebufferHandle
{
    u32 m_Id = 0;
    u32 m_Generation = 0;

    explicit operator bool() const { return m_Id != 0; }
    friend bool operator==(const FramebufferHandle&, const FramebufferHandle&) = default;
};

enum class RenderTargetKind : u8
{
    BackBuffer,
    Framebuffer
};

struct RenderTarget
{
    RenderTargetKind  m_Kind = RenderTargetKind::BackBuffer;
    FramebufferHandle m_Framebuffer{};
};

enum class LoadOp : u8
{
    Load,
    Clear,
    DontCare
};

enum class StoreOp : u8
{
    Store,
    DontCare
};

inline u32 CalcFullMipLevels(u32 w, u32 h)
{
    u32 levels = 1;
    while (w > 1 || h > 1)
    {
        w = (w > 1) ? (w >> 1) : 1;
        h = (h > 1) ? (h >> 1) : 1;
        ++levels;
    }
    return levels;
}

inline u32 ResolveMipLevels(const AttachmentSpec& spec, u32 width, u32 height)
{
    switch (spec.m_MipPolicy)
    {
    case MipPolicy::OneLevel:
        return 1;

    case MipPolicy::AllocateFullChain:
    case MipPolicy::AutoGenerate:
        return CalcFullMipLevels(width, height);
    }

    return 1;
}

constexpr bool IsColorFormat(TextureFormat fmt)
{
    switch (fmt)
    {
        case TextureFormat::R8:
        case TextureFormat::RG8:
        case TextureFormat::RGB8:
        case TextureFormat::RGBA8:
        case TextureFormat::SRGB8:
        case TextureFormat::SRGB8A8:
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F:
            return true;
        default:
            return false;
    }
}

constexpr bool IsDepthFormat(TextureFormat fmt)
{
    return fmt == TextureFormat::Depth32F;
}

constexpr bool IsDepthStencilFormat(TextureFormat fmt)
{
    return fmt == TextureFormat::Depth24Stencil8;
}

inline void ValidateAttachmentSpec(const AttachmentSpec& spec)
{
    switch (spec.m_Kind)
    {
    case AttachmentKind::Color:
        CORE_ASSERT(IsColorFormat(spec.m_Format), "Color attachment requires a color format");
        break;

    case AttachmentKind::Depth:
        CORE_ASSERT(IsDepthFormat(spec.m_Format), "Depth attachment requires a depth-only format");
        CORE_ASSERT(spec.m_MipPolicy == MipPolicy::OneLevel,
                    "Depth attachment currently should use OneLevel mip policy");
        break;

    case AttachmentKind::DepthStencil:
        CORE_ASSERT(IsDepthStencilFormat(spec.m_Format),
                    "DepthStencil attachment requires a depth-stencil format");
        CORE_ASSERT(spec.m_MipPolicy == MipPolicy::OneLevel,
                    "DepthStencil attachment currently should use OneLevel mip policy");
        break;
    }
}

inline void ValidateRenderTargetDesc(const RenderTargetDesc& desc)
{
    CORE_ASSERT(desc.m_Width > 0 && desc.m_Height > 0, "RenderTargetDesc size must be > 0");

    for (const auto& color : desc.m_ColorAttachments)
    {
        CORE_ASSERT(color.m_Kind == AttachmentKind::Color,
                    "All color attachments must use AttachmentKind::Color");
        ValidateAttachmentSpec(color);
    }

    if (desc.m_DepthAttachment.has_value())
    {
        const auto& depth = *desc.m_DepthAttachment;
        CORE_ASSERT(depth.m_Kind == AttachmentKind::Depth ||
                    depth.m_Kind == AttachmentKind::DepthStencil,
                    "Depth attachment must be Depth or DepthStencil");
        ValidateAttachmentSpec(depth);
    }
}

inline TextureDesc ToTextureDesc(const RenderTargetDesc& rt, const AttachmentSpec& spec)
{
    TextureDesc td{};
    td.m_Type         = TextureType::Tex2D;
    td.m_Format       = spec.m_Format;
    td.m_Width        = rt.m_Width;
    td.m_Height       = rt.m_Height;
    td.m_Sampler      = spec.m_Sampler;
    td.m_MipLevels    = static_cast<u8>(ResolveMipLevels(spec, rt.m_Width, rt.m_Height));
    td.b_GenerateMips = (spec.m_MipPolicy == MipPolicy::AutoGenerate);
    return td;
}