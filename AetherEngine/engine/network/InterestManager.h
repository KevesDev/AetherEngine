#pragma once

#include "../ecs/Registry.h"
#include "../ecs/Components.h"
#include "SessionTypes.h"
#include <unordered_map>
#include <vector>

namespace aether {

    /**
     * InterestManager
     *
     * Maintains a coarse spatial partition of the world and per-session
     * interest sets. ReplicationSystem queries this to determine which
     * connections should receive updates for a given entity.
     */
    class InterestManager {
    public:
        explicit InterestManager(float cellSize = 16.0f)
            : m_CellSize(cellSize) {}

        void Update(Registry& reg);

        const std::vector<EntityID>* GetInterestedSessions(EntityID /*entity*/) const {
            // TODO: concrete mapping from entities to sessions will be
            // implemented alongside the session/connection layer.
            return nullptr;
        }

    private:
        float m_CellSize;

        // TODO: map from spatial cell to entities and sessions.
    };

} // namespace aether

