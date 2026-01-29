#pragma once

#include "Core/Application.h"
#include "Windows/Window.h"

extern Application* CreateApplication(const WindowProperties& args);

// --- Actual entry point ---
int main(int argc, char** argv)
{

    Log::Init();
    CORE_INFO("Logging System Initialized");

    WindowProperties props;
    props.m_Title = "Game Engine Application";
    CORE_INFO("Creating Application with title: {}", props.m_Title);

    auto app = CreateApplication(props);

    app->Run();
    

    delete app;
    //std::cout<<"Creating Application with title: "<< props.m_Title <<std::endl;
    // auto app = CreateApplication(props);
    // app->Run();
    // delete app;

    return 0;
}