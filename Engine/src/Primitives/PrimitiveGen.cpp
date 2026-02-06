#include "Primitives/PrimitiveGen.h"
#include<glm/gtc/constants.hpp>

static constexpr float PI = glm::pi<float>();
static constexpr float TWO_PI = glm::two_pi<float>();

MeshGL PrimitiveGen::CreatePlane(uint32_t segX, uint32_t segY)
{
    segX = std::max(1u, segX);
    segY = std::max(1u, segY);

    // Ground plane: XZ, y=0, normal +Y, size 1 ([-0.5,0.5])
    const uint32_t vx = segX + 1;
    const uint32_t vz = segY + 1;

    std::vector<PrimVertex> verts;
    std::vector<uint32_t> inds;
    verts.reserve(vx * vz);
    inds.reserve(segX * segY * 6);

    auto vidx = [&](uint32_t x, uint32_t z) { return z * vx + x; };

    for (uint32_t z = 0; z <= segY; ++z)
    {
        float fz = (float)z / (float)segY;     // 0..1
        float pz = -0.5f + fz * 1.0f;
        for (uint32_t x = 0; x <= segX; ++x)
        {
            float fx = (float)x / (float)segX;
            float px = -0.5f + fx * 1.0f;
            verts.emplace_back(px, 0.0f, pz, fx, 1-fz, 0.f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
            //ushVertex(verts, px, 0.0f, pz, fx, 1.0f - fz, 0.0f, 1.0f, 0.0f);
        }
    }

    // Winding for +Y: (a,c,b) and (a,d,c)
    for (uint32_t z = 0; z < segY; ++z)
    for (uint32_t x = 0; x < segX; ++x)
    {
        uint32_t a = vidx(x,   z);
        uint32_t b = vidx(x+1, z);
        uint32_t c = vidx(x+1, z+1);
        uint32_t d = vidx(x,   z+1);

        inds.push_back(a); inds.push_back(c); inds.push_back(b);
        inds.push_back(a); inds.push_back(d); inds.push_back(c);
    }

    BuildTangentSign(verts, inds);
    return MeshGL(std::move(verts), std::move(inds));
}

MeshGL PrimitiveGen::CreateCube(bool hardNormals)
{
    std::vector<PrimVertex> v;
    std::vector<uint32_t>  i;
    v.reserve(24);
    i.reserve(36);

    auto addFace = [&](float nx,float ny,float nz,
                       float ax,float ay,float az,  // face "right" direction in position space
                       float bx,float by,float bz,  // face "up" direction
                       float cx,float cy,float cz)  // face center
    {
        // quad corners: center + (-a-b), (+a-b), (+a+b), (-a+b)
        // with a,b half-extents = 0.5*dir
        const float hx=0.5f*ax, hy=0.5f*ay, hz=0.5f*az;
        const float ux=0.5f*bx, uy=0.5f*by, uz=0.5f*bz;

        PrimVertex p0{}; // uv (0,0)
        p0.px = cx - hx - ux; p0.py = cy - hy - uy; p0.pz = cz - hz - uz; p0.u = 0; p0.v = 0;
        PrimVertex p1{}; // uv (1,0)
        p1.px = cx + hx - ux; p1.py = cy + hy - uy; p1.pz = cz + hz - uz; p1.u = 1; p1.v = 0;
        PrimVertex p2{}; // uv (1,1)
        p2.px = cx + hx + ux; p2.py = cy + hy + uy; p2.pz = cz + hz + uz; p2.u = 1; p2.v = 1;
        PrimVertex p3{}; // uv (0,1)
        p3.px = cx - hx + ux; p3.py = cy - hy + uy; p3.pz = cz - hz + uz; p3.u = 0; p3.v = 1;

        auto setN = [&](PrimVertex& pv)
        {
            if (hardNormals) { pv.nx = nx; pv.ny = ny; pv.nz = nz; }
            else
            {
                float nnx = pv.px, nny = pv.py, nnz = pv.pz;
                Normalize3Safe(nnx,nny,nnz);
                pv.nx = nnx; pv.ny = nny; pv.nz = nnz;
            }
            pv.tx = pv.ty = pv.tz = 0.0f; pv.tw = 1.0f;
        };

        setN(p0); setN(p1); setN(p2); setN(p3);

        const uint32_t base = (uint32_t)v.size();
        v.push_back(p0); v.push_back(p1); v.push_back(p2); v.push_back(p3);

        // CCW for outward normal
        i.push_back(base+0); i.push_back(base+1); i.push_back(base+2);
        i.push_back(base+2); i.push_back(base+3); i.push_back(base+0);
    };

    // Faces: each defined by (normal) + (right axis) + (up axis) + center at ±0.5 along normal
    // +Z
    addFace(0,0, 1,  1,0,0,  0,1,0,  0,0, 0.5f);
    // -Z
    addFace(0,0,-1, -1,0,0,  0,1,0,  0,0,-0.5f);
    // +X
    addFace(1,0,0,  0,0,-1,  0,1,0,  0.5f,0,0);
    // -X
    addFace(-1,0,0, 0,0, 1,  0,1,0, -0.5f,0,0);
    // +Y
    addFace(0,1,0,  1,0,0,  0,0,-1, 0, 0.5f,0);
    // -Y
    addFace(0,-1,0, 1,0,0,  0,0, 1, 0,-0.5f,0);

    // Build tangents from UVs (works for both hard/smooth normals)
    BuildTangentSign(v, i);
    return MeshGL(std::move(v), std::move(i));
}

MeshGL PrimitiveGen::CreateSphere(uint32_t segments, uint32_t rings)
{
    segments = std::max(3u, segments);
    rings    = std::max(2u, rings);

    const float R = 0.5f;

    // duplicate seam: (segments+1) x (rings+1)
    uint32_t vx = segments + 1;
    uint32_t vy = rings + 1;

    std::vector<PrimVertex> verts;
    std::vector<uint32_t> inds;
    verts.reserve(vx * vy);
    inds.reserve(segments * rings * 6);

    auto vidx = [&](uint32_t x, uint32_t y){ return y * vx + x; };

    for (uint32_t y=0; y<=rings; ++y)
    {
        float fy = (float)y / (float)rings; // 0..1
        float theta = fy * PI;             // 0..pi
        float st = std::sin(theta);
        float ct = std::cos(theta);

        for (uint32_t x=0; x<=segments; ++x)
        {
            float fx = (float)x / (float)segments; // 0..1
            float phi = fx * TWO_PI;                 // 0..2pi
            float sp = std::sin(phi);
            float cp = std::cos(phi);

            float nx = st * cp;
            float ny = ct;
            float nz = st * sp;
            // pos = normal * R
            verts.emplace_back(nx*R, ny*R, nz*R, fx, 1.0f-fy, nx, ny, nz, 0.f, 0.f, 0.f, 1.f);
            //PushVertex(verts, nx*R, ny*R, nz*R, fx, 1.0f-fy, nx, ny, nz);
        }
    }

    for (uint32_t y=0; y<rings; ++y)
    for (uint32_t x=0; x<segments; ++x)
    {
        uint32_t a = vidx(x,   y);
        uint32_t b = vidx(x+1, y);
        uint32_t c = vidx(x+1, y+1);
        uint32_t d = vidx(x,   y+1);

        // CCW outward
        inds.push_back(a); inds.push_back(b); inds.push_back(c);
        inds.push_back(c); inds.push_back(d); inds.push_back(a);
    }

    BuildTangentSign(verts, inds);
    return MeshGL(std::move(verts), std::move(inds));
}

MeshGL PrimitiveGen::CreateCylinder(uint32_t radialSegments, uint32_t heightSegments, bool capped)
{
    radialSegments = std::max(3u, radialSegments);
    heightSegments = std::max(1u, heightSegments);

    const float R = 0.5f;
    const float y0 = -0.5f;
    const float y1 = +0.5f;

    std::vector<PrimVertex> verts;
    std::vector<uint32_t> inds;

    // ---------- side surface ----------
    const uint32_t sx = radialSegments + 1; // duplicate seam
    const uint32_t sy = heightSegments + 1;

    const uint32_t sideBase = 0;
    verts.reserve(sx*sy + (capped ? (2 * (radialSegments + 2)) : 0));
    inds.reserve(radialSegments * heightSegments * 6 + (capped ? (radialSegments * 3 * 2) : 0));

    auto sidx = [&](uint32_t x, uint32_t y){ return sideBase + y*sx + x; };

    for (uint32_t y=0; y<=heightSegments; ++y)
    {
        float fy = (float)y / (float)heightSegments;
        float py = y0 + fy * (y1 - y0);

        for (uint32_t x=0; x<=radialSegments; ++x)
        {
            float fx = (float)x / (float)radialSegments;
            float ang = fx * TWO_PI;
            float ca = std::cos(ang);
            float sa = std::sin(ang);

            float px = R * ca;
            float pz = R * sa;

            verts.emplace_back(px, py, pz, fx, 1.0f - fy, ca, 0.0f, sa, 0.0f, 0.0f, 0.0f, 1.0f);
        }
    }

    for (uint32_t y=0; y<heightSegments; ++y)
    for (uint32_t x=0; x<radialSegments; ++x)
    {
        uint32_t a = sidx(x,   y);
        uint32_t b = sidx(x+1, y);
        uint32_t c = sidx(x+1, y+1);
        uint32_t d = sidx(x,   y+1);

        // CCW outward
        inds.push_back(a); inds.push_back(b); inds.push_back(c);
        inds.push_back(c); inds.push_back(d); inds.push_back(a);
    }

    // ---------- caps (separate vertices => hard edge) ----------
    if (capped)
    {
        auto addCap = [&](bool top)
        {
            float ny = top ? +1.0f : -1.0f;
            float py = top ? y1 : y0;

            uint32_t base = (uint32_t)verts.size();

            // center
            verts.emplace_back(0.0f, py, 0.0f, 0.5f, 0.5f, 0.0f, ny, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
            //PushVertex(verts, 0.0f, py, 0.0f, 0.5f, 0.5f, 0.0f, ny, 0.0f);

            // ring (duplicate seam)
            for (uint32_t x=0; x<=radialSegments; ++x)
            {
                float fx = (float)x / (float)radialSegments;
                float ang = fx * TWO_PI;
                float ca = std::cos(ang);
                float sa = std::sin(ang);

                float px = R * ca;
                float pz = R * sa;

                // disk UV from xz
                float u = 0.5f + 0.5f*ca;
                float v = 0.5f + 0.5f*sa;
                
                verts.emplace_back(px, py, pz, u, v, 0.0f, ny, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
            }

            uint32_t center = base;

            for (uint32_t x=0; x<radialSegments; ++x)
            {
                uint32_t r0 = base + 1 + x;
                uint32_t r1 = base + 1 + x + 1;

                if (top)
                {
                    // make +Y : (center, r1, r0)
                    inds.push_back(center);
                    inds.push_back(r1);
                    inds.push_back(r0);
                }
                else
                {
                    // -Y : (center, r0, r1)
                    inds.push_back(center);
                    inds.push_back(r0);
                    inds.push_back(r1);
                }
            }
        };

        addCap(true);
        addCap(false);
    }

    BuildTangentSign(verts, inds);
    return MeshGL(std::move(verts), std::move(inds));
}

MeshGL PrimitiveGen::CreateCone(uint32_t radialSegments, bool capped)
{
    radialSegments = std::max(3u, radialSegments);

    const float R = 0.5f;
    const float yBase = -0.5f;
    const float yApex = +0.5f;
    const float kSlope = R / (yApex - yBase); // R/H = 0.5/1 = 0.5

    std::vector<PrimVertex> verts;
    std::vector<uint32_t> inds;

    // base ring for side (duplicate seam)
    const uint32_t ringBase = 0;
    const uint32_t ringCount = radialSegments + 1;
    verts.reserve(ringCount + radialSegments + (capped ? (radialSegments + 2) : 0));
    inds.reserve(radialSegments * 3 + (capped ? radialSegments * 3 : 0));

    for (uint32_t x=0; x<=radialSegments; ++x)
    {
        float fx = (float)x / (float)radialSegments;
        float ang = fx * TWO_PI;
        float ca = std::cos(ang);
        float sa = std::sin(ang);

        float px = R * ca;
        float pz = R * sa;

        // smooth side normal from implicit gradient: (ca, kSlope, sa)
        float nx = ca, ny = kSlope, nz = sa;
        Normalize3Safe(nx,ny,nz);

        // UV: u around, v=1 at base
        verts.emplace_back(px, yBase, pz, fx, 1.0f, nx, ny, nz, 0.0f, 0.0f, 0.0f, 1.0f);
        //PushVertex(verts, px, yBase, pz, fx, 1.0f, nx, ny, nz);
    }

    // apex vertices: one per segment (avoid singular apex sharing)
    const uint32_t apexBase = (uint32_t)verts.size();
    for (uint32_t x=0; x<radialSegments; ++x)
    {
        float fx0 = (float)x / (float)radialSegments;
        float fx1 = (float)(x+1) / (float)radialSegments;
        float fxm = 0.5f*(fx0 + fx1);
        float angm = fxm * TWO_PI;

        float ca = std::cos(angm);
        float sa = std::sin(angm);

        float nx = ca, ny = kSlope, nz = sa;
        Normalize3Safe(nx,ny,nz);

        // v=0 at apex
        verts.emplace_back(0.0f, yApex, 0.0f, fxm, 0.0f, nx, ny, nz, 0.0f, 0.0f, 0.0f, 1.0f);
        //PushVertex(verts, 0.0f, yApex, 0.0f, fxm, 0.0f, nx, ny, nz);
    }

    // side triangles: (base_i, apex_i, base_{i+1}) => outward
    for (uint32_t x=0; x<radialSegments; ++x)
    {
        uint32_t a = ringBase + x;
        uint32_t b = ringBase + x + 1;
        uint32_t ap = apexBase + x;

        inds.push_back(a);
        inds.push_back(ap);
        inds.push_back(b);
    }

    // base cap
    if (capped)
    {
        uint32_t base = (uint32_t)verts.size();

        // center
        verts.emplace_back(0.0f, yBase, 0.0f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        //PushVertex(verts, 0.0f, yBase, 0.0f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f);

        // ring (duplicate seam)
        for (uint32_t x=0; x<=radialSegments; ++x)
        {
            float fx = (float)x / (float)radialSegments;
            float ang = fx * TWO_PI;
            float ca = std::cos(ang);
            float sa = std::sin(ang);

            float px = R * ca;
            float pz = R * sa;

            float u = 0.5f + 0.5f*ca;
            float v = 0.5f + 0.5f*sa;
            verts.emplace_back(px, yBase, pz, u, v, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        }

        uint32_t center = base;

        for (uint32_t x=0; x<radialSegments; ++x)
        {
            uint32_t r0 = base + 1 + x;
            uint32_t r1 = base + 1 + x + 1;

            // -Y outward
            inds.push_back(center);
            inds.push_back(r0);
            inds.push_back(r1);
        }
    }

    BuildTangentSign(verts, inds);
    return MeshGL(std::move(verts), std::move(inds));
}


MeshGL PrimitiveGen::CreateTorus(uint32_t radialSegments, uint32_t tubeSegments, float minorMajorRatio)
{
    radialSegments = std::max(3u, radialSegments);
    tubeSegments   = std::max(3u, tubeSegments);

    // unit torus: major radius ~0.5, minor = major * ratio
    float major = 0.5f;
    float ratio = std::clamp(minorMajorRatio, 0.05f, 0.49f);
    float minor = major * ratio;

    // duplicate seam in both directions
    uint32_t ru = radialSegments + 1;
    uint32_t rv = tubeSegments + 1;

    std::vector<PrimVertex> verts;
    std::vector<uint32_t> inds;
    verts.reserve(ru * rv);
    inds.reserve(radialSegments * tubeSegments * 6);

    auto vidx = [&](uint32_t u, uint32_t v){ return v * ru + u; };

    for (uint32_t v=0; v<=tubeSegments; ++v)
    {
        float fv = (float)v / (float)tubeSegments;
        float angV = fv * TWO_PI;
        float cv = std::cos(angV);
        float sv = std::sin(angV);

        for (uint32_t u=0; u<=radialSegments; ++u)
        {
            float fu = (float)u / (float)radialSegments;
            float angU = fu * TWO_PI;
            float cu = std::cos(angU);
            float su = std::sin(angU);

            // position
            float ring = major + minor * cv;
            float px = ring * cu;
            float py = minor * sv;
            float pz = ring * su;

            // normal (from tube center to surface)
            float nx = cu * cv;
            float ny = sv;
            float nz = su * cv;
            Normalize3Safe(nx,ny,nz);
            verts.emplace_back(px, py, pz, fu, 1.0f - fv, nx, ny, nz, 0.0f, 0.f, 0.f, 1.f);
            //PushVertex(verts, px, py, pz, fu, 1.0f - fv, nx, ny, nz);
        }
    }

    for (uint32_t v=0; v<tubeSegments; ++v)
    for (uint32_t u=0; u<radialSegments; ++u)
    {
        uint32_t a = vidx(u,   v);
        uint32_t b = vidx(u+1, v);
        uint32_t c = vidx(u+1, v+1);
        uint32_t d = vidx(u,   v+1);

        // CCW outward
        inds.push_back(a); inds.push_back(b); inds.push_back(c);
        inds.push_back(c); inds.push_back(d); inds.push_back(a);
    }

    BuildTangentSign(verts, inds);
    return MeshGL(std::move(verts), std::move(inds));
}


MeshGL PrimitiveGen::CreateAnnulus2D(uint32_t radialSegments)
{
    radialSegments = std::max(3u, radialSegments);

    // unit annulus on XY plane, z=0, normal +Z
    const float rOuter = 0.5f;
    const float rInner = 0.25f; // 你也可以改成参数/常量配置

    const uint32_t ring = radialSegments + 1; // duplicate seam

    std::vector<PrimVertex> verts;
    std::vector<uint32_t> inds;
    verts.reserve(ring * 2);
    inds.reserve(radialSegments * 6);

    // outer ring
    for (uint32_t s=0; s<=radialSegments; ++s)
    {
        float fs = (float)s / (float)radialSegments;
        float a = fs * TWO_PI;
        float ca = std::cos(a);
        float sa = std::sin(a);

        float x = rOuter * ca;
        float y = rOuter * sa;

        // simple planar uv mapping
        float u = 0.5f + 0.5f * (x / rOuter);
        float v = 0.5f + 0.5f * (y / rOuter);

        verts.emplace_back(x, y, 0.0f, u, v, 0.0f, 0.0f, 1.0f, 0.f, 0.f, 0.f, 1.f);
        //PushVertex(verts, x, y, 0.0f, u, v, 0.0f, 0.0f, 1.0f);
    }

    // inner ring
    const uint32_t innerBase = (uint32_t)verts.size();
    for (uint32_t s=0; s<=radialSegments; ++s)
    {
        float fs = (float)s / (float)radialSegments;
        float a = fs * TWO_PI;
        float ca = std::cos(a);
        float sa = std::sin(a);

        float x = rInner * ca;
        float y = rInner * sa;

        float u = 0.5f + 0.5f * (x / rOuter);
        float v = 0.5f + 0.5f * (y / rOuter);
        
        verts.emplace_back(x, y, 0.0f, u, v, 0.0f, 0.0f, 1.0f, 0.f, 0.f, 0.f, 1.f);
        //PushVertex(verts, x, y, 0.0f, u, v, 0.0f, 0.0f, 1.0f);
    }

    // quads between outer/inner
    for (uint32_t s=0; s<radialSegments; ++s)
    {
        uint32_t o0 = s;
        uint32_t o1 = s + 1;
        uint32_t i0 = innerBase + s;
        uint32_t i1 = innerBase + s + 1;

        // +Z facing:
        // tri1: outer0, outer1, inner0
        // tri2: outer1, inner1, inner0
        inds.push_back(o0); inds.push_back(o1); inds.push_back(i0);
        inds.push_back(o1); inds.push_back(i1); inds.push_back(i0);
    }

    BuildTangentSign(verts, inds);
    return MeshGL(std::move(verts), std::move(inds));
}