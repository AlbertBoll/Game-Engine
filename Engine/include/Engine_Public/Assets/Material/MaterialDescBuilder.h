#pragma once

#include "Assets/Material/MaterialDesc.h"

// Header-only builder for MaterialDesc.
//
// Goals:
// 1) Reduce verbose manual MaterialDesc construction
// 2) Keep texture-slot / flag state consistent
// 3) Support "clone then tweak" workflows via From(...)
// 4) Stay independent from MaterialManager lifetime / cache logic
//
// Important:
// - High-level texture helpers like AlbedoMap()/NormalMap() keep flags in sync.
// - Raw Texture()/ClearTexture() do NOT automatically modify feature flags.
//   They are lower-level APIs for callers who want explicit control.
class MaterialDescBuilder
{
public:
    MaterialDescBuilder() = default;

    explicit MaterialDescBuilder(const MaterialDesc& desc)
        : m_Desc(desc)
    {
    }

    static MaterialDescBuilder From(const MaterialDesc& desc)
    {
        return MaterialDescBuilder(desc);
    }

public:
    // -------------------- Reset --------------------
    MaterialDescBuilder& Reset()
    {
        m_Desc = MaterialDesc{};
        return *this;
    }

    // -------------------- Basic fields --------------------
    MaterialDescBuilder& Shader(ShaderHandle shader)
    {
        m_Desc.m_Shader = shader;
        return *this;
    }

    MaterialDescBuilder& Albedo(const Vec3f& color)
    {
        m_Desc.m_Params.m_Albedo = color;
        return *this;
    }

    MaterialDescBuilder& Emissive(const Vec3f& color)
    {
        m_Desc.m_Params.m_Emissive = color;
        return *this;
    }

    MaterialDescBuilder& Metallic(float v)
    {
        m_Desc.m_Params.m_Metallic = v;
        return *this;
    }

    MaterialDescBuilder& Roughness(float v)
    {
        m_Desc.m_Params.m_Roughness = v;
        return *this;
    }

    MaterialDescBuilder& Exposure(float v)
    {
        m_Desc.m_Params.m_Exposure = v;
        return *this;
    }

    MaterialDescBuilder& AO(float v)
    {
        m_Desc.m_Params.m_AO = v;
        return *this;
    }

    // --------------------------------------------------------
    // Surface mode
    // --------------------------------------------------------
    MaterialDescBuilder& SurfaceMode(MaterialSurfaceMode mode)
    {
        m_Desc.m_Params.m_SurfaceMode = mode;
        SyncAlphaTestFlagFromSurfaceMode();
        return *this;
    }

     MaterialDescBuilder& OpaqueSurface()
    {
        m_Desc.m_Params.m_SurfaceMode = MaterialSurfaceMode::Opaque;
        SyncAlphaTestFlagFromSurfaceMode();
        return *this;
    }

    MaterialDescBuilder& AlphaTestSurface()
    {
        m_Desc.m_Params.m_SurfaceMode = MaterialSurfaceMode::AlphaTest;
        SyncAlphaTestFlagFromSurfaceMode();
        return *this;
    }

    MaterialDescBuilder& TransparentSurface()
    {
        m_Desc.m_Params.m_SurfaceMode = MaterialSurfaceMode::Transparent;
        SyncAlphaTestFlagFromSurfaceMode();
        return *this;
    }

    MaterialDescBuilder& AdditiveSurface()
    {
        m_Desc.m_Params.m_SurfaceMode = MaterialSurfaceMode::Additive;
        SyncAlphaTestFlagFromSurfaceMode();
        return *this;
    }


     // -------------------- Flags --------------------
    // Whole-set replace. Use carefully.
    // In most editing code, SetFlag()/EnableFeature()/DisableFeature() are safer.
    MaterialDescBuilder& Flags(MaterialFlag flags)
    {
        m_Desc.m_Params.m_Flags = flags;
        return *this;
    }

    MaterialDescBuilder& EnableFlag(MaterialFlag flag)
    {
        ::EnableFlag(m_Desc.m_Params.m_Flags, flag);
        return *this;
    }

    MaterialDescBuilder& DisableFlag(MaterialFlag flag)
    {
        ::DisableFlag(m_Desc.m_Params.m_Flags, flag);
        return *this;
    }

