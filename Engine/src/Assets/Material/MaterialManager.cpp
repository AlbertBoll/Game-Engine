#include "Assets/Material/MaterialManager.h"
#include "Assets/Texture/TextureManager.h"
#include "Assets/Shader/ShaderManager.h"
#include <glad/gl.h>
#include <cstring> // std::memcpy
#include "Core/Hash.h"


namespace
{
    
    static void ValidateMaterialDesc(
        const MaterialDesc& desc,
        const ShaderManager& shaderMgr,
        const TextureManager& textureMgr)
    {
        CORE_ASSERT(shaderMgr.IsValid(desc.m_Shader), "MaterialDesc contains invalid shader handle");

        auto checkTex = [&](MaterialTexSlot slot, MaterialFlag flag, const char* msg)
        {
            if (!HasFlag(desc.m_Params.m_Flags, flag))
                return;

            const TextureHandle h = desc.m_Textures[(u8)slot].m_Texture;
            CORE_ASSERT(textureMgr.IsValid(h), msg);
        };

        checkTex(MaterialTexSlot::Albedo,            MaterialFlag::UseAlbedoMap,   "MaterialDesc: albedo map flag enabled but texture handle is invalid");
        checkTex(MaterialTexSlot::Normal,            MaterialFlag::UseNormalMap,   "MaterialDesc: normal map flag enabled but texture handle is invalid");
        checkTex(MaterialTexSlot::Metallic,          MaterialFlag::UseMetallicMap,  "MaterialDesc: invalid metallic map");
        checkTex(MaterialTexSlot::Roughness,         MaterialFlag::UseRoughnessMap, "MaterialDesc: invalid roughness map");
        checkTex(MaterialTexSlot::AO,                MaterialFlag::UseAOMap,        "MaterialDesc: invalid AO map");
        checkTex(MaterialTexSlot::Emissive,          MaterialFlag::UseEmissiveMap,  "MaterialDesc: invalid emissive map");
        checkTex(MaterialTexSlot::ORM,               MaterialFlag::UseORMMap,       "MaterialDesc: invalid ORM map");
    }

    static bool UsePackedORM(const MaterialDesc& desc)
    {
        return HasFlag(desc.m_Params.m_Flags, MaterialFlag::UseORMMap);
    }

}

std::size_t MaterialKeyHash::operator()(const MaterialKey& k) const noexcept
{
    //return (std::size_t)MaterialManager::HashKey64(k);
    std::size_t h = Combine(k.count);

    for (u8 i = 0; i < k.count; ++i)
        AppendOne(h, k.items[i]);

    return h;


}

// -------------------- Hash utilities --------------------
u64 MaterialManager::HashCombine64(u64 a, u64 b) noexcept
{
    a ^= b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2);
    return a;
}

// u64 MaterialManager::HashKey64(const MaterialKey& k) noexcept
// {
//     u64 h = 0xcbf29ce484222325ull;

//     for (u8 i = 0; i < k.count; ++i)
//         h = HashCombine64(h, k.items[i]);

//     return h;
//}

u64 MaterialManager::HashHandle64(ShaderHandle h) noexcept
{
    return HashCombine64((u64)h.m_Id, (u64)h.m_Generation);
}

u64 MaterialManager::HashParams64(const MaterialParams& p) noexcept
{
    auto hashFloatBits = [](u64 h, float v) noexcept
    {
        u32 bits = 0;
        std::memcpy(&bits, &v, sizeof(bits));
        return HashCombine64(h, (u64)bits);
    };

    auto hashVec3Bits = [&](u64 h, const Vec3f& v) noexcept
    {
        h = hashFloatBits(h, v.x);
        h = hashFloatBits(h, v.y);
        h = hashFloatBits(h, v.z);
        return h;
    };

    u64 h = 0xcbf29ce484222325ull;
    h = hashFloatBits(h, p.m_Metallic);
    h = hashFloatBits(h, p.m_Roughness);
    h = hashFloatBits(h, p.m_Exposure);
    h = hashFloatBits(h, p.m_AO);
    h = HashCombine64(h, (u64)(u32)p.m_Flags);
    h = hashVec3Bits(h, p.m_Albedo);
    h = hashVec3Bits(h, p.m_Emissive);
    h = HashCombine64(h, (u64)(u8)p.m_SurfaceMode);
    return h;
}

