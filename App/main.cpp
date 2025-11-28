#include "Core/Core.h"
#include <iostream>
#include "Core/Utility.h"
#include "Windows/Window.h"


int main()
{
    WindowFlags a = WindowFlags::INVISIBLE;
    WindowFlags b = WindowFlags::BORDERLESS;
    
    auto c = a | b;
    std::cout<<"Hello world3"<<std::endl;
    std::cout<< static_cast<int>(c) <<std::endl;
    return 0;
    
}
