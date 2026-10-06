// Copyright 2019-2020 the donut authors. See AUTHORS.md

#include "Render/OpenGL/glad/glad.h"
#include <Window.h>
#include <iostream>

namespace Donut
{

Window::Window(const std::string& title, const int width, const int height)
{
    SDL_version compiledVersion, linkedVersion;
    SDL_VERSION(&compiledVersion);
    SDL_GetVersion(&linkedVersion);

    std::clog << "Initializing SDL..." << std::endl;
    std::clog << "SDL Version/Compiled " << uint32_t(compiledVersion.major) << "." << uint32_t(compiledVersion.minor) << "."
              << uint32_t(compiledVersion.patch) << std::endl;
    std::clog << "SDL Version/Linked " << uint32_t(linkedVersion.major) << "." << uint32_t(linkedVersion.minor) << "."
              << uint32_t(linkedVersion.patch) << std::endl;

    // Initialize SDL
    if (SDL_WasInit(0) == 0)
    {
        SDL_SetMainReady();
        if (SDL_Init(0) != 0)
            throw std::runtime_error("Could not initialize SDL: " + std::string(SDL_GetError()));

        if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
            throw std::runtime_error("Could not initialize SDL Video Subsystem: " + std::string(SDL_GetError()));

        if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
            throw std::runtime_error("Could not initialize SDL Timer Subsystem: " + std::string(SDL_GetError()));
    }

    const char* driver = SDL_GetCurrentVideoDriver();
    std::clog << "SDL Video Driver: " << (driver ? driver : "unknown") << std::endl;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    // request a 4.3 core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    const uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

    auto window = std::unique_ptr<SDL_Window, SDLDestroyer>(
        SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags));
    
    if (window == nullptr)
        throw std::runtime_error("Failed creating window: " + std::string(SDL_GetError()));

    _window = std::move(window);

    // Create a OpenGL context on SDL2
    SDL_GLContext context = SDL_GL_CreateContext(_window.get());
    if (context == nullptr)
        throw std::runtime_error("Failed to initialize the OpenGL context: " + std::string(SDL_GetError()));

    // FIX: Allocate the context pointer on the heap so it matches what unique_ptr expects
    SDL_GLContext* heapContext = new SDL_GLContext(context);
    _glContext = std::move(std::unique_ptr<SDL_GLContext, SDLDestroyer>(heapContext));

    // Load GL extensions using glad
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        throw std::runtime_error("Failed to initialize the OpenGL glad loader.");

    std::cout << "OpenGL version loaded: " << GLVersion.major << "." << GLVersion.minor << "\n"
              << "Vendor: " << glGetString(GL_VENDOR) << "\n"
              << "Renderer: " << glGetString(GL_RENDERER) << "\n"
              << "Version: " << glGetString(GL_VERSION) << "\n"
              << std::endl;

    if (!GLAD_GL_VERSION_4_3)
        throw std::runtime_error("Your OpenGL version is too low, expected 4.3 or higher.");

    SDL_ShowWindow(_window.get());
    SDL_RaiseWindow(_window.get());
}

void Window::SetTitle(const std::string& title)
{
    SDL_SetWindowTitle(_window.get(), title.c_str());
}

const std::string Window::GetTitle() const
{
    return SDL_GetWindowTitle(_window.get());
}

void Window::Swap()
{
    SDL_GL_SwapWindow(_window.get());
}

} // namespace Donut