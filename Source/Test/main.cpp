#include <memory>
#include <cstdint>
#include <cstdio>         // For printf or fprintf if needed
#include <crtdbg.h>       // For memory leak detection on Windows

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <spdlog/spdlog.h>

#include <Lucky/Graphics/Color.hpp>
#include <Lucky/Graphics/GraphicsDevice.hpp>

namespace
{
    constexpr int WINDOW_WIDTH = 1920;
    constexpr int WINDOW_HEIGHT = 1080;

    // RAII-based SDL Window holder for clarity
    struct SDLWindowDeleter {
        void operator()(SDL_Window* w) const {
            if (w) {
                SDL_DestroyWindow(w);
            }
        }
    };

    // Pointers to key resources
    std::unique_ptr<SDL_Window, SDLWindowDeleter> g_windowPtr{nullptr};
    std::shared_ptr<Lucky::GraphicsDevice> g_graphicsDevice{nullptr};

    std::uint64_t g_currentTime = 0;
}

// -------------------------------------------------------------------------
// Helper: Initialize the Graphics Subsystem
// Returns true on success, false on failure
// -------------------------------------------------------------------------
bool InitializeGraphics()
{
    // Prepare window attributes for an OpenGL-based GraphicsDevice
    const auto attributes = Lucky::GraphicsDevice::PrepareWindowAttributes(Lucky::GraphicsAPI::OpenGL);

    SDL_Window* rawWindow = SDL_CreateWindow(
        "Clear Screen Example",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        attributes
    );
    if (!rawWindow) {
        spdlog::error("Failed to create SDL window: {}", SDL_GetError());
        return false;
    }

    // Turn the raw pointer into our RAII-protected unique_ptr
    g_windowPtr.reset(rawWindow);

    // Create the GraphicsDevice with vertical sync enabled
    g_graphicsDevice = std::make_shared<Lucky::GraphicsDevice>(
        Lucky::GraphicsAPI::OpenGL,
        g_windowPtr.get(),
        Lucky::VerticalSyncType::AdaptiveEnabled
    );

    g_currentTime = SDL_GetPerformanceCounter();
    return true;
}

// -------------------------------------------------------------------------
// SDL Callbacks
// -------------------------------------------------------------------------
extern "C"
{

SDL_AppResult SDL_AppInit(void** /*appstate*/, int /*argc*/, char* /*argv*/[])
{
    // Enable memory leak detection (for debug builds on Windows)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Set metadata
    SDL_SetAppMetadata("Clear Screen Example", "1.0", "dev.jessechounard.clearscreen");

    // Init SDL
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) < 0) {
        spdlog::error("Failed to initialize SDL: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Attempt to create our window and graphics device
    if (!InitializeGraphics()) {
        // Already logged the error inside InitializeGraphics
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* /*appstate*/, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; // signals we want to end the app
    }
    return SDL_APP_CONTINUE;    // keep running
}

SDL_AppResult SDL_AppIterate(void* /*appstate*/)
{
    // Compute delta time in seconds
    const std::uint64_t newTime = SDL_GetPerformanceCounter();
    const double frameTime = static_cast<double>(newTime - g_currentTime) /
                             static_cast<double>(SDL_GetPerformanceFrequency());
    g_currentTime = newTime;

    // Begin the frame
    g_graphicsDevice->BeginFrame();

    // Just clearing the screen to a color
    g_graphicsDevice->ClearScreen(Lucky::Color::CornflowerBlue);

    // End the frame
    g_graphicsDevice->EndFrame();

    // Swap buffers
    SDL_GL_SwapWindow(g_windowPtr.get());

    // For demonstration: Sleep or small yields might be introduced here
    // e.g., SDL_Delay(1);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/)
{
    // Cleanup in reverse order of creation
    g_graphicsDevice.reset(); // release device first
    g_windowPtr.reset();      // destroy window

    SDL_Quit();              // finally quit SDL
}

} // extern "C"
