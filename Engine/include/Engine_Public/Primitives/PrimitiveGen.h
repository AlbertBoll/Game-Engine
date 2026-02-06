#pragma once

#include "PrimitiveMesh.h"

#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

static inline float Dot3(float ax,float ay,float az, float bx,float by,float bz)
{ return ax*bx + ay*by + az*bz; }

static inline void Cross3(float ax,float ay,float az, float bx,float by,float bz,
                          float& rx,float& ry,float& rz)
{
    rx = ay*bz - az*by;
    ry = az*bx - ax*bz;
    rz = ax*by - ay*bx;
}

static inline float LenSq3(float x,float y,float z) { return x*x + y*y + z*z; }

static inline bool Normalize3Safe(float& x,float& y,float& z, float eps = 1e-20f)
{
    float l2 = x*x + y*y + z*z;
    if (l2 <= eps) return false;
    float inv = 1.0f / std::sqrt(l2);
    x *= inv; y *= inv; z *= inv;
    return true;
}

// 更稳的 fallback：选与 N “最不平行”的轴
static inline void OrthoFallbackTangent(float nx,float ny,float nz,
                                       float& tx,float& ty,float& tz)
{
    float anx = std::fabs(nx), any = std::fabs(ny), anz = std::fabs(nz);
    float ax=0, ay=0, az=0;
    if (anx <= any && anx <= anz) ax = 1.0f;
    else if (any <= anx && any <= anz) ay = 1.0f;
    else az = 1.0f;

    // T = normalize(cross(A, N))
    Cross3(ax,ay,az, nx,ny,nz, tx,ty,tz);
    if (!Normalize3Safe(tx,ty,tz))
    {
        tx = 1; ty = 0; tz = 0;
    }
}

static inline void BuildTangentSign(std::vector<PrimVertex>& verts,
                             const std::vector<uint32_t>& indices)
{
    constexpr float kEpsDet      = 1e-10f;
    constexpr float kEpsTLenSq   = 1e-20f;
    constexpr float kEpsBAccLenSq= 1e-20f;

    struct Acc { float tx,ty,tz; float bx,by,bz; };
    std::vector<Acc> acc(verts.size(), {0,0,0, 0,0,0});

    // --- triangle pass: compute Ttri/Btri and accumulate ---
    for (size_t t = 0; t + 2 < indices.size(); t += 3)
    {
        uint32_t i0 = indices[t+0];
        uint32_t i1 = indices[t+1];
        uint32_t i2 = indices[t+2];
        if (i0 >= verts.size() || i1 >= verts.size() || i2 >= verts.size())
            continue;

        const PrimVertex& v0 = verts[i0];
        const PrimVertex& v1 = verts[i1];
        const PrimVertex& v2 = verts[i2];

        // position edges
        float x1 = v1.px - v0.px, y1 = v1.py - v0.py, z1 = v1.pz - v0.pz;
        float x2 = v2.px - v0.px, y2 = v2.py - v0.py, z2 = v2.pz - v0.pz;

        // uv edges
        float s1 = v1.u - v0.u,  t1 = v1.v - v0.v;
        float s2 = v2.u - v0.u,  t2 = v2.v - v0.v;

        float det = s1 * t2 - s2 * t1;
        if (std::fabs(det) < kEpsDet)
            continue;

        float r = 1.0f / det;

        float tx = (x1 * t2 - x2 * t1) * r;
        float ty = (y1 * t2 - y2 * t1) * r;
        float tz = (z1 * t2 - z2 * t1) * r;

        float bx = (x2 * s1 - x1 * s2) * r;
        float by = (y2 * s1 - y1 * s2) * r;
        float bz = (z2 * s1 - z1 * s2) * r;

        // 可选：面积加权（更稳，减少瘦长三角形污染）
        float ax, ay, az;
        Cross3(x1,y1,z1, x2,y2,z2, ax,ay,az);
        float w = std::sqrt(ax*ax + ay*ay + az*az); // 2*area
        if (w > 0.0f)
        {
            tx *= w; ty *= w; tz *= w;
            bx *= w; by *= w; bz *= w;
        }

        // accumulate to vertices
        acc[i0].tx += tx; acc[i0].ty += ty; acc[i0].tz += tz;
        acc[i1].tx += tx; acc[i1].ty += ty; acc[i1].tz += tz;
        acc[i2].tx += tx; acc[i2].ty += ty; acc[i2].tz += tz;

        acc[i0].bx += bx; acc[i0].by += by; acc[i0].bz += bz;
        acc[i1].bx += bx; acc[i1].by += by; acc[i1].bz += bz;
        acc[i2].bx += bx; acc[i2].by += by; acc[i2].bz += bz;
    }

    // --- per-vertex: orthonormalize T and compute handedness sign ---
    for (size_t vi = 0; vi < verts.size(); ++vi)
    {
        PrimVertex& v = verts[vi];

        // normalize normal (important!)
        float nx = v.nx, ny = v.ny, nz = v.nz;
        if (!Normalize3Safe(nx,ny,nz))
        {
            nx = 0; ny = 1; nz = 0; // fallback normal
        }
        v.nx = nx; v.ny = ny; v.nz = nz;

        // accumulated tangent
        float tx = acc[vi].tx, ty = acc[vi].ty, tz = acc[vi].tz;

        // if tangent too small -> fallback tangent, sign=+1
        if (LenSq3(tx,ty,tz) <= kEpsTLenSq)
        {
            OrthoFallbackTangent(nx,ny,nz, tx,ty,tz);
            v.tx = tx; v.ty = ty; v.tz = tz;
            v.tw = +1.0f;
            continue;
        }

        // Gram-Schmidt: T = normalize(T - N*dot(N,T))
        float ndt = Dot3(nx,ny,nz, tx,ty,tz);
        tx -= nx * ndt;
        ty -= ny * ndt;
        tz -= nz * ndt;

        if (!Normalize3Safe(tx,ty,tz))
        {
            OrthoFallbackTangent(nx,ny,nz, tx,ty,tz);
            v.tx = tx; v.ty = ty; v.tz = tz;
            v.tw = +1.0f;
            continue;
        }

        // compute sign using accumulated B (only for handedness)
        float bxAcc = acc[vi].bx, byAcc = acc[vi].by, bzAcc = acc[vi].bz;

        // C = cross(N, T)
        float cx, cy, cz;
        Cross3(nx,ny,nz, tx,ty,tz, cx,cy,cz);

        float sign = +1.0f;
        if (LenSq3(bxAcc,byAcc,bzAcc) > kEpsBAccLenSq)
        {
            sign = (Dot3(cx,cy,cz, bxAcc,byAcc,bzAcc) < 0.0f) ? -1.0f : +1.0f;
        }

        v.tx = tx; v.ty = ty; v.tz = tz;
        v.tw = sign;
    }
}


class PrimitiveGen
{
    // 
public:
    static MeshGL CreateCube(bool hardNormals = true);
    static MeshGL CreateSphere(uint32_t segments, uint32_t rings);
    static MeshGL CreateCylinder(uint32_t radialSegments, uint32_t heightSegments, bool capped);
    static MeshGL CreateCone(uint32_t radialSegments, bool capped);
    static MeshGL CreateTorus(uint32_t radialSegments, uint32_t tubeSegments, float minorMajorRatio);
    static MeshGL CreateAnnulus2D(uint32_t radialSegments);
    static MeshGL CreatePlane(uint32_t segX = 1, uint32_t segY = 1);
};