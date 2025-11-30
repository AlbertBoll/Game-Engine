#include "Core/Log.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void Log::Init()
{
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Engine.log", true));

    logSinks[0]->set_pattern("%^[%T] %n: %v%$");
    logSinks[1]->set_pattern("[%T] [%l] %n: %v");

    s_CoreLogger = std::make_shared<spdlog::logger>("Engine", begin(logSinks), end(logSinks));
    spdlog::register_logger(s_CoreLogger);
    s_CoreLogger->set_level(spdlog::level::trace);
    s_CoreLogger->flush_on(spdlog::level::trace);

    s_ClientLogger = std::make_shared<spdlog::logger>("App", begin(logSinks), end(logSinks));
    spdlog::register_logger(s_ClientLogger);
    s_ClientLogger->set_level(spdlog::level::trace);
    s_ClientLogger->flush_on(spdlog::level::trace);
}

void Log::TraceMessage(LogTo _type, std::string_view msg)
{
    if(_type == LogTo::CORE)
    {
        s_CoreLogger->trace("{}", msg);
    }
    else
    {
        s_ClientLogger->trace("{}", msg);
    }
}

void Log::InfoMessage(LogTo _type, std::string_view msg)
{
    if(_type == LogTo::CORE)
    {
        s_CoreLogger->info("{}", msg);
    }
    else
    {
        s_ClientLogger->info("{}", msg);
    }
}

void Log::WarnMessage(LogTo _type, std::string_view msg)
{
    if(_type == LogTo::CORE)
    {
        s_CoreLogger->warn("{}", msg);
    }
    else
    {
        s_ClientLogger->warn("{}", msg);
    }
}

void Log::ErrorMessage(LogTo _type, std::string_view msg)
{
    if(_type == LogTo::CORE)
    {
        s_CoreLogger->error("{}", msg);
    }
    else
    {
        s_ClientLogger->error("{}", msg);
    }
}

void Log::CriticalMessage(LogTo _type, std::string_view msg)
{
    if(_type == LogTo::CORE)
    {
        s_CoreLogger->critical("{}", msg);
    }
    else
    {
        s_ClientLogger->critical("{}", msg);
    }
}