#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(ENGINE_STATIC)
    #define ENGINE_API
  #else
    #if defined(ENGINE_SHARED)
      #define ENGINE_API __declspec(dllexport)
    #else
      #define ENGINE_API __declspec(dllimport)
    #endif
  #endif
#else
  #if __GNUC__ >= 4
    #define ENGINE_API __attribute__((visibility("default")))
  #else
    #define ENGINE_API
  #endif
#endif