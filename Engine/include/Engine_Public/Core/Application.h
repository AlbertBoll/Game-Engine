#pragma once

#include"Export.hpp"
#include"Base.h"
#include"Utility.h"
#include"LayerStack.h"
#include<mutex>



int main(int argc, char** argv);
class Window;
class WindowProperties;
class Event;
class WindowCloseEvent;
class WindowResizeEvent;
class Layer;

struct ApplicationCommandLineArgs
{
    int Count = 0;
    char** Args = nullptr;

    const char* operator[](int index) const
    {
        CORE_ASSERT(index < Count);
        return Args[index];
    }
};



struct ApplicationSpecification
{
    std::string Name = "Application";
    std::string WorkingDirectory;
    ApplicationCommandLineArgs CommandLineArgs;
};

class ENGINE_API Application
{
public:
    Application(const WindowProperties& prop);
    virtual ~Application();
    virtual void OnEvents(Event& e);

    static Application& Get() { return *s_Instance; }

    //const ApplicationSpecification& GetSpecification() const { return m_Specification; }

    void SubmitToMainThread(const std::function<void()>& function);
    Window& GetWindow(){return *m_Window;}
    void Close();
    void PushLayer(Layer* layer);
    void PushOverlay(Layer* layer);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* layer);
private:
    void Run();
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);
    void ExecuteMainThreadQueue();

private:
    //ApplicationSpecification m_Specification;
    Scoped<Window> m_Window;
    bool m_Running = true;
    bool m_Minimized = false;
    LayerStack m_LayerStack;
    float m_LastFrameTime = 0.0f;

    std::vector<std::function<void()>> m_MainThreadQueue;
    std::mutex m_MainThreadQueueMutex;

private:
    inline static Application* s_Instance{};
    friend int ::main(int argc, char** argv);


};

// To be defined in CLIENT
Application* CreateApplication(const WindowProperties& args);


