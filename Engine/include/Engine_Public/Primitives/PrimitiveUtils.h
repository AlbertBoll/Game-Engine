

// ------------------ Hash helpers ------------------
inline constexpr uint64_t HashCombine64(uint64_t a, uint64_t b)
{
    a ^= b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2);
    return a;
}


template<typename ... Hs>
inline constexpr uint64_t HashFields(uint64_t tag, Hs... hs)
{
    uint64_t h = tag;
    ((h = HashCombine64(h, (uint64_t)hs)), ...);
    return h;
}

inline constexpr uint64_t HashU32(uint32_t v) { return (uint64_t)v * 0x9ddfea08eb382d69ull; }
inline constexpr uint64_t HashU16(uint16_t v) { return (uint64_t)v * 0x9ddfea08eb382d69ull; }
inline constexpr uint64_t HashBool(bool v) { return v ? 0xF00DBEEF12345678ull : 0x12345678F00DBEEFull; }

inline constexpr uint64_t HashQuantizedFloat(float x, float scale = 1000.0f)
{
    const int64_t q = (int64_t)llround((double)x * (double)scale);
    return (uint64_t)q * 0xD6E8FEB86659FD93ull;
}

inline constexpr uint64_t HashAny(uint16_t v) {return HashU16(v);}
inline constexpr uint64_t HashAny(uint32_t v) {return HashU32(v);}
inline constexpr uint64_t HashAny(bool v) {return HashBool(v);}
inline constexpr uint64_t HashAny(float v) {return HashQuantizedFloat(v, 10000.f );}