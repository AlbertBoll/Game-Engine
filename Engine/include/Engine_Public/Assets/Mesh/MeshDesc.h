#pragma once

#include "Core/Base.h"

struct MeshHandle
{
    u32 m_Id = 0;          // slot index + 1
    u32 m_Generation = 0;  // avoid dangling deletion

    explicit operator bool() const { return m_Id != 0; }
    friend bool operator==(const MeshHandle&, const MeshHandle&) = default;
};

struct MeshReserveDesc
{
    u32 m_MeshSlots      = 256;
    u32 m_PrimCacheCount = 128;
    u32 m_FileCacheCount = 128;
};

struct MeshFileLoadDesc
{
    std::string_view m_Path;
    bool b_UseCache = true;
    std::string_view m_DebugName{};
};