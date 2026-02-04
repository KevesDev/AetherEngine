#pragma once
#include <vector>
#include <unordered_map>
#include "InputActions.h"

namespace aether {

    /**
     * Represents a single binding between a physical key and a logical gameplay intent.
     */
    struct FActionKeyMapping {
        int KeyCode;
        uint32_t ActionID;
        float Scale = 1.0f; // Used for negating axes (e.g., 'S' key is -1.0)
    };

    /**
     * An InputMappingContext (IMC) is a project-specific data asset.
     * Developers swap these to change control schemes (e.g., UI vs. Gameplay).
     */
    class InputMappingContext {
    public:
        InputMappingContext() = default;

        /**
         * Maps a physical key to an action ID.
         * The ActionID should match the hash from the project's InputAction manifest.
         */
        void AddMapping(int keyCode, uint32_t actionID, float scale = 1.0f) {
            m_Mappings.push_back({ keyCode, actionID, scale });
        }

        const std::vector<FActionKeyMapping>& GetMappings() const { return m_Mappings; }

        /**
         * Clears existing mappings. Used when loading a new context or
         * applying user-rebind overrides at runtime.
         */
        void Clear() { m_Mappings.clear(); }

    private:
        std::vector<FActionKeyMapping> m_Mappings;
    };

}