#pragma once

#include "System.h"
#include "../../ecs/Components.h"
#include "../../network/NetworkTransport.h"
#include "../../network/NetworkTypes.h"
#include "../../core/AetherTime.h"

namespace aether {

    /**
     * ReplicationSystem
     *
     * Runs in the Sync stage of the SystemScheduler.
     * Responsible for building per-tick, per-client snapshots from ECS state.
     * This is the authoritative source of world state for networking.
     */
    class ReplicationSystem : public ISystem {
    public:
        explicit ReplicationSystem(INetworkTransport* transport)
            : m_Transport(transport) {}

        const char* GetName() const override { return "ReplicationSystem"; }

        void OnUpdate(Registry& reg, float ts) override {
            if (!m_Transport)
                return;

            // Advance per-entity replication accumulators and build snapshots
            auto view = reg.view<ReplicationComponent, IDComponent, TransformComponent>();
            for (auto entityID : view) {
                auto* repl = reg.GetComponent<ReplicationComponent>(entityID);
                auto* idComp = reg.GetComponent<IDComponent>(entityID);
                auto* transform = reg.GetComponent<TransformComponent>(entityID);

                if (!repl || !idComp || !transform)
                    continue;

                if (repl->Mode == ReplicationMode::None)
                    continue;

                repl->Accumulator += ts;
                const float targetInterval = (repl->UpdateRateHz > 0.0f)
                    ? (1.0f / repl->UpdateRateHz)
                    : 0.0f;

                if (targetInterval <= 0.0f || repl->Accumulator < targetInterval)
                    continue;

                repl->Accumulator = 0.0f;

                // Build a minimal example payload (position/rotation only).
                NetworkMessageHeader header{};
                header.MessageType = 1; // e.g., Transform Snapshot
                header.SimulationTick = AetherTime::GetSimTick();

                struct PackedTransform {
                    float X, Y, Rotation;
                } packed{ transform->X, transform->Y, transform->Rotation };

                std::vector<std::uint8_t> buffer(sizeof(header) + sizeof(packed));
                std::memcpy(buffer.data(), &header, sizeof(header));
                std::memcpy(buffer.data() + sizeof(header), &packed, sizeof(packed));

                // TODO: Route to appropriate connections based on interest management.
                (void)buffer;
            }
        }

    private:
        INetworkTransport* m_Transport = nullptr;
    };

} // namespace aether

