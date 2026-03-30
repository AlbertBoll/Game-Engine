#pragma once 

struct ShaderReserveDesc
{
    u32 m_ShaderSlots = 128;
    u32 m_cacheEntries = 128;
};

enum class ShaderCacheMode: u8
{
    USE_CACHE,
    NO_CACHE
};

enum class ClearMode: u8
{
    KEEP_CAPACITY,
    FREE_MEMORY
};

struct ShaderHandle
{
    u32 m_Id = 0;          // slot index + 1
    u32 m_Generation = 0;  // prevent dangling

    explicit operator bool() const { return m_Id != 0; }
    friend bool operator==(const ShaderHandle&, const ShaderHandle&) = default;
};

enum class ShaderKeyKind : u8 { Files, Sources };

struct ShaderKey
{
    static constexpr u8 kMaxItems = 8;

    ShaderKeyKind kind = ShaderKeyKind::Files;
    u8 count = 0;
    std::array<u64, kMaxItems> items{}; // items[0..count) 有效

    friend bool operator==(const ShaderKey& a, const ShaderKey& b) noexcept
    {
        if (a.kind != b.kind || a.count != b.count) return false;
        for (u8 i = 0; i < a.count; ++i)
            if (a.items[i] != b.items[i]) return false;
        return true;
    }
};

struct ShaderKeyHash
{
    std::size_t operator()(const ShaderKey& k) const noexcept;
};

struct UniformHandle
{
    int32_t m_Loc = -1;
    constexpr explicit operator bool() const noexcept { return m_Loc >= 0; }
};