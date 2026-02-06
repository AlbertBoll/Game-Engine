#pragma once

#include "PrimitiveMesh.h"
#include<unordered_map>

using MeshId = uint32_t;

template<class Key>
struct PrimitiveTraits;

template<class Key>
concept PrimitiveKey = requires(const Key& k)
{
    { PrimitiveTraits<Key>::Hash(k) } -> std::convertible_to<uint64_t>;
    { PrimitiveTraits<Key>::Build(k) } -> std::same_as<MeshGL>;
    { PrimitiveTraits<Key>::Validate(k) }; // return void
};

class PrimitiveFactory
{
public:
    PrimitiveFactory() = default;

    template<PrimitiveKey Key> 
    MeshId GetOrCreate(const Key& key)
    {
        PrimitiveTraits<Key>::Validate(key);
        const uint64_t h = PrimitiveTraits<Key>::Hash(key);

        if (auto it = m_MeshLookUpMap.find(h); it != m_MeshLookUpMap.end())
            return it->second;

        MeshId id = (MeshId)m_Meshes.size();
        m_Meshes.emplace_back(PrimitiveTraits<Key>::Build(key));
        m_MeshLookUpMap.emplace(h, id);
        return id;
    }

    template<PrimitiveKey... Keys>
    void PreLoad(const Keys&... keys)
    {
        (GetOrCreate(keys), ...);
    }

    const MeshGL& Get(MeshId id) const
    {
        CORE_ASSERT(id < m_Meshes.size());
        return m_Meshes[id];
    }

    MeshGL& Get(MeshId id)
    {
        CORE_ASSERT(id < m_Meshes.size());
        return m_Meshes[id];
    }

    std::size_t Size() const { return m_Meshes.size(); }

private:
    std::vector<MeshGL> m_Meshes;
    std::unordered_map<uint64_t, MeshId> m_MeshLookUpMap;

};