#pragma once

#include <string>
#include <functional>
#include "../core/EngineVersion.h"

namespace aether {

    enum WindowMode {
        Windowed,
        Borderless,
        Fullscreen,
        Maximized
    };

    class Event;

    struct WindowProps {
        std::string Title;
        uint32_t Width;
        uint32_t Height;
        WindowMode Mode;
        bool VSync;

        WindowProps(const std::string& title = "Aether Engine",
            uint32_t width = 1280,
            uint32_t height = 720,
            WindowMode mode = WindowMode::Maximized,
            bool vsync = true
        )
            : Title(title), Width(width), Height(height), Mode(mode), VSync(vsync) {
        }
    };

    class Window {
    public:
        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        // --- Render Context Abstraction ---
        // Engine calls this to clear the screen at the start of a frame.
        // This removes the need for OpenGL headers in Engine.cpp
        virtual void Clear() const = 0;
        // ---------------------------------------

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual void SetVsync(bool enabled) = 0;
        virtual bool IsVsync() const = 0;

        virtual void* GetNativeWindow() const = 0;

        using EventCallbackFn = std::function<void(Event&)>;
        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

        static Window* Create(const WindowProps& props = WindowProps());
    };
}