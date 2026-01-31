#include "SDLWindow.h"

// Required to let ImGui see SDL events
#include "backends/imgui_impl_sdl2.h"

// Include our specific event types
#include "../events/ApplicationEvent.h"
#include "../events/KeyEvent.h"
#include "../events/MouseEvent.h" 
#include "../core/Log.h"

namespace aether {

    // --- Constructor / Destructor ---
    SDLWindow::SDLWindow(const WindowProps& props) {
        Init(props);
    }

    SDLWindow::~SDLWindow() {
        Shutdown();
    }

    // --- Initialization ---
    void SDLWindow::Init(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;
        m_Data.VSync = props.VSync;
        m_Data.Mode = props.Mode; // Store the mode

        // 1. Initialize SDL System
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
            Log::Write(LogLevel::Error, "SDL_Init Error: " + std::string(SDL_GetError()));
            return;
        }

        // 2. Setup Window Flags
        // Start with the base flags
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

        // Apply the requested Mode
        if (m_Data.Mode == WindowMode::Fullscreen) {
            window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_FULLSCREEN);
        }
        else if (m_Data.Mode == WindowMode::Borderless) {
            window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_BORDERLESS);
        }
        else if (m_Data.Mode == WindowMode::Maximized) {
            window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_MAXIMIZED);
        }

        // 3. Create the Native Window
        m_Window = (SDL_Window*)SDL_CreateWindow(
            m_Data.Title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            m_Data.Width, m_Data.Height,
            window_flags
        );

        if (!m_Window) {
            Log::Write(LogLevel::Error, "Failed to create window: " + std::string(SDL_GetError()));
            return;
        }

        // 4. Create OpenGL Context
        m_Context = SDL_GL_CreateContext((SDL_Window*)m_Window);
        SDL_GL_MakeCurrent((SDL_Window*)m_Window, m_Context);

        // 5. Apply VSync
        SetVsync(m_Data.VSync);
    }

    void SDLWindow::Shutdown() {
        SDL_GL_DeleteContext(m_Context);
        SDL_DestroyWindow((SDL_Window*)m_Window);
        SDL_Quit();
    }

    // --- The Event Loop (The Translator) ---
    void SDLWindow::OnUpdate() {
        SDL_Event event;

        // Poll every event from the Operating System
        while (SDL_PollEvent(&event)) {
            // Pass the raw event to ImGui FIRST
            // This lets ImGui update its state (mouse pos, clicks, typing)
            ImGui_ImplSDL2_ProcessEvent(&event);


            // If no callback is set, we can't process events
            if (!m_Data.EventCallback) continue;

            switch (event.type) {
                // --- Window Events ---
            case SDL_QUIT: {
                WindowCloseEvent e;
                m_Data.EventCallback(e);
                break;
            }
            case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    m_Data.Width = event.window.data1;
                    m_Data.Height = event.window.data2;
                    WindowResizeEvent e(m_Data.Width, m_Data.Height);
                    m_Data.EventCallback(e);
                }
                break;
            }

                                // --- Keyboard Events ---
            case SDL_KEYDOWN: {
                KeyPressedEvent e(event.key.keysym.sym, event.key.repeat);
                std::cout << "Key pressed!";
                m_Data.EventCallback(e);
                break;
            }
            case SDL_KEYUP: {
                KeyReleasedEvent e(event.key.keysym.sym);
                m_Data.EventCallback(e);
                break;
            }

                          // --- Mouse Events ---
            case SDL_MOUSEBUTTONDOWN: {
                MouseButtonPressedEvent e(event.button.button);
                m_Data.EventCallback(e);
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                MouseButtonReleasedEvent e(event.button.button);
                m_Data.EventCallback(e);
                break;
            }
            case SDL_MOUSEMOTION: {
                MouseMovedEvent e((float)event.motion.x, (float)event.motion.y);
                m_Data.EventCallback(e);
                break;
            }
            case SDL_MOUSEWHEEL: {
                MouseScrolledEvent e((float)event.wheel.x, (float)event.wheel.y);
                m_Data.EventCallback(e);
                break;
            }
            }
        }

        // Render the frame
        SDL_GL_SwapWindow((SDL_Window*)m_Window);
    }

    // --- Attributes ---
    void SDLWindow::SetVsync(bool enabled) {
        if (enabled)
            SDL_GL_SetSwapInterval(1);
        else
            SDL_GL_SetSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool SDLWindow::IsVsync() const {
        return m_Data.VSync;
    }

    // --- Factory Method ---
    Window* Window::Create(const WindowProps& props) {
        return new SDLWindow(props);
    }

}