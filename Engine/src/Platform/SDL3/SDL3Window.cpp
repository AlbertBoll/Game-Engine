#include"Platform/SDL3/SDL3Window.h"
#include"Events/WindowEvent.h"
#include"Core/Log.h"
#include <glad/gl.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>



static bool s_SDLInited = false;
//#include"Input/Codes.h"

void* SDL3Window::GetNativeWindow() const
{
    return (void*)m_Window;
}

void SDL3Window::Initialize(const WindowProperties &winProp)
{
    m_WindowProperties = winProp;

    if (!s_SDLInited)
    {
        SDL_SetMainReady(); // required if SDL_MAIN_HANDLED

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            CORE_ERROR("Failed to initialize SDL3");
            return;
        }
        s_SDLInited = true;
    }
    //SDL_SetHint(SDL_HINT_VIDEO_FORCE_EGL, "0");
 
    uint32_t flag = GetWindowFlag(m_WindowProperties);
    // Set OpenGL attributes
    // Use the core OpenGL profile
    //SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // Specify version 4.6
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    // Request a color buffer with 8-bits per RGBA channel
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    // Enable double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // Set the depth buffer size to 24 bits
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Force OpenGL to use hardware acceleration
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


    // Create the SDL window
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);
    if (!mode) 
    {
        CORE_ERROR("SDL_GetCurrentDisplayMode failed: {}", SDL_GetError());
    } 
    else 
    {
        CORE_INFO("Current mode: {}x{} @ {}Hz", mode->w, mode->h, mode->refresh_rate);
    }

    
    SetWindow(mode->w, mode->h, flag);
   
    CORE_ASSERT(m_Window, "SDL Window couldn't be created!");    

    m_Context = SDL_GL_CreateContext(m_Window);
  
	CORE_ASSERT(m_Context, "SDL_GL context couldn't be created!");

    SDL_GL_MakeCurrent(m_Window, (SDL_GLContext)m_Context);

    // ---- Load OpenGL functions via GLAD ----
    // Most GLAD builds require SDL_GL_GetProcAddress
    if (!gladLoaderLoadGL())
    {
        CORE_ERROR("gladLoaderLoadGL failed");
        return;
    }
    CORE_INFO("GL Version: {}", (const char*)glGetString(GL_VERSION));
    // ---- Mouse relative mode hint (optional) ----
    // This hint should usually be set BEFORE enabling relative mode, not strictly required here.
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1", SDL_HINT_OVERRIDE);

   
    //Set Window Minimum Size
	SDL_SetWindowMinimumSize(m_Window, m_WindowProperties.m_MinWidth, m_WindowProperties.m_MinHeight);

    if (m_WindowProperties.m_IsVsync)
    {
        //Set VSYNC;
        SDL_GL_SetSwapInterval(1);
    }
    else
    {
        SDL_GL_SetSwapInterval(0);
    }

}

SDL3Window::~SDL3Window()
{
    ShutDown();
}

void SDL3Window::ShutDown() 
{

    if (m_Context)
    {
        SDL_GL_DestroyContext((SDL_GLContext)m_Context);
        m_Context = nullptr;
    }

    if (m_Window)
    {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
}

void SDL3Window::OnUpdate()
{
    PumpEvents();
     // Minimal redraw so OS/compositor sees updates
    //glViewport(0, 0, m_WindowProperties.m_Width, m_WindowProperties.m_Height);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //SDL_GL_SwapWindow(m_Window);
}


void SDL3Window::PumpEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        {
            if(m_EventCallback)
            {
                WindowCloseEvent event;
                m_EventCallback(event);
            }
            break;
        }
        case SDL_EVENT_WINDOW_RESIZED:
        {
            if(m_EventCallback)
            {
                const uint32_t w = (uint32_t)e.window.data1;
                const uint32_t h = (uint32_t)e.window.data2;

                m_WindowProperties.m_Width = w;
                m_WindowProperties.m_Height = h;

                WindowResizeEvent event(w, h);
                m_EventCallback(event);
            }
            break;
        }
        default:
            break;
        }
    }


}




