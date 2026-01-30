#pragma once

#include  <string>
#include <functional>
#include "EngineVersion.h"
/*
* The Window is just an interface for creating and managing application windows.
* It defines what a window does, not how.
* The actual code/implementation should be in the SDLWindow class, hidden from
* the rest of the engine.
* We use the Factory as a static method that creates the correct window for the current platform.
*/

namespace aether {
	// Define window mode first
	enum WindowMode {
		Windowed,
		Borderless,
		Fullscreen
	};

	// Forward declare Event class
	class Event;

	// Defines properties for creating a window.
	struct WindowProps {
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		WindowMode Mode;
		bool VSync;

		// Constructor with default values
		WindowProps(const std::string& title = "Aether Engine",// +EngineVersion,
			uint32_t width = 1280,
			uint32_t height = 720,
			WindowMode mode = WindowMode::Windowed,
			bool vsync = true
			)
			: Title(title), Width(width), Height(height), Mode(mode), VSync(vsync){
		}
	};

	// The Window class must be Pure Virtual.
	// This ensures that engine/core has zero dependencies on SDL code.

	// Interface representing a desktop system based Window
	class Window {
	public:

		// Standard virtual destructor
		virtual ~Window() = default;

		// Update window (core loop)
		virtual void OnUpdate() = 0;

		// Getters
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetVsync(bool enabled) = 0;
		virtual bool IsVsync() const = 0;

		// Required for Graphics APIs (Vulkan/DirectX/OpenGL) later
		// Get the native window handle (for example, HWND on Windows)
		// This is needed for creating graphics contexts.
		virtual void* GetNativeWindow() const = 0;

		// Event callbacks (production key)
		// This allows the Window to say "I was resized" without the Engine polling for it.
		// Use std::function to store a lambda or function pointer.
		using EventCallbackFn = std::function<void(Event&)>;
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		// Static creation method (the Factory)
		// This is implemented in the .cpp file, effectively "choosing" the platform.
		static Window* Create(const WindowProps& props = WindowProps());
	};
}