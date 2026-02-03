#pragma once

namespace aether {

    // Standard ASCII/SDL compatible key mappings
    namespace Key {
        enum : int {
            Space = 32,
            Apostrophe = 39, /* ' */
            Comma = 44, /* , */
            Minus = 45, /* - */
            Period = 46, /* . */
            Slash = 47, /* / */

            D0 = 48, D1 = 49, D2 = 50, D3 = 51, D4 = 52,
            D5 = 53, D6 = 54, D7 = 55, D8 = 56, D9 = 57,

            Semicolon = 59, /* ; */
            Equal = 61, /* = */

            A = 97, B = 98, C = 99, D = 100, E = 101, F = 102,
            G = 103, H = 104, I = 105, J = 106, K = 107, L = 108,
            M = 109, N = 110, O = 111, P = 112, Q = 113, R = 114,
            S = 115, T = 116, U = 117, V = 118, W = 119, X = 120,
            Y = 121, Z = 122,

            Escape = 27,
            Enter = 13,
            Tab = 9,
            Backspace = 8,
            Insert = 1073741897,
            Delete = 127,
            Right = 1073741903,
            Left = 1073741904,
            Down = 1073741905,
            Up = 1073741906,

            // Modifiers
            // PRESERVED NAMES (Do not change)
            LeftShift = 1073742049,
            LeftCtrl = 1073742048,
            LeftAlt = 1073742050,

            RightShift = 1073742053,
            RightCtrl = 1073742052,
            RightAlt = 1073742054
        };
    }

    namespace Mouse {
        enum : int {
            ButtonLeft = 1,
            ButtonMiddle = 2,
            ButtonRight = 3
        };
    }
}