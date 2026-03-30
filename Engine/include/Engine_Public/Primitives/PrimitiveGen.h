#pragma once

#include "PrimitiveMesh.h"

#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>


class PrimitiveGen
{
public:
    static MeshGL CreateCube(bool hardNormals = true);
    static MeshGL CreateSphere(uint32_t segments, uint32_t rings);
    static MeshGL CreateCylinder(uint32_t radialSegments, uint32_t heightSegments, bool capped);
    static MeshGL CreateCone(uint32_t radialSegments, bool capped);
    static MeshGL CreateTorus(uint32_t radialSegments, uint32_t tubeSegments, float minorMajorRatio);
    static MeshGL CreateAnnulus2D(uint32_t radialSegments);
    static MeshGL CreatePlane(uint32_t segX = 1, uint32_t segY = 1);
    static MeshGL CreateTriangle();
};