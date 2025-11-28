function(fx_find_or_fetch_entt)
  find_package(EnTT QUIET CONFIG) # EnTT::EnTT
  if(NOT EnTT_FOUND)
    if(NOT ENABLE_FETCH_FALLBACK)
      message(FATAL_ERROR "EnTT not found and fallback disabled")
    endif()
    message(STATUS "[Fallback] Fetching EnTT v3.13.0")
    FetchContent_Declare(entt
      URL https://github.com/skypjack/entt/archive/refs/tags/v3.13.0.zip
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      # URL_HASH SHA256=<REAL_SHA256>
    )
    FetchContent_MakeAvailable(entt)
    if(NOT TARGET EnTT::EnTT)
      add_library(EnTT::EnTT INTERFACE IMPORTED)
      target_include_directories(EnTT::EnTT INTERFACE ${entt_SOURCE_DIR}/src)
    endif()
  endif()
endfunction()