#include"Platform/SDL3/SDL3Window.h"
#include <SDL3/SDL.h>

//#include"Input/Codes.h"

void* SDL3Window::GetNativeWindow() const
{
    return (void*)m_Window;
}

void SDL3Window::Initialize(const WindowProperties& winProp)
{

}

void SDL3Window::ShutDown() 
{

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

