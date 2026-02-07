#pragma once

// Include the abstract interface
#include "Window.h"

// Include SDL so we can use SDL_Window* and SDL_GLContext
#include <SDL.h>

namespace aether {

    class SDLWindow : public Window {
    public:
        SDLWindow(const WindowProps& props);
        virtual ~SDLWindow();

        void OnUpdate() override;
        virtual void Clear() const override;

		virtual void SetTitle(const std::string& title) override;

        inline uint32_t GetWidth() const override { return m_Data.Width; }
        inline uint32_t GetHeight() const override { return m_Data.Height; }

        // Window attributes
        // Note: keeping 'Vsync' lowercase to match your .cpp implementation
        void SetVsync(bool enabled) override;
        bool IsVsync() const override;

        inline void* GetNativeWindow() const override { return m_Window; }

        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

    private:
        virtual void Init(const WindowProps& props);
        virtual void Shutdown();

        // This uses the SDL Library type (SDL_Window*),
        // NOT our class type (SDLWindow*).
        SDL_Window* m_Window;

        SDL_GLContext m_Context;

        struct WindowData {
            std::string Title;
            uint32_t Width, Height;
            bool VSync; // Helper bool for state
            WindowMode Mode;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };

}