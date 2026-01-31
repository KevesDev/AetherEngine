#pragma once
#include "../input/Input.h" 

#include <SDL.h>

namespace aether {

    bool Input::IsKeyPressed(int keycode) {
        // SDL_GetKeyboardState returns a pointer to the internal state array
        const Uint8* state = SDL_GetKeyboardState(nullptr);

        // Convert the Engine keycode (which matches SDL Keycode) to an SDL Scancode
        // This is necessary because the State array is indexed by Scancodes (physical location)
        auto scancode = SDL_GetScancodeFromKey(keycode);

        // Check if that key is currently down
        return state[scancode];
    }

    bool Input::IsMouseButtonPressed(int button) {
        // SDL buttons are 1 for Left, 2 for Middle, 3 for Right
        // We check the global mouse state
        Uint32 state = SDL_GetMouseState(nullptr, nullptr);
        return (state & SDL_BUTTON(button));
    }

    std::pair<float, float> Input::GetMousePosition() {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return { (float)x, (float)y };
    }

    float Input::GetMouseX() {
        auto [x, y] = GetMousePosition();
        return x;
    }

    float Input::GetMouseY() {
        auto [x, y] = GetMousePosition();
        return y;
    }

}