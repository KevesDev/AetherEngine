#pragma once
#include "Event.h"
#include <sstream>

/* Key event classes for keyboard input handling
* SUMMARY:
* 1. KeyEvent: Abstract base class for keyboard events
* 2. KeyPressedEvent: Represents a key press, includes repeat count
* 3. KeyReleasedEvent: Represents a key release
*/

namespace aether {

    // Abstract base class for Key events to hold the keycode
	// Inherits from Event base class
    class KeyEvent : public Event {
    public:
        inline int GetKeyCode() const { return m_KeyCode; }
        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
    protected:
		// Protected constructor to prevent direct instantiation
        KeyEvent(int keycode) : m_KeyCode(keycode) {}
        int m_KeyCode;
    };

	// KeyPressedEvent class for key press events
    class KeyPressedEvent : public KeyEvent {
    public:
		// Constructor takes keycode and repeat count
        KeyPressedEvent(int keycode, int repeatCount)
            : KeyEvent(keycode), m_RepeatCount(repeatCount) {
        }

		// Accessor for repeat count: this indicates how many times the key press has been repeated
        inline int GetRepeatCount() const { return m_RepeatCount; }

		// Override ToString for detailed logging
        std::string ToString() const override {
			std::stringstream ss; // String stream for building the output string
            ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
            return ss.str();
        }

		// Macro to define event type
        EVENT_CLASS_TYPE(KeyPressed)
    private:
        int m_RepeatCount;
    };

	// KeyReleasedEvent class for key release events
    class KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << m_KeyCode;
            return ss.str();
        }
		// Macro to define event type
        EVENT_CLASS_TYPE(KeyReleased)
    };
}