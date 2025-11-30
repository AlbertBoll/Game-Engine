#pragma once

#include "Base.h"
#include "Utility.h"
#include <format>
#include "glm/gtx/string_cast.hpp"


namespace spdlog { class logger; }   // forward declaration

enum class LogTo
{
    CORE,
    CLIENT
};

class Log
{

public:
    static void Init();

    //static Ref<spdlog::logger>& GetCoreLogger(){return s_CoreLogger;}
    //static Ref<spdlog::logger>& GetClientLogger(){return s_ClientLogger;}

    template<LogTo _type, typename... Args>
    static void Trace(std::format_string<Args...> fmt, Args&&... args)
    {
        TraceMessage(_type, std::format(fmt, std::forward<Args>(args)...));
    }

    template<LogTo _type, typename... Args>
    static void Info(std::format_string<Args...> fmt, Args&&... args)
    {
        InfoMessage(_type, std::format(fmt, std::forward<Args>(args)...));
    }

    template<LogTo _type, typename... Args>
    static void Warn(std::format_string<Args...> fmt, Args&&... args)
    {
        WarnMessage(_type, std::format(fmt, std::forward<Args>(args)...));
    }

    template<LogTo _type, typename... Args>
    static void Error(std::format_string<Args...> fmt, Args&&... args)
    {
        ErrorMessage(_type, std::format(fmt, std::forward<Args>(args)...));
    }

    template<LogTo _type, typename... Args>
    static void Critical(std::format_string<Args...> fmt, Args&&... args)
    {
        CriticalMessage(_type, std::format(fmt, std::forward<Args>(args)...));
    }

private:
    static void TraceMessage(LogTo _type, std::string_view msg);
    static void InfoMessage(LogTo _type, std::string_view msg);
    static void WarnMessage(LogTo _type, std::string_view msg);
    static void ErrorMessage(LogTo _type, std::string_view msg);
    static void CriticalMessage(LogTo _type, std::string_view msg);




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
    #define Core_TRACE(...)         Log::Trace<LogTo::CORE>(__VA_ARGS__)//Log::GetCoreLogger()->trace(__VA_ARGS__)
    #define Core_INFO(...)          Log::Info<LogTo::CORE>(__VA_ARGS__)//Log::GetCoreLogger()->info(__VA_ARGS__)
    #define Core_WARN(...)          Log::Warn<LogTo::CORE>(__VA_ARGS__)//Log::GetCoreLogger()->warn(__VA_ARGS__)
    #define Core_ERROR(...)         Log::Error<LogTo::CORE>(__VA_ARGS__)//Log::GetCoreLogger()->error(__VA_ARGS__)
    #define Core_CRITICAL(...)      Log::Critical<LogTo::CORE>(__VA_ARGS__)// Log::GetCoreLogger()->critical(__VA_ARGS__)


    //Client log macros
    #define TRACE(...)              Log::Trace<LogTo::CLIENT>(__VA_ARGS__)//Log::GetClientLogger()->trace(__VA_ARGS__)
    #define INFO(...)               Log::Info<LogTo::CLIENT>(__VA_ARGS__)//Log::GetClientLogger()->info(__VA_ARGS__)
    #define WARN(...)               Log::Warn<LogTo::CLIENT>(__VA_ARGS__)//Log::GetClientLogger()->warn(__VA_ARGS__)
    #define ERROR(...)              Log::Error<LogTo::CLIENT>(__VA_ARGS__)//Log::GetClientLogger()->error(__VA_ARGS__)
    #define CRITICAL(...)           Log::Critical<LogTo::CLIENT>(__VA_ARGS__)//Log::GetClientLogger()->critical(__VA_ARGS__)

#else //strip log message away in release build
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
