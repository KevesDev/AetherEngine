#pragma once

#include <utility> // Needed for std::pair

namespace aether {

    class Input {
    public:
        // Returns true if the key is currently held down
        static bool IsKeyPressed(int keycode);

        // Returns true if the mouse button is held down
        static bool IsMouseButtonPressed(int button);

        // Returns the mouse position as (x, y)
        static std::pair<float, float> GetMousePosition();

        // Helpers for just X or Y
        static float GetMouseX();
        static float GetMouseY();
    };

}