u64 MaterialManager::HashTexture64(const MaterialTexture& t) noexcept
{
    u64 h = 0;
    h = HashCombine64(h, (u64)t.m_Texture.m_Id);
    h = HashCombine64(h, (u64)t.m_Texture.m_Generation);
    return h;
}

// -------------------- Lifetime --------------------
MaterialManager::~MaterialManager()
{
    Clear(false);
}

void MaterialManager::Reserve(u32 materialSlots, u32 cacheEntries)
{
    m_Slots.reserve(materialSlots);
    m_FreeList.reserve(materialSlots);

    if (cacheEntries)
        m_Cache.reserve(cacheEntries);
}

void MaterialManager::Reserve(const ReserveDesc& d)
{
    Reserve(d.m_MaterialSlots, d.m_CacheEntries);
}

// -------------------- Slot helpers --------------------
MaterialHandle MaterialManager::AllocateSlot()
{
    if (!m_FreeList.empty())
    {
        const u32 idx = m_FreeList.back();
        m_FreeList.pop_back();
        return MaterialHandle{ idx + 1, m_Slots[idx].m_Generation };
    }

    m_Slots.push_back({});
    const u32 idx = (u32)m_Slots.size() - 1;
    return MaterialHandle{ idx + 1, m_Slots[idx].m_Generation };
}

MaterialManager::Slot* MaterialManager::GetSlot(MaterialHandle h)
{
    if (!h)
        return nullptr;

    const u32 idx = h.m_Id - 1;
    if (idx >= m_Slots.size())
        return nullptr;

    Slot& s = m_Slots[idx];
    if (!s.b_Alive)
        return nullptr;
    if (s.m_Generation != h.m_Generation)
        return nullptr;

    return &s;
}

const MaterialManager::Slot* MaterialManager::GetSlot(MaterialHandle h) const
{
    return const_cast<MaterialManager*>(this)->GetSlot(h);
}

bool MaterialManager::IsValid(MaterialHandle h) const
{
    return GetSlot(h) != nullptr;
}

bool MaterialManager::IsUniquelyOwned(MaterialHandle h) const
{
    const Slot* s = GetSlot(h);
    return s && s->m_RefCount == 1;
}

const MaterialDesc& MaterialManager::GetDesc(MaterialHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetDesc: invalid MaterialHandle");
    return s->m_Desc;
}

ShaderHandle MaterialManager::GetShaderHandle(MaterialHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetShaderHandle: invalid MaterialHandle");
    return s->m_Desc.m_Shader;
}

u64 MaterialManager::GetApplyHash(MaterialHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "GetApplyHash: invalid MaterialHandle");
    return s->m_ApplyHash;
}

// -------------------- Cache helpers --------------------
MaterialHandle MaterialManager::PeekFromCache(const MaterialKey& key)
{
    auto it = m_Cache.find(key);
    if (it == m_Cache.end())
        return {};

    const MaterialHandle h = it->second;
    if (!GetSlot(h))
    {
        m_Cache.erase(it);
        return {};
    }

    return h;
}

MaterialHandle MaterialManager::AcquireFromCache(const MaterialKey& key)
{
    MaterialHandle h = PeekFromCache(key);
    if (!h)
        return {};

    Slot* s = GetSlot(h);
    CORE_ASSERT(s, "AcquireFromCache: cache returned invalid handle");

    ++s->m_RefCount;
    return h;
}

