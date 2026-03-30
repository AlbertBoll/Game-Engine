// #include "Core/Utility.h"
// #include "Windows/Window.h"
// #include "Core/BuildAndPlatformDetection.h"
// #include "Core/Base.h"
// #include "Events/Event.h"
// #include "Input/Codes.h"
//#include "Core/Application.h"
#include "Renderer/CommandQueue.h"
#include "Primitives/PrimitiveTraits.h"
#include "Core/EntryPoint.h"
#include "Math/Math.h"
#include "Assets/Mesh/MeshManager.h"
#include "Assets/Shader/ShaderManager.h"
#include "Assets/Material/MaterialManager.h"
#include "Assets/Material/MaterialPresets.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderTarget.h"
#include "Renderer/FramebufferManager.h"


class SimulationApp: public Application
{
public:
    SimulationApp(const WindowProperties& prop)
        : Application(prop)
    {
        //INFO("size of glm::vec3: {}", sizeof(Math::Vec3f));
        ShaderManager sh;
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