uint32_t SDL3Window::GetWindowFlag(const WindowProperties &winProp)
{
    uint32_t flag = SDL_WINDOW_OPENGL;
    if (has_flag(winProp.flag, WindowFlags::RESIZABLE))
        flag |= SDL_WINDOW_RESIZABLE;
    if (has_flag(winProp.flag, WindowFlags::BORDERLESS))
        flag |= SDL_WINDOW_BORDERLESS;
    if (has_flag(winProp.flag, WindowFlags::FULLSCREEN))
        flag |= SDL_WINDOW_FULLSCREEN;
    if (has_flag(winProp.flag, WindowFlags::INVISIBLE))
        flag |= SDL_WINDOW_HIDDEN;  
    return flag;
}

void SDL3Window::SetWindow(int DisplayWidth, int DisplayHeight, uint32_t flag)
{
    int topLeftPosX{};
    int topLeftPosY{};

    switch (m_WindowProperties.m_WinPos)
    {
    case WindowPos::TopLeft:
        topLeftPosX = DisplayWidth / 2 - m_WindowProperties.m_Width -   m_WindowProperties.m_XPaddingToCenterY;
        topLeftPosY = DisplayHeight / 2 - m_WindowProperties.m_Height - m_WindowProperties.m_YPaddingToCenterX;
        break;
    
    case WindowPos::TopRight:
        topLeftPosX = DisplayWidth / 2  + m_WindowProperties.m_XPaddingToCenterY;
        topLeftPosY = DisplayHeight / 2 - m_WindowProperties.m_Height - m_WindowProperties.m_YPaddingToCenterX;
        break;

    case WindowPos::ButtomLeft:
        topLeftPosX = DisplayWidth / 2 - m_WindowProperties.m_Width - m_WindowProperties.m_XPaddingToCenterY;
        topLeftPosY = DisplayHeight / 2 + m_WindowProperties.m_YPaddingToCenterX;
        break;

    case WindowPos::ButtomRight:
        topLeftPosX = DisplayWidth / 2  + m_WindowProperties.m_XPaddingToCenterY;
        topLeftPosY = DisplayHeight / 2 + m_WindowProperties.m_YPaddingToCenterX;
        break;

    case WindowPos::Center:
        topLeftPosX = DisplayWidth / 2 - m_WindowProperties.m_Width / 2;
        topLeftPosY = DisplayHeight / 2 - m_WindowProperties.m_Height / 2;
        break;

    }
 
    CORE_INFO("title='{}' w={} h={} flags=0x{:X}",
          m_WindowProperties.m_Title,
          m_WindowProperties.m_Width,
          m_WindowProperties.m_Height,
          flag);
    m_Window = SDL_CreateWindow(m_WindowProperties.m_Title.c_str(), m_WindowProperties.m_Width, m_WindowProperties.m_Height, flag);
    // SDL_SetWindowPosition(m_Window, 100, 100);
    // SDL_ShowWindow(m_Window);
    SDL_SetWindowPosition(m_Window, topLeftPosX, topLeftPosY);
}
// static KeyCode SDLScancodeToKeyCode(SDL_Scancode sc)
// {
//     switch (sc)
//     {
//         case SDL_SCANCODE_SPACE:        return Key::Space;
//         case SDL_SCANCODE_APOSTROPHE:   return Key::Apostrophe;
//         case SDL_SCANCODE_COMMA:        return Key::Comma;
//         case SDL_SCANCODE_MINUS:        return Key::Minus;
//         case SDL_SCANCODE_PERIOD:       return Key::Period;
//         case SDL_SCANCODE_SLASH:        return Key::Slash;

//         case SDL_SCANCODE_0:            return Key::D0;
//         case SDL_SCANCODE_1:            return Key::D1;
//         case SDL_SCANCODE_2:            return Key::D2;
//         case SDL_SCANCODE_3:            return Key::D3;
//         case SDL_SCANCODE_4:            return Key::D4;
//         case SDL_SCANCODE_5:            return Key::D5;
//         case SDL_SCANCODE_6:            return Key::D6;
//         case SDL_SCANCODE_7:            return Key::D7;
//         case SDL_SCANCODE_8:            return Key::D8;
//         case SDL_SCANCODE_9:            return Key::D9;