MaterialHandle MaterialManager::StoreToCache(MaterialKey&& key, MaterialHandle h)
{
    Slot* s = GetSlot(h);
    CORE_ASSERT(s, "StoreToCache: invalid MaterialHandle");

    if (auto it = m_Cache.find(key); it != m_Cache.end())
    {
        if (!GetSlot(it->second))
        {
            m_Cache.erase(it);
        }
        else
        {
            CORE_ASSERT(it->second == h, "StoreToCache: duplicate live MaterialKey");
        }
    }

    s->b_HasKey = true;
    s->m_Key = std::move(key);

    if (s->m_RefCount == 0)
        s->m_RefCount = 1;

    m_Cache.insert_or_assign(s->m_Key, h);
    return h;
}

void MaterialManager::RemoveFromCache(MaterialHandle h, Slot& s)
{
    if (!s.b_HasKey)
        return;

    auto it = m_Cache.find(s.m_Key);
    if (it != m_Cache.end() && it->second == h)
        m_Cache.erase(it);

    s.b_HasKey = false;
    s.m_Key.count = 0;
}

// -------------------- Key building --------------------
// Cache key includes ALL texture slots, not only enabled ones.
// This preserves full material identity and avoids collapsing materials
// that differ only in currently-disabled textures.
MaterialKey MaterialManager::BuildCacheKey(const MaterialDesc& desc) const
{
    MaterialKey k{};
    k.count = 0;

    auto pushItem = [&](u64 value)
    {
        CORE_ASSERT(k.count < MaterialKey::kMaxItems, "MaterialKey overflow");
        k.items[k.count++] = value;
    };

    // Shader identity
    pushItem(HashHandle64(desc.m_Shader));

    // Full params (includes flags)
    pushItem(HashParams64(desc.m_Params));

    // Explicit flags item
    pushItem(HashCombine64(0xF1A6u, (u64)(u32)desc.m_Params.m_Flags));

    // Include all texture slots in the cache identity
    for (u32 i = 0; i < kMaterialMaxTextures; ++i)
    {
        const MaterialTexture& t = desc.m_Textures[i];
        const u64 texHash = HashTexture64(t);
        pushItem(texHash);
        pushItem(HashCombine64((u64)i, texHash));
    }

    return k;
}

// Apply hash reflects only FINAL GPU-visible material state.
// Disabled texture slots do not affect the GPU state, so they do not participate here.
u64 MaterialManager::BuildApplyHash(const MaterialDesc& desc) const
{
    u64 h = 0xcbf29ce484222325ull;

    h = HashCombine64(h, HashHandle64(desc.m_Shader));
    h = HashCombine64(h, HashParams64(desc.m_Params));
    h = HashCombine64(h, (u64)(u32)desc.m_Params.m_Flags);

    auto addTex = [&](MaterialTexSlot slot, MaterialFlag enableFlag)
    {
        if (!HasFlag(desc.m_Params.m_Flags, enableFlag))
            return;

        const MaterialTexture& t = desc.m_Textures[(u8)slot];
        const u64 texHash = HashTexture64(t);

        h = HashCombine64(h, texHash);
        h = HashCombine64(h, HashCombine64((u64)(u8)slot, texHash));
    };

    addTex(MaterialTexSlot::Albedo,            MaterialFlag::UseAlbedoMap);
    addTex(MaterialTexSlot::Normal,            MaterialFlag::UseNormalMap);
    addTex(MaterialTexSlot::Emissive,          MaterialFlag::UseEmissiveMap);

    if (UsePackedORM(desc))
    {
        addTex(MaterialTexSlot::ORM, MaterialFlag::UseORMMap);
    }
    else
    {
        addTex(MaterialTexSlot::Metallic,  MaterialFlag::UseMetallicMap);
        addTex(MaterialTexSlot::Roughness, MaterialFlag::UseRoughnessMap);
        addTex(MaterialTexSlot::AO,        MaterialFlag::UseAOMap);
    }


    return h;
}

