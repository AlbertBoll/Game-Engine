#pragma once

#include "Base.h"
#include "Utility.h"

#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

class Log
{
public:
    static void Init();

    static Ref<spdlog::logger>& GetCoreLogger(){return s_CoreLogger;}
    static Ref<spdlog::logger>& GetClientLogger(){return s_ClientLogger;}


private:
    inline static Ref<spdlog::logger> s_CoreLogger;
    inline static Ref<spdlog::logger> s_ClientLogger;

};

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

#ifdef BUILD_DEBUG  //activate logger in debug build
    //Core log macros
    #define Core_TRACE(...)         Log::GetCoreLogger()->trace(__VA_ARGS__)
    #define Core_INFO(...)          Log::GetCoreLogger()->info(__VA_ARGS__)
    #define Core_WARN(...)          Log::GetCoreLogger()->warn(__VA_ARGS__)
    #define Core_ERROR(...)         Log::GetCoreLogger()->error(__VA_ARGS__)
    #define Core_CRITICAL(...)      Log::GetCoreLogger()->critical(__VA_ARGS__)


    //Client log macros
    #define TRACE(...)              Log::GetClientLogger()->trace(__VA_ARGS__)
    #define INFO(...)               Log::GetClientLogger()->info(__VA_ARGS__)
    #define WARN(...)               Log::GetClientLogger()->warn(__VA_ARGS__)
    #define ERROR(...)              Log::GetClientLogger()->error(__VA_ARGS__)
    #define CRITICAL(...)           Log::GetClientLogger()->critical(__VA_ARGS__)
#else
    //Core log macros
    #define Core_TRACE(...)         (void)0
    #define Core_INFO(...)          (void)0
    #define Core_WARN(...)          (void)0
    #define Core_ERROR(...)         (void)0
    #define Core_CRITICAL(...)      (void)0

    //Client log macros
    #define TRACE(...)              (void)0
    #define INFO(...)               (void)0
    #define WARN(...)               (void)0
    #define ERROR(...)              (void)0
    #define CRITICAL(...)           (void)0

#endif
