#include"Core/Application.h"
#include"Core/Layer.h"
#include"Core/Time.h"
#include"Windows/Window.h"
#include"Input/Input.h"
#include"Events/Event.h"
#include"Events/WindowEvent.h"

Application::Application(const WindowProperties& prop)
{
    Time::Init();
    Time::SetTargetFPS(60.0);      // 0 => uncapped frame
    Time::SetMaxDeltaSeconds(0.25);
    m_Window = Window::Create(prop);
    Input::Init(*m_Window);
    CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    m_Window = Window::Create(prop);
}   

Application::~Application()
{
    Input::Shutdown();
}


void Application::OnEvents(Event& e)
{

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

        Input::BeginFrame();

        ExecuteMainThreadQueue();

        Time::EndFrame(m_Window->GetWindowProperties().m_IsVsync);

    }
}

