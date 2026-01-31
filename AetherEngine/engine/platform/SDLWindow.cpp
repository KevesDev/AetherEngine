#include "SDLWindow.h"

// Required to let ImGui see SDL events
#include "backends/imgui_impl_sdl2.h"

// Events
#include "../events/ApplicationEvent.h"
#include "../events/KeyEvent.h"
#include "../events/MouseEvent.h" 

// Logging & Macros
#include "../core/Log.h"

// Allowed here because this is the specific SDL/OpenGL implementation
#include <SDL_opengl.h> 

namespace aether {

    SDLWindow::SDLWindow(const WindowProps& props) {
        Init(props);
    }

    SDLWindow::~SDLWindow() {
        Shutdown();
    }

    void SDLWindow::Init(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;
        m_Data.VSync = props.VSync;
        m_Data.Mode = props.Mode;

        // 1. Initialize SDL System
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
            AETHER_CORE_ERROR("SDL_Init Error: {0}", SDL_GetError());
            return;
        }

        // 2. Setup Window Flags
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

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
            AETHER_CORE_ERROR("Failed to create window: {0}", SDL_GetError());
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

    // --- Abstraction Implementation ---
    void SDLWindow::Clear() const {
        // This is the implementation detail hidden from the Engine
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    // ---------------------------------------

    void SDLWindow::OnUpdate() {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (!m_Data.EventCallback) continue;

            switch (event.type) {
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
            case SDL_KEYDOWN: {
                KeyPressedEvent e(event.key.keysym.sym, event.key.repeat);
                m_Data.EventCallback(e);
                break;
            }
            case SDL_KEYUP: {
                KeyReleasedEvent e(event.key.keysym.sym);
                m_Data.EventCallback(e);
                break;
            }
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

        SDL_GL_SwapWindow((SDL_Window*)m_Window);
    }

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

    Window* Window::Create(const WindowProps& props) {
        return new SDLWindow(props);
    }
}