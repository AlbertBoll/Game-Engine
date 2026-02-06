#pragma once

#include "BuildAndPlatformDetection.h"
#include <cstdint>

#ifdef BUILD_DEBUG
    #if defined(PLATFORM_WINDOWS)
		#define DEBUGBREAK() __debugbreak()
	#elif defined(PLATFORM_LINUX)
		#include <signal.h>
		#define DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define ENABLE_ASSERTS
#else
    #define DEBUGBREAK()
#endif


#define EXPAND_MACRO(x) x
#define STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define MOVEONLY(TypeName)            \
	TypeName(TypeName&&) noexcept; \
	TypeName& operator=(TypeName&&) noexcept; \
	TypeName(const TypeName&) = delete;      \
	TypeName& operator=(const TypeName&) = delete;

#define MOVE_DEFAULT_ONLY(TypeName)            \
	TypeName(TypeName&&) noexcept = default; \
	TypeName& operator=(TypeName&&) noexcept = default; \
	TypeName(const TypeName&) = delete;      \
	TypeName& operator=(const TypeName&) = delete;

static inline const void* OffsetPtr(std::size_t offsetBytes)
{
	return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(offsetBytes));
}

#include "Assert.h"
#include "Log.h"
//#include "Export.hpp"