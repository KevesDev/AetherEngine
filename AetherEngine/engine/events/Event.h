#pragma once

#include <string>
#include <functional>
#include <iostream>

/* A lightweight, custom event type system.
* Allows us to avoid RTTI/Dynamic cast, which is much more expensive.
* Based on the idea of "type tags" or "type IDs".
* 
* SUMMARY:
* 1. Define distinct event types and categories using enums.
* 2. Use macros to reduce boilerplate in event subclasses.
* 3. Implement an EventDispatcher to route events based on type.
* 4. Provide easy logging support via operator overloading.
*/

#define AETHER_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace aether {

	/* Event Types : Distinct IDs for every specific event
	*  Event Categories: Bitfields for grouping related events (e.g., input events, application events)
	*/
	enum EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		FileDrop
	};

	/* Event Categories: Bitfield for fast filtering (e.g., "Is this an Input event?")
	*	We can use bitwise operations to check category membership efficiently.
	*	For example, to check if an event is in the Input category:
	*	if (event.GetCategoryFlags() & EventCategoryInput) { ... }
	*/
	enum EventCategory {
		EventCategoryApplication = (1 << 0), // 1
		EventCategoryInput = (1 << 1), // 2
		EventCategoryKeyboard = (1 << 2), // 4
		EventCategoryMouse = (1 << 3), // 8
		EventCategoryMouseButton = (1 << 4) // 16
	};

	/* Macros to reduce boilerplate in subclasses (Industry Standard practice)
	* These macros define standard methods for event type and category retrieval.
	* Note: They're fairly generic/common in indie engine codebases and may flag as AI-generated.
	* Usage:
	* In a subclass, use EVENT_CLASS_TYPE(EventTypeName) to define type methods.
	*/
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
                               virtual EventType GetEventType() const override { return GetStaticType(); }\
                               virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event {
	public:
		virtual ~Event() = default;
		bool Handled = false; // Allows layers to "consume" events (e.g., UI blocks clicks from reaching the game world)

		/* Pure virtual methods to be implemented by subclasses:
		* Get the specific event type, name, and category flags
		* Also provides a default ToString() implementation
		*/
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		// Fast check: "Is this event in the 'Input' category?"
		inline bool IsInCategory(EventCategory category) {
			return GetCategoryFlags() & category;
		}
	};

	/* The Dispatcher: The engine room that routes events safely.
	*  Given an event, it checks its type and calls the appropriate handler function.
	*  The handler function is a std::function that takes the specific event type and returns
	* a bool indicating if it was handled.
	* Finally, it marks the event as handled if the function returns true.
	*/
	class EventDispatcher {
		// Reference to the event being dispatched
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) : m_Event(event) {}

		// Dispatches the event ONLY if the type matches T
		// Calls the provided function and returns true if handled
		template<typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_Event.GetEventType() == T::GetStaticType()) {
				// Determine if the event was handled by the function
				m_Event.Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		// Reference to the event being dispatched
		Event& m_Event;
	};

	// Output stream operator for easy logging: Log::Info("Event: {}", event);
	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}
}