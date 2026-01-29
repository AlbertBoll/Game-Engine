#include"Core/Application.h"
#include"Core/Layer.h"
#include"Core/Time.h"
#include"Windows/Window.h"
#include"Input/Input.h"
#include"Events/Event.h"
#include"Events/WindowEvent.h"

static void EnvironmentInfo()
{
    #ifdef PLATFORM_WINDOWS
        #if defined(BUILD_DEBUG)
            CORE_INFO("Running on Windows: Configuration DEBUG");
        else
            CORE_INFO("Running on Windows: Configuration RELEASE");
        #endif
    #elif PLATFORM_LINUX
        #if defined(BUILD_DEBUG)
            CORE_INFO("Running on Linux: Configuration DEBUG");
        #else
            CORE_INFO("Running on Linux: Configuration RELEASE");
        #endif
    #else
        CORE_INFO("Running on Unknown Platform");
    #endif
}

Application::Application(const WindowProperties& prop)
{

    EnvironmentInfo();
    CORE_INFO("Environment Info Logged in Application");


    m_Window = Window::Create(prop);
    CORE_INFO("Window Created in Application");

    Input::Init(*m_Window);
    CORE_INFO("Input System Initialized in Application");

    CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    m_Window->SetEventCallback(BIND_EVENT_FN(OnEvents));

}   

Application::~Application()
{
    Input::Shutdown();
    CORE_INFO("Application Shutdown Complete");
}


void Application::OnEvents(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
    dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
    {
        if (e.Handled)
            break;
        (*it)->OnEvent(e);      // <-- LAYER OnEvent INVOKED HERE
    }
}

void Application::SubmitToMainThread(const std::function<void()>& function)
{
    std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

	m_MainThreadQueue.emplace_back(function);
}

void Application::ExecuteMainThreadQueue()
{
    std::vector<std::function<void()>> queue;
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
        queue.swap(m_MainThreadQueue);
    }

    for(auto& func: queue) func();
}

void Application::PopLayer(Layer* layer)
{
    SubmitToMainThread([this, layer]()
    {
        m_LayerStack.PopLayer(layer);
    });
}

void Application::PopOverlay(Layer* layer)
{
    SubmitToMainThread([this, layer]()
    {
        m_LayerStack.PopOverlay(layer);
    });
}


void Application::PushLayer(Layer* layer)
{
    m_LayerStack.PushLayer(layer);
    layer->OnAttach();
}

void Application::PushOverlay(Layer* layer)
{
    m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

void Application::Close()
{
    m_Running = false;
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
    m_Running = false;
    return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{

    if (e.GetWidth() == 0 || e.GetHeight() == 0)
    {
        m_Minimized = true;
        return false;
    }

    m_Minimized = false;
    return false;
}

void Application::Run()
{

    Time::Init();
    CORE_INFO("Time System Initialized in Application");
    Time::SetTargetFPS(60.0);      // 0 => uncapped frame
    Time::SetMaxDeltaSeconds(0.25);

    while (m_Running)
    {
        if(m_Minimized)
        {
            Time::SetBaselineNow();

            // Optional: still run main-thread queue (safe if tasks don’t assume rendering)
            ExecuteMainThreadQueue();

            // Throttle CPU while minimized
            Time::SleepSeconds(1.0 / 60.0);
            continue;
        }
        const double dt = Time::StartFrame();
        //m_Window->PollEvents();
        m_Window->OnUpdate();
        Input::BeginFrame();

        ExecuteMainThreadQueue();

     
        for (Layer* layer : m_LayerStack)
            layer->OnUpdate(Timestep(dt));

        m_Window->SwapBuffers(); // present after rendering

        Time::EndFrame(m_Window->GetWindowProperties().m_IsVsync);

    }
}