// -------------------- Uniform cache --------------------
void MaterialManager::CacheUniformHandles(Slot& s) const
{
    CORE_ASSERT(m_ShaderMgr, "MaterialManager: ShaderManager not set");

    Shader& sh = m_ShaderMgr->Get(s.m_Desc.m_Shader);

    s.m_U.u_Albedo    = sh.GetUniformHandle("u_Albedo");
    s.m_U.u_Metallic  = sh.GetUniformHandle("u_Metallic");
    s.m_U.u_Roughness = sh.GetUniformHandle("u_Roughness");
    s.m_U.u_AO        = sh.GetUniformHandle("u_AO");
    s.m_U.u_Exposure  = sh.GetUniformHandle("u_Exposure");
    s.m_U.u_Emissive  = sh.GetUniformHandle("u_Emissive");
    s.m_U.u_Flags     = sh.GetUniformHandle("u_MaterialFlags");

    s.m_U.u_AlbedoMap    = sh.GetUniformHandle("u_AlbedoMap");
    s.m_U.u_NormalMap    = sh.GetUniformHandle("u_NormalMap");
    s.m_U.u_MetallicMap  = sh.GetUniformHandle("u_MetallicMap");
    s.m_U.u_RoughnessMap = sh.GetUniformHandle("u_RoughnessMap");
    s.m_U.u_AOMap        = sh.GetUniformHandle("u_AOMap");
    s.m_U.u_EmissiveMap  = sh.GetUniformHandle("u_EmissiveMap");
    s.m_U.u_ORMMap       = sh.GetUniformHandle("u_ORMMap");

    s.m_CachedProgram = sh.GetProgramHandle();
}

void MaterialManager::RefreshUniformHandlesIfNeeded(Slot& s) const
{
    CORE_ASSERT(m_ShaderMgr, "MaterialManager: ShaderManager not set");

    Shader& sh = m_ShaderMgr->Get(s.m_Desc.m_Shader);
    const u32 prog = sh.GetProgramHandle();

    if (s.m_CachedProgram != prog)
        CacheUniformHandles(s);
}

