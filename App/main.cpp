#include <iostream>
#include "Core/Utility.h"
#include "Windows/Window.h"
#include "Core/BuildAndPlatformDetection.h"
#include "Core/Base.h"
#include "Events/Event.h"
#include "Input/Codes.h"
#include "Core/Application.h"


int main()
{
    WindowFlags a = WindowFlags::INVISIBLE;
    WindowFlags b = WindowFlags::BORDERLESS;
    EventType c = EventType::AppTick;
    Log::Init();
    //auto c = a | b;
    std::cout<< static_cast<int>(c) <<std::endl;
    #ifdef BUILD_DEBUG
        std::cout<<"Hello World"<<std::endl;
    #endif

    ERROR("This is my second engine {}!", "too");
    WARN("This is my third engine {}!", "too");
    INFO("Keycode: {}!", uint16_t(Key::D));
    return 0;
    
}
