#include "Core/Core.h"
#include <iostream>
#include "Core/Utility.h"
#include "Windows/Window.h"
#include "Core/BuildAndPlatformDetection.h"


int main()
{
    WindowFlags a = WindowFlags::INVISIBLE;
    WindowFlags b = WindowFlags::BORDERLESS;
    
    auto c = a | b;
    std::cout<< static_cast<int>(c) <<std::endl;
    #ifdef BUILD_RELEASE
        std::cout<<"Hello World"<<std::endl;
    #endif
    return 0;
    
}