// -------------------- GPU apply --------------------
// Assumes the correct program is already bound.
void MaterialManager::ApplyUniformsAndTextures(const Slot& s) const
{
    CORE_ASSERT(m_ShaderMgr, "MaterialManager: ShaderManager not set");
    CORE_ASSERT(m_TextureMgr, "MaterialManager: TextureManager not set");

    Shader& sh = m_ShaderMgr->Get(s.m_Desc.m_Shader);
    const MaterialParams& p = s.m_Desc.m_Params;

    sh.SetUniform(s.m_U.u_Albedo,    p.m_Albedo);
    sh.SetUniform(s.m_U.u_Emissive,  p.m_Emissive);
    sh.SetUniform(s.m_U.u_Metallic,  p.m_Metallic);
    sh.SetUniform(s.m_U.u_AO,        p.m_AO);
    sh.SetUniform(s.m_U.u_Roughness, p.m_Roughness);
    sh.SetUniform(s.m_U.u_Exposure,  p.m_Exposure);

    if (s.m_U.u_Flags)
        sh.SetUniform(s.m_U.u_Flags, (int)(u32)p.m_Flags);

    u32 unit = 0;
    auto bindResolved = [&](TextureHandle tex, UniformHandle samplerLoc)
    {
        m_TextureMgr->Bind(tex, unit);
        if (samplerLoc)
            sh.SetUniform(samplerLoc, (int)unit);
        ++unit;
    };

    // Always bind these samplers
    bindResolved(
        ResolveTextureOrDefault(s, MaterialTexSlot::Albedo, DefaultTextureKind::White),
        s.m_U.u_AlbedoMap);

    bindResolved(
        ResolveTextureOrDefault(s, MaterialTexSlot::Normal, DefaultTextureKind::FlatNormal),
        s.m_U.u_NormalMap);

    bindResolved(
        ResolveTextureOrDefault(s, MaterialTexSlot::Emissive, DefaultTextureKind::White),
        s.m_U.u_EmissiveMap);

    if (HasFlag(p.m_Flags, MaterialFlag::UseORMMap))
    {
        bindResolved(
            ResolveTextureOrDefault(s, MaterialTexSlot::ORM, DefaultTextureKind::WhiteORM),
            s.m_U.u_ORMMap);
    }
    else
    {
        bindResolved(
            ResolveTextureOrDefault(s, MaterialTexSlot::Metallic, DefaultTextureKind::WhiteR),
            s.m_U.u_MetallicMap);

        bindResolved(
            ResolveTextureOrDefault(s, MaterialTexSlot::Roughness, DefaultTextureKind::WhiteR),
            s.m_U.u_RoughnessMap);

        bindResolved(
            ResolveTextureOrDefault(s, MaterialTexSlot::AO, DefaultTextureKind::WhiteR),
            s.m_U.u_AOMap);
    }

    // auto bindTex = [&](MaterialTexSlot slot, UniformHandle samplerLoc, MaterialFlag enableFlag)
    // {
    //     if (!HasFlag(p.m_Flags, enableFlag))
    //         return;

    //     const MaterialTexture& t = s.m_Desc.m_Textures[(u8)slot];
    //     CORE_ASSERT(m_TextureMgr->IsValid(t.m_Texture), "Material flag enabled but texture handle is invalid");
    //     //CORE_ASSERT(t.m_TexId != 0, "Material flag enabled but texture id is 0");

    //     m_TextureMgr->Bind(t.m_Texture, unit);

    //     if (samplerLoc)
    //         sh.SetUniform(samplerLoc, (int)unit);

    //     ++unit;
    // };

    // bindTex(MaterialTexSlot::Albedo,            s.m_U.u_AlbedoMap,   MaterialFlag::UseAlbedoMap);
    // bindTex(MaterialTexSlot::Normal,            s.m_U.u_NormalMap,   MaterialFlag::UseNormalMap);
    // bindTex(MaterialTexSlot::Emissive,          s.m_U.u_EmissiveMap, MaterialFlag::UseEmissiveMap);

    // if (UsePackedORM(s.m_Desc))
    // {
    //     bindTex(MaterialTexSlot::ORM, s.m_U.u_ORMMap, MaterialFlag::UseORMMap);
    // }
    // else
    // {
    //     bindTex(MaterialTexSlot::Metallic,  s.m_U.u_MetallicMap,  MaterialFlag::UseMetallicMap);
    //     bindTex(MaterialTexSlot::Roughness, s.m_U.u_RoughnessMap, MaterialFlag::UseRoughnessMap);
    //     bindTex(MaterialTexSlot::AO,        s.m_U.u_AOMap,        MaterialFlag::UseAOMap);
    // }
}

// -------------------- Public API --------------------
MaterialHandle MaterialManager::GetOrCreate(const MaterialDesc& desc, MaterialCacheMode cacheMode)
{
    CORE_ASSERT(m_ShaderMgr, "MaterialManager: call SetShaderManager() first");
    CORE_ASSERT(m_TextureMgr, "MaterialManager: call SetTextureManager() first");
    ValidateMaterialDesc(desc, *m_ShaderMgr, *m_TextureMgr);

    //CORE_ASSERT(m_ShaderMgr->IsValid(desc.m_Shader), "GetOrCreate: invalid shader handle");

    MaterialKey cacheKey = BuildCacheKey(desc);
    const u64 applyHash = BuildApplyHash(desc);

    if (cacheMode == MaterialCacheMode::UseCache)
    {
        if (MaterialHandle cached = AcquireFromCache(cacheKey))
            return cached;
    }

    MaterialHandle h = AllocateSlot();
    Slot& s = m_Slots[h.m_Id - 1];

    s.b_Alive = true;
    s.m_RefCount = 1;

    s.b_HasKey = false;
    s.m_Key.count = 0;

    s.m_Desc = desc;
    s.m_U = CachedUniforms{};
    s.m_CachedProgram = 0;
    s.m_ApplyHash = applyHash;

    CacheUniformHandles(s);

    if (cacheMode == MaterialCacheMode::UseCache)
        return StoreToCache(std::move(cacheKey), h);

    return h;
}

