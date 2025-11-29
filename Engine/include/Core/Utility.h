#pragma once
#include<memory>
#include<utility>
#include<variant>
#include <type_traits>


#define NONCOPYABLE(className) className(const className&) = delete;\
								className& operator=(const className&) = delete
	
#define NONMOVABLE(className) className(className&&) = delete;\
								className& operator=(className&&) = delete
	
#define NONCOPYMOVABLE(className)  NONCOPYABLE(className);\
									NONMOVABLE(className)

template<typename T, typename deleter = std::default_delete<T>>
using Scoped = std::unique_ptr<T, deleter>;

//using default deleter
template<typename T, typename ... Args>
constexpr Scoped<T> CreateScopedPtr(Args&& ... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

//using custom deleter
template<typename T, typename deleter, typename ... Args>
constexpr Scoped<T, deleter> CreateScopedPtr(Args&& ... args)
{
    return std::unique_ptr<T, deleter>(new T{ std::forward<Args>(args)... });
}


template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename... Args>
constexpr Ref<T> CreateRefPtr(Args&& ... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename deleter, typename... Args>
constexpr Ref<T> CreateRefPtr(Args&& ... args)
{
    return std::shared_ptr<T>(new T{ std::forward<Args>(args)... }, deleter{});
}                                    

//*****************************define universal hash function************************"""
template<typename T>
static inline void hash_combine(size_t& seed, const T& val)
{
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


template<typename T>
static inline void hash_val(size_t& seed, const T& val)
{
    hash_combine(seed, val);
}

template<typename T, typename... Types>
static inline void hash_val(size_t& seed, const T& val, const Types&... args)
{
    hash_combine(seed, val);
    hash_val(seed, args...);
}

template<typename... Types>
static inline size_t hash_val(const Types&... args)
{
    size_t seed = 0;
    hash_val(seed, args...);
    return seed;
}

inline std::variant<std::false_type, std::true_type> bool_variant(bool condition)
{
    if (condition) return std::true_type{};
    else return std::false_type{};
}


template<class E> struct enable_bitmask : std::false_type {};
template<class E> constexpr bool enable_bitmask_v = enable_bitmask<E>::value;

template<class E> 
requires std::is_enum_v<E> && enable_bitmask_v<E>
constexpr E operator|(E a, E b) {
  using U = std::underlying_type_t<E>;
  return static_cast<E>(static_cast<U>(a) | static_cast<U>(b));
}

template<class E>
requires std::is_enum_v<E> && enable_bitmask_v<E>
constexpr E operator&(E a, E b) {
  using U = std::underlying_type_t<E>;
  return static_cast<E>(static_cast<U>(a) & static_cast<U>(b));
}