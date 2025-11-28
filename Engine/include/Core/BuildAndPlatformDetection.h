#pragma once

// Build-type (uses standard NDEBUG)
#if defined(NDEBUG)
  #define BUILD_RELEASE
#else
  #define BUILD_DEBUG  
#endif

// Platform
#if defined(_WIN32)                        // Win32 + Win64
  #define PLATFORM_WINDOWS
#elif defined(__APPLE__)||defined(__MACH__) // macOS / iOS / tvOS
  #include <TargetConditionals.h>
  /* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
  #if TARGET_IPHONE_SIMULATOR == 1
    #error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
  #define PLATFORM_LINUX 
#else
  #define PLATFORM_WINDOWS 0
  #define PLATFORM_LINUX   0
  #define PLATFORM_APPLE   0
#endif