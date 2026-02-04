#pragma once
#include <string>
#include <cstdint>

namespace aether {

    /**
     * Defines the data format a Logic Graph should expect from an action.
     */
    enum class EInputAxisType : uint8_t {
        None = 0,
        Digital, // Boolean (e.g., Jump)
        Axis1D,  // Float (e.g., Throttle)
        Axis2D   // Vector2 (e.g., Movement)
    };

    /**
     * An InputAction is a developer-defined data asset.
     */
    struct InputAction {
        std::string Name;
        uint32_t ActionID;
        EInputAxisType AxisType;

        InputAction(const std::string& name, EInputAxisType type)
            : Name(name), AxisType(type) {
            // Hashing the name allows for O(1) lookups in the InputComponent map
            ActionID = std::hash<std::string>{}(name);
        }
    };

}