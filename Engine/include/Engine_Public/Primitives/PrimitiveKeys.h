#pragma once

struct CubeKey
{
    bool m_HardNormals = true;
};

struct SphereKey
{
    uint16_t m_Seg = 64;
    uint16_t m_Ring = 32;
};

struct CylinderKey
{
    uint16_t m_Seg  = 48; 
    uint16_t m_HSeg = 1;  
    bool m_Capped   = true;
};

struct ConeKey
{
    uint16_t m_Seg  = 48; 
    bool m_Capped   = true;
};

struct TorusKey
{
    uint16_t m_Seg  = 64; 
    uint16_t m_Tube = 24; 
    uint16_t m_RatioQ = 250; // 0.250
};

struct AnnulusKey
{
    uint16_t m_Seg = 64; 
};

struct PlaneKey
{
    uint16_t m_SegX = 1; 
    uint16_t m_SegY = 1; 
};