#pragma once

#include "PrimitiveMesh.h"
#include "PrimitiveKeys.h"
#include "PrimitiveCache.h"
#include "PrimitiveGen.h"
#include "PrimitiveUtils.h"

template<>
struct PrimitiveTraits<CubeKey>
{
    static void Validate(const CubeKey&) { }

    static uint64_t Hash(const CubeKey& k)
    {
        uint64_t tag = 0xC0B3u;
        return HashFields(tag, 
            HashAny(k.m_HardNormals)
        );

        // h = HashCombine64(h, HashBool(k.m_HardNormals));
        // return h;
    }

    static MeshGL Build(const CubeKey& k)
    {
        return PrimitiveGen::CreateCube(k.m_HardNormals);   
    }
 
};

template<>
struct PrimitiveTraits<SphereKey>
{
    static void Validate(const SphereKey& k)
    {
        CORE_ASSERT(k.m_Seg >= 3);
        CORE_ASSERT(k.m_Ring >= 2);
    }

    static uint64_t Hash(const SphereKey& k)
    {
        uint64_t tag = 0x53504852ull; // "SPHR"
        return HashFields(tag, 
            HashAny(k.m_Seg), 
            HashAny(k.m_Ring)
        );

        // h = HashCombine64(h, HashU16(k.m_Seg));
        // h = HashCombine64(h, HashU16(k.m_Ring));
        // return h;
    }

    static MeshGL Build(const SphereKey& k)
    {
        return PrimitiveGen::CreateSphere(k.m_Seg, k.m_Ring);
    }
};
 
template<>
struct PrimitiveTraits<CylinderKey>
{
    static void Validate(const CylinderKey& k)
    {
        CORE_ASSERT(k.m_Seg >= 3);
        CORE_ASSERT(k.m_HSeg >= 1);
    }

    static uint64_t Hash(const CylinderKey& k)
    {
        uint64_t tag = 0x43594C4Eull; // "CYLN"
        return HashFields(tag, 
            HashAny(k.m_Seg), 
            HashAny(k.m_HSeg),
            HashAny(k.m_Capped)
        );

        // h = HashCombine64(h, HashU16(k.m_Seg));
        // h = HashCombine64(h, HashU16(k.m_HSeg));
        // h = HashCombine64(h, HashBool(k.m_Capped));
        // return h;
    }

    static MeshGL Build(const CylinderKey& k)
    {
        return PrimitiveGen::CreateCylinder(k.m_Seg, k.m_HSeg, k.m_Capped);
    }
};

template<>
struct PrimitiveTraits<ConeKey>
{
    static void Validate(const ConeKey& k)
    {
        CORE_ASSERT(k.m_Seg >= 3);
    }

    static uint64_t Hash(const ConeKey& k)
    {
        uint64_t tag = 0x434F4E45ull; // "CONE"
        return HashFields(tag, 
            HashAny(k.m_Seg), 
            HashAny(k.m_Capped)
        );

        // h = HashCombine64(h, HashU16(k.m_Seg));
        // h = HashCombine64(h, HashBool(k.m_Capped));
        // return h;
    }

    static MeshGL Build(const ConeKey& k)
    {
        // 约定：side smooth + base cap hard + seam hard（拆顶点）
        return PrimitiveGen::CreateCone(k.m_Seg, k.m_Capped);
    }
};

template<>
struct PrimitiveTraits<TorusKey>
{
    static void Validate(const TorusKey& k)
    {
        CORE_ASSERT(k.m_Seg >= 3);
        CORE_ASSERT(k.m_Tube >= 3);
        CORE_ASSERT(k.m_RatioQ >= 10);
        CORE_ASSERT(k.m_RatioQ <= 490);
    }

    static uint64_t Hash(const TorusKey& k)
    {
        uint64_t tag = 0x544F5253ull; // "TORS"
        return HashFields(tag, 
            HashAny(k.m_Seg), 
            HashAny(k.m_Tube),
            HashAny(k.m_RatioQ)
        );

        // h = HashCombine64(h, HashU16(k.m_Seg));
        // h = HashCombine64(h, HashU16(k.m_Tube));
        // h = HashCombine64(h, HashU16(k.m_RatioQ));
        // return h;
    }

    static MeshGL Build(const TorusKey& k)
    {
        const float ratio = (float)k.m_RatioQ / 1000.0f;
        return PrimitiveGen::CreateTorus(k.m_Seg, k.m_Tube, ratio);
    }
};

template<>
struct PrimitiveTraits<AnnulusKey>
{
    static void Validate(const AnnulusKey& k)
    {
        CORE_ASSERT(k.m_Seg >= 3);
    }

    static uint64_t Hash(const AnnulusKey& k)
    {
        uint64_t tag = 0x52494E47ull; // "RING"
        return HashFields(tag, 
            HashAny(k.m_Seg)
        );
        
        // h = HashCombine64(h, HashU16(k.m_Seg));
        // return h;
    }

    static MeshGL Build(const AnnulusKey& k)
    {
        return PrimitiveGen::CreateAnnulus2D(k.m_Seg);
    }
};