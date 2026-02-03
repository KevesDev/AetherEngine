#pragma once

#include "Event.h"
#include <vector>
#include <sstream>
/* Event class for window resize events
* SUMMARY:
* Inherits from Event base class
* Stores new width and height
* Provides accessors for dimensions
* Overrides ToString for logging
*/

namespace aether {
	class WindowResizeEvent : public Event {
	public:
		// Event type and category; uses macros from Event.h
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {}

		// Override ToString for detailed logging
		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		// Provides a string representation of the event for logging
		std::string ToString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		// Macros to define event type and category
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		// Dimensions after resize
		unsigned int m_Width, m_Height;
	};

	/* Simple event indicating the window is closing
	*  Inherits from Event, uses macros for type/category
	*/
	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() = default;
		EVENT_CLASS_TYPE(WindowClose)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class FileDropEvent : public Event
	{
	public:
		FileDropEvent(const std::vector<std::string>& paths)
			: m_Paths(paths) {
		}

		inline const std::vector<std::string>& GetPaths() const { return m_Paths; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "FileDropEvent: " << m_Paths.size() << " files dropped.";
			return ss.str();
		}

		EVENT_CLASS_TYPE(FileDrop)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		std::vector<std::string> m_Paths;
	};
}