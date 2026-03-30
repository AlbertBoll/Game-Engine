#pragma once

#include "Core/HandlePool.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

//PrimitiveTraits
#include "Primitives/PrimitiveTraits.h"

// MeshGL
#include "Primitives/PrimitiveMesh.h"

#include "MeshDesc.h"

class MeshManager
{
public:
    MeshManager() = default;
    ~MeshManager() = default;

    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager&) = delete;

    void Reserve(const MeshReserveDesc& desc)
    {
        Reserve(desc.m_MeshSlots, desc.m_PrimCacheCount, desc.m_FileCacheCount);
    }

    void Reserve(u32 meshCount, u32 primCacheCount = 0, u32 fileCacheCount = 0)
    {
        m_Pool.Reserve(meshCount);
        if (primCacheCount) m_PrimCache.reserve(primCacheCount);
        if (fileCacheCount) m_FileCache.reserve(fileCacheCount);
    }

    // ---------------- Primitive：Key -> Hash -> Cache ----------------
    template<PrimitiveKey Key>
    MeshHandle GetOrCreate(const Key& key, std::string_view debugName = {})
    {
        PrimitiveTraits<Key>::Validate(key);

        // use Traits provide hash
        const u64 primKey = PrimitiveTraits<Key>::Hash(key);

        // cache hit
        if (auto it = m_PrimCache.find(primKey); it != m_PrimCache.end())
        {
            MeshHandle h = it->second;
            if (Slot* s = GetSlot(h))
            {
                s->refCount++;
                return h;
            }
            m_PrimCache.erase(it); // 残留坏 handle（极少）
        }

        // miss -> build -> create
        MeshGL mesh = PrimitiveTraits<Key>::Build(key);
        MeshHandle h = CreateInternal(std::move(mesh), debugName);

        if (Slot* s = GetSlot(h))
        {
            s->hasPrimKey = true;
            s->primKey = primKey;
        }

        m_PrimCache.emplace(primKey, h);
        return h;
    }

    // ---------------- File：Path -> Cache（loader user provided） ----------------
    template<class Loader>
    MeshHandle GetOrCreateFromFile(const MeshFileLoadDesc& d, Loader&& loader)
    {
        CORE_ASSERT(loader, "MeshManager: loader is required");
        if (d.m_Path.empty()) return {};

        std::string pathKey(d.m_Path);

        if (d.b_UseCache)
        {
            if (auto it = m_FileCache.find(pathKey); it != m_FileCache.end())
            {
                MeshHandle h = it->second;
                if (Slot* s = GetSlot(h))
                {
                    s->refCount++;
                    return h;
                }
                m_FileCache.erase(it);
            }
        }

        MeshGL mesh = loader(d.m_Path);
        MeshHandle h = CreateInternal(std::move(mesh), d.m_DebugName);

        if (Slot* s = GetSlot(h))
        {
            s->hasFileKey = true;
            s->fileKey = pathKey;
        }

        if (d.b_UseCache)
            m_FileCache.emplace(std::move(pathKey), h);

        return h;
    }

    // ---------------- Access ----------------
    bool IsValid(MeshHandle h) const { return GetSlot(h) != nullptr; }

    const MeshGL& GetGL(MeshHandle h) const;
    MeshGL&       GetGL(MeshHandle h);

    u32 GetIndexCount(MeshHandle h) const;

    // ---------------- Lifetime ----------------
    void Destroy(MeshHandle h);
    void Clear(bool keepCapacity = true);

private:
    struct Slot
    {
        bool alive = false;
        u32  generation = 1;

        MeshGL mesh{};          // 需要 MeshGL 默认构造为空
        u32   refCount = 0;

        bool hasPrimKey = false;
        u64  primKey = 0;

        bool hasFileKey = false;
        std::string fileKey{};
    };

private:
    static HandlePool<Slot>::Handle ToPool(MeshHandle h) { return { h.m_Id, h.m_Generation }; }
    static MeshHandle FromPool(HandlePool<Slot>::Handle h) { return { h.id, h.generation }; }

    Slot*       GetSlot(MeshHandle h)       { return m_Pool.Get(ToPool(h)); }
    const Slot* GetSlot(MeshHandle h) const { return m_Pool.Get(ToPool(h)); }

    MeshHandle CreateInternal(MeshGL&& mesh, std::string_view debugName);

private:
    HandlePool<Slot> m_Pool;

    std::unordered_map<u64, MeshHandle>         m_PrimCache; // primHash -> handle
    std::unordered_map<std::string, MeshHandle> m_FileCache; // path     -> handle
};