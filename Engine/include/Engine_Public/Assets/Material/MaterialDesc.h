#pragma once

#include "Core/Base.h"
#include "Math/Math.h"

using namespace Math;

enum class MaterialFlag : u32
{
    None           = 0,
    UseAlbedoMap   = 1u << 0,
    UseNormalMap   = 1u << 1,
    UseMetallicMap   = 1u << 2,
    UseRoughnessMap  = 1u << 3,
    UseAOMap         = 1u << 4,
    UseEmissiveMap   = 1u << 5,
    UseORMMap        = 1u << 6,   // R = AO, G = Roughness, B = Metallic
    TwoSided         = 1u << 7,
    AlphaTest        = 1u << 8,
};

// ------------------------------------------------------------
// Material surface mode
// ------------------------------------------------------------
enum class MaterialSurfaceMode : u8
{
    Opaque = 0,
    AlphaTest,
    Transparent,
    Additive
};

// ------------------------------------------------------------
// Material texture slots
// ------------------------------------------------------------
enum class MaterialTexSlot : u8
{
    Albedo = 0,
    Normal,
    Metallic,
    Roughness,
    AO,
    Emissive,
    ORM,      // packed: R=AO, G=Roughness, B=Metallic
    Count
};


// -------------------- Material Params --------------------
struct MaterialParams
{
    float m_Metallic  = 0.0f;
    float m_Roughness = 1.0f;
    float m_Exposure  = 1.0f;

    Vec3f m_Albedo   = Vec3f{1.0f, 1.0f, 1.0f};
    Vec3f m_Emissive = Vec3f{0.0f, 0.0f, 0.0f};

    float m_AO = 1.0f; // scalar AO fallback when no AO map / ORM
    float m_Pad0 = 0.0f;
    float m_Pad1 = 0.0f;
    float m_Pad2 = 0.0f;

    MaterialFlag        m_Flags       = MaterialFlag::None;
    MaterialSurfaceMode m_SurfaceMode = MaterialSurfaceMode::Opaque;
};

// -------------------- Material Handle --------------------
struct MaterialHandle
{
    u32 m_Id = 0;          // slot index + 1
    u32 m_Generation = 0;  // stale-handle protection

    explicit operator bool() const { return m_Id != 0; }
    friend bool operator==(const MaterialHandle&, const MaterialHandle&) = default;
};

inline constexpr bool HasAnyFlag(MaterialFlag flags, MaterialFlag mask) noexcept
{
    return (((u32)flags & (u32)mask) != 0);
}

inline constexpr bool HasAllFlags(MaterialFlag flags, MaterialFlag mask) noexcept
{
    return ((u32)flags & (u32)mask) == (u32)mask;
}

inline constexpr bool HasNoFlag(MaterialFlag flags, MaterialFlag mask) noexcept
{
    return ((u32)flags & (u32)mask) == 0;
}

inline void EnableFlag(MaterialFlag& flags, MaterialFlag flag) noexcept
{
    flags = (MaterialFlag)((u32)flags | (u32)flag);
}

inline void DisableFlag(MaterialFlag& flags, MaterialFlag flag) noexcept
{
    flags = (MaterialFlag)((u32)flags & ~(u32)flag);
}

inline void SetFlag(MaterialFlag& flags, MaterialFlag flag, bool enabled) noexcept
{
    if (enabled)
        EnableFlag(flags, flag);
    else
        DisableFlag(flags, flag);
}

inline void ToggleFlag(MaterialFlag& flags, MaterialFlag flag) noexcept
{
    flags = (MaterialFlag)((u32)flags ^ (u32)flag);
}

// -------------------- Material Flag --------------------
// Note:
// - Texture feature flags (UseAlbedoMap / UseNormalMap / ...) are consumed by MaterialManager.
// - Render-state-like flags (TwoSided / AlphaTest) should ultimately be handled by Renderer / pipeline state.

inline constexpr MaterialFlag operator|(MaterialFlag a, MaterialFlag b)
{
    return (MaterialFlag)((u32)a | (u32)b);
}

inline constexpr MaterialFlag operator&(MaterialFlag a, MaterialFlag b)
{
    return (MaterialFlag)((u32)a & (u32)b);
}

inline constexpr MaterialFlag operator~(MaterialFlag v)
{
    return (MaterialFlag)(~(u32)v);
}

inline MaterialFlag& operator|=(MaterialFlag& a, MaterialFlag b)
{
    a = a | b;
    return a;
}

inline MaterialFlag& operator&=(MaterialFlag& a, MaterialFlag b)
{
    a = a & b;
    return a;
}

inline constexpr bool HasFlag(MaterialFlag v, MaterialFlag f)
{
    return (((u32)v & (u32)f) != 0);
}