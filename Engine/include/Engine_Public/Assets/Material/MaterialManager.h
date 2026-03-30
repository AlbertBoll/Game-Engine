#pragma once

#include "Core/Base.h"
#include "Assets/Material/MaterialDesc.h"
#include "Assets/Texture/TextureDesc.h"
#include "Assets/Shader/ShaderDesc.h"
#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

class ShaderManager;
class TextureManager;

// -------------------- Material Cache Mode --------------------
enum class MaterialCacheMode : u8
{
    UseCache = 0,
    NoCache  = 1,
};


static constexpr u8 kMaterialMaxTextures = (u8)MaterialTexSlot::Count;

struct MaterialTexture
{
    // TextureTarget m_Target = TextureTarget::Tex2D;
    // u32 m_TexId = 0; // raw OpenGL texture object id
    TextureHandle m_Texture{};
};

// -------------------- Material Desc --------------------
struct MaterialDesc
{
    ShaderHandle   m_Shader{};
    MaterialParams m_Params{};
    std::array<MaterialTexture, kMaterialMaxTextures> m_Textures{};
};

// -------------------- Material Cache Key --------------------
// Used only for material-object caching / sharing identity.
//
// Important:
// This key includes ALL texture slots, not only currently enabled ones.
// That prevents incorrect canonicalization when a currently-disabled texture
// may later become visible after flags are changed via Update().
struct MaterialKey
{
    static constexpr u8 kMaxItems = 3 + kMaterialMaxTextures * 2;

    u8 count = 0;
    std::array<u64, kMaxItems> items{};

    friend bool operator==(const MaterialKey& a, const MaterialKey& b) noexcept
    {
        if (a.count != b.count)
            return false;

        for (u8 i = 0; i < a.count; ++i)
        {
            if (a.items[i] != b.items[i])
                return false;
        }

        return true;
    }
};

struct MaterialKeyHash
{
    std::size_t operator()(const MaterialKey& k) const noexcept;
};




// -------------------- MaterialManager --------------------
class ENGINE_API MaterialManager
{
    friend struct MaterialKeyHash;

public:
    struct ReserveDesc
    {
        u32 m_MaterialSlots = 256;
        u32 m_CacheEntries  = 256;
    };

private:
    struct CachedUniforms
    {
        UniformHandle u_Albedo{};
        UniformHandle u_Metallic{};
        UniformHandle u_Roughness{};
        UniformHandle u_AO{};
        UniformHandle u_Exposure{};
        UniformHandle u_Emissive{};
        UniformHandle u_Flags{};

        UniformHandle u_AlbedoMap{};
        UniformHandle u_NormalMap{};
        UniformHandle u_MetallicMap{};
        UniformHandle u_RoughnessMap{};
        UniformHandle u_AOMap{};
        UniformHandle u_EmissiveMap{};
        UniformHandle u_ORMMap{};
    };

    struct Slot
    {
        bool b_Alive = false;
        u32  m_Generation = 1;

        // Number of references to this material slot.
        u32 m_RefCount = 0;

        // Valid only when this slot participates in the sharing cache.
        bool b_HasKey = false;
        MaterialKey m_Key{};

        MaterialDesc   m_Desc{};
        CachedUniforms m_U{};

        // GL program used when uniform handles were fetched.
        u32 m_CachedProgram = 0;

        // Hash used only for "skip redundant material apply" logic.
        // This exists even when cache mode is NoCache.
        //
        // Unlike MaterialKey, this hash only reflects the FINAL GPU-visible material state
        // (shader + params + currently enabled textures).
        u64 m_ApplyHash = 0;
    };

public:
    MaterialManager() = default;
    ~MaterialManager();

    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    void Reserve(u32 materialSlots, u32 cacheEntries = 0);
    void Reserve(const ReserveDesc& d);

    void SetShaderManager(ShaderManager& sm) { m_ShaderMgr = &sm; }
    void SetTextureManager(TextureManager& tm) {m_TextureMgr = &tm; }

    // Create or share a material.
    MaterialHandle GetOrCreate(
        const MaterialDesc& desc,
        MaterialCacheMode cacheMode = MaterialCacheMode::UseCache);

    MaterialHandle Clone(
    MaterialHandle src,
    MaterialCacheMode cacheMode = MaterialCacheMode::NoCache);

    MaterialHandle Duplicate(MaterialHandle src);
   

    // Update may replace the incoming handle in-place.
    // Caller does NOT need to manually capture a return value.
    void Update(
        MaterialHandle& h,
        const MaterialDesc& desc,
        MaterialCacheMode cacheMode = MaterialCacheMode::UseCache);

    // Release one reference.
    // For cached/shared materials this decrements ref-count;
    // for unique materials this frees the slot.
    void Release(MaterialHandle h);

    // Backward-compatible wrapper.
    void Destroy(MaterialHandle h) { Release(h); }

    void Bind(MaterialHandle h) const;

    // Split bind API:
    // 1) BindShader() binds program only.
    // 2) ApplyMaterial() uploads material parameters and binds textures only.
    //
    // ApplyMaterial() assumes the correct shader program is already bound.
    void BindShader(MaterialHandle h) const;
    void ApplyMaterial(MaterialHandle h) const;

    bool IsValid(MaterialHandle h) const;
    bool IsUniquelyOwned(MaterialHandle h) const;

    const MaterialDesc& GetDesc(MaterialHandle h) const;

    // Useful for renderer-side sorting / bucketing.
    ShaderHandle GetShaderHandle(MaterialHandle h) const;
    u64          GetApplyHash(MaterialHandle h) const;

    void Clear(bool keepCapacity = true);

    // Call this if something outside MaterialManager changes GL program/material state tracking.
    void InvalidateBindCache() const;

private:
    static u64 HashCombine64(u64 a, u64 b) noexcept;
    //static u64 HashKey64(const MaterialKey& k) noexcept;
    static u64 HashHandle64(ShaderHandle h) noexcept;
    static u64 HashParams64(const MaterialParams& p) noexcept;
    static u64 HashTexture64(const MaterialTexture& t) noexcept;

    MaterialKey BuildCacheKey(const MaterialDesc& desc) const;
    u64         BuildApplyHash(const MaterialDesc& desc) const;

    void CacheUniformHandles(Slot& s) const;
    void RefreshUniformHandlesIfNeeded(Slot& s) const;
    void ApplyUniformsAndTextures(const Slot& s) const;

private:
    MaterialHandle AllocateSlot();
    Slot*          GetSlot(MaterialHandle h);
    const Slot*    GetSlot(MaterialHandle h) const;

    MaterialHandle PeekFromCache(const MaterialKey& key);
    MaterialHandle AcquireFromCache(const MaterialKey& key);
    MaterialHandle StoreToCache(MaterialKey&& key, MaterialHandle h);
    void           RemoveFromCache(MaterialHandle h, Slot& s);
    TextureHandle ResolveTextureOrDefault(const Slot& s,
                                      MaterialTexSlot slot,
                                      DefaultTextureKind defKind) const;

private:
    ShaderManager* m_ShaderMgr = nullptr;
    TextureManager* m_TextureMgr = nullptr;

    std::vector<Slot> m_Slots;
    std::vector<u32>  m_FreeList;
    std::unordered_map<MaterialKey, MaterialHandle, MaterialKeyHash> m_Cache;

    mutable u32 m_BoundProgram = 0;
    mutable u64 m_BoundMaterialApplyHash = 0;
};