    MaterialDescBuilder& SetFlag(MaterialFlag flag, bool enabled = true)
    {
        ::SetFlag(m_Desc.m_Params.m_Flags, flag, enabled);
        return *this;
    }

    // Convenience helpers for common render-style flags
    MaterialDescBuilder& TwoSided(bool enabled = true)
    {
        ::SetFlag(m_Desc.m_Params.m_Flags, MaterialFlag::TwoSided, enabled);
        return *this;
    }

    MaterialDescBuilder& AlphaTest(bool enabled = true)
    {
        ::SetFlag(m_Desc.m_Params.m_Flags, MaterialFlag::AlphaTest, enabled);
        return *this;
    }

    // -------------------- Raw texture access --------------------
    // These do NOT modify feature flags.
    // Use these only when you want explicit control.
    MaterialDescBuilder& Texture(MaterialTexSlot slot, const MaterialTexture& tex)
    {
        m_Desc.m_Textures[(u8)slot] = tex;
        return *this;
    }

    MaterialDescBuilder& Texture(MaterialTexSlot slot, TextureHandle texture)
    {
        m_Desc.m_Textures[(u8)slot].m_Texture = texture;
        return *this;
    }

    // -------------------- Feature texture helpers --------------------
    // These DO keep flags in sync automatically.

    MaterialDescBuilder& AlbedoMap(TextureHandle texture)
    {
        SetTextureAndEnable(MaterialTexSlot::Albedo, texture, MaterialFlag::UseAlbedoMap);
        return *this;
    }

    MaterialDescBuilder& NormalMap(TextureHandle texture)
    {
        SetTextureAndEnable(MaterialTexSlot::Normal, texture, MaterialFlag::UseNormalMap);
        return *this;
    }

     MaterialDescBuilder& MetallicMap(TextureHandle texture)
    {
        SetTextureAndEnable(MaterialTexSlot::Metallic, texture, MaterialFlag::UseMetallicMap);
        return *this;
    }

    MaterialDescBuilder& RoughnessMap(TextureHandle texture)
    {
        SetTextureAndEnable(MaterialTexSlot::Roughness, texture, MaterialFlag::UseRoughnessMap);
        return *this;
    }

    MaterialDescBuilder& EmissiveMap(TextureHandle texture)
    {
        SetTextureAndEnable(MaterialTexSlot::Emissive, texture, MaterialFlag::UseEmissiveMap);
        return *this;
    }

    MaterialDescBuilder& AOMap(TextureHandle texture)
    {
        SetTextureAndEnable(MaterialTexSlot::AO, texture, MaterialFlag::UseAOMap);
        return *this;
    }

    // --------------------------------------------------------
    // Packed ORM
    // --------------------------------------------------------
    MaterialDescBuilder& ORMMap(TextureHandle texture)
    {
        SetTextureAndEnable(MaterialTexSlot::ORM, texture, MaterialFlag::UseORMMap);
        return *this;
    }



    MaterialDescBuilder& ClearAlbedoMap()
    {
        ClearTextureAndDisable(MaterialTexSlot::Albedo, MaterialFlag::UseAlbedoMap);
        return *this;
    }

    MaterialDescBuilder& ClearNormalMap()
    {
        ClearTextureAndDisable(MaterialTexSlot::Normal, MaterialFlag::UseNormalMap);
        return *this;
    }

    MaterialDescBuilder& ClearMetallicMap()
    {
        ClearTextureAndDisable(MaterialTexSlot::Metallic, MaterialFlag::UseMetallicMap);
        return *this;
    }

    MaterialDescBuilder& ClearRoughnessMap()
    {
        ClearTextureAndDisable(MaterialTexSlot::Roughness, MaterialFlag::UseRoughnessMap);
        return *this;
    }

    MaterialDescBuilder& ClearAOMap()
    {
        ClearTextureAndDisable(MaterialTexSlot::AO, MaterialFlag::UseAOMap);
        return *this;
    }

    MaterialDescBuilder& ClearEmissiveMap()
    {
        ClearTextureAndDisable(MaterialTexSlot::Emissive, MaterialFlag::UseEmissiveMap);
        return *this;
    }

    MaterialDescBuilder& ClearORMMap()
    {
        ClearTextureAndDisable(MaterialTexSlot::ORM, MaterialFlag::UseORMMap);
        return *this;
    }

