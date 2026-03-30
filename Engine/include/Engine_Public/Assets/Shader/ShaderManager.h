#pragma once

#include "Assets/Shader/Shader.h"
//#include "Assets/Shader/ShaderDesc.h"
//#include "Core/Base.h"


class ENGINE_API ShaderManager
{
public:
    ShaderManager() = default;
    ~ShaderManager();

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    void Reserve(u32 shaderCount, u32 cacheEntries = 0);
    void Reserve(const ShaderReserveDesc& desc);

    // --- Build or reuse ---
    ShaderHandle GetOrCreateFromFiles(std::span<const std::string_view> files,
                                      ShaderCacheMode mode = ShaderCacheMode::USE_CACHE);

    ShaderHandle GetOrCreateFromFiles(std::initializer_list<const char*> files,
                                      ShaderCacheMode mode = ShaderCacheMode::USE_CACHE);

    ShaderHandle GetOrCreateFromSources(std::span<const ShaderStageSource> sources,
                                        ShaderCacheMode mode = ShaderCacheMode::USE_CACHE);

    // --- Bind + access ---
    void Bind(ShaderHandle h) const;
    void Unbind() const;

    Shader&       Get(ShaderHandle h);
    const Shader& Get(ShaderHandle h) const;

    bool IsValid(ShaderHandle h) const;

    // --- Lifetime ---
    void Destroy(ShaderHandle h);

    void Clear(ClearMode mode = ClearMode::KEEP_CAPACITY);

    // 外部如果绕过 manager 直接 glUseProgram(...)，调用一次
    void InvalidateBindCache() const;

public:
    // hash helpers（给 ShaderKeyHash 用；放 public 避免 friend/重复实现）
    static u64 HashString64(std::string_view s);
    static u64 HashCombine64(u64 a, u64 b);
    static u64 HashStageItem(ShaderStage st, u64 payloadHash, u64 kindTag);
    static ShaderStage InferStageFromPath(std::string_view p);

private:
    struct Slot
    {
        bool b_Alive = false;
        u32  m_Generation = 1;

        Shader m_Shader{};
        u32    m_RefCount = 0;

        bool     b_HasKey = false;
        ShaderKey m_Key{};
    };

private:
    ShaderHandle GetOrCreateFromSources(std::span<const ShaderStageSource> sources,
                                        bool useCache = true);


private:
    std::vector<Slot> m_Slots;
    std::vector<u32>  m_FreeList;

    std::unordered_map<ShaderKey, ShaderHandle, ShaderKeyHash> m_Cache;

    // bind cache (const bind => mutable)
    mutable u32 m_BoundProgram = 0;

private:
    ShaderHandle AllocateSlot();
    Slot*       GetSlot(ShaderHandle h);
    const Slot* GetSlot(ShaderHandle h) const;

    ShaderHandle TryGetFromCache(const ShaderKey& key);
    ShaderHandle StoreToCache(ShaderKey&& key, ShaderHandle h);
};