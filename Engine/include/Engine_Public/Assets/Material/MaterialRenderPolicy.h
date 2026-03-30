#pragma once

#include "Assets/Material/MaterialDesc.h"
#include "Renderer/Renderer.h"

// ------------------------------------------------------------
// Final render bucket category chosen for submission
// ------------------------------------------------------------
enum class MaterialRenderCategory : u8
{
    Opaque,
    Transparent
};

// ------------------------------------------------------------
// Result of material -> renderer submission policy
// ------------------------------------------------------------
struct MaterialSubmitInfo
{
    MaterialRenderCategory m_Category = MaterialRenderCategory::Opaque;
    PipelineKey            m_Pipeline = 0;
};

// ------------------------------------------------------------
// Build renderer submission policy from a material description
//
// Rules:
// - Opaque      -> opaque pass, no blending, depth write on
// - AlphaTest   -> opaque pass, no blending, depth write on
// - Transparent -> transparent pass, alpha blending, depth write off
// - Additive    -> transparent pass, additive blending, depth write off
//
// TwoSided only affects cull mode.
// ------------------------------------------------------------
inline MaterialSubmitInfo BuildSubmitInfo(const MaterialDesc& md)
{
    PipelineState p{};

    // Common defaults
    p.b_DepthTest = true;
    p.m_DepthFunc = DepthFunc::Less;

    p.m_CullMode = HasFlag(md.m_Params.m_Flags, MaterialFlag::TwoSided)
        ? CullMode::None
        : CullMode::Back;

    switch (md.m_Params.m_SurfaceMode)
    {
    case MaterialSurfaceMode::Opaque:
        p.b_DepthWrite = true;
        p.m_BlendMode  = BlendMode::Opaque;
        return MaterialSubmitInfo{
            .m_Category = MaterialRenderCategory::Opaque,
            .m_Pipeline = BuildPipelineKey(p)
        };

    case MaterialSurfaceMode::AlphaTest:
        // Still treated as opaque pass.
        // Shader should handle discard / alpha clip.
        p.b_DepthWrite = true;
        p.m_BlendMode  = BlendMode::Opaque;
        return MaterialSubmitInfo{
            .m_Category = MaterialRenderCategory::Opaque,
            .m_Pipeline = BuildPipelineKey(p)
        };

    case MaterialSurfaceMode::Transparent:
        p.b_DepthWrite = false;
        p.m_DepthFunc  = DepthFunc::LessEqual;
        p.m_BlendMode  = BlendMode::Alpha;
        return MaterialSubmitInfo{
            .m_Category = MaterialRenderCategory::Transparent,
            .m_Pipeline = BuildPipelineKey(p)
        };

    case MaterialSurfaceMode::Additive:
        p.b_DepthWrite = false;
        p.m_DepthFunc  = DepthFunc::LessEqual;
        p.m_BlendMode  = BlendMode::Additive;
        return MaterialSubmitInfo{
            .m_Category = MaterialRenderCategory::Transparent,
            .m_Pipeline = BuildPipelineKey(p)
        };
    }

    CORE_ASSERT(false, "BuildSubmitInfo: unknown MaterialSurfaceMode");

    p.b_DepthWrite = true;
    p.m_BlendMode  = BlendMode::Opaque;

    return MaterialSubmitInfo{
        .m_Category = MaterialRenderCategory::Opaque,
        .m_Pipeline = BuildPipelineKey(p)
    };
}

// ------------------------------------------------------------
// Convenience helpers
// ------------------------------------------------------------
inline bool IsTransparentCategory(MaterialRenderCategory c)
{
    return c == MaterialRenderCategory::Transparent;
}

inline bool IsOpaqueCategory(MaterialRenderCategory c)
{
    return c == MaterialRenderCategory::Opaque;
}

inline bool IsTransparentMaterial(const MaterialDesc& md)
{
    return BuildSubmitInfo(md).m_Category == MaterialRenderCategory::Transparent;
}

inline bool IsOpaqueMaterial(const MaterialDesc& md)
{
    return BuildSubmitInfo(md).m_Category == MaterialRenderCategory::Opaque;
}