    // -------------------- Access / build --------------------
    const MaterialDesc& Get() const
    {
        return m_Desc;
    }

    // Raw build: returns whatever the builder currently contains.
    // Use this when you intentionally want full manual control.
    MaterialDesc Build() const
    {
        return m_Desc;
    }

    // Checked build: validates common mistakes.
    // This is the recommended path for most callers.
    MaterialDesc BuildChecked() const
    {
        Validate(m_Desc);
        return m_Desc;
    }

    MaterialDescBuilder& ClearTexture(MaterialTexSlot slot)
    {
        m_Desc.m_Textures[(u8)slot] = MaterialTexture{};
        return *this;
    }

private:
    // MaterialDescBuilder& Texture(MaterialTexSlot slot, u32 texId, TextureHandle texture)
    // {
    //     m_Desc.m_Textures[(u8)slot] = MaterialTexture{ texture };
    //     return *this;
    // }




    static void Validate(const MaterialDesc& desc)
    {
        CORE_ASSERT(desc.m_Shader, "MaterialDescBuilder: shader handle is not set");

        auto validateMap = [&](MaterialTexSlot slot, MaterialFlag flag, const char* msg)
        {
            if (!HasAnyFlag(desc.m_Params.m_Flags, flag))
                return;

            const MaterialTexture& t = desc.m_Textures[(u8)slot];
            CORE_ASSERT(t.m_Texture, msg);
        };

        validateMap(MaterialTexSlot::Albedo,
                    MaterialFlag::UseAlbedoMap,
                    "MaterialDescBuilder: UseAlbedoMap enabled but albedo texture is invalid");

        validateMap(MaterialTexSlot::Normal,
                    MaterialFlag::UseNormalMap,
                    "MaterialDescBuilder: UseNormalMap enabled but normal texture is invalid");

        validateMap(MaterialTexSlot::Metallic,
                    MaterialFlag::UseMetallicMap,
                    "MaterialDescBuilder: UseMetallicMap enabled but metallic texture is invalid");

        validateMap(MaterialTexSlot::Roughness,
                    MaterialFlag::UseRoughnessMap,
                    "MaterialDescBuilder: UseRoughnessMap enabled but roughness texture is invalid");

        validateMap(MaterialTexSlot::AO,
                    MaterialFlag::UseAOMap,
                    "MaterialDescBuilder: UseAOMap enabled but AO texture is invalid");

        validateMap(MaterialTexSlot::Emissive,
                    MaterialFlag::UseEmissiveMap,
                    "MaterialDescBuilder: UseEmissiveMap enabled but emissive texture is invalid");

        validateMap(MaterialTexSlot::ORM,
                    MaterialFlag::UseORMMap,
                    "MaterialDescBuilder: UseORMMap enabled but ORM texture is invalid");

        if (desc.m_Params.m_SurfaceMode == MaterialSurfaceMode::AlphaTest)
        {
            CORE_ASSERT(HasFlag(desc.m_Params.m_Flags, MaterialFlag::AlphaTest),
                        "MaterialDescBuilder: AlphaTest surface requires MaterialFlag::AlphaTest");
        }
        else
        {
            CORE_ASSERT(!HasFlag(desc.m_Params.m_Flags, MaterialFlag::AlphaTest),
                        "MaterialDescBuilder: AlphaTest flag should only be used with AlphaTest surface");
        }
    }

    void SetTextureAndEnable(MaterialTexSlot slot, TextureHandle texture, MaterialFlag flag)
    {
        m_Desc.m_Textures[(u8)slot] = MaterialTexture{ texture };
        ::EnableFlag(m_Desc.m_Params.m_Flags, flag);
    }

    void ClearTextureAndDisable(MaterialTexSlot slot, MaterialFlag flag)
    {
        m_Desc.m_Textures[(u8)slot] = MaterialTexture{};
        ::DisableFlag(m_Desc.m_Params.m_Flags, flag);
    }

    void SyncAlphaTestFlagFromSurfaceMode()
    {
        const bool alphaTest = (m_Desc.m_Params.m_SurfaceMode == MaterialSurfaceMode::AlphaTest);
        ::SetFlag(m_Desc.m_Params.m_Flags, MaterialFlag::AlphaTest, alphaTest);
    }


private:
    MaterialDesc m_Desc{};
};