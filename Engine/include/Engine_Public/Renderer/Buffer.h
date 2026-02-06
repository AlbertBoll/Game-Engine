#pragma once
#include <cstdint>

using Handle = std::uint32_t;

enum class BufferUsage : uint8_t
{
    StaticDraw,
    DynamicDraw,
    StreamDraw
};

enum class BufferUpdateMode : uint8_t
{
    SubData,
    OrphanThenSubData
};

unsigned int ToGLUsage(BufferUsage u);