MaterialHandle MaterialManager::Clone(MaterialHandle src, MaterialCacheMode cacheMode)
{
    const Slot* s = GetSlot(src);
    CORE_ASSERT(s, "Clone: invalid MaterialHandle");

    return GetOrCreate(s->m_Desc, cacheMode);
}

MaterialHandle MaterialManager::Duplicate(MaterialHandle src)
{
    return Clone(src, MaterialCacheMode::NoCache);
}

void MaterialManager::Update(MaterialHandle &h, const MaterialDesc &desc, MaterialCacheMode cacheMode)
{
    CORE_ASSERT(m_ShaderMgr, "MaterialManager: call SetShaderManager() first");
    CORE_ASSERT(m_TextureMgr, "MaterialManager: call SetTextureManager() first");
    ValidateMaterialDesc(desc, *m_ShaderMgr, *m_TextureMgr);

    Slot* current = GetSlot(h);
    if (!current)
    {
        h = GetOrCreate(desc, cacheMode);
        return;
    }

    MaterialKey newCacheKey = BuildCacheKey(desc);
    const u64 newApplyHash = BuildApplyHash(desc);

    // If cache is enabled and another canonical slot already exists for the new desc,
    // reuse it instead of mutating the current slot into a duplicate.
    if (cacheMode == MaterialCacheMode::UseCache)
    {
        MaterialHandle existing = PeekFromCache(newCacheKey);
        if (existing && existing != h)
        {
            Slot* dst = GetSlot(existing);
            CORE_ASSERT(dst, "Update: cache returned invalid handle");

            ++dst->m_RefCount;
            Release(h);
            h = existing;
            return;
        }
    }

    // Shared slot: do copy-on-write.
    // We must not mutate a shared canonical material in place.
    if (current->m_RefCount > 1)
    {
        --current->m_RefCount;
        h = GetOrCreate(desc, cacheMode);
        return;
    }

    // Unique slot: safe to rebuild in place.
    RemoveFromCache(h, *current);

    current->m_Desc = desc;
    current->m_U = CachedUniforms{};
    current->m_CachedProgram = 0;
    current->m_ApplyHash = newApplyHash;

    current->b_HasKey = false;
    current->m_Key.count = 0;

    CacheUniformHandles(*current);

    if (cacheMode == MaterialCacheMode::UseCache)
        StoreToCache(std::move(newCacheKey), h);

    // Current material contents changed, so local bind-skip cache must be invalidated.
    InvalidateBindCache();
}

void MaterialManager::BindShader(MaterialHandle h) const
{
    const Slot* s = GetSlot(h);
    if (!s)
        return;

    CORE_ASSERT(m_ShaderMgr, "MaterialManager: ShaderManager not set");
    CORE_ASSERT(m_TextureMgr, "MaterialManager: TextureManager not set");
    // Bind program through ShaderManager.
    m_ShaderMgr->Bind(s->m_Desc.m_Shader);

    RefreshUniformHandlesIfNeeded(*const_cast<Slot*>(s));

    Shader& sh = m_ShaderMgr->Get(s->m_Desc.m_Shader);
    m_BoundProgram = sh.GetProgramHandle();
}

