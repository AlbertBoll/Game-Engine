#include "Assets/Mesh/MeshManager.h"

MeshHandle MeshManager::CreateInternal(MeshGL&& mesh, std::string_view /*debugName*/)
{
    auto ph = m_Pool.Allocate();
    MeshHandle h = FromPool(ph);

    Slot* s = m_Pool.Get(ph);
    CORE_ASSERT(s);

    // MeshGL/VAO/VBO/IBO move-assign first Release then move
    s->mesh = std::move(mesh);
    s->refCount = 1;

    s->hasPrimKey = false;
    s->primKey = 0;

    s->hasFileKey = false;
    s->fileKey.clear();

    return h;
}

const MeshGL& MeshManager::GetGL(MeshHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "Invalid MeshHandle");
    return s->mesh;
}

MeshGL& MeshManager::GetGL(MeshHandle h)
{
    Slot* s = GetSlot(h);
    CORE_ASSERT(s, "Invalid MeshHandle");
    return s->mesh;
}

u32 MeshManager::GetIndexCount(MeshHandle h) const
{
    const Slot* s = GetSlot(h);
    CORE_ASSERT(s, "Invalid MeshHandle");
    return s->mesh.m_IndexCount;
}

void MeshManager::Destroy(MeshHandle h)
{
    Slot* s = GetSlot(h);
    if (!s) return;

    if (s->refCount > 1)
    {
        s->refCount--;
        return;
    }

    // last ref: remove from caches (only if entry matches current handle)
    if (s->hasPrimKey)
    {
        auto it = m_PrimCache.find(s->primKey);
        if (it != m_PrimCache.end() && it->second == h)
            m_PrimCache.erase(it);
        s->hasPrimKey = false;
        s->primKey = 0;
    }

    if (s->hasFileKey)
    {
        auto it = m_FileCache.find(s->fileKey);
        if (it != m_FileCache.end() && it->second == h)
            m_FileCache.erase(it);
        s->hasFileKey = false;
        s->fileKey.clear();
    }

    // free GPU
    s->mesh = MeshGL{};
    s->refCount = 0;

    m_Pool.Free(ToPool(h));
}

void MeshManager::Clear(bool keepCapacity)
{
    m_PrimCache.clear();
    m_FileCache.clear();

    for (auto& s : m_Pool.Slots())
    {
        if (s.alive)
        {
            s.mesh = MeshGL{};
            s.refCount = 0;
            s.hasPrimKey = false;
            s.primKey = 0;
            s.hasFileKey = false;
            s.fileKey.clear();
        }
    }

    if (keepCapacity)
        m_Pool.ResetKeepCapacity();
    else
        m_Pool = HandlePool<Slot>{};
}