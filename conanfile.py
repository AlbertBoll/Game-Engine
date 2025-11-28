from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.cmake import cmake_layout

class AppConan(ConanFile):
    name = "my-app"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = (CMakeToolchain, CMakeDeps)
    exports_sources = "CMakeLists.txt", "cmake/*", "core/*", "App/*"

    # Swap to your own refs if you have local recipes for imgui-docking, glad2, sdl3
    requires = (
        "glm/1.0.1",
        "glfw/3.4",
        "entt/3.15.0",
        "sdl/3.2.20"
        # e.g. "imgui-docking/1.91.0@you/stable",
        # e.g. "glad2/2.0.8@you/stable",
        # e.g. "sdl3/3.1.0@you/stable",
    )

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()