void MaterialManager::Bind(MaterialHandle h) const
{
    const Slot* s = GetSlot(h);
    if (!s)
        return;

    CORE_ASSERT(m_ShaderMgr, "MaterialManager: ShaderManager not set");
    CORE_ASSERT(m_TextureMgr, "MaterialManager: TextureManager not set");

    m_ShaderMgr->Bind(s->m_Desc.m_Shader);
    RefreshUniformHandlesIfNeeded(*const_cast<Slot*>(s));

    Shader& sh = m_ShaderMgr->Get(s->m_Desc.m_Shader);
    const u32 prog = sh.GetProgramHandle();

    if (m_BoundProgram == prog && m_BoundMaterialApplyHash == s->m_ApplyHash)
        return;

    ApplyUniformsAndTextures(*s);

    m_BoundProgram = prog;
    m_BoundMaterialApplyHash = s->m_ApplyHash;
}

void MaterialManager::ApplyMaterial(MaterialHandle h) const
{
    const Slot* s = GetSlot(h);
    if (!s)
        return;

    CORE_ASSERT(m_ShaderMgr, "MaterialManager: ShaderManager not set");
    CORE_ASSERT(m_TextureMgr, "MaterialManager: TextureManager not set");

    // Assumes the correct shader program is already bound by the caller.
    RefreshUniformHandlesIfNeeded(*const_cast<Slot*>(s));

    Shader& sh = m_ShaderMgr->Get(s->m_Desc.m_Shader);
    const u32 prog = sh.GetProgramHandle();

    if (m_BoundProgram == prog && m_BoundMaterialApplyHash == s->m_ApplyHash)
        return;

    ApplyUniformsAndTextures(*s);

    m_BoundProgram = prog;
    m_BoundMaterialApplyHash = s->m_ApplyHash;
}

void MaterialManager::Release(MaterialHandle h)
{
    Slot* s = GetSlot(h);
    if (!s)
        return;

    CORE_ASSERT(s->m_RefCount > 0, "Release: ref-count underflow");

    if (s->m_RefCount > 1)
    {
        --s->m_RefCount;
        return;
    }

    RemoveFromCache(h, *s);

    s->m_Desc = MaterialDesc{};
    s->m_U = CachedUniforms{};
    s->m_CachedProgram = 0;
    s->m_ApplyHash = 0;
    s->m_RefCount = 0;

    s->b_Alive = false;
    ++s->m_Generation;

    m_FreeList.push_back(h.m_Id - 1);

    InvalidateBindCache();
}

void MaterialManager::Clear(bool keepCapacity)
{
    m_Cache.clear();

    for (auto& s : m_Slots)
    {
        s.b_Alive = false;
        s.m_RefCount = 0;
        s.b_HasKey = false;
        s.m_Key.count = 0;
        s.m_Desc = MaterialDesc{};
        s.m_U = CachedUniforms{};
        s.m_CachedProgram = 0;
        s.m_ApplyHash = 0;
        ++s.m_Generation;
    }

    m_FreeList.clear();

    if (keepCapacity)
    {
        m_FreeList.reserve(m_Slots.size());
        for (u32 i = 0; i < (u32)m_Slots.size(); ++i)
            m_FreeList.push_back(i);
    }
    else
    {
        m_Slots.clear();
        m_FreeList.clear();
    }

    InvalidateBindCache();
}

void MaterialManager::InvalidateBindCache() const
{
    m_BoundProgram = 0;
    m_BoundMaterialApplyHash = 0;

    if (m_ShaderMgr)
        m_ShaderMgr->InvalidateBindCache();

    if (m_TextureMgr)
        m_TextureMgr->InvalidateBindCache();
}
TextureHandle MaterialManager::ResolveTextureOrDefault(const Slot &s, MaterialTexSlot slot, DefaultTextureKind defKind) const
{
    const TextureHandle real = s.m_Desc.m_Textures[(u8)slot].m_Texture;
    if (real && m_TextureMgr->IsValid(real))
        return real;

    return m_TextureMgr->GetDefaultTexture(defKind);
}