//         case SDL_SCANCODE_A:            return Key::A;
//         case SDL_SCANCODE_B:            return Key::B;
//         case SDL_SCANCODE_C:            return Key::C;
//         case SDL_SCANCODE_D:            return Key::D;
//         case SDL_SCANCODE_E:            return Key::E;
//         case SDL_SCANCODE_F:            return Key::F;
//         case SDL_SCANCODE_G:            return Key::G;
//         case SDL_SCANCODE_H:            return Key::H;
//         case SDL_SCANCODE_I:            return Key::I;
//         case SDL_SCANCODE_J:            return Key::J;
//         case SDL_SCANCODE_K:            return Key::K;
//         case SDL_SCANCODE_L:            return Key::L;
//         case SDL_SCANCODE_M:            return Key::M;
//         case SDL_SCANCODE_N:            return Key::N;
//         case SDL_SCANCODE_O:            return Key::O;
//         case SDL_SCANCODE_P:            return Key::P;
//         case SDL_SCANCODE_Q:            return Key::Q;
//         case SDL_SCANCODE_R:            return Key::R;
//         case SDL_SCANCODE_S:            return Key::S;
//         case SDL_SCANCODE_T:            return Key::T;
//         case SDL_SCANCODE_U:            return Key::U;
//         case SDL_SCANCODE_V:            return Key::V;
//         case SDL_SCANCODE_W:            return Key::W;
//         case SDL_SCANCODE_X:            return Key::X;
//         case SDL_SCANCODE_Y:            return Key::Y;
//         case SDL_SCANCODE_Z:            return Key::Z;

//         case SDL_SCANCODE_ESCAPE:       return Key::Escape;
//         case SDL_SCANCODE_RETURN:       return Key::Enter;
//         case SDL_SCANCODE_TAB:          return Key::Tab;
//         case SDL_SCANCODE_BACKSPACE:    return Key::Backspace;
//         case SDL_SCANCODE_INSERT:       return Key::Insert;
//         case SDL_SCANCODE_DELETE:       return Key::Delete;
//         case SDL_SCANCODE_RIGHT:        return Key::Right;
//         case SDL_SCANCODE_LEFT:         return Key::Left;
//         case SDL_SCANCODE_DOWN:         return Key::Down;
//         case SDL_SCANCODE_UP:           return Key::Up;
//         case SDL_SCANCODE_PAGEUP:       return Key::PageUp;
//         case SDL_SCANCODE_PAGEDOWN:     return Key::PageDown;
//         case SDL_SCANCODE_HOME:         return Key::Home;
//         case SDL_SCANCODE_END:          return Key::End;
//         case SDL_SCANCODE_CAPSLOCK:     return Key::CapsLock;
//         case SDL_SCANCODE_SCROLLLOCK:   return Key::ScrollLock;
//         case SDL_SCANCODE_NUMLOCKCLEAR: return Key::NumLock;
//         case SDL_SCANCODE_PRINTSCREEN:  return Key::PrintScreen;
//         case SDL_SCANCODE_PAUSE:        return Key::Pause;

//         case SDL_SCANCODE_F1:           return Key::F1;
//         case SDL_SCANCODE_F2:           return Key::F2;
//         case SDL_SCANCODE_F3:           return Key::F3;
//         case SDL_SCANCODE_F4:           return Key::F4;
//         case SDL_SCANCODE_F5:           return Key::F5;
//         case SDL_SCANCODE_F6:           return Key::F6;
//         case SDL_SCANCODE_F7:           return Key::F7;
//         case SDL_SCANCODE_F8:           return Key::F8;
//         case SDL_SCANCODE_F9:           return Key::F9;
//         case SDL_SCANCODE_F10:          return Key::F10;
//         case SDL_SCANCODE_F11:          return Key::F11;
//         case SDL_SCANCODE_F12:          return Key::F12;

//         case SDL_SCANCODE_LSHIFT:       return Key::LeftShift;
//         case SDL_SCANCODE_LCTRL:        return Key::LeftControl;
//         case SDL_SCANCODE_LALT:         return Key::LeftAlt;
//         case SDL_SCANCODE_LGUI:         return Key::LeftSuper;
//         case SDL_SCANCODE_RSHIFT:       return Key::RightShift;
//         case SDL_SCANCODE_RCTRL:        return Key::RightControl;
//         case SDL_SCANCODE_RALT:         return Key::RightAlt;
//         case SDL_SCANCODE_RGUI:         return Key::RightSuper;
//         case SDL_SCANCODE_APPLICATION:  return Key::Menu;
//         default:                        return Key::Unknown;
//     }
// };

void SDL3Window::SwapBuffers()
{
    SDL_GL_SwapWindow(m_Window);
}