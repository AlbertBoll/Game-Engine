#pragma once
#include "Core/Utility.h"

enum class WindowFlags: uint8_t
{
    INVISIBLE = 0x1,
    FULLSCREEN = 0X2,
    BORDERLESS = 0x4,
    RESIZABLE = 0x8
};

template<>
struct enable_bitmask<WindowFlags> : std::true_type {};
