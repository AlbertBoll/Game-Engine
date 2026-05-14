#pragma once

struct ENGINE_API CubeKey
{
    bool m_HardNormals = true;
};

struct ENGINE_API SphereKey
{
    uint16_t m_Seg = 64;
    uint16_t m_Ring = 32;
};

struct ENGINE_API CylinderKey
{
    uint16_t m_Seg  = 48; 
    uint16_t m_HSeg = 1;  
    bool m_Capped   = true;
};

struct ENGINE_API ConeKey
{
    uint16_t m_Seg  = 48; 
    bool m_Capped   = true;
};

struct ENGINE_API TorusKey
{
    uint16_t m_Seg  = 64; 
    uint16_t m_Tube = 24; 
    uint16_t m_RatioQ = 250; // 0.250
};

struct ENGINE_API AnnulusKey
{
    uint16_t m_Seg = 64; 
};

struct ENGINE_API PlaneKey
{
    uint16_t m_SegX = 1; 
    uint16_t m_SegY = 1; 
};

struct ENGINE_API TriangleKey
{
   
};