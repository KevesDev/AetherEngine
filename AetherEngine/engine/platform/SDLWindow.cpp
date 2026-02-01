#include "SDLWindow.h"
#include <glad/glad.h>
// Required to let ImGui see SDL events
#include "backends/imgui_impl_sdl2.h"
#include <imgui.h> // Required to access ImGui::GetCurrentContext()

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

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            AETHER_CORE_CRITICAL("SDL could not initialize! SDL_Error: {0}", SDL_GetError());
            return;
        }

        // --- Explicit Context Attributes ---
        // Requesting a 4.5 Core Profile prevents driver-level "Compatibility" hangs
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        m_Window = SDL_CreateWindow(
            m_Data.Title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            (int)m_Data.Width,
            (int)m_Data.Height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );

        m_Context = SDL_GL_CreateContext(m_Window);
        SDL_GL_MakeCurrent(m_Window, m_Context);

        int status = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
        AETHER_ASSERT(status, "Failed to initialize GLAD!");

        // Sync with vertical retrace (VSync)
        SDL_GL_SetSwapInterval(1);
    }

    void SDLWindow::Shutdown() {
        if (m_Context) SDL_GL_DeleteContext(m_Context);
        if (m_Window) SDL_DestroyWindow((SDL_Window*)m_Window);
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

            // Prevent crash if ImGui is not initialized (e.g. in Client Runtime)
            // The Client has no ImGui context, so calling ProcessEvent will assert/crash.
            if (ImGui::GetCurrentContext() != nullptr) {
                ImGui_ImplSDL2_ProcessEvent(&event);
            }

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
        // SDL_GL_SetSwapInterval returns -1 on error
        if (SDL_GL_SetSwapInterval(enabled ? 1 : 0) < 0) {
            AETHER_CORE_WARN("Failed to set VSync to {0}: {1}", enabled, SDL_GetError());
        }
        else {
            m_Data.VSync = enabled;
        }
    }

    bool SDLWindow::IsVsync() const {
        return m_Data.VSync;
    }

    Window* Window::Create(const WindowProps& props) {
        return new SDLWindow(props);
    }
}