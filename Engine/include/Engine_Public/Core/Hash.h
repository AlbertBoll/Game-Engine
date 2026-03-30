#pragma once
#include <cstddef>
#include <functional>
#include <type_traits>

inline std::size_t CombineRaw(std::size_t a, std::size_t b) noexcept
{
    a ^= b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2);
    return a;
}

template<class T, class = void>
struct HasStdHash : std::false_type {};

template<class T>
struct HasStdHash<T, std::void_t<decltype(std::hash<T>{}(std::declval<const T&>()))>>
    : std::true_type {};

template<class T>
struct CustomHash; // specialization

template<class T, class = void>
struct HasCustomHash : std::false_type {};

template<class T>
struct HasCustomHash<T, std::void_t<decltype(CustomHash<T>{}(std::declval<const T&>()))>>
    : std::true_type {};

template<class T>
inline std::size_t Value(const T& v) noexcept
{
    using U = std::decay_t<T>;

    if constexpr (std::is_enum_v<U>)
    {
        using E = std::underlying_type_t<U>;
        return std::hash<E>{}(static_cast<E>(v));
    }
    else if constexpr (HasStdHash<U>::value)
    {
        return std::hash<U>{}(v);
    }
    else if constexpr (HasCustomHash<U>::value)
    {
        return CustomHash<U>{}(v);
    }
    else
    {
        static_assert(sizeof(U) == 0, "No hash available for this type");
        return 0;
    }
}

template<class T>
inline void AppendOne(std::size_t& seed, const T& v) noexcept
{
    seed = CombineRaw(seed, Value(v));
}

template<typename ... Ts>
inline std::size_t Combine(const Ts&... xs) noexcept
{
    std::size_t seed = 0;
    (AppendOne(seed, xs), ...);
    return seed;
}