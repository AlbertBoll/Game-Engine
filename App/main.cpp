#include "Core/EntryPoint.h"
#include "DemoTriangleLayer.h"


class SimulationApp: public Application
{
public:
    SimulationApp(const WindowProperties& prop)
        : Application(prop)
    {
       PushLayer(new DemoTriangleLayer());
    }
    ~SimulationApp() override = default;
};

Application* CreateApplication(const WindowProperties& prop)
{
    return new SimulationApp(prop);
}
// int main()
// {
//     WindowFlags a = WindowFlags::INVISIBLE;
//     WindowFlags b = WindowFlags::BORDERLESS;
//     EventType c = EventType::AppTick;
//     Log::Init();
//     //auto c = a | b;
//     std::cout<< static_cast<int>(c) <<std::endl;
//     #ifdef BUILD_DEBUG
//         std::cout<<"Hello World"<<std::endl;
//     #endif

//     ERROR("This is my second engine {}!", "too");
//     WARN("This is my third engine {}!", "too");
//     INFO("Keycode: {}!", uint16_t(Key::D));
//     return 0;
    
// }

// int main()
// {
//     std::cout << "Hello World from Game Engine!" << std::endl;
// }


